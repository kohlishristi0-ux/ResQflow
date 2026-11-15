#ifndef WAREHOUSE_LINKED_LIST_H
#define WAREHOUSE_LINKED_LIST_H

#include <string>
#include "Warehouse.h"
#include "WarehouseHashTable.h"

class WarehouseLinkedList
{
private:
    struct Node
    {
        Warehouse data;
        Node* next;
        Node(const Warehouse& warehouse) : data(warehouse), next(nullptr) {}
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
    bool updateWarehouse(int warehouseId, std::string name,
                         std::string address, std::string contactNumber);

    // Backward-compatible overload; coordinates are ignored.
    bool updateWarehouse(int warehouseId, std::string name,
                         std::string address, double legacyLatitude,
                         double legacyLongitude, std::string contactNumber);

    void displayAllWarehouses() const;
    void clear();
};

#endif
