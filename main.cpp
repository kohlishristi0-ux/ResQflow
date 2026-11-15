// ============================================================================
//  ResQflow - Resource Planning and Emergency Coordination System (Phase 3)
//
//  Layering used by every menu below:
//      console input -> validation -> manager (linked list / heap / graph, in memory)
//                    -> repository (prepared statements) -> MySQL
//
//  Consistency rules (these fix the "memory says yes, database says no" family of bugs):
//    1. Operations that touch several tables (stock + allocation + ledger + activity log)
//       run inside ONE database transaction (DbTx). Any failure rolls the whole thing back.
//    2. The in-memory structure is restored (snapshot copy) whenever the transaction fails,
//       so memory and MySQL never disagree about the outcome.
//    3. IDs come from the managers' next-ID counters (seeded from MySQL), never from getCount()+1.
// ============================================================================
#include <iostream>
#include <string>
#include <cstdlib>
#include <limits>

#include "Database/UserRepository.h"
#include "Authentication.h"
#include "PasswordHasher.h"

#include "Database/DatabaseConnection.h"
#include "Database/ResourceRepository.h"
#include "Database/WarehouseRepository.h"
#include "Database/InventoryRepository.h"
#include "Database/EmergencyRepository.h"
#include "Database/AllocationRepository.h"
#include "Database/ResponderRepository.h"
#include "Database/WarehouseTransferRepository.h"
#include "Database/ActivityHistoryRepository.h"
#include "Database/LocationRepository.h"

#include "ResourceLinkedList.h"
#include "WarehouseLinkedList.h"
#include "InventoryManager.h"
#include "EmergencyMaxHeap.h"
#include "EmergencyManager.h"
#include "AllocationManager.h"
#include "ResponderManager.h"
#include "WarehouseTransferManager.h"
#include "ActivityHistory.h"
#include "LocationGraph.h"
#include "RoutePlanner.h"

using namespace std;

// ---------------------------------------------------------------------------
// Shared services (one instance, filled in main()).
// ---------------------------------------------------------------------------
struct Services
{
    DatabaseConnection* db = nullptr;

    ResourceLinkedList* resources = nullptr;
    WarehouseLinkedList* warehouses = nullptr;
    InventoryManager* inventory = nullptr;
    EmergencyManager* emergencies = nullptr;
    AllocationManager* allocations = nullptr;
    ResponderManager* responders = nullptr;
    WarehouseTransferManager* transfers = nullptr;
    ActivityHistory* history = nullptr;
    LocationGraph* graph = nullptr;
    RoutePlanner* planner = nullptr;

    ResourceRepository* resourceRepo = nullptr;
    WarehouseRepository* warehouseRepo = nullptr;
    InventoryRepository* inventoryRepo = nullptr;
    EmergencyRepository* emergencyRepo = nullptr;
    AllocationRepository* allocationRepo = nullptr;
    ResponderRepository* responderRepo = nullptr;
    WarehouseTransferRepository* transferRepo = nullptr;
    ActivityHistoryRepository* activityRepo = nullptr;
    LocationRepository* locationRepo = nullptr;

    int userId = 0;          // logged-in user (0 = nobody)
    string actor = "SYSTEM"; // name written to the activity history
};

static Services S;

// RAII transaction: rolls back automatically unless commit() succeeded.
class DbTx
{
    DatabaseConnection& db;
    bool active;
public:
    explicit DbTx(DatabaseConnection& connection) : db(connection), active(connection.beginTransaction()) {}
    ~DbTx() { if (active) db.rollbackTransaction(); }
    DbTx(const DbTx&) = delete;
    DbTx& operator=(const DbTx&) = delete;
    bool began() const { return active; }
    bool commit()
    {
        if (!active) return false;
        active = false;
        return db.commitTransaction();
    }
};

// Activity records created during one operation. persist() writes them to MySQL (inside the
// operation's transaction); discard() removes them from memory when the operation fails.
class ActivityMark
{
    int baseline;
public:
    ActivityMark() : baseline(S.history->getLastActivityId()) {}
    bool persist() const
    {
        int last = S.history->getLastActivityId();
        for (int id = baseline + 1; id <= last; ++id)
        {
            const ActivityRecord* record = S.history->searchByActivityId(id);
            if (record != nullptr && !S.activityRepo->saveActivity(*record))
                return false;
        }
        return true;
    }
    void discard() const
    {
        while (S.history->getLastActivityId() > baseline)
            S.history->deleteActivity(S.history->getLastActivityId());
    }
};

static void note(ActivityType type, const string& description,
                 int emergencyId = -1, int allocationId = -1, int resourceId = -1,
                 int warehouseId = -1, int destinationId = -1, int responderId = -1, int quantity = 0)
{
    S.history->addActivity(type, description, S.actor, emergencyId, allocationId,
                           resourceId, warehouseId, destinationId, responderId, quantity);
}

static void reportDbFailure(const char* what)
{
    cout << "ERROR: " << what << " could not be saved to MySQL; the change was rolled back.\n";
    if (S.db != nullptr && S.db->getLastError()[0] != '\0')
        cout << "       Database says: " << S.db->getLastError() << "\n";
}

// ---------------------------------------------------------------------------
// Console input helpers
// ---------------------------------------------------------------------------
static bool readInt(const string& prompt, int& value)
{
    cout << prompt;
    if (cin >> value) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return true; }
    if (cin.eof()) { cout << "\nInput closed.\n"; exit(0); }
    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid numeric input.\n"; return false;
}
static bool readDouble(const string& prompt, double& value)
{
    cout << prompt;
    if (cin >> value) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return true; }
    if (cin.eof()) { cout << "\nInput closed.\n"; exit(0); }
    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid numeric input.\n"; return false;
}
static string trimCopy(const string& text)
{
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}
static string readText(const string& prompt)
{
    string value; cout << prompt;
    if (!getline(cin, value)) { cout << "\nInput closed.\n"; exit(0); }
    return trimCopy(value);
}
// Text field with a maximum length that matches the MySQL column size.
static bool validText(const string& value, size_t maxLength, const char* fieldName, bool required = true)
{
    if (required && value.empty()) { cout << fieldName << " cannot be empty.\n"; return false; }
    if (value.length() > maxLength) { cout << fieldName << " is too long (maximum " << maxLength << " characters).\n"; return false; }
    return true;
}
static bool validEmergencyStatusChoice(int s) { return s >= 1 && s <= 5; }
static EmergencyStatus statusFromChoice(int s)
{
    return s == 1 ? EmergencyStatus::REPORTED : s == 2 ? EmergencyStatus::VERIFIED :
           s == 3 ? EmergencyStatus::IN_PROGRESS : s == 4 ? EmergencyStatus::RESOLVED : EmergencyStatus::CANCELLED;
}
static const char* statusName(EmergencyStatus st)
{
    switch (st)
    {
        case EmergencyStatus::REPORTED: return "REPORTED";
        case EmergencyStatus::VERIFIED: return "VERIFIED";
        case EmergencyStatus::IN_PROGRESS: return "IN_PROGRESS";
        case EmergencyStatus::RESOLVED: return "RESOLVED";
        case EmergencyStatus::CANCELLED: return "CANCELLED";
    }
    return "UNKNOWN";
}
static void warnIfLowStock(const Inventory& item)
{
    if (item.isLowStock())
        cout << "WARNING: low stock - resource " << item.getResourceId() << " in warehouse "
             << item.getWarehouseId() << " has " << item.getAvailableQuantity()
             << " available (minimum " << item.getMinimumStockLevel() << ").\n";
}

void displayMainMenu(UserRole role)
{
    cout << "\n";
    cout << "============================================\n";
    cout << "              RESQFLOW SYSTEM               \n";
    cout << "     Emergency Resource Management System   \n";
    cout << "============================================\n";

    cout << "Logged-in Role: " << userRoleToString(role) << "\n";
    cout << "--------------------------------------------\n";

    if (role == UserRole::ADMIN)
    {
        cout << "1.  Resource Management\n";
        cout << "2.  Warehouse Management\n";
        cout << "3.  Inventory Management\n";
        cout << "4.  Emergency Management\n";
        cout << "5.  Allocation Management\n";
        cout << "6.  Responder Management\n";
        cout << "7.  Warehouse Transfer Management\n";
        cout << "8.  Location and Route Planning\n";
        cout << "9.  Activity History\n";
        cout << "10. User Management\n";
    }
    else if (role == UserRole::COORDINATOR)
    {
        cout << "1.  Resource Management\n";
        cout << "2.  Warehouse Management\n";
        cout << "3.  Inventory Management\n";
        cout << "4.  Emergency Management\n";
        cout << "5.  Allocation Management\n";
        cout << "6.  Responder Management\n";
        cout << "7.  Warehouse Transfer Management\n";
        cout << "8.  Location and Route Planning\n";
    }
    else
    {
        cout << "1. My Responder Dashboard\n";
    }

    cout << "0.  Logout\n";
    cout << "============================================\n";
    cout << "Enter your choice: ";
}

bool isMenuOptionAllowed(UserRole role, int choice)
{
    if (choice == 0)
        return true;

    if (role == UserRole::ADMIN)
        return choice >= 1 && choice <= 10;

    if (role == UserRole::COORDINATOR)
        return choice >= 1 && choice <= 8;

    if (role == UserRole::RESPONDER)
        return choice == 1;

    return false;
}

