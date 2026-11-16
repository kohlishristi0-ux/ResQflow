#include "../include/OperationsModule.h"
#include <iostream>
#include <new>
#include <limits>
#include <cstring>
#include <cstdlib>

using namespace std;

Warehouse::Warehouse() : warehouseId(0), warehouseName(""), address(""), contactNumber(""), active(true)
{
}

Warehouse::Warehouse(int id, string name, string warehouseAddress, string contact)
    : warehouseId(id), warehouseName(name), address(warehouseAddress), contactNumber(contact), active(true)
{
}

Warehouse::Warehouse(int id, string name, string warehouseAddress, double, double, string contact)
    : Warehouse(id, name, warehouseAddress, contact)
{
}

int Warehouse::getWarehouseId() const
{
    return warehouseId;
}
string Warehouse::getWarehouseName() const
{
    return warehouseName;
}
string Warehouse::getAddress() const
{
    return address;
}
string Warehouse::getContactNumber() const
{
    return contactNumber;
}
bool Warehouse::isActive() const
{
    return active;
}

void Warehouse::setWarehouseName(string name)
{
    warehouseName = name;
}
void Warehouse::setAddress(string warehouseAddress)
{
    address = warehouseAddress;
}
void Warehouse::setContactNumber(string contact)
{
    contactNumber = contact;
}
void Warehouse::setStatus(bool status)
{
    active = status;
}

void Warehouse::displayWarehouse() const
{
    cout << "\n-----------------------------";
    cout << "\nWarehouse ID: " << warehouseId;
    cout << "\nWarehouse Name: " << warehouseName;
    cout << "\nFull Address: " << address;
    cout << "\nContact Number: " << contactNumber;
    cout << "\nStatus: " << (active ? "Active" : "Inactive");
    cout << "\n-----------------------------\n";
}

WarehouseHashTable::WarehouseHashTable(int initialCapacity)
{
    if (initialCapacity < 1)
        initialCapacity = 10;

    capacity = initialCapacity;
    count = 0;
    loadFactor = 0.0;
    table = new Entry*[capacity]();
}

WarehouseHashTable::~WarehouseHashTable()
{
    clear();
    delete[] table;
}

int WarehouseHashTable::hashFunction(int warehouseId) const
{
    return warehouseId % capacity;
}

bool WarehouseHashTable::resizeTable()
{
    if (capacity > (numeric_limits<int>::max() - 1) / 2)
        return false;

    int newCapacity = capacity * 2 + 1;

    Entry** newTable = new (nothrow) Entry*[newCapacity]();

    if (newTable == nullptr)
        return false;

    for (int i = 0; i < capacity; i++)
    {
        Entry* current = table[i];

        while (current != nullptr)
        {
            Entry* nextNode = current->next;
            int newIndex = current->warehouseId % newCapacity;

            current->next = newTable[newIndex];
            newTable[newIndex] = current;

            current = nextNode;
        }
    }

    delete[] table;
    table = newTable;
    capacity = newCapacity;
    loadFactor = static_cast<double>(count) / capacity;

    return true;
}

bool WarehouseHashTable::insert(int warehouseId, Warehouse* warehouse)
{
    if (warehouseId <= 0 || warehouse == nullptr)
        return false;

    if (warehouse->getWarehouseId() != warehouseId)
        return false;

    if (search(warehouseId) != nullptr)
        return false;

    if (static_cast<double>(count + 1) / capacity > 0.75)
    {
        if (!resizeTable())
            return false;
    }

    int index = hashFunction(warehouseId);

    Entry* newEntry = new (nothrow) Entry(warehouseId, warehouse);

    if (newEntry == nullptr)
        return false;

    newEntry->next = table[index];
    table[index] = newEntry;

    count++;
    loadFactor = static_cast<double>(count) / capacity;

    return true;
}

Warehouse* WarehouseHashTable::search(int warehouseId) const
{
    if (warehouseId <= 0)
        return nullptr;

    int index = hashFunction(warehouseId);
    Entry* current = table[index];

    while (current != nullptr)
    {
        if (current->warehouseId == warehouseId)
            return current->warehouse;

        current = current->next;
    }

    return nullptr;
}

