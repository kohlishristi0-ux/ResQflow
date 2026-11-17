#include "OperationsModule.h"

#include <mysql.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

int main()
{
    DatabaseConnection database;

    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr || password[0] == '\0')
    {
        cout << "ERROR: RESQFLOW_DB_PASSWORD is not set.\n";
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow"))
    {
        cout << "DATABASE CONNECTION FAILED: "
             << database.getLastError() << '\n';
        return 1;
    }

    cout << "Database connected successfully.\n";

    MYSQL* connection = database.getConnection();

    int sourceWarehouseId = 0;
    int destinationWarehouseId = 0;
    int resourceId = 0;

    const char* warehouseQuery =
        "SELECT warehouse_id FROM warehouses "
        "WHERE is_active = TRUE ORDER BY warehouse_id LIMIT 2";

    if (mysql_query(connection, warehouseQuery) != 0)
    {
        cout << "Could not retrieve warehouses: "
             << mysql_error(connection) << '\n';
        return 1;
    }

    MYSQL_RES* warehouseResult = mysql_store_result(connection);

    if (warehouseResult == nullptr ||
        mysql_num_rows(warehouseResult) < 2)
    {
        if (warehouseResult != nullptr)
            mysql_free_result(warehouseResult);

        cout << "TEST PREREQUISITE: At least two active warehouses are required.\n";
        return 0;
    }

    MYSQL_ROW warehouseRow = mysql_fetch_row(warehouseResult);

    sourceWarehouseId = atoi(warehouseRow[0]);

    warehouseRow = mysql_fetch_row(warehouseResult);

    destinationWarehouseId = atoi(warehouseRow[0]);

    mysql_free_result(warehouseResult);

    const char* resourceQuery =
        "SELECT resource_id FROM resources "
        "WHERE is_active = TRUE ORDER BY resource_id LIMIT 1";

    if (mysql_query(connection, resourceQuery) != 0)
    {
        cout << "Could not retrieve resource: "
             << mysql_error(connection) << '\n';
        return 1;
    }

    MYSQL_RES* resourceResult = mysql_store_result(connection);

    if (resourceResult == nullptr ||
        mysql_num_rows(resourceResult) < 1)
    {
        if (resourceResult != nullptr)
            mysql_free_result(resourceResult);

        cout << "TEST PREREQUISITE: At least one active resource is required.\n";
        return 0;
    }

    MYSQL_ROW resourceRow = mysql_fetch_row(resourceResult);
    resourceId = atoi(resourceRow[0]);

    mysql_free_result(resourceResult);

    cout << "\nTEST DATA:\n";
    cout << "Source warehouse: " << sourceWarehouseId << '\n';
    cout << "Destination warehouse: " << destinationWarehouseId << '\n';
    cout << "Resource: " << resourceId << "\n\n";

    WarehouseTransferRepository repository(database);

    ResourceLinkedList resources;
    WarehouseLinkedList warehouses;
    InventoryManager inventoryManager(resources, warehouses);
    WarehouseTransferManager transferManager(inventoryManager, warehouses);

    int transferId = 0;

    if (mysql_query(connection,
        "SELECT COALESCE(MAX(transfer_id), 0) + 1 "
        "FROM warehouse_transfers") != 0)
    {
        cout << "Could not generate test transfer ID.\n";
        return 1;
    }

    MYSQL_RES* idResult = mysql_store_result(connection);
    MYSQL_ROW idRow = mysql_fetch_row(idResult);

    transferId = atoi(idRow[0]);

    mysql_free_result(idResult);

    WarehouseTransfer transfer(
        transferId,
        sourceWarehouseId,
        destinationWarehouseId,
        resourceId,
        5,
        "Repository Test User",
        "Warehouse transfer repository test",
        time(nullptr)
    );

    if (!repository.saveTransfer(transfer))
    {
        cout << "TEST 1 FAILED: Save transfer.\n";
        return 1;
    }

    cout << "TEST 1 PASSED: Transfer saved.\n";

    if (!repository.transferExists(transferId))
    {
        cout << "TEST 2 FAILED: Transfer existence check.\n";
        return 1;
    }

    cout << "TEST 2 PASSED: Transfer existence verified.\n";

    if (!repository.loadTransfers(transferManager))
    {
        cout << "TEST 3 FAILED: Load transfers.\n";
        return 1;
    }

    WarehouseTransfer* loadedTransfer =
        transferManager.searchTransfer(transferId);

    if (loadedTransfer == nullptr)
    {
        cout << "TEST 3 FAILED: Transfer was not restored in manager.\n";
        return 1;
    }

    cout << "TEST 3 PASSED: Transfer loaded into manager.\n";

    transfer.markCompleted(time(nullptr));

    if (!repository.updateTransfer(transfer))
    {
        cout << "TEST 4 FAILED: Update transfer.\n";
        return 1;
    }

    cout << "TEST 4 PASSED: Transfer status updated.\n";

    if (!repository.loadTransfers(transferManager))
    {
        cout << "TEST 5 FAILED: Reload transfers.\n";
        return 1;
    }

    loadedTransfer = transferManager.searchTransfer(transferId);

    if (loadedTransfer == nullptr ||
        loadedTransfer->getStatus() != TransferStatus::COMPLETED)
    {
        cout << "TEST 5 FAILED: Completed status was not restored.\n";
        return 1;
    }

    cout << "TEST 5 PASSED: Completed status restored correctly.\n";

    string cleanupDetails =
        "DELETE FROM warehouse_transfer_details WHERE transfer_id = "
        + to_string(transferId);

    string cleanupTransfer =
        "DELETE FROM warehouse_transfers WHERE transfer_id = "
        + to_string(transferId);

    if (mysql_query(connection, cleanupDetails.c_str()) != 0 ||
        mysql_query(connection, cleanupTransfer.c_str()) != 0)
    {
        cout << "WARNING: Test cleanup failed: "
             << mysql_error(connection) << '\n';
        return 1;
    }

    cout << "\nALL WAREHOUSE TRANSFER REPOSITORY TESTS PASSED!\n";

    return 0;
}