// ============================================================================
//  RESOURCE MANAGEMENT
// ============================================================================
static void displayResourceMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== RESOURCE MANAGEMENT ==========\n"
                "1 Add resource\n2 Search resource by ID\n3 Search resource by name\n4 Update resource details\n"
                "5 Deactivate resource\n6 Reactivate resource\n7 Display all resources\n8 View stock of a resource\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            int id, minimum;
            if (!readInt("Resource ID (positive number): ", id)) continue;
            string name = readText("Name: "), category = readText("Category: "),
                   description = readText("Description: "), unit = readText("Unit (e.g. Litre, Box): ");
            if (!readInt("Minimum stock level: ", minimum)) continue;

            if (id <= 0) { cout << "Resource ID must be a positive number.\n"; continue; }
            if (minimum < 0) { cout << "Minimum stock level cannot be negative.\n"; continue; }
            if (!validText(name, 120, "Name") || !validText(category, 80, "Category") ||
                !validText(unit, 30, "Unit") || !validText(description, 2000, "Description", false)) continue;

            if (S.resources->isDuplicate(id)) { cout << "Resource ID " << id << " already exists.\n"; continue; }
            if (S.resourceRepo->resourceExists(id))
            {
                cout << "Resource ID " << id << " exists in the database as a DEACTIVATED resource.\n"
                        "Use 'Reactivate resource' to bring it back, or choose a different ID.\n";
                continue;
            }

            Resource resource(id, name, category, description, unit, minimum);
            if (!S.resources->addResource(resource)) { cout << "Resource was rejected by the resource list.\n"; continue; }

            ActivityMark mark; DbTx tx(*S.db);
            if (tx.began() && S.resourceRepo->saveResource(resource) && mark.persist() && tx.commit())
                cout << "Resource " << id << " added and saved to MySQL.\n";
            else
            {
                S.resources->deleteResource(id);   // keep memory identical to the database
                mark.discard();
                reportDbFailure("Resource");
            }
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Resource ID: ", id)) continue;
            Resource* r = S.resources->searchResource(id);
            if (!r) { cout << "Not found.\n"; continue; }
            cout << "ID: " << r->getResourceId() << "\nName: " << r->getResourceName() << "\nCategory: " << r->getCategory()
                 << "\nDescription: " << r->getDescription() << "\nUnit: " << r->getUnit()
                 << "\nMinimum stock level: " << r->getMinimumStockLevel()
                 << "\nStatus: " << (r->isActive() ? "Active" : "Inactive") << "\n";
        }
        else if (op == 3)
        {
            string fragment = readText("Name (or part of it): ");
            if (S.resources->displayMatchingName(fragment) == 0) cout << "No resource matches '" << fragment << "'.\n";
        }
        else if (op == 4)
        {
            int id, minimum;
            if (!readInt("Resource ID: ", id)) continue;
            Resource* r = S.resources->searchResource(id);
            if (!r) { cout << "Not found.\n"; continue; }
            string name = readText("New name: "), category = readText("New category: "),
                   description = readText("New description: "), unit = readText("New unit: ");
            if (!readInt("New minimum stock level: ", minimum)) continue;
            if (minimum < 0) { cout << "Minimum stock level cannot be negative.\n"; continue; }
            if (!validText(name, 120, "Name") || !validText(category, 80, "Category") ||
                !validText(unit, 30, "Unit") || !validText(description, 2000, "Description", false)) continue;

            Resource before = *r;
            if (!S.resources->updateResource(id, name, category, description, unit, minimum)) { cout << "Update rejected.\n"; continue; }

            DbTx tx(*S.db);
            if (tx.began() && S.resourceRepo->updateResource(*r) && tx.commit())
                cout << "Resource updated and saved to MySQL.\n";
            else
            {
                *r = before;
                reportDbFailure("Resource update");
            }
        }
        else if (op == 5)
        {
            int id;
            if (!readInt("Resource ID: ", id)) continue;
            if (!S.resources->searchResource(id)) { cout << "Not found (or already inactive).\n"; continue; }
            if (S.inventory->hasStockForResource(id)) { cout << "Denied: stock of this resource still exists in a warehouse. Move or remove it first.\n"; continue; }
            if (S.allocations->hasActiveAllocationForResource(id)) { cout << "Denied: the resource has active allocations.\n"; continue; }
            if (S.resourceRepo->deactivateResource(id) && S.resources->deleteResource(id))
                cout << "Resource deactivated (history is kept in MySQL).\n";
            else reportDbFailure("Deactivation");
        }
        else if (op == 6)
        {
            int id;
            if (!readInt("Resource ID to reactivate: ", id)) continue;
            if (S.resources->isDuplicate(id)) { cout << "Resource is already active.\n"; continue; }
            if (!S.resourceRepo->reactivateResource(id)) { cout << "No deactivated resource with this ID exists.\n"; continue; }
            if (S.resourceRepo->loadActiveResources(*S.resources)) cout << "Resource reactivated.\n";
            else cout << "Resource reactivated in MySQL but the list could not be reloaded; restart ResQflow.\n";
        }
        else if (op == 7) S.resources->displayAllResources();
        else if (op == 8)
        {
            int id;
            if (!readInt("Resource ID: ", id)) continue;
            if (!S.resources->searchResource(id)) { cout << "Not found.\n"; continue; }
            S.inventory->displayInventoryByResource(id);
        }
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  WAREHOUSE MANAGEMENT  (city + area + full address; no coordinates asked)
// ============================================================================
static bool readWarehouseAddress(string& address, const char* prefix)
{
    string city = readText(string(prefix) + "City: ");
    string area = readText(string(prefix) + "Area / Locality: ");
    string full = readText(string(prefix) + "Complete address: ");
    if (!validText(city, 80, "City") || !validText(area, 80, "Area") || !validText(full, 100, "Complete address")) return false;
    address = "City: " + city + ", Area: " + area + ", Address: " + full;
    if (address.length() > 255) { cout << "Combined address is too long (maximum 255 characters).\n"; return false; }
    return true;
}