bool WarehouseHashTable::remove(int warehouseId)
{
    if (warehouseId <= 0)
        return false;

    int index = hashFunction(warehouseId);
    Entry* current = table[index];
    Entry* previous = nullptr;

    while (current != nullptr)
    {
        if (current->warehouseId == warehouseId)
        {
            if (previous == nullptr)
                table[index] = current->next;
            else
                previous->next = current->next;

            delete current;
            count--;
            loadFactor = static_cast<double>(count) / capacity;

            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

bool WarehouseHashTable::contains(int warehouseId) const
{
    return search(warehouseId) != nullptr;
}

int WarehouseHashTable::getCount() const
{
    return count;
}

int WarehouseHashTable::getCapacity() const
{
    return capacity;
}

double WarehouseHashTable::getLoadFactor() const
{
    return loadFactor;
}

void WarehouseHashTable::displayTable() const
{
    cout << "\n========== WAREHOUSE HASH TABLE ==========\n";

    for (int i = 0; i < capacity; i++)
    {
        cout << "Index " << i << ": ";

        Entry* current = table[i];

        if (current == nullptr)
        {
            cout << "EMPTY";
        }
        else
        {
            while (current != nullptr)
            {
                cout << "[" << current->warehouseId << "]";

                if (current->next != nullptr)
                    cout << " -> ";

                current = current->next;
            }
        }

        cout << endl;
    }

    cout << "\nTotal Entries: " << count;
    cout << "\nCapacity: " << capacity;
    cout << "\nLoad Factor: " << loadFactor;
    cout << "\n==========================================\n";
}

void WarehouseHashTable::clear()
{
    for (int i = 0; i < capacity; i++)
    {
        Entry* current = table[i];

        while (current != nullptr)
        {
            Entry* temp = current;
            current = current->next;
            delete temp;
        }

        table[i] = nullptr;
    }

    count = 0;
    loadFactor = 0.0;
}

WarehouseLinkedList::WarehouseLinkedList() : head(nullptr), tail(nullptr), count(0)
{
}

WarehouseLinkedList::~WarehouseLinkedList()
{
    clear();
}

bool WarehouseLinkedList::isEmpty() const
{
    return head == nullptr;
}
bool WarehouseLinkedList::isDuplicate(int id) const
{
    return warehouseIndex.contains(id);
}
int WarehouseLinkedList::getCount() const
{
    return count;
}

bool WarehouseLinkedList::addWarehouse(const Warehouse& warehouse)
{
    int id = warehouse.getWarehouseId();
    if (id <= 0 || isDuplicate(id) || !warehouse.isActive())
        return false;

    Node* node = new Node(warehouse);
    if (!warehouseIndex.insert(id, &node->data))
    {
        delete node;
        return false;
    }

    if (!head)
        head = tail = node;
    else
    {
        tail->next = node;
        tail = node;
    }
    ++count;
    return true;
}

bool WarehouseLinkedList::deleteWarehouse(int id)
{
    if (id <= 0)
        return false;
    Node* current = head;
    Node* previous = nullptr;
    while (current)
    {
        if (current->data.getWarehouseId() == id)
        {
            if (!warehouseIndex.remove(id))
                return false;
            if (!previous)
                head = current->next;
            else
                previous->next = current->next;
            if (current == tail)
                tail = previous;
            delete current;
            --count;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

Warehouse* WarehouseLinkedList::searchWarehouse(int id)
{
    return warehouseIndex.search(id);
}
const Warehouse* WarehouseLinkedList::searchWarehouse(int id) const
{
    return warehouseIndex.search(id);
}

bool WarehouseLinkedList::updateWarehouse(int id, string name, string address, string contact)
{
    Warehouse* warehouse = searchWarehouse(id);
    if (!warehouse || name.empty() || address.empty())
        return false;
    warehouse->setWarehouseName(name);
    warehouse->setAddress(address);
    warehouse->setContactNumber(contact);
    return true;
}

bool WarehouseLinkedList::updateWarehouse(int id, string name, string address, double, double, string contact)
{
    return updateWarehouse(id, name, address, contact);
}

void WarehouseLinkedList::displayAllWarehouses() const
{
    cout << "\n========== WAREHOUSE LIST ==========";
    if (isEmpty())
    {
        cout << "\nNo warehouses available.\n";
        return;
    }
    Node* current = head;
    while (current)
    {
        current->data.displayWarehouse();
        current = current->next;
    }
    cout << "\nTotal Warehouses: " << count << "\n====================================\n";
}

void WarehouseLinkedList::clear()
{
    while (head)
        deleteWarehouse(head->data.getWarehouseId());
}

WarehouseRepository::WarehouseRepository(DatabaseConnection& db) : database(db)
{
}

bool WarehouseRepository::saveWarehouse(const Warehouse& warehouse)
{
    if (!database.isConnected() || warehouse.getWarehouseId() <= 0 || warehouse.getWarehouseName().empty() ||
        warehouse.getAddress().empty() || !warehouse.isActive() ||
        warehouseExists(warehouse.getWarehouseId()))
        return false;

    const char* query = "INSERT INTO warehouses "
                        "(warehouse_id, warehouse_name, address, latitude, longitude, contact_number) "
                        "VALUES (?, ?, ?, 0, 0, ?)";

    MYSQL_STMT* stmt = database.prepareStatement(query);
    if (!stmt)
        return false;

    int id = warehouse.getWarehouseId();
    const std::string& name = warehouse.getWarehouseName();
    const std::string& address = warehouse.getAddress();
    const std::string& contact = warehouse.getContactNumber();
    unsigned long nameLength = name.length(), addressLength = address.length(),
                  contactLength = contact.length();

    MYSQL_BIND bind[4] = {};
    bindInt(bind[0], id);
    bindString(bind[1], name, nameLength);
    bindString(bind[2], address, addressLength);
    bindString(bind[3], contact, contactLength);

    bool ok = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;
    mysql_stmt_close(stmt);
    return ok;
}

bool WarehouseRepository::loadActiveWarehouses(WarehouseLinkedList& warehouseList)
{
    if (!database.isConnected())
        return false;
    const char* query = "SELECT warehouse_id, warehouse_name, address, contact_number "
                        "FROM warehouses WHERE is_active = TRUE ORDER BY warehouse_id";
    if (mysql_query(database.getConnection(), query) != 0)
        return false;
    MYSQL_RES* result = mysql_store_result(database.getConnection());
    if (!result)
        return false;

    warehouseList.clear();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int id = std::stoi(row[0]);
        std::string name = row[1] ? row[1] : "";
        std::string address = row[2] ? row[2] : "";
        std::string contact = row[3] ? row[3] : "";
        Warehouse warehouse(id, name, address, contact);
        if (!warehouseList.addWarehouse(warehouse))
        {
            warehouseList.clear();
            mysql_free_result(result);
            return false;
        }
    }
    mysql_free_result(result);
    return true;
}

bool WarehouseRepository::warehouseExists(int warehouseId)
{
    if (!database.isConnected() || warehouseId <= 0)
        return false;
    const char* query = "SELECT warehouse_id FROM warehouses WHERE warehouse_id = ?";
    MYSQL_STMT* stmt = database.prepareStatement(query);
    if (!stmt)
        return false;
    MYSQL_BIND param[1] = {};
    bindInt(param[0], warehouseId);
    bool exists = false;
    if (mysql_stmt_bind_param(stmt, param) == 0 && mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_store_result(stmt) == 0)
        exists = mysql_stmt_num_rows(stmt) > 0;
    mysql_stmt_close(stmt);
    return exists;
}

bool WarehouseRepository::updateWarehouse(const Warehouse& warehouse)
{
    if (!database.isConnected() || warehouse.getWarehouseId() <= 0 || warehouse.getWarehouseName().empty() ||
        warehouse.getAddress().empty() || !warehouseExists(warehouse.getWarehouseId()))
        return false;

    const char* query = "UPDATE warehouses SET warehouse_name = ?, address = ?, contact_number = ? "
                        "WHERE warehouse_id = ?";
    MYSQL_STMT* stmt = database.prepareStatement(query);
    if (!stmt)
        return false;

    const std::string& name = warehouse.getWarehouseName();
    const std::string& address = warehouse.getAddress();
    const std::string& contact = warehouse.getContactNumber();
    unsigned long nameLength = name.length(), addressLength = address.length(),
                  contactLength = contact.length();
    int id = warehouse.getWarehouseId();

    MYSQL_BIND bind[4] = {};
    bindString(bind[0], name, nameLength);
    bindString(bind[1], address, addressLength);
    bindString(bind[2], contact, contactLength);
    bindInt(bind[3], id);

    bool ok = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;
    mysql_stmt_close(stmt);
    return ok;
}

bool WarehouseRepository::deactivateWarehouse(int warehouseId)
{
    if (!database.isConnected() || warehouseId <= 0)
        return false;
    const char* query = "UPDATE warehouses SET is_active = FALSE WHERE warehouse_id = ?";
    MYSQL_STMT* stmt = database.prepareStatement(query);
    if (!stmt)
        return false;
    MYSQL_BIND bind[1] = {};
    bindInt(bind[0], warehouseId);
    bool ok = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0 &&
              mysql_stmt_affected_rows(stmt) == 1;
    mysql_stmt_close(stmt);
    return ok;
}

Inventory::Inventory()
{
    inventoryId = 0;
    resourceId = 0;
    warehouseId = 0;
    availableQuantity = 0;
    reservedQuantity = 0;
    minimumStockLevel = 0;
}

Inventory::Inventory(int inventoryId, int resourceId, int warehouseId, int availableQuantity,
                     int minimumStockLevel)
    : Inventory(inventoryId, resourceId, warehouseId, availableQuantity, 0, minimumStockLevel)
{
}

Inventory::Inventory(int inventoryId, int resourceId, int warehouseId, int availableQuantity,
                     int reservedQuantity, int minimumStockLevel)
{
    this->inventoryId = inventoryId;
    this->resourceId = resourceId;
    this->warehouseId = warehouseId;
    this->availableQuantity = availableQuantity >= 0 ? availableQuantity : 0;
    this->reservedQuantity = reservedQuantity >= 0 ? reservedQuantity : 0;
    this->minimumStockLevel = minimumStockLevel >= 0 ? minimumStockLevel : 0;
}

int Inventory::getInventoryId() const
{
    return inventoryId;
}
int Inventory::getResourceId() const
{
    return resourceId;
}
int Inventory::getWarehouseId() const
{
    return warehouseId;
}
int Inventory::getAvailableQuantity() const
{
    return availableQuantity;
}
int Inventory::getReservedQuantity() const
{
    return reservedQuantity;
}
int Inventory::getMinimumStockLevel() const
{
    return minimumStockLevel;
}

int Inventory::getTotalStock() const
{
    return availableQuantity + reservedQuantity;
}

bool Inventory::addStock(int quantity)
{
    if (quantity <= 0 || quantity > numeric_limits<int>::max() - getTotalStock())
        return false;
    availableQuantity += quantity;
    return true;
}

bool Inventory::removeStock(int quantity)
{
    if (quantity <= 0 || quantity > availableQuantity)
        return false;
    availableQuantity -= quantity;
    return true;
}

bool Inventory::reserveStock(int quantity)
{
    if (quantity <= 0 || quantity > availableQuantity)
        return false;
    availableQuantity -= quantity;
    reservedQuantity += quantity;
    return true;
}

bool Inventory::releaseReservedStock(int quantity)
{
    if (quantity <= 0 || quantity > reservedQuantity)
        return false;
    reservedQuantity -= quantity;
    availableQuantity += quantity;
    return true;
}

bool Inventory::dispatchStock(int quantity)
{
    if (quantity <= 0 || quantity > reservedQuantity)
        return false;
    reservedQuantity -= quantity;
    return true;
}

bool Inventory::returnStock(int quantity)
{
    return addStock(quantity);
}

void Inventory::setMinimumStockLevel(int level)
{
    if (level >= 0)
        minimumStockLevel = level;
}

bool Inventory::isLowStock() const
{
    return availableQuantity <= minimumStockLevel;
}

void Inventory::displayInventory() const
{
    cout << "\n--------------------------------";
    cout << "\nInventory ID: " << inventoryId;
    cout << "\nResource ID: " << resourceId;
    cout << "\nWarehouse ID: " << warehouseId;
    cout << "\nAvailable Quantity: " << availableQuantity;
    cout << "\nReserved Quantity: " << reservedQuantity;
    cout << "\nTotal Physical Stock: " << getTotalStock();
    cout << "\nMinimum Stock Level: " << minimumStockLevel;
    cout << "\nStock Status: " << (isLowStock() ? "Low Stock" : "Sufficient Stock");
    cout << "\n--------------------------------\n";
}

InventoryManager::InventoryManager(ResourceLinkedList& resources, WarehouseLinkedList& warehouses)
{
    head = nullptr;
    count = 0;
    nextInventoryId = 1;
    resourceList = &resources;
    warehouseList = &warehouses;
}

InventoryManager::~InventoryManager()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }
}

bool InventoryManager::isEmpty() const
{
    return head == nullptr;
}

bool InventoryManager::isWarehouseActive(int warehouseId) const
{
    const Warehouse* warehouse = warehouseList->searchWarehouse(warehouseId);
    return warehouse != nullptr && warehouse->isActive();
}

int InventoryManager::getCount() const
{
    return count;
}

bool InventoryManager::addInventory(int resourceId, int warehouseId, int quantity, int minimumStockLevel)
{
    Resource* resource = resourceList->searchResource(resourceId);
    Warehouse* warehouse = warehouseList->searchWarehouse(warehouseId);
    if (resource == nullptr || warehouse == nullptr || !resource->isActive() || !warehouse->isActive())
        return false;
    if (resourceId <= 0 || warehouseId <= 0 || quantity < 0 || minimumStockLevel < 0 ||
        nextInventoryId <= 0 || nextInventoryId == numeric_limits<int>::max() ||
        count == numeric_limits<int>::max())
        return false;
    if (findInventory(resourceId, warehouseId) != nullptr)
        return false;

    Inventory inventory(nextInventoryId, resourceId, warehouseId, quantity, minimumStockLevel);
    Node* newNode = new (nothrow) Node(inventory);
    if (newNode == nullptr)
        return false;

    if (head == nullptr)
        head = newNode;
    else
    {
        Node* current = head;
        while (current->next != nullptr)
            current = current->next;
        current->next = newNode;
    }
    ++count;
    ++nextInventoryId;
    return true;
}

bool InventoryManager::loadInventoryRecord(int inventoryId, int resourceId, int warehouseId,
                                           int availableQuantity, int reservedQuantity, int minimumStockLevel)
{
    Resource* resource = resourceList->searchResource(resourceId);
    Warehouse* warehouse = warehouseList->searchWarehouse(warehouseId);

    if (inventoryId <= 0 || inventoryId == numeric_limits<int>::max() || resourceId <= 0 ||
        warehouseId <= 0 || availableQuantity < 0 || reservedQuantity < 0 || minimumStockLevel < 0 ||
        reservedQuantity > numeric_limits<int>::max() - availableQuantity || resource == nullptr ||
        warehouse == nullptr || !resource->isActive() || !warehouse->isActive() ||
        searchInventory(inventoryId) != nullptr || findInventory(resourceId, warehouseId) != nullptr ||
        count == numeric_limits<int>::max())
    {
        return false;
    }

    Inventory inventory(inventoryId, resourceId, warehouseId, availableQuantity, reservedQuantity,
                        minimumStockLevel);

    Node* newNode = new (nothrow) Node(inventory);

    if (newNode == nullptr)
        return false;

    if (head == nullptr)
        head = newNode;
    else
    {
        Node* current = head;

        while (current->next != nullptr)
            current = current->next;

        current->next = newNode;
    }

    ++count;

    if (inventoryId >= nextInventoryId)
        nextInventoryId = inventoryId + 1;

    return true;
}

bool InventoryManager::deleteInventory(int inventoryId)
{
    if (inventoryId <= 0 || head == nullptr)
        return false;
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getInventoryId() != inventoryId)
    {
        previous = current;
        current = current->next;
    }
    if (current == nullptr || current->data.getTotalStock() != 0)
        return false;
    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    delete current;
    --count;
    return true;
}

bool InventoryManager::discardInventory(int inventoryId)
{
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getInventoryId() != inventoryId)
    {
        previous = current;
        current = current->next;
    }
    if (current == nullptr)
        return false;
    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    delete current;
    --count;
    return true;
}

bool InventoryManager::hasStockForResource(int resourceId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getResourceId() == resourceId && current->data.getTotalStock() > 0)
            return true;
    return false;
}

