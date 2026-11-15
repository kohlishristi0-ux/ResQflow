#include "Database/DatabaseConnection.h"
#include "Database/WarehouseRepository.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

int main()
{
    const char* password = std::getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        std::cout << "Database password not found.\n";
        return 1;
    }

    DatabaseConnection database;

    if (!database.connect(
        "127.0.0.1",
        "root",
        password,
        "resqflow",
        3306))
    {
        std::cout << "Database connection failed: "
                  << database.getLastError() << '\n';
        return 1;
    }

    std::cout << "Database connected successfully.\n";

    WarehouseRepository repository(database);

    int testId = 1000000000 + static_cast<int>(time(nullptr) % 1000000000);

    Warehouse warehouse(
        testId,
        "Repository Test Warehouse",
        "Dehradun Testing Location",
        30.3165,
        78.0322,
        "9876543210"
    );

    // Test 1: Save warehouse
    if (!repository.saveWarehouse(warehouse))
    {
        std::cout << "FAIL: Warehouse insertion failed.\n";
        return 1;
    }

    std::cout << "PASS: Warehouse inserted.\n";

    // Test 2: Check warehouse existence
    if (!repository.warehouseExists(testId))
    {
        std::cout << "FAIL: Warehouse existence check failed.\n";
        return 1;
    }

    std::cout << "PASS: Warehouse existence verified.\n";

    // Test 3: Load active warehouses
    WarehouseLinkedList warehouseList;

    if (!repository.loadActiveWarehouses(warehouseList))
    {
        std::cout << "FAIL: Warehouse loading failed.\n";
        return 1;
    }

    Warehouse* loaded = warehouseList.searchWarehouse(testId);

    if (loaded == nullptr ||
        loaded->getWarehouseName() != "Repository Test Warehouse")
    {
        std::cout << "FAIL: Loaded warehouse data incorrect.\n";
        return 1;
    }

    std::cout << "PASS: Active warehouses loaded.\n";

    // Test 4: Update warehouse
    Warehouse updatedWarehouse(
        testId,
        "Updated Test Warehouse",
        "Updated Testing Location",
        28.6139,
        77.2090,
        "9999999999"
    );

    if (!repository.updateWarehouse(updatedWarehouse))
    {
        std::cout << "FAIL: Warehouse update failed.\n";
        return 1;
    }

    std::cout << "PASS: Warehouse updated.\n";

    // Test 5: Verify updated data
    if (!repository.loadActiveWarehouses(warehouseList))
    {
        std::cout << "FAIL: Reload after update failed.\n";
        return 1;
    }

    loaded = warehouseList.searchWarehouse(testId);

    if (loaded == nullptr ||
        loaded->getWarehouseName() != "Updated Test Warehouse")
    {
        std::cout << "FAIL: Updated warehouse data incorrect.\n";
        return 1;
    }

    std::cout << "PASS: Updated data verified.\n";

    // Test 6: Deactivate warehouse
    if (!repository.deactivateWarehouse(testId))
    {
        std::cout << "FAIL: Warehouse deactivation failed.\n";
        return 1;
    }

    std::cout << "PASS: Warehouse deactivated.\n";

    // Test 7: Verify inactive warehouse is not loaded
    if (!repository.loadActiveWarehouses(warehouseList))
    {
        std::cout << "FAIL: Reload after deactivation failed.\n";
        return 1;
    }

    if (warehouseList.searchWarehouse(testId) != nullptr)
    {
        std::cout << "FAIL: Inactive warehouse appeared in active list.\n";
        return 1;
    }

    std::cout << "PASS: Inactive warehouse excluded from active list.\n";

    // Test 8: Verify soft deletion
    if (!repository.warehouseExists(testId))
    {
        std::cout << "FAIL: Warehouse record should still exist.\n";
        return 1;
    }

    std::cout << "PASS: Soft deletion verified.\n";

    database.disconnect();

    std::cout << "\nALL WAREHOUSE REPOSITORY TESTS PASSED!\n";

    return 0;
}
