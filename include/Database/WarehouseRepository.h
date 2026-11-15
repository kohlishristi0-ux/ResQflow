#ifndef WAREHOUSE_REPOSITORY_H
#define WAREHOUSE_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../Warehouse.h"
#include "../WarehouseLinkedList.h"

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

#endif