bool InventoryManager::hasStockInWarehouse(int warehouseId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getWarehouseId() == warehouseId && current->data.getTotalStock() > 0)
            return true;
    return false;
}

int InventoryManager::collectByResource(int resourceId, Inventory** out, int capacity)
{
    int found = 0;
    for (Node* current = head; current != nullptr && found < capacity; current = current->next)
        if (current->data.getResourceId() == resourceId)
            out[found++] = &current->data;
    return found;
}

Inventory* InventoryManager::searchInventory(int inventoryId)
{
    if (inventoryId <= 0)
        return nullptr;
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getInventoryId() == inventoryId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

Inventory* InventoryManager::findInventory(int resourceId, int warehouseId)
{
    if (resourceId <= 0 || warehouseId <= 0)
        return nullptr;
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getResourceId() == resourceId && current->data.getWarehouseId() == warehouseId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

bool InventoryManager::addStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->addStock(quantity);
}
bool InventoryManager::removeStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->removeStock(quantity);
}
bool InventoryManager::reserveStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->reserveStock(quantity);
}
bool InventoryManager::releaseReservedStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->releaseReservedStock(quantity);
}
bool InventoryManager::dispatchStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->dispatchStock(quantity);
}
bool InventoryManager::returnStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->returnStock(quantity);
}

bool InventoryManager::transferStock(int sourceWarehouseId, int destinationWarehouseId, int resourceId,
                                     int quantity)
{
    if (quantity <= 0 || sourceWarehouseId <= 0 || destinationWarehouseId <= 0 || resourceId <= 0 ||
        sourceWarehouseId == destinationWarehouseId)
        return false;
    if (!isWarehouseActive(sourceWarehouseId) || !isWarehouseActive(destinationWarehouseId))
        return false;
    Inventory* source = findInventory(resourceId, sourceWarehouseId);
    Inventory* destination = findInventory(resourceId, destinationWarehouseId);
    if (source == nullptr || destination == nullptr || quantity > source->getAvailableQuantity())
        return false;
    if (quantity > numeric_limits<int>::max() - destination->getTotalStock())
        return false;

    if (!source->removeStock(quantity))
        return false;
    if (!destination->addStock(quantity))
    {
        source->addStock(quantity);
        return false;
    }
    return true;
}

void InventoryManager::displayAllInventory() const
{
    if (isEmpty())
    {
        cout << "\nNo inventory records available.\n";
        return;
    }
    Node* current = head;
    while (current != nullptr)
    {
        current->data.displayInventory();
        current = current->next;
    }
    cout << "\nTotal Inventory Records: " << count << endl;
}

void InventoryManager::displayInventoryByWarehouse(int warehouseId) const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getWarehouseId() == warehouseId)
        {
            current->data.displayInventory();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo inventory found for this warehouse.\n";
}

void InventoryManager::displayInventoryByResource(int resourceId) const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getResourceId() == resourceId)
        {
            current->data.displayInventory();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo inventory found for this resource.\n";
}

void InventoryManager::displayLowStock() const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.isLowStock())
        {
            current->data.displayInventory();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo low-stock records found.\n";
}

void InventoryManager::clearInventory()
{
    while (head != nullptr)
    {
        Node* temp = head;
        head = head->next;
        delete temp;
    }

    count = 0;
    nextInventoryId = 1;
}

InventoryRepository::InventoryRepository(DatabaseConnection& db) : database(db)
{
}

bool InventoryRepository::saveInventory(const Inventory& inventory)
{
    if (!database.isConnected() || inventory.getInventoryId() <= 0 || inventory.getResourceId() <= 0 ||
        inventory.getWarehouseId() <= 0 || inventory.getAvailableQuantity() < 0 ||
        inventory.getReservedQuantity() < 0 || inventory.getMinimumStockLevel() < 0)
    {
        return false;
    }

    const char* query = "INSERT INTO inventory "
                        "(inventory_id, resource_id, warehouse_id, available_quantity, "
                        "reserved_quantity, minimum_stock_level) "
                        "VALUES (?, ?, ?, ?, ?, ?)";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int inventoryId = inventory.getInventoryId();
    int resourceId = inventory.getResourceId();
    int warehouseId = inventory.getWarehouseId();
    int available = inventory.getAvailableQuantity();
    int reserved = inventory.getReservedQuantity();
    int minimum = inventory.getMinimumStockLevel();

    MYSQL_BIND params[6];
    memset(params, 0, sizeof(params));

    int* values[6] = {&inventoryId, &resourceId, &warehouseId, &available, &reserved, &minimum};

    for (int i = 0; i < 6; ++i)
    {
        params[i].buffer_type = MYSQL_TYPE_LONG;
        params[i].buffer = values[i];
    }

    bool success = mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool InventoryRepository::loadInventory(InventoryManager& inventoryManager)
{
    if (!database.isConnected())
        return false;

    const char* query = "SELECT i.inventory_id, i.resource_id, i.warehouse_id, "
                        "i.available_quantity, i.reserved_quantity, i.minimum_stock_level "
                        "FROM inventory i "
                        "JOIN resources r ON r.resource_id = i.resource_id AND r.is_active = TRUE "
                        "JOIN warehouses w ON w.warehouse_id = i.warehouse_id AND w.is_active = TRUE "
                        "ORDER BY i.inventory_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    inventoryManager.clearInventory();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        if (row[0] == nullptr || row[1] == nullptr || row[2] == nullptr || row[3] == nullptr ||
            row[4] == nullptr || row[5] == nullptr)
        {
            success = false;
            break;
        }

        int inventoryId = atoi(row[0]);
        int resourceId = atoi(row[1]);
        int warehouseId = atoi(row[2]);
        int available = atoi(row[3]);
        int reserved = atoi(row[4]);
        int minimum = atoi(row[5]);

        if (!inventoryManager.loadInventoryRecord(inventoryId, resourceId, warehouseId, available, reserved,
                                                  minimum))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        inventoryManager.clearInventory();

    return success;
}

