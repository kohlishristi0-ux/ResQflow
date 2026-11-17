#include "OperationsModule.h"
#include <mysql.h>
#include <iostream>
#include <cstdlib>

using namespace std;

int getDatabaseValue(MYSQL* connection, const char* query)
{
    if (mysql_query(connection, query) != 0)
        return 0;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return 0;

    MYSQL_ROW row = mysql_fetch_row(result);
    int value = row && row[0] ? atoi(row[0]) : 0;

    mysql_free_result(result);
    return value;
}

int main()
{
    DatabaseConnection database;

    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "Database password environment variable missing.\n";
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow"))
    {
        cout << "Database connection failed: "
             << database.getLastError() << endl;
        return 1;
    }

    cout << "Database connected successfully.\n";

    MYSQL* connection = database.getConnection();

    int resourceId = getDatabaseValue(
        connection,
        "SELECT resource_id FROM resources WHERE is_active = TRUE LIMIT 1"
    );

    int warehouseId = getDatabaseValue(
        connection,
        "SELECT warehouse_id FROM warehouses WHERE is_active = TRUE LIMIT 1"
    );

    int emergencyId = getDatabaseValue(
        connection,
        "SELECT emergency_id FROM emergencies "
        "WHERE status NOT IN ('RESOLVED', 'CANCELLED') LIMIT 1"
    );

    int allocationId = getDatabaseValue(
        connection,
        "SELECT COALESCE(MAX(allocation_id), 0) + 1 FROM allocations"
    );

    if (resourceId <= 0 || warehouseId <= 0 ||
        emergencyId <= 0 || allocationId <= 0)
    {
        cout << "TEST PREREQUISITE: Active resource, active warehouse "
             << "and active emergency records are required.\n";
        return 1;
    }

    ResourceLinkedList resources;
    WarehouseLinkedList warehouses;

    InventoryManager inventoryManager(resources, warehouses);

    EmergencyMaxHeap emergencyHeap;
    EmergencyManager emergencyManager;
    emergencyManager.connectPriorityHeap(emergencyHeap);

    AllocationManager allocationManager(
        inventoryManager, emergencyManager, resources
    );

    AllocationRepository repository(database);

    Allocation allocation(
        allocationId, emergencyId, resourceId, warehouseId, 10
    );

    if (!allocation.addAllocatedQuantity(5) ||
        !allocation.dispatchQuantity(2) ||
        !allocation.returnQuantity(1) ||
        !repository.saveAllocation(allocation))
    {
        cout << "TEST FAILED: Allocation save.\n";
        return 1;
    }

    cout << "PASS: Allocation saved.\n";

    if (!repository.allocationExists(allocationId))
    {
        cout << "TEST FAILED: Allocation existence.\n";
        return 1;
    }

    cout << "PASS: Allocation existence.\n";

    Allocation updatedAllocation(
        allocationId, emergencyId, resourceId, warehouseId, 10
    );

    if (!updatedAllocation.addAllocatedQuantity(6) ||
        !updatedAllocation.dispatchQuantity(3) ||
        !updatedAllocation.returnQuantity(1) ||
        !repository.updateAllocation(updatedAllocation))
    {
        cout << "TEST FAILED: Allocation update.\n";
        return 1;
    }

    cout << "PASS: Allocation update.\n";

    if (!repository.loadAllocations(allocationManager))
    {
        cout << "TEST FAILED: Allocation loading.\n";
        return 1;
    }

    Allocation* loaded = allocationManager.searchAllocation(allocationId);

    if (loaded == nullptr ||
        loaded->getRequestedQuantity() != 10 ||
        loaded->getAllocatedQuantity() != 6 ||
        loaded->getDispatchedQuantity() != 3 ||
        loaded->getReturnedQuantity() != 1 ||
        loaded->getStatus() != AllocationStatus::DISPATCHED)
    {
        cout << "TEST FAILED: Allocation restoration values.\n";
        return 1;
    }

    cout << "PASS: Allocation restored correctly.\n";

    if (repository.allocationExists(-1))
    {
        cout << "TEST FAILED: Invalid allocation ID validation.\n";
        return 1;
    }

    cout << "PASS: Invalid allocation ID validation.\n";

    string cleanupQuery =
    "DELETE FROM allocations WHERE allocation_id = "
    + to_string(allocationId);

if (mysql_query(connection, cleanupQuery.c_str()) != 0)
    return 0;
}
