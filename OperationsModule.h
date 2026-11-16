#ifndef OPERATIONS_MODULE_H
#define OPERATIONS_MODULE_H

#include <string>
#include <ctime>
#include "ResourceModule.h"
#include "EmergencyModule.h"
#include "ResponderModule.h"

using namespace std;

class Warehouse
{
private:
    int warehouseId;
    std::string warehouseName;
    std::string address;
    std::string contactNumber;
    bool active;

public:
    Warehouse();
    Warehouse(int id, std::string name, std::string address, std::string contactNumber);

    Warehouse(int id, std::string name, std::string address, double legacyLatitude, double legacyLongitude,
              std::string contactNumber);

    int getWarehouseId() const;
    std::string getWarehouseName() const;
    std::string getAddress() const;
    std::string getContactNumber() const;
    bool isActive() const;

    void setWarehouseName(std::string name);
    void setAddress(std::string address);
    void setContactNumber(std::string contactNumber);
    void setStatus(bool status);

    void displayWarehouse() const;
};

class WarehouseHashTable
{
private:
    struct Entry
    {
        int warehouseId;
        Warehouse* warehouse;
        Entry* next;

        Entry(int id, Warehouse* ptr) : warehouseId(id), warehouse(ptr), next(nullptr)
        {
        }
    };

    Entry** table;
    int capacity;
    int count;
    double loadFactor;

    int hashFunction(int warehouseId) const;
    bool resizeTable();

public:
    WarehouseHashTable(int initialCapacity = 10);
    ~WarehouseHashTable();

    WarehouseHashTable(const WarehouseHashTable&) = delete;
    WarehouseHashTable& operator=(const WarehouseHashTable&) = delete;

    bool insert(int warehouseId, Warehouse* warehouse);
    Warehouse* search(int warehouseId) const;
    bool remove(int warehouseId);

    bool contains(int warehouseId) const;
    int getCount() const;
    int getCapacity() const;
    double getLoadFactor() const;

    void displayTable() const;
    void clear();
};

class WarehouseLinkedList
{
private:
    struct Node
    {
        Warehouse data;
        Node* next;
        Node(const Warehouse& warehouse) : data(warehouse), next(nullptr)
        {
        }
    };

    Node* head;
    Node* tail;
    int count;
    WarehouseHashTable warehouseIndex;

public:
    WarehouseLinkedList();
    ~WarehouseLinkedList();
    WarehouseLinkedList(const WarehouseLinkedList&) = delete;
    WarehouseLinkedList& operator=(const WarehouseLinkedList&) = delete;

    bool isEmpty() const;
    bool isDuplicate(int warehouseId) const;
    int getCount() const;
    bool addWarehouse(const Warehouse& warehouse);
    bool deleteWarehouse(int warehouseId);
    Warehouse* searchWarehouse(int warehouseId);
    const Warehouse* searchWarehouse(int warehouseId) const;
    bool updateWarehouse(int warehouseId, std::string name, std::string address, std::string contactNumber);

    bool updateWarehouse(int warehouseId, std::string name, std::string address, double legacyLatitude,
                         double legacyLongitude, std::string contactNumber);

    void displayAllWarehouses() const;
    void clear();
};

class WarehouseRepository
{
private:
    DatabaseConnection& database;

public:
    explicit WarehouseRepository(DatabaseConnection& db);

    bool saveWarehouse(const Warehouse& warehouse);
    bool loadActiveWarehouses(WarehouseLinkedList& warehouseList);
    bool updateWarehouse(const Warehouse& warehouse);
    bool deactivateWarehouse(int warehouseId);
    bool warehouseExists(int warehouseId);
};

class Inventory
{
private:
    int inventoryId;
    int resourceId;
    int warehouseId;
    int availableQuantity;
    int reservedQuantity;
    int minimumStockLevel;

public:
    Inventory();
    Inventory(int inventoryId, int resourceId, int warehouseId, int availableQuantity, int minimumStockLevel);

    Inventory(int inventoryId, int resourceId, int warehouseId, int availableQuantity, int reservedQuantity,
              int minimumStockLevel);
    int getInventoryId() const;
    int getResourceId() const;
    int getWarehouseId() const;
    int getAvailableQuantity() const;
    int getReservedQuantity() const;
    int getMinimumStockLevel() const;
    int getTotalStock() const;

    bool addStock(int quantity);
    bool removeStock(int quantity);
    bool reserveStock(int quantity);
    bool releaseReservedStock(int quantity);
    bool dispatchStock(int quantity);
    bool returnStock(int quantity);

    void setMinimumStockLevel(int level);
    bool isLowStock() const;
    void displayInventory() const;
};

class InventoryManager
{
private:
    struct Node
    {
        Inventory data;
        Node* next;

        Node(const Inventory& inventory)
        {
            data = inventory;
            next = nullptr;
        }
    };