bool InventoryRepository::updateInventory(const Inventory& inventory)
{
    if (!database.isConnected() || inventory.getInventoryId() <= 0 || inventory.getAvailableQuantity() < 0 ||
        inventory.getReservedQuantity() < 0 || inventory.getMinimumStockLevel() < 0)
    {
        return false;
    }

    const char* query = "UPDATE inventory SET available_quantity = ?, "
                        "reserved_quantity = ?, minimum_stock_level = ? "
                        "WHERE inventory_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int available = inventory.getAvailableQuantity();
    int reserved = inventory.getReservedQuantity();
    int minimum = inventory.getMinimumStockLevel();
    int inventoryId = inventory.getInventoryId();

    MYSQL_BIND params[4];
    memset(params, 0, sizeof(params));

    int* values[4] = {&available, &reserved, &minimum, &inventoryId};

    for (int i = 0; i < 4; ++i)
    {
        params[i].buffer_type = MYSQL_TYPE_LONG;
        params[i].buffer = values[i];
    }

    bool success = mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool InventoryRepository::deleteInventory(int inventoryId)
{
    if (!database.isConnected() || inventoryId <= 0)
        return false;

    const char* query = "DELETE FROM inventory "
                        "WHERE inventory_id = ? "
                        "AND available_quantity = 0 "
                        "AND reserved_quantity = 0";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND param;
    memset(&param, 0, sizeof(param));

    bindInt(param, inventoryId);

    bool success = mysql_stmt_bind_param(stmt, &param) == 0 && mysql_stmt_execute(stmt) == 0 &&
                   mysql_stmt_affected_rows(stmt) == 1;

    mysql_stmt_close(stmt);
    return success;
}

bool InventoryRepository::inventoryExists(int inventoryId)
{
    if (!database.isConnected() || inventoryId <= 0)
        return false;

    const char* query = "SELECT inventory_id FROM inventory WHERE inventory_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND input;
    memset(&input, 0, sizeof(input));

    bindInt(input, inventoryId);

    bool exists = false;
    int returnedId = 0;

    MYSQL_BIND output;
    memset(&output, 0, sizeof(output));

    bindInt(output, returnedId);

    if (mysql_stmt_bind_param(stmt, &input) == 0 && mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_bind_result(stmt, &output) == 0 && mysql_stmt_store_result(stmt) == 0)
    {
        exists = mysql_stmt_fetch(stmt) == 0;
    }

    mysql_stmt_close(stmt);
    return exists;
}

bool InventoryRepository::recordMovement(int resourceId, int warehouseId, const char* transactionType,
                                         int quantity, const char* referenceType, long long referenceId,
                                         int performedBy, const char* notes)
{
    if (!database.isConnected() || resourceId <= 0 || warehouseId <= 0 || transactionType == nullptr ||
        quantity <= 0)
        return false;

    MYSQL_STMT* stmt = database.prepareStatement("INSERT INTO resource_transactions "
                                                 "(resource_id, warehouse_id, transaction_type, quantity, "
                                                 "reference_type, reference_id, performed_by, notes) "
                                                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    if (stmt == nullptr)
        return false;

    std::string type = transactionType;
    std::string refType = referenceType ? referenceType : "";
    std::string note = notes ? notes : "";
    unsigned long typeLength = static_cast<unsigned long>(type.length());
    unsigned long refTypeLength = static_cast<unsigned long>(refType.length());
    unsigned long noteLength = static_cast<unsigned long>(note.length());

    DbBool refTypeNull = refType.empty() ? 1 : 0;
    DbBool refIdNull = referenceId <= 0 ? 1 : 0;
    DbBool userNull = performedBy <= 0 ? 1 : 0;
    DbBool noteNull = note.empty() ? 1 : 0;

    MYSQL_BIND params[8];
    memset(params, 0, sizeof(params));

    bindInt(params[0], resourceId);
    bindInt(params[1], warehouseId);
    bindString(params[2], type, typeLength);
    bindInt(params[3], quantity);
    bindString(params[4], refType, refTypeLength);
    params[4].is_null = &refTypeNull;
    bindLongLong(params[5], referenceId);
    params[5].is_null = &refIdNull;
    bindInt(params[6], performedBy);
    params[6].is_null = &userNull;
    bindString(params[7], note, noteLength);
    params[7].is_null = &noteNull;

    bool success = mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

Allocation::Allocation()
{
    allocationId = 0;
    emergencyId = 0;
    resourceId = 0;
    warehouseId = 0;
    requestedQuantity = 0;
    allocatedQuantity = 0;
    dispatchedQuantity = 0;
    returnedQuantity = 0;
    status = AllocationStatus::PENDING;
}

Allocation::Allocation(int allocationId, int emergencyId, int resourceId, int warehouseId,
                       int requestedQuantity)
{
    this->allocationId = allocationId;
    this->emergencyId = emergencyId;
    this->resourceId = resourceId;
    this->warehouseId = warehouseId;
    this->requestedQuantity = requestedQuantity > 0 ? requestedQuantity : 0;
    allocatedQuantity = 0;
    dispatchedQuantity = 0;
    returnedQuantity = 0;
    status = AllocationStatus::PENDING;
}

Allocation::Allocation(int allocationId, int emergencyId, int resourceId, int warehouseId,
                       int requestedQuantity, int allocatedQuantity, int dispatchedQuantity,
                       int returnedQuantity, AllocationStatus status)
{
    this->allocationId = allocationId;
    this->emergencyId = emergencyId;
    this->resourceId = resourceId;
    this->warehouseId = warehouseId;
    this->requestedQuantity = requestedQuantity;
    this->allocatedQuantity = allocatedQuantity;
    this->dispatchedQuantity = dispatchedQuantity;
    this->returnedQuantity = returnedQuantity;
    this->status = status;
}

int Allocation::getAllocationId() const
{
    return allocationId;
}

int Allocation::getEmergencyId() const
{
    return emergencyId;
}

int Allocation::getResourceId() const
{
    return resourceId;
}

int Allocation::getWarehouseId() const
{
    return warehouseId;
}

int Allocation::getRequestedQuantity() const
{
    return requestedQuantity;
}

int Allocation::getAllocatedQuantity() const
{
    return allocatedQuantity;
}

int Allocation::getDispatchedQuantity() const
{
    return dispatchedQuantity;
}

int Allocation::getReturnedQuantity() const
{
    return returnedQuantity;
}

int Allocation::getNetDispatchedQuantity() const
{
    return dispatchedQuantity - returnedQuantity;
}

int Allocation::getOutstandingQuantity() const
{
    return requestedQuantity - allocatedQuantity;
}

AllocationStatus Allocation::getStatus() const
{
    return status;
}

bool Allocation::addAllocatedQuantity(int quantity)
{
    if (quantity <= 0 || quantity > getOutstandingQuantity())
        return false;

    if (status == AllocationStatus::CANCELLED || status == AllocationStatus::COMPLETED)
        return false;

    allocatedQuantity += quantity;

    if (allocatedQuantity == requestedQuantity)
        status = AllocationStatus::ALLOCATED;
    else
        status = AllocationStatus::PARTIALLY_ALLOCATED;

    return true;
}

bool Allocation::dispatchQuantity(int quantity)
{
    if (quantity <= 0 || quantity > allocatedQuantity - dispatchedQuantity)
        return false;

    if (status == AllocationStatus::CANCELLED || status == AllocationStatus::COMPLETED)
        return false;

    dispatchedQuantity += quantity;
    status = AllocationStatus::DISPATCHED;

    return true;
}

bool Allocation::returnQuantity(int quantity)
{
    if (quantity <= 0 || quantity > dispatchedQuantity - returnedQuantity)
        return false;
    if (status == AllocationStatus::CANCELLED)
        return false;
    returnedQuantity += quantity;
    return true;
}

bool Allocation::completeAllocation()
{
    if (status == AllocationStatus::CANCELLED || dispatchedQuantity != requestedQuantity)
        return false;

    status = AllocationStatus::COMPLETED;
    return true;
}

bool Allocation::cancelAllocation()
{
    if (status == AllocationStatus::COMPLETED || status == AllocationStatus::CANCELLED)
        return false;

    status = AllocationStatus::CANCELLED;
    return true;
}

void Allocation::displayAllocation() const
{
    cout << "\n--------------------------------";
    cout << "\nAllocation ID: " << allocationId;
    cout << "\nEmergency ID: " << emergencyId;
    cout << "\nResource ID: " << resourceId;
    cout << "\nWarehouse ID: " << warehouseId;
    cout << "\nRequested Quantity: " << requestedQuantity;
    cout << "\nAllocated Quantity: " << allocatedQuantity;
    cout << "\nDispatched Quantity: " << dispatchedQuantity;
    cout << "\nReturned Quantity: " << returnedQuantity;
    cout << "\nNet Dispatched Quantity: " << getNetDispatchedQuantity();
    cout << "\nOutstanding Quantity: " << getOutstandingQuantity();

    cout << "\nStatus: ";

    switch (status)
    {
        case AllocationStatus::PENDING:
            cout << "Pending";
            break;

        case AllocationStatus::PARTIALLY_ALLOCATED:
            cout << "Partially Allocated";
            break;

        case AllocationStatus::ALLOCATED:
            cout << "Allocated";
            break;

        case AllocationStatus::DISPATCHED:
            cout << "Dispatched";
            break;

        case AllocationStatus::COMPLETED:
            cout << "Completed";
            break;

        case AllocationStatus::CANCELLED:
            cout << "Cancelled";
            break;
    }

    cout << "\n--------------------------------\n";
}

AllocationManager::AllocationManager(InventoryManager& inventory, EmergencyManager& emergency,
                                     ResourceLinkedList& resources)
{
    head = nullptr;
    count = 0;
    nextAllocationId = 1;
    inventoryManager = &inventory;
    emergencyManager = &emergency;
    resourceList = &resources;
}

AllocationManager::~AllocationManager()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }
}

bool AllocationManager::isEmpty() const
{
    return head == nullptr;
}
int AllocationManager::getCount() const
{
    return count;
}

bool AllocationManager::createAllocation(int emergencyId, int resourceId, int warehouseId,
                                         int requestedQuantity)
{
    if (requestedQuantity <= 0 || nextAllocationId <= 0 || nextAllocationId == numeric_limits<int>::max())
        return false;
    Emergency* emergency = emergencyManager->searchEmergency(emergencyId);
    Resource* resource = resourceList->searchResource(resourceId);
    Inventory* inventory = inventoryManager->findInventory(resourceId, warehouseId);
    if (emergency == nullptr || resource == nullptr || inventory == nullptr || !resource->isActive() ||
        !inventoryManager->isWarehouseActive(warehouseId) ||
        emergency->getStatus() == EmergencyStatus::RESOLVED ||
        emergency->getStatus() == EmergencyStatus::CANCELLED)
        return false;

    int quantityToAllocate = inventory->getAvailableQuantity() < requestedQuantity
                                     ? inventory->getAvailableQuantity()
                                     : requestedQuantity;
    Allocation allocation(nextAllocationId, emergencyId, resourceId, warehouseId, requestedQuantity);
    if (quantityToAllocate > 0 && !allocation.addAllocatedQuantity(quantityToAllocate))
        return false;

    Node* newNode = new (nothrow) Node(allocation);
    if (newNode == nullptr)
        return false;
    if (quantityToAllocate > 0 &&
        !inventoryManager->reserveStock(inventory->getInventoryId(), quantityToAllocate))
    {
        delete newNode;
        return false;
    }
    if (head == nullptr)
        head = newNode;
    else
    {
        Node* current = head;
        while (current->next != nullptr)
            current = current->next;
        current->next = newNode;
    }
    count++;
    nextAllocationId++;
    return true;
}

int AllocationManager::peekNextAllocationId() const
{
    return nextAllocationId;
}

Allocation* AllocationManager::findActiveAllocationForEmergency(int emergencyId)
{
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == emergencyId &&
            current->data.getStatus() != AllocationStatus::CANCELLED &&
            current->data.getStatus() != AllocationStatus::COMPLETED)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

bool AllocationManager::removeAllocation(int allocationId)
{
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getAllocationId() != allocationId)
    {
        previous = current;
        current = current->next;
    }
    if (current == nullptr)
        return false;

    int reserved = current->data.getAllocatedQuantity() - current->data.getDispatchedQuantity();
    if (reserved > 0)
    {
        Inventory* inventory = inventoryManager->findInventory(current->data.getResourceId(),
                                                               current->data.getWarehouseId());
        if (inventory != nullptr)
            inventoryManager->releaseReservedStock(inventory->getInventoryId(), reserved);
    }

    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    delete current;
    --count;
    return true;
}

