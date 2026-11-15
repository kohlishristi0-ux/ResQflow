#ifndef INVENTORY_MANAGER_H
#define INVENTORY_MANAGER_H

#include "Inventory.h"
#include "ResourceLinkedList.h"
#include "WarehouseLinkedList.h"

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

    bool loadInventoryRecord(int inventoryId, int resourceId, int warehouseId,
                         int availableQuantity, int reservedQuantity,
                         int minimumStockLevel);

    bool deleteInventory(int inventoryId);
    // Unconditional removal used to roll back an inventory record whose database insert failed.
    bool discardInventory(int inventoryId);

    // Safety checks used before a resource or warehouse is deactivated.
    bool hasStockForResource(int resourceId) const;
    bool hasStockInWarehouse(int warehouseId) const;
    // Fills out[] with inventory records of one resource (at most capacity); returns how many.
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

#endif