static void displayWarehouseMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== WAREHOUSE MANAGEMENT ==========\n1 Add warehouse\n2 Search warehouse\n3 Update warehouse\n"
                "4 Deactivate warehouse\n5 Display all\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            int id;
            if (!readInt("Warehouse ID (positive number): ", id)) continue;
            if (id <= 0) { cout << "Warehouse ID must be a positive number.\n"; continue; }
            string name = readText("Warehouse name: ");
            string address;
            if (!validText(name, 120, "Warehouse name") || !readWarehouseAddress(address, "")) continue;
            string contact = readText("Contact number: ");
            if (!validText(contact, 25, "Contact number", false)) continue;

            if (S.warehouses->isDuplicate(id)) { cout << "Warehouse ID already exists.\n"; continue; }
            if (S.warehouseRepo->warehouseExists(id)) { cout << "Warehouse ID exists in MySQL as a deactivated warehouse; choose another ID.\n"; continue; }

            Warehouse w(id, name, address, contact);
            if (!S.warehouses->addWarehouse(w)) { cout << "Warehouse was rejected (check the details).\n"; continue; }
            DbTx tx(*S.db);
            if (tx.began() && S.warehouseRepo->saveWarehouse(w) && tx.commit())
                cout << "Warehouse added and saved to MySQL.\n";
            else { S.warehouses->deleteWarehouse(id); reportDbFailure("Warehouse"); }
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Warehouse ID: ", id)) continue;
            const Warehouse* w = S.warehouses->searchWarehouse(id);
            if (w) w->displayWarehouse(); else cout << "Not found.\n";
        }
        else if (op == 3)
        {
            int id;
            if (!readInt("Warehouse ID: ", id)) continue;
            Warehouse* w = S.warehouses->searchWarehouse(id);
            if (!w) { cout << "Not found.\n"; continue; }
            string name = readText("New warehouse name: ");
            string address;
            if (!validText(name, 120, "Warehouse name") || !readWarehouseAddress(address, "New ")) continue;
            string contact = readText("New contact number: ");
            if (!validText(contact, 25, "Contact number", false)) continue;

            Warehouse before = *w;
            if (!S.warehouses->updateWarehouse(id, name, address, contact)) { cout << "Update rejected.\n"; continue; }
            if (S.warehouseRepo->updateWarehouse(*w)) cout << "Warehouse updated and saved to MySQL.\n";
            else { *w = before; reportDbFailure("Warehouse update"); }
        }
        else if (op == 4)
        {
            int id;
            if (!readInt("Warehouse ID: ", id)) continue;
            if (!S.warehouses->searchWarehouse(id)) { cout << "Not found (or already inactive).\n"; continue; }
            if (S.inventory->hasStockInWarehouse(id)) { cout << "Denied: the warehouse still holds stock. Transfer it out first.\n"; continue; }
            if (S.allocations->hasActiveAllocationForWarehouse(id)) { cout << "Denied: the warehouse has active allocations.\n"; continue; }
            if (S.transfers->hasPendingTransferForWarehouse(id)) { cout << "Denied: the warehouse has pending transfers.\n"; continue; }
            if (S.warehouseRepo->deactivateWarehouse(id) && S.warehouses->deleteWarehouse(id)) cout << "Warehouse deactivated.\n";
            else reportDbFailure("Deactivation");
        }
        else if (op == 5) S.warehouses->displayAllWarehouses();
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  INVENTORY MANAGEMENT
//  Every stock change = inventory row + stock-movement ledger row + activity entry, atomically.
// ============================================================================
static void displayInventoryMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== INVENTORY MANAGEMENT ==========\n1 Add inventory record\n2 Search inventory\n3 Add stock\n4 Remove stock\n"
                "5 Reserve stock\n6 Release reserved stock\n7 Dispatch stock\n8 Return stock\n9 Transfer stock between warehouses\n"
                "10 Delete empty inventory record\n11 Display all\n12 Low-stock alerts\n13 View stock by warehouse\n14 Update minimum stock level\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            int rid, wid, q, m;
            if (!readInt("Resource ID: ", rid) || !readInt("Warehouse ID: ", wid) ||
                !readInt("Initial quantity: ", q) || !readInt("Minimum stock: ", m)) continue;
            if (q < 0 || m < 0) { cout << "Quantity and minimum stock cannot be negative.\n"; continue; }
            if (!S.inventory->addInventory(rid, wid, q, m))
            { cout << "Could not create inventory. The resource and warehouse must be active, and the pair must not already have a record.\n"; continue; }
            Inventory* item = S.inventory->findInventory(rid, wid);
            int newId = item ? item->getInventoryId() : 0;
            ActivityMark mark; DbTx tx(*S.db);
            if (q > 0) note(ActivityType::STOCK_ADDED, "Initial stock registered", -1, -1, rid, wid, -1, -1, q);
            bool ok = item && tx.began() && S.inventoryRepo->saveInventory(*item) &&
                      (q == 0 || S.inventoryRepo->recordMovement(rid, wid, "STOCK_IN", q, "INVENTORY", newId, S.userId, "Initial stock")) &&
                      mark.persist() && tx.commit();
            if (ok) { cout << "Inventory record " << newId << " created and saved.\n"; warnIfLowStock(*item); }
            else { if (newId) S.inventory->discardInventory(newId); mark.discard(); reportDbFailure("Inventory record"); }
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Inventory ID: ", id)) continue;
            Inventory* item = S.inventory->searchInventory(id);
            if (item) item->displayInventory(); else cout << "Not found.\n";
        }
        else if (op >= 3 && op <= 8)
        {
            int id, q;
            if (!readInt("Inventory ID: ", id) || !readInt("Quantity: ", q)) continue;
            Inventory* item = S.inventory->searchInventory(id);
            if (!item) { cout << "Not found.\n"; continue; }
            if (q <= 0) { cout << "Quantity must be greater than zero.\n"; continue; }

            Inventory before = *item;
            bool applied = op == 3 ? S.inventory->addStock(id, q) : op == 4 ? S.inventory->removeStock(id, q) :
                           op == 5 ? S.inventory->reserveStock(id, q) : op == 6 ? S.inventory->releaseReservedStock(id, q) :
                           op == 7 ? S.inventory->dispatchStock(id, q) : S.inventory->returnStock(id, q);
            if (!applied) { cout << "Rejected: not enough stock for this operation (negative inventory is not allowed).\n"; continue; }

            const char* ledger = op == 3 ? "STOCK_IN" : op == 4 ? "ADJUSTMENT" : op == 5 ? "RESERVE" : op == 6 ? "RELEASE" : op == 7 ? "DISPATCH" : "RETURN";
            int rid = item->getResourceId(), wid = item->getWarehouseId();
            ActivityMark mark; DbTx tx(*S.db);
            if (op == 3) note(ActivityType::STOCK_ADDED, "Stock added", -1, -1, rid, wid, -1, -1, q);
            else if (op == 4) note(ActivityType::STOCK_REMOVED, "Stock removed (manual adjustment)", -1, -1, rid, wid, -1, -1, q);
            else if (op == 5) note(ActivityType::STOCK_RESERVED, "Stock reserved manually", -1, -1, rid, wid, -1, -1, q);
            else if (op == 7) note(ActivityType::STOCK_DISPATCHED, "Stock dispatched manually", -1, -1, rid, wid, -1, -1, q);
            else if (op == 8) note(ActivityType::STOCK_RETURNED, "Stock returned", -1, -1, rid, wid, -1, -1, q);
            bool ok = tx.began() && S.inventoryRepo->updateInventory(*item) &&
                      S.inventoryRepo->recordMovement(rid, wid, ledger, q, "INVENTORY", id, S.userId, "Manual inventory operation") &&
                      mark.persist() && tx.commit();
            if (ok) { cout << "Inventory operation saved.\n"; warnIfLowStock(*item); }
            else { *item = before; mark.discard(); reportDbFailure("Inventory operation"); }
        }
        else if (op == 9)
        {
            int src, dst, rid, q;
            if (!readInt("Source warehouse ID: ", src) || !readInt("Destination warehouse ID: ", dst) ||
                !readInt("Resource ID: ", rid) || !readInt("Quantity: ", q)) continue;
            Inventory* a = S.inventory->findInventory(rid, src);
            Inventory* b = S.inventory->findInventory(rid, dst);
            if (!a || !b) { cout << "Both warehouses need an inventory record for this resource.\n"; continue; }
            Inventory beforeA = *a, beforeB = *b;
            if (!S.inventory->transferStock(src, dst, rid, q)) { cout << "Transfer rejected (check quantity, warehouses and available stock).\n"; continue; }
            ActivityMark mark; DbTx tx(*S.db);
            note(ActivityType::WAREHOUSE_TRANSFERRED, "Direct stock transfer between warehouses", -1, -1, rid, src, dst, -1, q);
            bool ok = tx.began() && S.inventoryRepo->updateInventory(*a) && S.inventoryRepo->updateInventory(*b) &&
                      S.inventoryRepo->recordMovement(rid, src, "TRANSFER_OUT", q, "INVENTORY", 0, S.userId, "Direct transfer") &&
                      S.inventoryRepo->recordMovement(rid, dst, "TRANSFER_IN", q, "INVENTORY", 0, S.userId, "Direct transfer") &&
                      mark.persist() && tx.commit();
            if (ok) { cout << "Transfer completed and saved.\n"; warnIfLowStock(*a); }
            else { *a = beforeA; *b = beforeB; mark.discard(); reportDbFailure("Transfer"); }
        }
        else if (op == 10)
        {
            int id;
            if (!readInt("Inventory ID: ", id)) continue;
            Inventory* item = S.inventory->searchInventory(id);
            if (!item) { cout << "Not found.\n"; continue; }
            if (item->getTotalStock() != 0) { cout << "Denied: only records with zero stock can be deleted.\n"; continue; }
            if (S.inventoryRepo->deleteInventory(id) && S.inventory->deleteInventory(id)) cout << "Inventory record deleted.\n";
            else reportDbFailure("Delete");
        }
        else if (op == 11) S.inventory->displayAllInventory();
        else if (op == 12) S.inventory->displayLowStock();
        else if (op == 13)
        {
            int id;
            if (readInt("Warehouse ID: ", id)) S.inventory->displayInventoryByWarehouse(id);
        }
        else if (op == 14)
        {
            int id, m;
            if (!readInt("Inventory ID: ", id) || !readInt("New minimum stock level: ", m)) continue;
            Inventory* item = S.inventory->searchInventory(id);
            if (!item) { cout << "Not found.\n"; continue; }
            if (m < 0) { cout << "Minimum stock cannot be negative.\n"; continue; }
            Inventory before = *item;
            item->setMinimumStockLevel(m);
            if (S.inventoryRepo->updateInventory(*item)) { cout << "Minimum stock level updated.\n"; warnIfLowStock(*item); }
            else { *item = before; reportDbFailure("Minimum stock update"); }
        }
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  EMERGENCY MANAGEMENT
// ============================================================================
// Persists one allocation change (allocation row + inventory row + ledger + requirement + activity) atomically.
static bool persistAllocationChange(const Allocation& allocation, const Inventory* inventory,
                                    const char* ledgerType, int ledgerQuantity, const ActivityMark& mark,
                                    bool isNew)
{
    DbTx tx(*S.db);
    if (!tx.began()) return false;
    bool ok = isNew ? S.allocationRepo->saveAllocation(allocation) : S.allocationRepo->updateAllocation(allocation);
    ok = ok && (inventory == nullptr || S.inventoryRepo->updateInventory(*inventory));
    ok = ok && (ledgerType == nullptr || ledgerQuantity <= 0 ||
                S.inventoryRepo->recordMovement(allocation.getResourceId(), allocation.getWarehouseId(), ledgerType,
                                                ledgerQuantity, "ALLOCATION", allocation.getAllocationId(), S.userId, nullptr));
    ok = ok && S.allocationRepo->syncRequirement(allocation.getEmergencyId(), allocation.getResourceId());
    ok = ok && mark.persist();
    return ok && tx.commit();
}

// Cancels every not-yet-dispatched allocation of an emergency and frees its reserved stock.
// Allocations with dispatched goods stay open so the goods can still be returned through the normal workflow.
static void cancelOpenAllocationsOf(int emergencyId)
{
    int skippedDispatched = 0;

    for (int id = 1; id < S.allocations->peekNextAllocationId(); ++id)
    {
        Allocation* a = S.allocations->searchAllocation(id);

        if (a == nullptr || a->getEmergencyId() != emergencyId ||
            a->getStatus() == AllocationStatus::CANCELLED || a->getStatus() == AllocationStatus::COMPLETED)
            continue;

        if (a->getDispatchedQuantity() > 0) { ++skippedDispatched; continue; }

        Allocation beforeAllocation = *a;
        Inventory* inventory = S.inventory->findInventory(a->getResourceId(), a->getWarehouseId());
        Inventory beforeInventory = inventory ? *inventory : Inventory();
        int released = a->getAllocatedQuantity();

        if (!S.allocations->cancelAllocation(id)) { cout << "Allocation " << id << " could not be cancelled.\n"; continue; }

        ActivityMark mark;
        note(ActivityType::ALLOCATION_CANCELLED, "Allocation cancelled because the emergency was closed",
             emergencyId, id, a->getResourceId(), a->getWarehouseId(), -1, -1, released);

        if (persistAllocationChange(*a, inventory, "RELEASE", released, mark, false))
            cout << "Allocation " << id << " cancelled; " << released << " unit(s) returned to stock.\n";
        else
        {
            *a = beforeAllocation;
            if (inventory) *inventory = beforeInventory;
            mark.discard();
            reportDbFailure("Allocation cancellation");
        }
    }

    if (skippedDispatched > 0)
        cout << skippedDispatched << " allocation(s) with dispatched stock were left open; return the goods, then cancel/complete them.\n";
}