bool AllocationManager::hasActiveAllocationForResource(int resourceId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getResourceId() == resourceId &&
            current->data.getStatus() != AllocationStatus::CANCELLED &&
            current->data.getStatus() != AllocationStatus::COMPLETED)
            return true;
    return false;
}

bool AllocationManager::hasActiveAllocationForWarehouse(int warehouseId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getWarehouseId() == warehouseId &&
            current->data.getStatus() != AllocationStatus::CANCELLED &&
            current->data.getStatus() != AllocationStatus::COMPLETED)
            return true;
    return false;
}

Allocation* AllocationManager::searchAllocation(int allocationId)
{
    if (allocationId <= 0)
        return nullptr;
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getAllocationId() == allocationId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

bool AllocationManager::allocateAdditionalStock(int allocationId, int quantity)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || quantity <= 0 || allocation->getStatus() == AllocationStatus::CANCELLED ||
        allocation->getStatus() == AllocationStatus::COMPLETED ||
        quantity > allocation->getOutstandingQuantity())
        return false;
    Emergency* emergency = emergencyManager->searchEmergency(allocation->getEmergencyId());
    Resource* resource = resourceList->searchResource(allocation->getResourceId());
    if (emergency == nullptr || resource == nullptr || !resource->isActive() ||
        !inventoryManager->isWarehouseActive(allocation->getWarehouseId()) ||
        emergency->getStatus() == EmergencyStatus::RESOLVED ||
        emergency->getStatus() == EmergencyStatus::CANCELLED)
        return false;
    Inventory* inventory =
            inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
    if (inventory == nullptr || quantity > inventory->getAvailableQuantity())
        return false;
    if (!inventoryManager->reserveStock(inventory->getInventoryId(), quantity))
        return false;
    if (!allocation->addAllocatedQuantity(quantity))
    {
        inventoryManager->releaseReservedStock(inventory->getInventoryId(), quantity);
        return false;
    }
    return true;
}

bool AllocationManager::dispatchAllocation(int allocationId, int quantity)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || quantity <= 0 || allocation->getStatus() == AllocationStatus::CANCELLED ||
        allocation->getStatus() == AllocationStatus::COMPLETED ||
        quantity > allocation->getAllocatedQuantity() - allocation->getDispatchedQuantity())
        return false;
    Emergency* emergency = emergencyManager->searchEmergency(allocation->getEmergencyId());
    if (emergency == nullptr || emergency->getStatus() == EmergencyStatus::CANCELLED ||
        emergency->getStatus() == EmergencyStatus::RESOLVED ||
        !inventoryManager->isWarehouseActive(allocation->getWarehouseId()))
        return false;
    Inventory* inventory =
            inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
    if (inventory == nullptr || quantity > inventory->getReservedQuantity())
        return false;
    if (quantity > allocation->getAllocatedQuantity() - allocation->getDispatchedQuantity())
        return false;
    if (!inventoryManager->dispatchStock(inventory->getInventoryId(), quantity))
        return false;
    if (!allocation->dispatchQuantity(quantity))
        return false;
    return true;
}

bool AllocationManager::returnDispatchedStock(int allocationId, int quantity)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || quantity <= 0 || allocation->getStatus() == AllocationStatus::CANCELLED ||
        quantity > allocation->getDispatchedQuantity() - allocation->getReturnedQuantity())
        return false;
    Inventory* inventory =
            inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
    if (inventory == nullptr)
        return false;
    if (!inventoryManager->returnStock(inventory->getInventoryId(), quantity))
        return false;
    if (!allocation->returnQuantity(quantity))
        return false;
    return true;
}

bool AllocationManager::completeAllocation(int allocationId)
{
    Allocation* allocation = searchAllocation(allocationId);
    return allocation != nullptr && allocation->completeAllocation();
}

bool AllocationManager::cancelAllocation(int allocationId)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || allocation->getStatus() == AllocationStatus::CANCELLED ||
        allocation->getStatus() == AllocationStatus::COMPLETED)
        return false;
    int remainingReserved = allocation->getAllocatedQuantity() - allocation->getDispatchedQuantity();
    Inventory* inventory = nullptr;
    if (remainingReserved > 0)
    {
        inventory =
                inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
        if (inventory == nullptr || remainingReserved > inventory->getReservedQuantity())
            return false;
    }
    if (remainingReserved > 0 &&
        !inventoryManager->releaseReservedStock(inventory->getInventoryId(), remainingReserved))
        return false;
    return allocation->cancelAllocation();
}

void AllocationManager::displayAllAllocations() const
{
    if (isEmpty())
    {
        cout << "\nNo allocation records available.\n";
        return;
    }
    Node* current = head;
    while (current != nullptr)
    {
        current->data.displayAllocation();
        current = current->next;
    }
    cout << "\nTotal Allocations: " << count << endl;
}

void AllocationManager::displayAllocationsByEmergency(int emergencyId) const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == emergencyId)
        {
            current->data.displayAllocation();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo allocations found for this emergency.\n";
}

void AllocationManager::displayAllocationsByStatus(AllocationStatus status) const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getStatus() == status)
        {
            current->data.displayAllocation();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo allocations found with this status.\n";
}

void AllocationManager::clearAllocations()
{
    Node* current = head;

    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    count = 0;
    nextAllocationId = 1;
}

bool AllocationManager::loadAllocationRecord(int allocationId, int emergencyId, int resourceId,
                                             int warehouseId, int requestedQuantity, int allocatedQuantity,
                                             int dispatchedQuantity, int returnedQuantity,
                                             AllocationStatus status)
{
    if (allocationId <= 0 || emergencyId <= 0 || resourceId <= 0 || warehouseId <= 0 ||
        requestedQuantity <= 0 || allocatedQuantity < 0 || allocatedQuantity > requestedQuantity ||
        dispatchedQuantity < 0 || dispatchedQuantity > allocatedQuantity || returnedQuantity < 0 ||
        returnedQuantity > dispatchedQuantity || count == numeric_limits<int>::max())
        return false;

    if (status != AllocationStatus::PENDING && status != AllocationStatus::PARTIALLY_ALLOCATED &&
        status != AllocationStatus::ALLOCATED && status != AllocationStatus::DISPATCHED &&
        status != AllocationStatus::COMPLETED && status != AllocationStatus::CANCELLED)
        return false;

    Node* current = head;

    while (current != nullptr)
    {
        if (current->data.getAllocationId() == allocationId)
            return false;

        current = current->next;
    }

    Allocation allocation(allocationId, emergencyId, resourceId, warehouseId, requestedQuantity,
                          allocatedQuantity, dispatchedQuantity, returnedQuantity, status);

    Node* newNode = new (nothrow) Node(allocation);

    if (newNode == nullptr)
        return false;

    if (head == nullptr)
        head = newNode;
    else
    {
        current = head;

        while (current->next != nullptr)
            current = current->next;

        current->next = newNode;
    }

    count++;

    if (allocationId >= nextAllocationId)
    {
        if (allocationId == numeric_limits<int>::max())
            nextAllocationId = 0;
        else
            nextAllocationId = allocationId + 1;
    }

    return true;
}