    Node* head;
    int count;
    int nextInventoryId;

    ResourceLinkedList* resourceList;
    WarehouseLinkedList* warehouseList;

public:
    InventoryManager(ResourceLinkedList& resources, WarehouseLinkedList& warehouses);
    ~InventoryManager();

    InventoryManager(const InventoryManager&) = delete;
    InventoryManager& operator=(const InventoryManager&) = delete;

    bool isEmpty() const;
    bool isWarehouseActive(int warehouseId) const;
    int getCount() const;

    bool addInventory(int resourceId, int warehouseId, int quantity, int minimumStockLevel);

    bool loadInventoryRecord(int inventoryId, int resourceId, int warehouseId, int availableQuantity,
                             int reservedQuantity, int minimumStockLevel);

    bool deleteInventory(int inventoryId);
    bool discardInventory(int inventoryId);

    bool hasStockForResource(int resourceId) const;
    bool hasStockInWarehouse(int warehouseId) const;
    int collectByResource(int resourceId, Inventory** out, int capacity);

    Inventory* searchInventory(int inventoryId);
    Inventory* findInventory(int resourceId, int warehouseId);

    bool addStock(int inventoryId, int quantity);
    bool removeStock(int inventoryId, int quantity);
    bool reserveStock(int inventoryId, int quantity);
    bool releaseReservedStock(int inventoryId, int quantity);
    bool dispatchStock(int inventoryId, int quantity);
    bool returnStock(int inventoryId, int quantity);

    bool transferStock(int sourceWarehouseId, int destinationWarehouseId, int resourceId, int quantity);

    void displayAllInventory() const;
    void displayInventoryByWarehouse(int warehouseId) const;
    void displayInventoryByResource(int resourceId) const;
    void displayLowStock() const;
    void clearInventory();
};

class InventoryRepository
{
private:
    DatabaseConnection& database;

public:
    explicit InventoryRepository(DatabaseConnection& db);

    bool saveInventory(const Inventory& inventory);
    bool loadInventory(InventoryManager& inventoryManager);
    bool updateInventory(const Inventory& inventory);
    bool deleteInventory(int inventoryId);
    bool inventoryExists(int inventoryId);

    bool recordMovement(int resourceId, int warehouseId, const char* transactionType, int quantity,
                        const char* referenceType = nullptr, long long referenceId = 0, int performedBy = 0,
                        const char* notes = nullptr);
};

enum class AllocationStatus
{
    PENDING,
    PARTIALLY_ALLOCATED,
    ALLOCATED,
    DISPATCHED,
    COMPLETED,
    CANCELLED
};

class Allocation
{
private:
    int allocationId;
    int emergencyId;
    int resourceId;
    int warehouseId;
    int requestedQuantity;
    int allocatedQuantity;
    int dispatchedQuantity;
    int returnedQuantity;
    AllocationStatus status;

public:
    Allocation();
    Allocation(int allocationId, int emergencyId, int resourceId, int warehouseId, int requestedQuantity);
    Allocation(int allocationId, int emergencyId, int resourceId, int warehouseId, int requestedQuantity,
               int allocatedQuantity, int dispatchedQuantity, int returnedQuantity, AllocationStatus status);
    int getAllocationId() const;
    int getEmergencyId() const;
    int getResourceId() const;
    int getWarehouseId() const;
    int getRequestedQuantity() const;
    int getAllocatedQuantity() const;
    int getDispatchedQuantity() const;
    int getReturnedQuantity() const;
    int getNetDispatchedQuantity() const;
    int getOutstandingQuantity() const;
    AllocationStatus getStatus() const;

    bool addAllocatedQuantity(int quantity);
    bool dispatchQuantity(int quantity);
    bool returnQuantity(int quantity);
    bool completeAllocation();
    bool cancelAllocation();

    void displayAllocation() const;
};

class AllocationManager
{
private:
    struct Node
    {
        Allocation data;
        Node* next;

        Node(const Allocation& allocation)
        {
            data = allocation;
            next = nullptr;
        }
    };

    Node* head;
    int count;
    int nextAllocationId;

    InventoryManager* inventoryManager;
    EmergencyManager* emergencyManager;
    ResourceLinkedList* resourceList;

public:
    AllocationManager(InventoryManager& inventory, EmergencyManager& emergency,
                      ResourceLinkedList& resources);
    ~AllocationManager();

    AllocationManager(const AllocationManager&) = delete;
    AllocationManager& operator=(const AllocationManager&) = delete;

    bool isEmpty() const;
    int getCount() const;

    bool createAllocation(int emergencyId, int resourceId, int warehouseId, int requestedQuantity);

    Allocation* searchAllocation(int allocationId);
    int peekNextAllocationId() const;

    Allocation* findActiveAllocationForEmergency(int emergencyId);