static void releaseRespondersOf(int emergencyId)
{
    for (int guard = 0; guard < 10000; ++guard)
    {
        Responder* r = S.responders->findAssignedToEmergency(emergencyId);
        if (!r) return;
        int rid = r->getResponderId();
        ActivityMark mark; DbTx tx(*S.db);
        note(ActivityType::RESPONDER_RELEASED, "Responder released because the emergency was closed", emergencyId, -1, -1, -1, -1, rid);
        if (tx.began() && S.responderRepo->releaseResponder(rid) && mark.persist() && tx.commit())
        {
            S.responders->releaseResponder(rid);
            cout << "Responder " << rid << " released.\n";
        }
        else { mark.discard(); reportDbFailure("Responder release"); return; }
    }
}

// Single entry point for every status change (coordinator menu and responder dashboard).
static bool changeEmergencyStatus(int id, EmergencyStatus target, const string& reason)
{
    Emergency* e = S.emergencies->searchEmergency(id);
    if (!e) { cout << "Emergency not found.\n"; return false; }
    if (!S.emergencies->canChangeStatus(id, target))
    {
        cout << "Invalid lifecycle transition: " << statusName(e->getStatus()) << " -> " << statusName(target) << ".\n"
             << "Allowed: REPORTED->VERIFIED|CANCELLED, VERIFIED->IN_PROGRESS|CANCELLED, IN_PROGRESS->RESOLVED|CANCELLED.\n"
             << "RESOLVED and CANCELLED are final.\n";
        return false;
    }

    string from = statusName(e->getStatus());
    ActivityMark mark; DbTx tx(*S.db);
    note(ActivityType::EMERGENCY_STATUS_CHANGED, "Emergency " + to_string(id) + ": " + from + " -> " + statusName(target) + " (" + reason + ")", id);
    bool saved = tx.began() && S.emergencyRepo->updateEmergencyStatus(id, target, reason.c_str(), S.userId) &&
                 mark.persist() && tx.commit();
    if (!saved) { mark.discard(); reportDbFailure("Emergency status"); return false; }

    if (!S.emergencies->updateEmergencyStatus(id, target))
    {   // cannot happen after canChangeStatus(); reported loudly instead of hidden
        cout << "CRITICAL: MySQL was updated but the in-memory priority queue refused the change. Restart ResQflow.\n";
        return false;
    }

    if (target == EmergencyStatus::CANCELLED || target == EmergencyStatus::RESOLVED)
    {
        if (target == EmergencyStatus::CANCELLED) cancelOpenAllocationsOf(id);
        releaseRespondersOf(id);
    }
    return true;
}

static void displayEmergencyMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== EMERGENCY MANAGEMENT ==========\n1 Register emergency\n2 Search emergency\n3 Update details\n"
                "4 Change severity\n5 Update affected people\n6 Change status\n7 Display all\n8 Display by status\n"
                "9 Cancel emergency\n10 Emergency status history\n11 Priority queue (highest priority first)\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            string type = readText("Type: "), loc = readText("Location: "), desc = readText("Description: ");
            int sev, people; double lat, lon;
            if (!readInt("Severity (1-5): ", sev) || !readInt("Affected people: ", people) ||
                !readDouble("Latitude (-90..90): ", lat) || !readDouble("Longitude (-180..180): ", lon)) continue;
            if (!validText(type, 100, "Type") || !validText(loc, 255, "Location") || !validText(desc, 2000, "Description", false)) continue;
            if (sev < 1 || sev > 5) { cout << "Severity must be between 1 and 5.\n"; continue; }
            if (people < 1) { cout << "Affected people must be at least 1.\n"; continue; }
            if (lat < -90 || lat > 90 || lon < -180 || lon > 180) { cout << "Coordinates out of range.\n"; continue; }

            int newId = S.emergencies->peekNextEmergencyId();
            if (!S.emergencies->addEmergency(type, loc, desc, sev, lat, lon, people)) { cout << "Registration rejected.\n"; continue; }
            Emergency* e = S.emergencies->searchEmergency(newId);
            ActivityMark mark; DbTx tx(*S.db);
            note(ActivityType::EMERGENCY_REPORTED, "Emergency registered: " + type + " at " + loc, newId);
            if (e && tx.began() && S.emergencyRepo->saveEmergency(*e) && mark.persist() && tx.commit())
                cout << "Emergency registered and saved. ID: " << newId << "\n";
            else { S.emergencies->deleteEmergency(newId); mark.discard(); reportDbFailure("Emergency"); }
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Emergency ID: ", id)) continue;
            Emergency* e = S.emergencies->searchEmergency(id);
            if (e) e->displayEmergency(); else cout << "Not found.\n";
        }
        else if (op == 3)
        {
            int id;
            if (!readInt("Emergency ID: ", id)) continue;
            Emergency* e = S.emergencies->searchEmergency(id);
            if (!e) { cout << "Not found.\n"; continue; }
            if (e->getStatus() == EmergencyStatus::RESOLVED || e->getStatus() == EmergencyStatus::CANCELLED)
            { cout << "Denied: " << statusName(e->getStatus()) << " emergencies are final and cannot be edited.\n"; continue; }
            string type = readText("New type: "), loc = readText("New location: "), desc = readText("New description: ");
            if (!validText(type, 100, "Type") || !validText(loc, 255, "Location") || !validText(desc, 2000, "Description", false)) continue;

            string oldType = e->getEmergencyType(), oldLoc = e->getLocation(), oldDesc = e->getDescription();
            if (!S.emergencies->updateEmergencyDetails(id, type, loc, desc)) { cout << "ERROR: Emergency Manager rejected the update.\n"; continue; }
            if (S.emergencyRepo->updateEmergencyDetails(*e)) cout << "Details updated and saved to MySQL.\n";
            else
            {
                S.emergencies->updateEmergencyDetails(id, oldType, oldLoc, oldDesc);
                reportDbFailure("Emergency details");
            }
        }
        else if (op == 4 || op == 5)
        {
            int id, v;
            if (!readInt("Emergency ID: ", id) || !readInt(op == 4 ? "New severity (1-5): " : "Affected people: ", v)) continue;
            Emergency* e = S.emergencies->searchEmergency(id);
            if (!e) { cout << "Not found.\n"; continue; }
            int oldValue = op == 4 ? e->getSeverity() : e->getAffectedPeople();
            bool ok = op == 4 ? S.emergencies->updateEmergencySeverity(id, v) : S.emergencies->updateEmergencyAffectedPeople(id, v);
            if (!ok) { cout << "Rejected (value out of range or emergency is final).\n"; continue; }
            if (op == 4 ? S.emergencyRepo->updateEmergencySeverity(*e) : S.emergencyRepo->updateEmergencyAffectedPeople(*e))
                cout << "Updated; priority queue re-ordered.\n";
            else
            {
                if (op == 4) S.emergencies->updateEmergencySeverity(id, oldValue); else S.emergencies->updateEmergencyAffectedPeople(id, oldValue);
                reportDbFailure("Emergency update");
            }
        }
        else if (op == 6)
        {
            int id, s;
            if (!readInt("Emergency ID: ", id)) continue;
            Emergency* e = S.emergencies->searchEmergency(id);
            if (!e) { cout << "Not found.\n"; continue; }
            cout << "Current status: " << statusName(e->getStatus()) << "\n";
            if (!readInt("New status: 1 REPORTED, 2 VERIFIED, 3 IN_PROGRESS, 4 RESOLVED, 5 CANCELLED: ", s)) continue;
            if (!validEmergencyStatusChoice(s)) { cout << "Invalid status.\n"; continue; }
            if (changeEmergencyStatus(id, statusFromChoice(s), "Updated by " + S.actor)) cout << "Status updated.\n";
        }
        else if (op == 7) S.emergencies->displayAllEmergencies();
        else if (op == 8)
        {
            int s;
            if (!readInt("Status 1 REPORTED, 2 VERIFIED, 3 IN_PROGRESS, 4 RESOLVED, 5 CANCELLED: ", s)) continue;
            if (validEmergencyStatusChoice(s)) S.emergencies->displayEmergenciesByStatus(statusFromChoice(s)); else cout << "Invalid status.\n";
        }
        else if (op == 9)
        {
            int id;
            if (!readInt("Emergency ID to cancel: ", id)) continue;
            string reason = readText("Reason: ");
            if (!validText(reason, 400, "Reason")) continue;
            if (changeEmergencyStatus(id, EmergencyStatus::CANCELLED, "Cancelled: " + reason)) cout << "Emergency cancelled.\n";
        }
        else if (op == 10)
        {
            int id;
            if (!readInt("Emergency ID: ", id)) continue;
            if (!S.emergencies->searchEmergency(id)) { cout << "Not found.\n"; continue; }
            if (!S.emergencyRepo->displayStatusHistory(id)) reportDbFailure("History query");
        }
        else if (op == 11)
        {
            Emergency top;
            if (!S.emergencies->peekHighestPriority(top)) { cout << "\nNo active emergencies in the priority queue.\n"; continue; }
            cout << "\nNext emergency to serve: ID " << top.getEmergencyId() << " (" << top.getEmergencyType()
                 << ", score " << top.calculatePriorityScore() << ")\n";
            S.emergencies->displayPriorityQueue();
        }
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  ALLOCATION MANAGEMENT
// ============================================================================
static bool createAllocationFor(int emergencyId, int rid, int wid, int qty)
{
    int id = S.allocations->peekNextAllocationId();
    if (!S.allocations->createAllocation(emergencyId, rid, wid, qty))
    {
        cout << "Allocation rejected: the emergency must be active, and the resource must be active with an inventory record in that active warehouse.\n";
        return false;
    }
    Allocation* a = S.allocations->searchAllocation(id);
    Inventory* inv = S.inventory->findInventory(rid, wid);
    ActivityMark mark;
    note(ActivityType::ALLOCATION_CREATED, "Allocation created for requirement of " + to_string(qty), emergencyId, id, rid, wid, -1, -1, a ? a->getAllocatedQuantity() : 0);
    if (a && a->getAllocatedQuantity() > 0)
        note(ActivityType::STOCK_RESERVED, "Stock reserved for allocation " + to_string(id), emergencyId, id, rid, wid, -1, -1, a->getAllocatedQuantity());
    if (a && inv && persistAllocationChange(*a, inv, "RESERVE", a->getAllocatedQuantity(), mark, true))
    {
        cout << "Allocation " << id << " created: " << a->getAllocatedQuantity() << " of " << qty << " requested unit(s) reserved"
             << (a->getAllocatedQuantity() < qty ? " (partial - not enough stock; use 'Allocate additional stock' later)" : "") << ".\n";
        warnIfLowStock(*inv);
        return true;
    }
    S.allocations->removeAllocation(id);   // also releases the reserved stock in memory
    mark.discard();
    reportDbFailure("Allocation");
    return false;
}