AllocationRepository::AllocationRepository(DatabaseConnection& db) : database(db)
{
}

bool AllocationRepository::saveAllocation(const Allocation& allocation)
{
    if (!database.isConnected() || allocation.getAllocationId() <= 0 || allocation.getEmergencyId() <= 0 ||
        allocation.getResourceId() <= 0 || allocation.getWarehouseId() <= 0 ||
        allocation.getRequestedQuantity() <= 0 || allocation.getAllocatedQuantity() < 0 ||
        allocation.getAllocatedQuantity() > allocation.getRequestedQuantity() ||
        allocation.getDispatchedQuantity() < 0 ||
        allocation.getDispatchedQuantity() > allocation.getAllocatedQuantity() ||
        allocation.getReturnedQuantity() < 0 ||
        allocation.getReturnedQuantity() > allocation.getDispatchedQuantity())
        return false;

    const char* query = "INSERT INTO allocations "
                        "(allocation_id, emergency_id, resource_id, warehouse_id, "
                        "requested_quantity, allocated_quantity, dispatched_quantity, "
                        "returned_quantity, status) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int id = allocation.getAllocationId();
    int emergency = allocation.getEmergencyId();
    int resource = allocation.getResourceId();
    int warehouse = allocation.getWarehouseId();
    int requested = allocation.getRequestedQuantity();
    int allocated = allocation.getAllocatedQuantity();
    int dispatched = allocation.getDispatchedQuantity();
    int returned = allocation.getReturnedQuantity();

    const char* status = nullptr;

    switch (allocation.getStatus())
    {
        case AllocationStatus::PENDING:
            status = "PENDING";
            break;
        case AllocationStatus::PARTIALLY_ALLOCATED:
            status = "PARTIALLY_ALLOCATED";
            break;
        case AllocationStatus::ALLOCATED:
            status = "ALLOCATED";
            break;
        case AllocationStatus::DISPATCHED:
            status = "DISPATCHED";
            break;
        case AllocationStatus::COMPLETED:
            status = "COMPLETED";
            break;
        case AllocationStatus::CANCELLED:
            status = "CANCELLED";
            break;
    }

    if (status == nullptr)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    unsigned long statusLength = static_cast<unsigned long>(strlen(status));

    MYSQL_BIND params[9];
    memset(params, 0, sizeof(params));

    int* values[8] = {&id, &emergency, &resource, &warehouse, &requested, &allocated, &dispatched, &returned};

    for (int i = 0; i < 8; i++)
    {
        params[i].buffer_type = MYSQL_TYPE_LONG;
        params[i].buffer = values[i];
    }

    params[8].buffer_type = MYSQL_TYPE_STRING;
    params[8].buffer = const_cast<char*>(status);
    params[8].buffer_length = statusLength;
    params[8].length = &statusLength;

    bool success = mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool AllocationRepository::loadAllocations(AllocationManager& allocationManager)
{
    if (!database.isConnected())
        return false;

    const char* query = "SELECT allocation_id, emergency_id, resource_id, warehouse_id, "
                        "requested_quantity, allocated_quantity, dispatched_quantity, "
                        "returned_quantity, status "
                        "FROM allocations ORDER BY allocation_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    allocationManager.clearAllocations();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        for (int i = 0; i < 9; i++)
        {
            if (row[i] == nullptr)
            {
                success = false;
                break;
            }
        }

        if (!success)
            break;

        AllocationStatus status;

        if (strcmp(row[8], "PENDING") == 0)
            status = AllocationStatus::PENDING;
        else if (strcmp(row[8], "PARTIALLY_ALLOCATED") == 0)
            status = AllocationStatus::PARTIALLY_ALLOCATED;
        else if (strcmp(row[8], "ALLOCATED") == 0)
            status = AllocationStatus::ALLOCATED;
        else if (strcmp(row[8], "DISPATCHED") == 0)
            status = AllocationStatus::DISPATCHED;
        else if (strcmp(row[8], "COMPLETED") == 0)
            status = AllocationStatus::COMPLETED;
        else if (strcmp(row[8], "CANCELLED") == 0)
            status = AllocationStatus::CANCELLED;
        else
        {
            success = false;
            break;
        }

        if (!allocationManager.loadAllocationRecord(atoi(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]),
                                                    atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]),
                                                    status))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        allocationManager.clearAllocations();

    return success;
}

