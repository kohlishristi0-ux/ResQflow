#ifndef WAREHOUSE_HASH_TABLE_H
#define WAREHOUSE_HASH_TABLE_H

#include "Warehouse.h"

class WarehouseHashTable
{
private:
    struct Entry
    {
        int warehouseId;
        Warehouse* warehouse;
        Entry* next;

        Entry(int id, Warehouse* ptr) : warehouseId(id), warehouse(ptr), next(nullptr) {}
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

#endif