static void displayAllocationMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== ALLOCATION MANAGEMENT ==========\n1 Create allocation (one resource)\n2 Search allocation\n3 Allocate additional stock\n"
                "4 Dispatch allocation\n5 Return dispatched stock\n6 Complete allocation\n7 Cancel allocation\n8 Display all\n"
                "9 Filter by emergency\n10 Add several requirements to an emergency\n11 Show emergency requirements (from MySQL)\n"
                "12 Auto-allocate a requirement across warehouses\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            int e, r, w, q;
            if (!readInt("Emergency ID: ", e) || !readInt("Resource ID: ", r) || !readInt("Warehouse ID: ", w) || !readInt("Requested quantity: ", q)) continue;
            if (q <= 0) { cout << "Requested quantity must be greater than zero.\n"; continue; }
            createAllocationFor(e, r, w, q);
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Allocation ID: ", id)) continue;
            Allocation* a = S.allocations->searchAllocation(id);
            if (a) a->displayAllocation(); else cout << "Not found.\n";
        }
        else if (op >= 3 && op <= 7)
        {
            int id;
            if (!readInt("Allocation ID: ", id)) continue;
            Allocation* a = S.allocations->searchAllocation(id);
            if (!a) { cout << "Not found.\n"; continue; }
            int q = 0;
            if (op <= 5 && !readInt("Quantity: ", q)) continue;
            if (op <= 5 && q <= 0) { cout << "Quantity must be greater than zero.\n"; continue; }

            Allocation beforeAllocation = *a;
            Inventory* inv = S.inventory->findInventory(a->getResourceId(), a->getWarehouseId());
            Inventory beforeInventory = inv ? *inv : Inventory();
            int reservedBefore = a->getAllocatedQuantity() - a->getDispatchedQuantity();

            bool ok = op == 3 ? S.allocations->allocateAdditionalStock(id, q) : op == 4 ? S.allocations->dispatchAllocation(id, q) :
                      op == 5 ? S.allocations->returnDispatchedStock(id, q) : op == 6 ? S.allocations->completeAllocation(id) :
                      S.allocations->cancelAllocation(id);
            if (!ok) { cout << "Rejected: check quantities, stock availability and the allocation/emergency state.\n"; continue; }

            const char* ledger = nullptr; int ledgerQty = 0;
            ActivityMark mark;
            int e = a->getEmergencyId(), r = a->getResourceId(), w = a->getWarehouseId();
            if (op == 3) { ledger = "RESERVE"; ledgerQty = q; note(ActivityType::STOCK_RESERVED, "Additional stock reserved", e, id, r, w, -1, -1, q); }
            else if (op == 4) { ledger = "DISPATCH"; ledgerQty = q; note(ActivityType::STOCK_DISPATCHED, "Stock dispatched to emergency", e, id, r, w, -1, -1, q); }
            else if (op == 5) { ledger = "RETURN"; ledgerQty = q; note(ActivityType::STOCK_RETURNED, "Dispatched stock returned", e, id, r, w, -1, -1, q); }
            else if (op == 7) { ledger = "RELEASE"; ledgerQty = reservedBefore; note(ActivityType::ALLOCATION_CANCELLED, "Allocation cancelled", e, id, r, w, -1, -1, reservedBefore); }

            if (persistAllocationChange(*a, inv, ledger, ledgerQty, mark, false))
            { cout << "Allocation updated and saved.\n"; if (inv) warnIfLowStock(*inv); }
            else
            {
                *a = beforeAllocation;
                if (inv) *inv = beforeInventory;
                mark.discard();
                reportDbFailure("Allocation update");
            }
        }
        else if (op == 8) S.allocations->displayAllAllocations();
        else if (op == 9)
        {
            int id;
            if (readInt("Emergency ID: ", id)) S.allocations->displayAllocationsByEmergency(id);
        }
        else if (op == 10)
        {
            int e;
            if (!readInt("Emergency ID: ", e)) continue;
            if (!S.emergencies->searchEmergency(e)) { cout << "Emergency not found.\n"; continue; }
            cout << "Enter requirements one by one. Resource ID 0 finishes.\n";
            while (true)
            {
                int r, w, q;
                if (!readInt("Resource ID (0 = done): ", r)) continue;
                if (r == 0) break;
                if (!readInt("Warehouse ID: ", w) || !readInt("Quantity: ", q)) continue;
                if (q <= 0) { cout << "Quantity must be greater than zero.\n"; continue; }
                createAllocationFor(e, r, w, q);
            }
        }
        else if (op == 11)
        {
            int e;
            if (!readInt("Emergency ID: ", e)) continue;
            if (!S.allocationRepo->displayRequirements(e)) reportDbFailure("Requirement query");
        }
        else if (op == 12)
        {
            int e, r, q;
            if (!readInt("Emergency ID: ", e) || !readInt("Resource ID: ", r) || !readInt("Total quantity needed: ", q)) continue;
            if (q <= 0) { cout << "Quantity must be greater than zero.\n"; continue; }
            if (!S.emergencies->searchEmergency(e)) { cout << "Emergency not found.\n"; continue; }

            const int CAP = 256;
            Inventory* stock[CAP];
            int n = S.inventory->collectByResource(r, stock, CAP);
            // selection sort: warehouse with the most available stock first (fewest allocations needed)
            for (int i = 0; i < n; ++i)
            {
                int best = i;
                for (int j = i + 1; j < n; ++j)
                    if (stock[j]->getAvailableQuantity() > stock[best]->getAvailableQuantity()) best = j;
                Inventory* t = stock[i]; stock[i] = stock[best]; stock[best] = t;
            }
            int remaining = q;
            for (int i = 0; i < n && remaining > 0; ++i)
            {
                int available = stock[i]->getAvailableQuantity();
                if (available <= 0 || !S.inventory->isWarehouseActive(stock[i]->getWarehouseId())) continue;
                int take = available < remaining ? available : remaining;
                if (!createAllocationFor(e, r, stock[i]->getWarehouseId(), take)) break;
                remaining -= take;
            }
            if (remaining > 0) cout << "Auto-allocation finished: " << remaining << " unit(s) could not be covered by current stock.\n";
            else cout << "Auto-allocation finished: the full requirement is reserved.\n";
        }
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  RESPONDER MANAGEMENT
// ============================================================================
static bool emergencyAcceptsResponders(int emergencyId)
{
    Emergency* e = S.emergencies->searchEmergency(emergencyId);
    if (!e) { cout << "Emergency not found.\n"; return false; }
    if (e->getStatus() == EmergencyStatus::RESOLVED || e->getStatus() == EmergencyStatus::CANCELLED)
    { cout << "Denied: the emergency is " << statusName(e->getStatus()) << ".\n"; return false; }
    return true;
}

// Persist "responder assigned" (responders + assignments + activity) atomically.
static bool persistAssignment(int rid, int eid)
{
    ActivityMark mark; DbTx tx(*S.db);
    note(ActivityType::RESPONDER_ASSIGNED, "Responder assigned to emergency", eid, -1, -1, -1, -1, rid);
    if (tx.began() && S.responderRepo->assignResponder(rid, eid) && mark.persist() && tx.commit())
        return true;
    mark.discard();
    return false;
}