bool AllocationRepository::updateAllocation(const Allocation& allocation)
{
    if (!database.isConnected() || allocation.getAllocationId() <= 0 ||
        allocation.getAllocatedQuantity() < 0 ||
        allocation.getAllocatedQuantity() > allocation.getRequestedQuantity() ||
        allocation.getDispatchedQuantity() < 0 ||
        allocation.getDispatchedQuantity() > allocation.getAllocatedQuantity() ||
        allocation.getReturnedQuantity() < 0 ||
        allocation.getReturnedQuantity() > allocation.getDispatchedQuantity())
        return false;

    if (!allocationExists(allocation.getAllocationId()))
        return false;

    const char* query = "UPDATE allocations SET allocated_quantity = ?, "
                        "dispatched_quantity = ?, returned_quantity = ?, status = ? "
                        "WHERE allocation_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int allocated = allocation.getAllocatedQuantity();
    int dispatched = allocation.getDispatchedQuantity();
    int returned = allocation.getReturnedQuantity();
    int id = allocation.getAllocationId();

    const char* status = nullptr;

    switch (allocation.getStatus())
    {
        case AllocationStatus::PENDING:
            status = "PENDING";
            break;
        case AllocationStatus::PARTIALLY_ALLOCATED:
            status = "PARTIALLY_ALLOCATED";
            break;
        case AllocationStatus::ALLOCATED:
            status = "ALLOCATED";
            break;
        case AllocationStatus::DISPATCHED:
            status = "DISPATCHED";
            break;
        case AllocationStatus::COMPLETED:
            status = "COMPLETED";
            break;
        case AllocationStatus::CANCELLED:
            status = "CANCELLED";
            break;
    }

    if (status == nullptr)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    unsigned long statusLength = static_cast<unsigned long>(strlen(status));

    MYSQL_BIND params[5];
    memset(params, 0, sizeof(params));

    bindInt(params[0], allocated);

    bindInt(params[1], dispatched);

    bindInt(params[2], returned);

    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = const_cast<char*>(status);
    params[3].buffer_length = statusLength;
    params[3].length = &statusLength;

    bindInt(params[4], id);

    bool success = mysql_stmt_bind_param(stmt, params) == 0 && mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool AllocationRepository::allocationExists(int allocationId)
{
    if (!database.isConnected() || allocationId <= 0)
        return false;

    const char* query = "SELECT allocation_id FROM allocations WHERE allocation_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND input;
    memset(&input, 0, sizeof(input));

    bindInt(input, allocationId);

    int returnedId = 0;

    MYSQL_BIND output;
    memset(&output, 0, sizeof(output));

    bindInt(output, returnedId);

    bool exists = false;

    if (mysql_stmt_bind_param(stmt, &input) == 0 && mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_bind_result(stmt, &output) == 0 && mysql_stmt_store_result(stmt) == 0)
    {
        exists = mysql_stmt_fetch(stmt) == 0;
    }

    mysql_stmt_close(stmt);
    return exists;
}

bool AllocationRepository::syncRequirement(int emergencyId, int resourceId)
{
    if (!database.isConnected() || emergencyId <= 0 || resourceId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    MYSQL_BIND parameters[2];
    memset(parameters, 0, sizeof(parameters));
    bindInt(parameters[0], emergencyId);
    bindInt(parameters[1], resourceId);

    MYSQL_STMT* deleteStatement = database.prepareStatement(
            "DELETE FROM emergency_requirements WHERE emergency_id = ? AND resource_id = ?");

    bool ok = deleteStatement != nullptr && mysql_stmt_bind_param(deleteStatement, parameters) == 0 &&
              mysql_stmt_execute(deleteStatement) == 0;

    if (deleteStatement)
        mysql_stmt_close(deleteStatement);

    if (!ok)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND insertParameters[2];
    memset(insertParameters, 0, sizeof(insertParameters));
    bindInt(insertParameters[0], emergencyId);
    bindInt(insertParameters[1], resourceId);

    MYSQL_STMT* insertStatement = database.prepareStatement(
            "INSERT INTO emergency_requirements "
            "(emergency_id, resource_id, required_quantity, fulfilled_quantity) "
            "SELECT emergency_id, resource_id, SUM(requested_quantity), SUM(allocated_quantity) "
            "FROM allocations WHERE emergency_id = ? AND resource_id = ? AND status <> 'CANCELLED' "
            "GROUP BY emergency_id, resource_id");

    ok = insertStatement != nullptr && mysql_stmt_bind_param(insertStatement, insertParameters) == 0 &&
         mysql_stmt_execute(insertStatement) == 0;

    if (insertStatement)
        mysql_stmt_close(insertStatement);

    if (!ok)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool AllocationRepository::displayRequirements(int emergencyId)
{
    if (!database.isConnected() || emergencyId <= 0)
        return false;

    MYSQL* connection = database.getConnection();
    std::string query = "SELECT r.resource_id, res.resource_name, r.required_quantity, r.fulfilled_quantity "
                        "FROM emergency_requirements r JOIN resources res ON res.resource_id = r.resource_id "
                        "WHERE r.emergency_id = " +
                        std::to_string(emergencyId) + " ORDER BY r.resource_id";

    if (mysql_query(connection, query.c_str()) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    std::cout << "\n===== RESOURCE REQUIREMENTS FOR EMERGENCY " << emergencyId << " =====\n";

    MYSQL_ROW row;
    int rows = 0;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        std::cout << "Resource " << row[0] << " (" << row[1] << "): required " << row[2] << ", fulfilled "
                  << row[3] << '\n';
        ++rows;
    }

    if (rows == 0)
        std::cout << "No requirements recorded.\n";

    mysql_free_result(result);
    return true;
}

WarehouseTransfer::WarehouseTransfer()
    : transferId(0), sourceWarehouseId(0), destinationWarehouseId(0), resourceId(0), quantity(0),
      status(TransferStatus::REQUESTED), requestedAt(0), completedAt(0)
{
}

WarehouseTransfer::WarehouseTransfer(int id, int sourceId, int destinationId, int resource,
                                     int transferQuantity, const string& actor, const string& transferRemarks,
                                     time_t requestTime)
    : transferId(id), sourceWarehouseId(sourceId), destinationWarehouseId(destinationId),
      resourceId(resource), quantity(transferQuantity), status(TransferStatus::REQUESTED), requestedBy(actor),
      remarks(transferRemarks), requestedAt(requestTime), completedAt(0)
{
}

int WarehouseTransfer::getTransferId() const
{
    return transferId;
}
int WarehouseTransfer::getSourceWarehouseId() const
{
    return sourceWarehouseId;
}
int WarehouseTransfer::getDestinationWarehouseId() const
{
    return destinationWarehouseId;
}
int WarehouseTransfer::getResourceId() const
{
    return resourceId;
}
int WarehouseTransfer::getQuantity() const
{
    return quantity;
}
TransferStatus WarehouseTransfer::getStatus() const
{
    return status;
}
string WarehouseTransfer::getRequestedBy() const
{
    return requestedBy;
}
string WarehouseTransfer::getRemarks() const
{
    return remarks;
}
time_t WarehouseTransfer::getRequestedAt() const
{
    return requestedAt;
}
time_t WarehouseTransfer::getCompletedAt() const
{
    return completedAt;
}

void WarehouseTransfer::markCompleted(time_t completionTime)
{
    if (status == TransferStatus::REQUESTED && completionTime >= 0)
    {
        status = TransferStatus::COMPLETED;
        completedAt = completionTime;
    }
}

void WarehouseTransfer::markCancelled()
{
    if (status == TransferStatus::REQUESTED)
        status = TransferStatus::CANCELLED;
}

bool WarehouseTransfer::restoreStatus(TransferStatus restoredStatus, time_t completionTime)
{
    if (restoredStatus == TransferStatus::REQUESTED)
    {
        status = restoredStatus;
        completedAt = 0;
        return true;
    }

    if (restoredStatus == TransferStatus::COMPLETED)
    {
        if (completionTime <= 0)
            return false;

        status = restoredStatus;
        completedAt = completionTime;
        return true;
    }

    if (restoredStatus == TransferStatus::CANCELLED)
    {
        status = restoredStatus;
        completedAt = 0;
        return true;
    }

    return false;
}

const char* transferStatusToString(TransferStatus transferStatus)
{
    switch (transferStatus)
    {
        case TransferStatus::REQUESTED:
            return "REQUESTED";
        case TransferStatus::COMPLETED:
            return "COMPLETED";
        case TransferStatus::CANCELLED:
            return "CANCELLED";
    }
    return "UNKNOWN";
}

void WarehouseTransfer::display() const
{
    cout << "\n----------------------------------------\n";
    cout << "Transfer ID: " << transferId << '\n';
    cout << "Source Warehouse ID: " << sourceWarehouseId << '\n';
    cout << "Destination Warehouse ID: " << destinationWarehouseId << '\n';
    cout << "Resource ID: " << resourceId << '\n';
    cout << "Quantity: " << quantity << '\n';
    cout << "Status: " << transferStatusToString(status) << '\n';
    cout << "Requested By: " << requestedBy << '\n';
    cout << "Remarks: " << remarks << '\n';
    cout << "Requested At: " << requestedAt << '\n';
    cout << "Completed At: " << completedAt << '\n';
    cout << "----------------------------------------\n";
}

WarehouseTransferManager::Node::Node(const WarehouseTransfer& transfer) : data(transfer), next(nullptr)
{
}

WarehouseTransferManager::WarehouseTransferManager(InventoryManager& inventory,
                                                   WarehouseLinkedList& warehouses, ActivityHistory* history)
    : head(nullptr), tail(nullptr), count(0), nextTransferId(1), inventoryManager(&inventory),
      warehouseList(&warehouses), activityHistory(history)
{
}

WarehouseTransferManager::~WarehouseTransferManager()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* nextNode = current->next;
        delete current;
        current = nextNode;
    }
}

bool WarehouseTransferManager::isEmpty() const
{
    return head == nullptr;
}

int WarehouseTransferManager::getCount() const
{
    return count;
}

void WarehouseTransferManager::connectActivityHistory(ActivityHistory* history)
{
    activityHistory = history;
}

bool WarehouseTransferManager::areWarehousesValid(int sourceId, int destinationId) const
{
    if (sourceId <= 0 || destinationId <= 0 || sourceId == destinationId)
        return false;

    const Warehouse* source = warehouseList->searchWarehouse(sourceId);
    const Warehouse* destination = warehouseList->searchWarehouse(destinationId);

    return source != nullptr && destination != nullptr && source->isActive() && destination->isActive();
}

bool WarehouseTransferManager::createTransfer(int sourceWarehouseId, int destinationWarehouseId,
                                              int resourceId, int quantity, const string& requestedBy,
                                              const string& remarks)
{
    if (!areWarehousesValid(sourceWarehouseId, destinationWarehouseId) || resourceId <= 0 || quantity <= 0 ||
        requestedBy.empty() || count == numeric_limits<int>::max() || nextTransferId <= 0 ||
        nextTransferId == numeric_limits<int>::max())
    {
        return false;
    }

    const Inventory* sourceInventory = inventoryManager->findInventory(resourceId, sourceWarehouseId);
    const Inventory* destinationInventory =
            inventoryManager->findInventory(resourceId, destinationWarehouseId);

    if (sourceInventory == nullptr || destinationInventory == nullptr ||
        quantity > sourceInventory->getAvailableQuantity())
    {
        return false;
    }

    WarehouseTransfer transfer(nextTransferId, sourceWarehouseId, destinationWarehouseId, resourceId,
                               quantity, requestedBy, remarks, time(nullptr));
    Node* newNode = new (nothrow) Node(transfer);
    if (newNode == nullptr)
        return false;

    if (tail == nullptr)
        head = newNode;
    else
        tail->next = newNode;

    tail = newNode;
    ++count;
    ++nextTransferId;
    return true;
}

int WarehouseTransferManager::peekNextTransferId() const
{
    return nextTransferId;
}

bool WarehouseTransferManager::removeTransfer(int transferId)
{
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getTransferId() != transferId)
    {
        previous = current;
        current = current->next;
    }
    if (current == nullptr)
        return false;
    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    if (current == tail)
        tail = previous;
    delete current;
    --count;
    return true;
}

bool WarehouseTransferManager::hasPendingTransferForWarehouse(int warehouseId) const
{
    for (const Node* current = head; current != nullptr; current = current->next)
        if (current->data.getStatus() == TransferStatus::REQUESTED &&
            (current->data.getSourceWarehouseId() == warehouseId ||
             current->data.getDestinationWarehouseId() == warehouseId))
            return true;
    return false;
}

bool WarehouseTransferManager::executeTransfer(int transferId)
{
    WarehouseTransfer* transfer = searchTransfer(transferId);
    if (transfer == nullptr || transfer->getStatus() != TransferStatus::REQUESTED)
        return false;

    if (!areWarehousesValid(transfer->getSourceWarehouseId(), transfer->getDestinationWarehouseId()))
    {
        return false;
    }

    if (!inventoryManager->transferStock(transfer->getSourceWarehouseId(),
                                         transfer->getDestinationWarehouseId(), transfer->getResourceId(),
                                         transfer->getQuantity()))
    {
        return false;
    }

    transfer->markCompleted(time(nullptr));

    if (activityHistory != nullptr)
    {
        activityHistory->addActivity(ActivityType::WAREHOUSE_TRANSFERRED,
                                     "Warehouse stock transfer completed. Transfer ID: " +
                                             to_string(transferId),
                                     transfer->getRequestedBy(), -1, -1, transfer->getResourceId(),
                                     transfer->getSourceWarehouseId(), transfer->getDestinationWarehouseId(),
                                     -1, transfer->getQuantity());
    }

    return true;
}