    bool removeAllocation(int allocationId);

    bool hasActiveAllocationForResource(int resourceId) const;
    bool hasActiveAllocationForWarehouse(int warehouseId) const;

    bool allocateAdditionalStock(int allocationId, int quantity);
    bool dispatchAllocation(int allocationId, int quantity);
    bool returnDispatchedStock(int allocationId, int quantity);
    bool completeAllocation(int allocationId);
    bool cancelAllocation(int allocationId);

    void displayAllAllocations() const;
    void displayAllocationsByEmergency(int emergencyId) const;
    void displayAllocationsByStatus(AllocationStatus status) const;
    void clearAllocations();
    bool loadAllocationRecord(int allocationId, int emergencyId, int resourceId, int warehouseId,
                              int requestedQuantity, int allocatedQuantity, int dispatchedQuantity,
                              int returnedQuantity, AllocationStatus status);
};

class AllocationRepository
{
private:
    DatabaseConnection& database;

public:
    explicit AllocationRepository(DatabaseConnection& db);

    bool saveAllocation(const Allocation& allocation);

    bool loadAllocations(AllocationManager& allocationManager);

    bool updateAllocation(const Allocation& allocation);

    bool allocationExists(int allocationId);

    bool syncRequirement(int emergencyId, int resourceId);
    bool displayRequirements(int emergencyId);
};

enum class TransferStatus
{
    REQUESTED,
    COMPLETED,
    CANCELLED
};

class WarehouseTransfer
{
private:
    int transferId;
    int sourceWarehouseId;
    int destinationWarehouseId;
    int resourceId;
    int quantity;
    TransferStatus status;
    std::string requestedBy;
    std::string remarks;
    std::time_t requestedAt;
    std::time_t completedAt;

public:
    WarehouseTransfer();
    WarehouseTransfer(int id, int sourceId, int destinationId, int resource, int transferQuantity,
                      const std::string& actor, const std::string& transferRemarks, std::time_t requestTime);

    int getTransferId() const;
    int getSourceWarehouseId() const;
    int getDestinationWarehouseId() const;
    int getResourceId() const;
    int getQuantity() const;
    TransferStatus getStatus() const;
    std::string getRequestedBy() const;
    std::string getRemarks() const;
    std::time_t getRequestedAt() const;
    std::time_t getCompletedAt() const;

    void markCompleted(std::time_t completionTime);
    void markCancelled();
    bool restoreStatus(TransferStatus restoredStatus, std::time_t completionTime);
    void display() const;
};

const char* transferStatusToString(TransferStatus status);

class WarehouseTransferManager
{
private:
    struct Node
    {
        WarehouseTransfer data;
        Node* next;

        explicit Node(const WarehouseTransfer& transfer);
    };

    Node* head;
    Node* tail;
    int count;
    int nextTransferId;

    InventoryManager* inventoryManager;
    WarehouseLinkedList* warehouseList;
    ActivityHistory* activityHistory;

    bool areWarehousesValid(int sourceId, int destinationId) const;

public:
    WarehouseTransferManager(InventoryManager& inventory, WarehouseLinkedList& warehouses,
                             ActivityHistory* history = nullptr);
    ~WarehouseTransferManager();

    WarehouseTransferManager(const WarehouseTransferManager&) = delete;
    WarehouseTransferManager& operator=(const WarehouseTransferManager&) = delete;

    bool isEmpty() const;
    int getCount() const;

    void connectActivityHistory(ActivityHistory* history);

    bool createTransfer(int sourceWarehouseId, int destinationWarehouseId, int resourceId, int quantity,
                        const std::string& requestedBy, const std::string& remarks = "");

    int peekNextTransferId() const;
    bool removeTransfer(int transferId);
    bool hasPendingTransferForWarehouse(int warehouseId) const;

    bool executeTransfer(int transferId);
    bool cancelTransfer(int transferId);

    WarehouseTransfer* searchTransfer(int transferId);
    const WarehouseTransfer* searchTransfer(int transferId) const;

    void displayAllTransfers() const;
    void displayTransfersByWarehouse(int warehouseId) const;
    void displayTransfersByStatus(TransferStatus status) const;
    void clearTransfers();

    bool loadTransferRecord(int transferId, int sourceWarehouseId, int destinationWarehouseId, int resourceId,
                            int quantity, const std::string& requestedBy, const std::string& remarks,
                            std::time_t requestedAt, TransferStatus status, std::time_t completedAt);
};

class WarehouseTransferRepository
{
private:
    DatabaseConnection& database;

public:
    explicit WarehouseTransferRepository(DatabaseConnection& db);

    bool saveTransfer(const WarehouseTransfer& transfer);

    bool loadTransfers(WarehouseTransferManager& transferManager);

    bool updateTransfer(const WarehouseTransfer& transfer);

    bool transferExists(int transferId);
};

#endif