static void displayResponderMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== RESPONDER MANAGEMENT ==========\n1 Add responder\n2 Search responder\n3 Assign responder to emergency\n"
                "4 Assign next available responder (round-robin)\n5 Release responder\n6 Set off duty\n7 Set available\n"
                "8 Display all\n9 Display available\n10 Display assigned\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            int id; double lat, lon;
            if (!readInt("Responder ID (positive number): ", id)) continue;
            string name = readText("Name: "), role = readText("Role: "), contact = readText("Contact: "), spec = readText("Specialization: ");
            if (!readDouble("Base latitude: ", lat) || !readDouble("Base longitude: ", lon)) continue;
            if (id <= 0) { cout << "Responder ID must be positive.\n"; continue; }
            if (!validText(name, 120, "Name") || !validText(role, 80, "Role") || !validText(contact, 25, "Contact") ||
                !validText(spec, 120, "Specialization")) continue;
            if (S.responders->findResponder(id) || S.responderRepo->responderExists(id)) { cout << "Responder ID already exists.\n"; continue; }
            Responder r(id, name, role, contact, spec, lat, lon);
            if (!S.responders->addResponder(r)) { cout << "Responder rejected (check coordinates and details).\n"; continue; }
            DbTx tx(*S.db);
            if (tx.began() && S.responderRepo->saveResponder(r) && tx.commit()) cout << "Responder added and saved.\n";
            else { S.responders->removeResponder(id); reportDbFailure("Responder"); }
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Responder ID: ", id)) continue;
            Responder* r = S.responders->findResponder(id);
            if (r) r->displayResponder(); else cout << "Not found.\n";
        }
        else if (op == 3 || op == 4)
        {
            int rid = 0, eid;
            if (op == 3 && !readInt("Responder ID: ", rid)) continue;
            if (!readInt("Emergency ID: ", eid)) continue;
            if (!emergencyAcceptsResponders(eid)) continue;
            if (op == 3)
            {
                Responder* r = S.responders->findResponder(rid);
                if (!r) { cout << "Responder not found.\n"; continue; }
                if (r->getStatus() != ResponderStatus::AVAILABLE) { cout << "Denied: responder is " << r->getStatusText() << ", not available.\n"; continue; }
                if (!S.responders->assignResponder(rid, eid)) { cout << "Assignment rejected.\n"; continue; }
            }
            else if (!S.responders->assignNextAvailableResponder(eid, rid)) { cout << "No available responder.\n"; continue; }

            if (persistAssignment(rid, eid)) cout << "Responder " << rid << " assigned to emergency " << eid << ".\n";
            else { S.responders->releaseResponder(rid); reportDbFailure("Assignment"); }
        }
        else if (op == 5)
        {
            int id;
            if (!readInt("Responder ID: ", id)) continue;
            Responder* r = S.responders->findResponder(id);
            if (!r) { cout << "Not found.\n"; continue; }
            if (r->getStatus() != ResponderStatus::ASSIGNED) { cout << "Responder is not assigned to an emergency.\n"; continue; }
            int eid = r->getAssignedEmergencyId();
            ActivityMark mark; DbTx tx(*S.db);
            note(ActivityType::RESPONDER_RELEASED, "Responder released", eid, -1, -1, -1, -1, id);
            if (tx.began() && S.responderRepo->releaseResponder(id) && mark.persist() && tx.commit())
            { S.responders->releaseResponder(id); cout << "Responder released and available again.\n"; }
            else { mark.discard(); reportDbFailure("Release"); }
        }
        else if (op == 6 || op == 7)
        {
            int id;
            if (!readInt("Responder ID: ", id)) continue;
            Responder* r = S.responders->findResponder(id);
            if (!r) { cout << "Not found.\n"; continue; }
            Responder before = *r;
            bool ok = op == 6 ? S.responders->setResponderOffDuty(id) : S.responders->setResponderAvailable(id);
            if (!ok) { cout << "Denied: assigned responders must be released first.\n"; continue; }
            if (S.responderRepo->updateResponderStatus(id, r->getStatus())) cout << "Responder status saved.\n";
            else { *r = before; reportDbFailure("Responder status"); }
        }
        else if (op == 8) S.responders->displayAllResponders();
        else if (op == 9) S.responders->displayAvailableResponders();
        else if (op == 10) S.responders->displayAssignedResponders();
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  WAREHOUSE TRANSFERS
// ============================================================================
static void displayTransferMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== WAREHOUSE TRANSFERS ==========\n1 Create transfer request\n2 Search transfer\n3 Execute transfer\n"
                "4 Cancel transfer\n5 Display all\n6 Filter by warehouse\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            int s, d, r, q;
            if (!readInt("Source warehouse ID: ", s) || !readInt("Destination warehouse ID: ", d) ||
                !readInt("Resource ID: ", r) || !readInt("Quantity: ", q)) continue;
            string remarks = readText("Remarks: ");
            if (q <= 0) { cout << "Quantity must be greater than zero.\n"; continue; }
            if (s == d) { cout << "Source and destination must differ.\n"; continue; }
            if (!validText(remarks, 300, "Remarks", false)) continue;
            int id = S.transfers->peekNextTransferId();
            if (!S.transfers->createTransfer(s, d, r, q, S.actor, remarks))
            { cout << "Transfer request rejected: both warehouses need an inventory record of the resource and the source needs enough available stock.\n"; continue; }
            WarehouseTransfer* t = S.transfers->searchTransfer(id);
            if (t && S.transferRepo->saveTransfer(*t)) cout << "Transfer request " << id << " saved.\n";
            else { S.transfers->removeTransfer(id); reportDbFailure("Transfer request"); }
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Transfer ID: ", id)) continue;
            WarehouseTransfer* t = S.transfers->searchTransfer(id);
            if (t) t->display(); else cout << "Not found.\n";
        }
        else if (op == 3 || op == 4)
        {
            int id;
            if (!readInt("Transfer ID: ", id)) continue;
            WarehouseTransfer* t = S.transfers->searchTransfer(id);
            if (!t) { cout << "Not found.\n"; continue; }
            int src = t->getSourceWarehouseId(), dst = t->getDestinationWarehouseId(), rid = t->getResourceId(), q = t->getQuantity();

            WarehouseTransfer beforeTransfer = *t;
            Inventory* a = S.inventory->findInventory(rid, src);
            Inventory* b = S.inventory->findInventory(rid, dst);
            Inventory beforeA = a ? *a : Inventory(), beforeB = b ? *b : Inventory();
            ActivityMark mark;

            bool ok = op == 3 ? S.transfers->executeTransfer(id) : S.transfers->cancelTransfer(id);
            if (!ok) { cout << "Rejected: only PENDING transfers can be executed/cancelled, and stock must still be available.\n"; mark.discard(); continue; }

            DbTx tx(*S.db);
            bool saved = tx.began() && S.transferRepo->updateTransfer(*t);
            if (op == 3 && saved)
            {
                saved = a && b && S.inventoryRepo->updateInventory(*a) && S.inventoryRepo->updateInventory(*b) &&
                        S.inventoryRepo->recordMovement(rid, src, "TRANSFER_OUT", q, "TRANSFER", id, S.userId, nullptr) &&
                        S.inventoryRepo->recordMovement(rid, dst, "TRANSFER_IN", q, "TRANSFER", id, S.userId, nullptr);
            }
            saved = saved && mark.persist() && tx.commit();

            if (saved) cout << (op == 3 ? "Transfer executed: source and destination stock updated and saved.\n" : "Transfer cancelled and saved.\n");
            else
            {
                *t = beforeTransfer;
                if (a) *a = beforeA;
                if (b) *b = beforeB;
                mark.discard();
                reportDbFailure("Transfer");
            }
        }
        else if (op == 5) S.transfers->displayAllTransfers();
        else if (op == 6)
        {
            int id;
            if (readInt("Warehouse ID: ", id)) S.transfers->displayTransfersByWarehouse(id);
        }
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  LOCATIONS AND ROUTE PLANNING (stored / manually entered coordinates only)
// ============================================================================
static void displayLocationMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== LOCATION & ROUTE PLANNING ==========\n1 Add location\n2 Search location\n3 Deactivate/remove location\n"
                "4 Add connection\n5 Remove connection\n6 BFS traversal\n7 DFS traversal\n8 Shortest route (Dijkstra)\n"
                "9 Display graph\n10 Update location\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;

        if (op == 1)
        {
            int id; double lat, lon;
            if (!readInt("Location ID (positive number): ", id)) continue;
            string name = readText("Location name: ");
            if (!readDouble("Latitude: ", lat) || !readDouble("Longitude: ", lon)) continue;
            if (id <= 0) { cout << "Location ID must be positive.\n"; continue; }
            if (!validText(name, 150, "Location name")) continue;
            if (lat < -90 || lat > 90 || lon < -180 || lon > 180) { cout << "Coordinates out of range.\n"; continue; }
            if (S.graph->findLocation(id)) { cout << "Location ID already exists.\n"; continue; }
            if (S.locationRepo->locationExists(id)) { cout << "Location ID exists in MySQL as a deactivated location; choose another ID.\n"; continue; }
            Location l(id, name, lat, lon);
            if (!S.graph->addLocation(l)) { cout << "Location rejected.\n"; continue; }
            if (S.locationRepo->saveLocation(l)) cout << "Location added and saved.\n";
            else { S.graph->removeLocation(id); reportDbFailure("Location"); }
        }
        else if (op == 2)
        {
            int id;
            if (!readInt("Location ID: ", id)) continue;
            Location* l = S.graph->findLocation(id);
            if (l) l->displayLocation(); else cout << "Not found.\n";
        }
        else if (op == 3)
        {
            int id;
            if (!readInt("Location ID: ", id)) continue;
            if (!S.graph->findLocation(id)) { cout << "Not found.\n"; continue; }
            if (S.locationRepo->deactivateLocation(id) && S.graph->removeLocation(id)) cout << "Location and its roads removed.\n";
            else reportDbFailure("Location removal");
        }
        else if (op == 4)
        {
            int s, d; double dist;
            if (!readInt("Source location ID: ", s) || !readInt("Destination location ID: ", d) ||
                !readDouble("Distance in km (0 = calculate from stored coordinates): ", dist)) continue;
            Location* a = S.graph->findLocation(s);
            Location* b = S.graph->findLocation(d);
            if (!a || !b) { cout << "Both locations must exist.\n"; continue; }
            if (s == d) { cout << "A road must connect two different locations.\n"; continue; }
            if (dist == 0.0) { dist = a->distanceKmTo(*b); cout << "Calculated distance: " << dist << " km\n"; }
            if (dist <= 0.0) { cout << "Distance must be positive (the two coordinates are identical).\n"; continue; }
            if (S.graph->hasConnection(s, d)) { cout << "These locations are already connected.\n"; continue; }
            if (!S.graph->addConnection(s, d, dist)) { cout << "Connection rejected.\n"; continue; }
            if (S.locationRepo->saveConnection(s, d, dist)) cout << "Connection added and saved.\n";
            else { S.graph->removeConnection(s, d); reportDbFailure("Connection"); }
        }
        else if (op == 5)
        {
            int s, d;
            if (!readInt("Source location ID: ", s) || !readInt("Destination location ID: ", d)) continue;
            if (!S.graph->hasConnection(s, d)) { cout << "No such connection.\n"; continue; }
            if (S.locationRepo->deactivateConnection(s, d) && S.graph->removeConnection(s, d)) cout << "Connection removed.\n";
            else reportDbFailure("Connection removal");
        }
        else if (op == 6) { int id; if (readInt("Start location ID: ", id)) S.graph->breadthFirstSearch(id); }
        else if (op == 7) { int id; if (readInt("Start location ID: ", id)) S.graph->depthFirstSearch(id); }
        else if (op == 8) { int s, d; if (readInt("Source location ID: ", s) && readInt("Destination location ID: ", d)) S.planner->planRoute(s, d); }
        else if (op == 9) S.graph->displayGraph();
        else if (op == 10)
        {
            int id; double lat, lon;
            if (!readInt("Location ID: ", id)) continue;
            Location* l = S.graph->findLocation(id);
            if (!l) { cout << "Not found.\n"; continue; }
            string name = readText("New name: ");
            if (!readDouble("New latitude: ", lat) || !readDouble("New longitude: ", lon)) continue;
            if (!validText(name, 150, "Location name")) continue;
            Location before = *l;
            l->setLocationName(name);
            if (!l->setCoordinates(lat, lon)) { *l = before; cout << "Coordinates out of range.\n"; continue; }
            if (S.locationRepo->updateLocation(*l)) cout << "Location updated and saved. (Existing road lengths are not recalculated.)\n";
            else { *l = before; reportDbFailure("Location update"); }
        }
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  ACTIVITY HISTORY
// ============================================================================
static void displayActivityMenu()
{
    int op = -1;
    while (op != 0)
    {
        cout << "\n========== ACTIVITY HISTORY ==========\n1 Display all\n2 Search activity ID\n3 Filter by type (1-12)\n4 Filter by emergency\n"
                "5 Filter by allocation\n6 Filter by warehouse\n0 Back\n";
        if (!readInt("Choice: ", op)) continue;
        if (op == 1) S.history->displayAll();
        else if (op == 2) { int id; if (readInt("Activity ID: ", id)) { const ActivityRecord* a = S.history->searchByActivityId(id); if (a) a->display(); else cout << "Not found.\n"; } }
        else if (op == 3) { int t; if (readInt("Activity type 1-12: ", t) && t >= 1 && t <= 12) S.history->displayByType(static_cast<ActivityType>(t - 1)); else cout << "Invalid type.\n"; }
        else if (op == 4) { int id; if (readInt("Emergency ID: ", id)) S.history->displayByEmergency(id); }
        else if (op == 5) { int id; if (readInt("Allocation ID: ", id)) S.history->displayByAllocation(id); }
        else if (op == 6) { int id; if (readInt("Warehouse ID: ", id)) S.history->displayByWarehouse(id); }
        else if (op != 0) cout << "Invalid choice.\n";
    }
}