bool WarehouseTransferManager::cancelTransfer(int transferId)
{
    WarehouseTransfer* transfer = searchTransfer(transferId);
    if (transfer == nullptr || transfer->getStatus() != TransferStatus::REQUESTED)
        return false;

    transfer->markCancelled();
    return transfer->getStatus() == TransferStatus::CANCELLED;
}

WarehouseTransfer* WarehouseTransferManager::searchTransfer(int transferId)
{
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getTransferId() == transferId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

const WarehouseTransfer* WarehouseTransferManager::searchTransfer(int transferId) const
{
    const Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getTransferId() == transferId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

void WarehouseTransferManager::displayAllTransfers() const
{
    if (isEmpty())
    {
        cout << "No warehouse transfers recorded.\n";
        return;
    }

    const Node* current = head;
    while (current != nullptr)
    {
        current->data.display();
        current = current->next;
    }
}

void WarehouseTransferManager::displayTransfersByWarehouse(int warehouseId) const
{
    const Node* current = head;
    bool found = false;

    while (current != nullptr)
    {
        if (current->data.getSourceWarehouseId() == warehouseId ||
            current->data.getDestinationWarehouseId() == warehouseId)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }

    if (!found)
        cout << "No transfers found for this warehouse.\n";
}

void WarehouseTransferManager::displayTransfersByStatus(TransferStatus status) const
{
    const Node* current = head;
    bool found = false;

    while (current != nullptr)
    {
        if (current->data.getStatus() == status)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }

    if (!found)
        cout << "No transfers found for this status.\n";
}

void WarehouseTransferManager::clearTransfers()
{
    Node* current = head;

    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    tail = nullptr;
    count = 0;
    nextTransferId = 1;
}

bool WarehouseTransferManager::loadTransferRecord(int transferId, int sourceWarehouseId,
                                                  int destinationWarehouseId, int resourceId, int quantity,
                                                  const string& requestedBy, const string& remarks,
                                                  time_t requestedAt, TransferStatus status,
                                                  time_t completedAt)
{
    if (transferId <= 0 || sourceWarehouseId <= 0 || destinationWarehouseId <= 0 ||
        sourceWarehouseId == destinationWarehouseId || resourceId <= 0 || quantity <= 0 ||
        requestedBy.empty() || requestedAt <= 0 || count == numeric_limits<int>::max())
        return false;

    if (searchTransfer(transferId) != nullptr)
        return false;

    WarehouseTransfer transfer(transferId, sourceWarehouseId, destinationWarehouseId, resourceId, quantity,
                               requestedBy, remarks, requestedAt);

    if (!transfer.restoreStatus(status, completedAt))
        return false;

    Node* node = new (nothrow) Node(transfer);

    if (!node)
        return false;

    if (!head)
        head = node;
    else
        tail->next = node;

    tail = node;
    ++count;

    if (transferId >= nextTransferId)
    {
        if (transferId == numeric_limits<int>::max())
            nextTransferId = 0;
        else
            nextTransferId = transferId + 1;
    }

    return true;
}

WarehouseTransferRepository::WarehouseTransferRepository(DatabaseConnection& db) : database(db)
{
}

bool WarehouseTransferRepository::saveTransfer(const WarehouseTransfer& transfer)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr)
        return false;

    string notes = "REQUESTED_BY=" + transfer.getRequestedBy() + "\nREMARKS=" + transfer.getRemarks();

    if (notes.length() > 500)
        return false;

    char* escapedNotes = new char[notes.length() * 2 + 1];

    unsigned long notesLength = mysql_real_escape_string(connection, escapedNotes, notes.c_str(),
                                                         static_cast<unsigned long>(notes.length()));

    string safeNotes(escapedNotes, notesLength);
    delete[] escapedNotes;

    string query = "INSERT INTO warehouse_transfers "
                   "(transfer_id, source_warehouse_id, destination_warehouse_id, "
                   "status, requested_by, notes, created_at, completed_at) VALUES (" +
                   to_string(transfer.getTransferId()) + ", " + to_string(transfer.getSourceWarehouseId()) +
                   ", " + to_string(transfer.getDestinationWarehouseId()) +
                   ", "
                   "'PENDING', NULL, '" +
                   safeNotes +
                   "', "
                   "FROM_UNIXTIME(" +
                   to_string(static_cast<long long>(transfer.getRequestedAt())) + "), NULL)";

    if (!database.beginTransaction())
        return false;

    if (mysql_query(connection, query.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    string detailQuery = "INSERT INTO warehouse_transfer_details "
                         "(transfer_id, resource_id, quantity) VALUES (" +
                         to_string(transfer.getTransferId()) + ", " + to_string(transfer.getResourceId()) +
                         ", " + to_string(transfer.getQuantity()) + ")";

    if (mysql_query(connection, detailQuery.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    if (!database.commitTransaction())
    {
        database.rollbackTransaction();
        return false;
    }

    return true;
}

bool WarehouseTransferRepository::loadTransfers(WarehouseTransferManager& transferManager)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr)
        return false;

    const char* query = "SELECT wt.transfer_id, wt.source_warehouse_id, "
                        "wt.destination_warehouse_id, wd.resource_id, wd.quantity, "
                        "wt.status, wt.notes, UNIX_TIMESTAMP(wt.created_at), "
                        "UNIX_TIMESTAMP(wt.completed_at) "
                        "FROM warehouse_transfers wt "
                        "INNER JOIN warehouse_transfer_details wd "
                        "ON wt.transfer_id = wd.transfer_id "
                        "ORDER BY wt.transfer_id";

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    transferManager.clearTransfers();

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int transferId = atoi(row[0]);
        int sourceId = atoi(row[1]);
        int destinationId = atoi(row[2]);
        int resourceId = atoi(row[3]);
        int quantity = atoi(row[4]);

        TransferStatus status;

        if (string(row[5]) == "PENDING")
            status = TransferStatus::REQUESTED;
        else if (string(row[5]) == "COMPLETED")
            status = TransferStatus::COMPLETED;
        else if (string(row[5]) == "CANCELLED")
            status = TransferStatus::CANCELLED;
        else
        {
            mysql_free_result(result);
            transferManager.clearTransfers();
            return false;
        }

        string notes = row[6] != nullptr ? row[6] : "";

        string requestedBy;
        string remarks;

        const string requesterPrefix = "REQUESTED_BY=";
        const string remarksPrefix = "\nREMARKS=";

        size_t requesterPosition = notes.find(requesterPrefix);
        size_t remarksPosition = notes.find(remarksPrefix);

        if (requesterPosition != string::npos && remarksPosition != string::npos &&
            remarksPosition > requesterPosition)
        {
            requestedBy = notes.substr(requesterPosition + requesterPrefix.length(),
                                       remarksPosition - (requesterPosition + requesterPrefix.length()));

            remarks = notes.substr(remarksPosition + remarksPrefix.length());
        }
        else
        {
            mysql_free_result(result);
            transferManager.clearTransfers();
            return false;
        }

        time_t requestedAt = row[7] != nullptr ? static_cast<time_t>(atoll(row[7])) : 0;

        time_t completedAt = row[8] != nullptr ? static_cast<time_t>(atoll(row[8])) : 0;

        if (!transferManager.loadTransferRecord(transferId, sourceId, destinationId, resourceId, quantity,
                                                requestedBy, remarks, requestedAt, status, completedAt))
        {
            mysql_free_result(result);
            transferManager.clearTransfers();
            return false;
        }
    }

    mysql_free_result(result);
    return true;
}

bool WarehouseTransferRepository::updateTransfer(const WarehouseTransfer& transfer)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr)
        return false;

    string status;

    if (transfer.getStatus() == TransferStatus::REQUESTED)
        status = "PENDING";
    else if (transfer.getStatus() == TransferStatus::COMPLETED)
        status = "COMPLETED";
    else if (transfer.getStatus() == TransferStatus::CANCELLED)
        status = "CANCELLED";
    else
        return false;

    string query = "UPDATE warehouse_transfers SET "
                   "status = '" +
                   status +
                   "', "
                   "completed_at = ";

    if (transfer.getStatus() == TransferStatus::COMPLETED)
    {
        query += "FROM_UNIXTIME(" + to_string(static_cast<long long>(transfer.getCompletedAt())) + ")";
    }
    else
    {
        query += "NULL";
    }

    query += " WHERE transfer_id = " + to_string(transfer.getTransferId()) + " AND status = 'PENDING'";

    if (!database.beginTransaction())
        return false;

    if (mysql_query(connection, query.c_str()) != 0 || mysql_affected_rows(connection) != 1)
    {
        cerr << "[WarehouseTransferRepository] status update failed: " << database.getLastError() << '\n';
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool WarehouseTransferRepository::transferExists(int transferId)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr || transferId <= 0)
        return false;

    string query = "SELECT transfer_id FROM warehouse_transfers "
                   "WHERE transfer_id = " +
                   to_string(transferId);

    if (mysql_query(connection, query.c_str()) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    bool exists = mysql_num_rows(result) > 0;

    mysql_free_result(result);

    return exists;
}
