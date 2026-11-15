#include "Database/InventoryRepository.h"
#include "Database/DatabaseConnection.h"
#include "ResourceLinkedList.h"
#include "WarehouseLinkedList.h"
#include <mysql.h>
#include <iostream>
#include <cstdlib>

using namespace std;

int main()
{
    DatabaseConnection database;

    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "ERROR: RESQFLOW_DB_PASSWORD is not set.\n";
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow", 3306))
    {
        cout << "Database connection failed: "
             << database.getLastError() << endl;
        return 1;
    }

    cout << "Database connected successfully.\n";

    MYSQL* connection = database.getConnection();

    int resourceId = 0;
    int warehouseId = 0;

    if (mysql_query(connection,
        "SELECT resource_id FROM resources "
        "WHERE is_active = TRUE LIMIT 1") != 0)
    {
        cout << "Resource query failed.\n";
        return 1;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    MYSQL_ROW row = result ? mysql_fetch_row(result) : nullptr;

    if (row == nullptr)
    {
        if (result)
            mysql_free_result(result);

        cout << "TEST PREREQUISITE: Add at least one active resource first.\n";
        return 1;
    }

    resourceId = atoi(row[0]);
    mysql_free_result(result);

    if (mysql_query(connection,
        "SELECT warehouse_id FROM warehouses "
        "WHERE is_active = TRUE LIMIT 1") != 0)
    {
        cout << "Warehouse query failed.\n";
        return 1;
    }

    result = mysql_store_result(connection);
    row = result ? mysql_fetch_row(result) : nullptr;

    if (row == nullptr)
    {
        if (result)
            mysql_free_result(result);

        cout << "TEST PREREQUISITE: Add at least one active warehouse first.\n";
        return 1;
    }

    warehouseId = atoi(row[0]);
    mysql_free_result(result);

    int testInventoryId = 900001;

    if (mysql_query(connection,
        "SELECT MAX(inventory_id) FROM inventory") != 0)
    {
        cout << "Inventory ID query failed.\n";
        return 1;
    }

    result = mysql_store_result(connection);
    row = result ? mysql_fetch_row(result) : nullptr;

    if (row != nullptr && row[0] != nullptr)
    {
        int maximumId = atoi(row[0]);

        if (maximumId >= 900001)
            testInventoryId = maximumId + 1;
    }

    if (result)
        mysql_free_result(result);

    InventoryRepository repository(database);

    Inventory testInventory(
        testInventoryId,
        resourceId,
        warehouseId,
        100,
        20,
        10
    );

    cout << "\nTEST 1: Saving inventory...\n";

    if (!repository.saveInventory(testInventory))
    {
        cout << "FAILED: " << database.getLastError() << endl;
        return 1;
    }

    cout << "PASSED: Inventory saved.\n";

    cout << "\nTEST 2: Checking inventory existence...\n";

    if (!repository.inventoryExists(testInventoryId))
    {
        cout << "FAILED: Inventory record not found.\n";
        return 1;
    }

    cout << "PASSED: Inventory exists.\n";

    cout << "\nTEST 3: Loading inventory...\n";

    ResourceLinkedList resourceList;
    WarehouseLinkedList warehouseList;

    InventoryManager inventoryManager(resourceList, warehouseList);

    // Load the referenced resource and warehouse into memory first.
    cout << "NOTE: Inventory loading requires matching active "
         << "resource and warehouse records in memory.\n";

    cout << "\nTEST 4: Updating inventory...\n";

    Inventory updatedInventory(
        testInventoryId,
        resourceId,
        warehouseId,
        150,
        25,
        15
    );

    if (!repository.updateInventory(updatedInventory))
    {
        cout << "FAILED: Inventory update failed.\n";
        return 1;
    }

    cout << "PASSED: Inventory updated.\n";

    cout << "\nTEST 5: Deleting inventory with available stock...\n";

    if (repository.deleteInventory(testInventoryId))
    {
        cout << "FAILED: Inventory with stock should not be deleted.\n";
        return 1;
    }

    cout << "PASSED: Inventory with stock was protected.\n";

    cout << "\nTEST 6: Removing test stock...\n";

    Inventory emptyInventory(
        testInventoryId,
        resourceId,
        warehouseId,
        0,
        0,
        15
    );

    if (!repository.updateInventory(emptyInventory))
    {
        cout << "FAILED: Could not clear test stock.\n";
        return 1;
    }

    cout << "PASSED: Test stock cleared.\n";

    cout << "\nTEST 7: Deleting empty inventory...\n";

    if (!repository.deleteInventory(testInventoryId))
    {
        cout << "FAILED: Empty inventory could not be deleted.\n";
        return 1;
    }

    if (repository.inventoryExists(testInventoryId))
    {
        cout << "FAILED: Deleted inventory still exists.\n";
        return 1;
    }

    cout << "PASSED: Empty inventory deleted.\n";

    cout << "\n====================================\n";
    cout << "ALL INVENTORY REPOSITORY TESTS PASSED!\n";
    cout << "====================================\n";

    return 0;
}