// ============================================================================
//  RESPONDER DASHBOARD (role RESPONDER only)
// ============================================================================
static void responderDashboard(int responderId, const User& user)
{
    Responder* responder = S.responders->findResponder(responderId);
    if (!responder) { cout << "Responder profile not found or inactive. Contact an Admin.\n"; return; }
    int option = -1;
    while (option != 0)
    {
        cout << "\n========== RESPONDER DASHBOARD ==========\n";
        cout << "Welcome, " << user.getFullName() << "\nResponder ID: " << responder->getResponderId()
             << "\nName: " << responder->getResponderName() << "\nSpecialization: " << responder->getSpecialization()
             << "\nCurrent status: " << responder->getStatusText() << "\n";
        if (responder->getAssignedEmergencyId() > 0)
        {
            cout << "\n--- Assigned Emergency ---\n";
            Emergency* emergency = S.emergencies->searchEmergency(responder->getAssignedEmergencyId());
            if (emergency) emergency->displayEmergency(); else cout << "Assigned emergency record is unavailable.\n";
        }
        if (!readInt("\n1. Mark available\n2. Mark off duty\n3. Update assigned emergency progress\n0. Logout\nChoice: ", option)) continue;

        if (option == 1 || option == 2)
        {
            Responder before = *responder;
            bool changed = option == 1 ? S.responders->setResponderAvailable(responderId) : S.responders->setResponderOffDuty(responderId);
            if (!changed) { cout << "Update denied. Assigned responders cannot change availability.\n"; continue; }
            if (S.responderRepo->updateResponderStatus(responderId, responder->getStatus())) cout << "Availability updated.\n";
            else { *responder = before; reportDbFailure("Availability"); }
        }
        else if (option == 3)
        {
            int emergencyId = responder->getAssignedEmergencyId();
            if (emergencyId <= 0) { cout << "No emergency is currently assigned to you.\n"; continue; }
            int statusChoice;
            if (!readInt("1. Mark IN_PROGRESS\n2. Mark RESOLVED\nChoice: ", statusChoice)) continue;
            if (statusChoice != 1 && statusChoice != 2) { cout << "Invalid choice.\n"; continue; }
            if (!S.emergencies->searchEmergency(emergencyId)) { cout << "Emergency unavailable.\n"; continue; }
            EmergencyStatus next = statusChoice == 1 ? EmergencyStatus::IN_PROGRESS : EmergencyStatus::RESOLVED;
            if (changeEmergencyStatus(emergencyId, next, "Updated by assigned responder " + user.getFullName()))
                cout << "Emergency status updated.\n";
            else cout << "Status update rejected. Check the emergency lifecycle (a coordinator must VERIFY it first).\n";
        }
        else if (option != 0) cout << "Invalid choice.\n";
    }
}

