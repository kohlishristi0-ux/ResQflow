#include <iostream>
#include "../include/WarehouseLinkedList.h"
#include "../include/WarehouseHashTable.h"

using namespace std;

int main()
{
    WarehouseLinkedList warehouses;

    cout << "\n========== WAREHOUSE LINKED LIST TEST ==========\n";

    warehouses.addWarehouse(Warehouse(101, "Central Warehouse", "Dehradun", 30.3165, 78.0322, "9876543210"));
    warehouses.addWarehouse(Warehouse(102, "Medical Warehouse", "Haridwar", 29.9457, 78.1642, "9876543211"));
    warehouses.addWarehouse(Warehouse(103, "Food Warehouse", "Rishikesh", 30.0869, 78.2676, "9876543212"));

    cout << "Insertion Test: " << (warehouses.getCount() == 3 ? "PASSED" : "FAILED") << endl;

    cout << "Duplicate Test: " << (!warehouses.addWarehouse(Warehouse(101, "Duplicate", "Delhi", 28.6139, 77.2090, "9999999999")) ? "PASSED" : "FAILED") << endl;

    cout << "Search Test: " << (warehouses.searchWarehouse(102) != nullptr ? "PASSED" : "FAILED") << endl;

    cout << "Invalid Search Test: " << (warehouses.searchWarehouse(999) == nullptr ? "PASSED" : "FAILED") << endl;

    warehouses.deleteWarehouse(102);

    cout << "Deletion Test: " << (warehouses.searchWarehouse(102) == nullptr && warehouses.getCount() == 2 ? "PASSED" : "FAILED") << endl;

    cout << "Remaining Warehouses: " << warehouses.getCount() << endl;

    cout << "\n========== HASH TABLE RESIZING TEST ==========\n";

    WarehouseHashTable hashTable;
    Warehouse records[8];

    for (int i = 0; i < 8; i++)
    {
        records[i] = Warehouse(200 + i, "Test Warehouse", "Dehradun", 30.3165, 78.0322, "9876543210");
        hashTable.insert(200 + i, &records[i]);
    }

    cout << "Resizing Test: " << (hashTable.getCapacity() > 10 ? "PASSED" : "FAILED") << endl;
    cout << "Hash Table Entries: " << hashTable.getCount() << endl;
    cout << "Hash Table Capacity: " << hashTable.getCapacity() << endl;
    cout << "Hash Table Load Factor: " << hashTable.getLoadFactor() << endl;

    cout << "\n========== TESTING COMPLETED ==========\n";

    return 0;
}