void responderRegistrationApprovalMenu(ResponderRepository& responders, ResponderManager& responderManager)
{
    int option = -1;
    while (option != 0)
    {
        cout << "\n====== RESPONDER REGISTRATION APPROVAL ======\n";
        cout << "1. View Pending Registrations\n";
        cout << "2. Approve Registration\n";
        cout << "3. Reject Registration\n";
        cout << "0. Back to User Management\nChoice: ";
        if (!(cin >> option)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (option == 1) responders.displayPendingResponderRegistrations();
        else if (option == 2 || option == 3)
        {
            int registrationId;
            cout << "Enter registration ID: ";
            if (!(cin >> registrationId)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid registration ID.\n"; continue; }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            bool ok = option == 2 ? responders.approveResponderRegistration(registrationId)
                                  : responders.rejectResponderRegistration(registrationId);
            if (ok)
            {
                if (option == 2) responders.loadActiveResponders(responderManager);
                cout << (option == 2 ? "Registration approved. Responder profile created and linked; login is now enabled.\n" : "Registration rejected. Login will remain disabled.\n");
            }
            else cout << "Operation failed. Check the registration ID and its current status.\n";
        }
        else if (option != 0) cout << "Invalid choice.\n";
    }
}

void userManagementMenu(UserRepository& repository, ResponderRepository& responderRepository, ResponderManager& responderManager, const User& currentUser)
{
    int option = -1;
    while (option != 0)
    {
        cout << "\n========== ADMIN USER MANAGEMENT ==========\n";
        cout << "1. View All Users\n";
        cout << "2. Create User Account\n";
        cout << "3. Deactivate User Account\n";
        cout << "4. Reactivate Deactivated Account\n";
        cout << "5. Responder Registration Approval\n";
        cout << "0. Back to Main Menu\nChoice: ";
        if (!(cin >> option)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (option == 1) repository.displayAllUsers();
        else if (option == 2)
        {
            string username, fullName, email, contact, password, confirm, roleText;
            UserRole role;
            cout << "Full name: "; getline(cin, fullName);
            cout << "Username: "; getline(cin, username);
            cout << "Email (optional): "; getline(cin, email);
            cout << "Contact number (optional): "; getline(cin, contact);
            cout << "Role (ADMIN / COORDINATOR / RESPONDER): "; getline(cin, roleText);
            if (fullName.empty() || username.empty() || !stringToUserRole(roleText, role) || repository.userExists(username)) { cout << "Invalid details, role, or username already exists.\n"; continue; }
            cout << "Password (minimum 10 characters): "; password = Authentication::promptPassword();
            cout << "Confirm password: "; confirm = Authentication::promptPassword();
            if (password.length() < 10 || password != confirm) { cout << "Password too short or confirmation does not match.\n"; continue; }
            string hash = PasswordHasher::hashPassword(password);
            if (hash.empty()) { cout << "Password hashing failed.\n"; continue; }
            User newUser(0, username, hash, fullName, email, contact, role);
            if (repository.saveUser(newUser)) cout << "User account created successfully.\n";
            else cout << "Could not create user account. Check database constraints.\n";
        }
        else if (option == 3 || option == 4)
        {
            int targetId;
            cout << (option == 3 ? "Enter user ID to deactivate: " : "Enter user ID to reactivate: ");
            if (!(cin >> targetId)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid user ID.\n"; continue; }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (option == 3)
            {
                if (repository.deactivateUserSafely(targetId, currentUser.getUserId())) cout << "User account deactivated.\n";
                else cout << "Deactivation denied or failed. You cannot deactivate yourself or the last active Admin.\n";
            }
            else
            {
                if (repository.reactivateUserSafely(targetId)) { responderRepository.activateResponderForUser(targetId); cout << "User account reactivated successfully.\n"; }
                else cout << "Reactivation failed. Check the user ID; the account may already be active or unavailable.\n";
            }
        }
        else if (option == 5) responderRegistrationApprovalMenu(responderRepository, responderManager);
        else if (option != 0) cout << "Invalid choice.\n";
    }
}

void responderSelfRegistration(ResponderRepository& responders)
{
    cout << "\n========== RESPONDER SELF-REGISTRATION ==========\n";
    cout << "Your application will remain pending until an Admin approves it.\n";

    string fullName, username, email, contact, password, confirm;
    string responderRole, specialization, city, area, baseAddress;

    cout << "Full name: ";
    getline(cin, fullName);
    cout << "Username: ";
    getline(cin, username);
    cout << "Email (optional): ";
    getline(cin, email);
    cout << "Contact number: ";
    getline(cin, contact);
    cout << "Responder role (e.g. Rescue / Medical): ";
    getline(cin, responderRole);
    cout << "Specialization: ";
    getline(cin, specialization);
    cout << "City: ";
    getline(cin, city);
    cout << "Area / Locality: ";
    getline(cin, area);
    cout << "Full Base Address: ";
    getline(cin, baseAddress);

    if (fullName.empty() || username.empty() || contact.empty() ||
        responderRole.empty() || specialization.empty() ||
        city.empty() || area.empty() || baseAddress.empty())
    {
        cout << "Required details cannot be empty. Registration cancelled.\n";
        return;
    }

    cout << "Create password (minimum 10 characters): ";
    password = Authentication::promptPassword();
    cout << "Confirm password: ";
    confirm = Authentication::promptPassword();

    if (password.length() < 10 || password != confirm)
    {
        cout << "Password too short or confirmation does not match.\n";
        return;
    }

    string hash = PasswordHasher::hashPassword(password);

    if (hash.empty())
    {
        cout << "Password hashing failed.\n";
        return;
    }

    if (!responders.submitResponderRegistration(
        username, hash, fullName, email, contact,
        responderRole, specialization, city, area, baseAddress))
    {
        cout << "Could not submit registration. Username may already exist or the database rejected the details.\n";
        return;
    }

    cout << "\nRegistration submitted successfully!\n";
    cout << "Status: PENDING ADMIN APPROVAL\n";
    cout << "You can log in after an Admin approves your application.\n";
}

// ============================================================================
//  START-UP: load every table into the in-memory structures
// ============================================================================
static bool loadApplicationData()
{
    cout << "\nLoading application data from database...\n";
    bool allLoaded = true;
    auto report = [&](bool ok, const char* what)
    {
        if (ok) cout << "[OK] " << what << " loaded.\n";
        else { cout << "[ERROR] Could not load " << what << ".\n"; allLoaded = false; }
    };

    // order matters: inventory needs resources + warehouses, allocations need emergencies, ...
    report(S.resourceRepo->loadActiveResources(*S.resources), "Resources");
    report(S.warehouseRepo->loadActiveWarehouses(*S.warehouses), "Warehouses");
    report(S.inventoryRepo->loadInventory(*S.inventory), "Inventory");
    report(S.emergencyRepo->loadEmergencies(*S.emergencies), "Emergencies");
    report(S.allocationRepo->loadAllocations(*S.allocations), "Allocations");
    report(S.responderRepo->loadActiveResponders(*S.responders), "Responders");
    report(S.transferRepo->loadTransfers(*S.transfers), "Warehouse transfers");
    report(S.locationRepo->loadLocations(*S.graph), "Locations");
    report(S.locationRepo->loadConnections(*S.graph), "Location connections");
    report(S.activityRepo->loadActivityHistory(*S.history), "Activity history");

    S.emergencies->ensureNextIdAtLeast(S.emergencyRepo->getMaxEmergencyId() + 1);
    return allLoaded;
}

static string envOrDefault(const char* name, const char* fallback)
{
    const char* value = getenv(name);
    return (value != nullptr && value[0] != '\0') ? string(value) : string(fallback);
}

int main()
{
    cout << "\nStarting ResQflow...\n";

    // Credentials come from the environment only; nothing is hard-coded in the source.
    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "\nERROR: Database password environment variable not found.\n";
        cout << "Set RESQFLOW_DB_PASSWORD before starting ResQflow.\n";
        return 1;
    }

    string host = envOrDefault("RESQFLOW_DB_HOST", "localhost");
    string user = envOrDefault("RESQFLOW_DB_USER", "root");
    string name = envOrDefault("RESQFLOW_DB_NAME", "resqflow");
    unsigned int port = static_cast<unsigned int>(atoi(envOrDefault("RESQFLOW_DB_PORT", "3306").c_str()));

    DatabaseConnection database;

    if (!database.connect(host.c_str(), user.c_str(), password, name.c_str(), port))
    {
        cout << "\nDatabase connection failed.\n";
        cout << "Error: " << database.getLastError() << "\n";
        return 1;
    }

    cout << "\nDatabase connected successfully.\n";

    ResourceLinkedList resources;
    WarehouseLinkedList warehouses;
    InventoryManager inventory(resources, warehouses);

    EmergencyMaxHeap emergencyHeap;
    EmergencyManager emergencies;
    emergencies.connectPriorityHeap(emergencyHeap);

    AllocationManager allocations(inventory, emergencies, resources);
    ActivityHistory history;
    WarehouseTransferManager transfers(inventory, warehouses, &history);
    ResponderManager responders;
    LocationGraph locationGraph;
    RoutePlanner routePlanner(&locationGraph);

    UserRepository userRepository(database);
    ResourceRepository resourceRepository(database);
    WarehouseRepository warehouseRepository(database);
    InventoryRepository inventoryRepository(database);
    EmergencyRepository emergencyRepository(database);
    AllocationRepository allocationRepository(database);
    ResponderRepository responderRepository(database);
    WarehouseTransferRepository transferRepository(database);
    ActivityHistoryRepository activityRepository(database);
    LocationRepository locationRepository(database);

    S.db = &database;
    S.resources = &resources; S.warehouses = &warehouses; S.inventory = &inventory;
    S.emergencies = &emergencies; S.allocations = &allocations; S.responders = &responders;
    S.transfers = &transfers; S.history = &history; S.graph = &locationGraph; S.planner = &routePlanner;
    S.resourceRepo = &resourceRepository; S.warehouseRepo = &warehouseRepository;
    S.inventoryRepo = &inventoryRepository; S.emergencyRepo = &emergencyRepository;
    S.allocationRepo = &allocationRepository; S.responderRepo = &responderRepository;
    S.transferRepo = &transferRepository; S.activityRepo = &activityRepository;
    S.locationRepo = &locationRepository;

    if (!loadApplicationData())
    {
        // Running on partially loaded data would create duplicate IDs and inconsistent stock.
        cout << "\nResQflow cannot start safely because some data could not be loaded.\n"
                "Check the database schema (run database/ResQflow_Database_Schema.sql, or\n"
                "database/Migration_001_Phase3_Alignment.sql for an older database) and try again.\n";
        database.disconnect();
        return 1;
    }

    cout << "\nDatabase loading process completed.\n";

    User loggedInUser;

    while (true)
    {
        cout << "\n========== RESQFLOW ENTRY =========="
             << "\n1. Login\n2. Register as Responder\n0. Exit\nChoice: ";
        int entryChoice = -1;
        if (!(cin >> entryChoice))
        {
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid choice.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (entryChoice == 0) break;
        if (entryChoice == 2) { responderSelfRegistration(responderRepository); continue; }
        if (entryChoice != 1) { cout << "Invalid choice.\n"; continue; }

        if (!Authentication::authenticate(userRepository, loggedInUser))
        {
            cout << "\nAuthentication failed. Returning to the entry screen.\n";
            continue;
        }

        S.userId = loggedInUser.getUserId();
        S.actor = loggedInUser.getFullName().empty() ? loggedInUser.getUsername() : loggedInUser.getFullName();

        if (loggedInUser.getRole() == UserRole::RESPONDER)
        {
            int responderId = responderRepository.findResponderIdForUser(loggedInUser.getUserId());
            if (responderId <= 0)
                cout << "\nThis Responder login is not linked to a responder profile. Contact an Admin.\n";
            else
                responderDashboard(responderId, loggedInUser);
            cout << "\nReturning to the login screen...\n";
            S.userId = 0; S.actor = "SYSTEM";
            continue;
        }

        int choice = -1;

        do
        {
            displayMainMenu(loggedInUser.getRole());

            if (!(cin >> choice))
            {
                if (cin.eof()) { choice = 0; break; }
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "\nInvalid input. Enter a number.\n";
                choice = -1;
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            // Role-based access control: enforced here, not just hidden in the menu text.
            if (!isMenuOptionAllowed(loggedInUser.getRole(), choice))
            {
                cout << "\nACCESS DENIED: You do not have permission to access this module.\n";
                continue;
            }

            switch (choice)
            {
                case 1: displayResourceMenu(); break;
                case 2: displayWarehouseMenu(); break;
                case 3: displayInventoryMenu(); break;
                case 4: displayEmergencyMenu(); break;
                case 5: displayAllocationMenu(); break;
                case 6: displayResponderMenu(); break;
                case 7: displayTransferMenu(); break;
                case 8: displayLocationMenu(); break;
                case 9: displayActivityMenu(); break;
                case 10: userManagementMenu(userRepository, responderRepository, responders, loggedInUser); break;
                case 0: cout << "\nLogging out of ResQflow...\n"; break;
                default: cout << "\nInvalid choice. Please try again.\n";
            }
        } while (choice != 0);

        S.userId = 0; S.actor = "SYSTEM";
        cout << "\nLogged out successfully. Returning to the login screen...\n";
    }

    database.disconnect();
    cout << "\nResQflow closed successfully.\n";
    return 0;
}
