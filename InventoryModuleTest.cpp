#include <iostream>
#include <limits>
#include "../include/OperationsModule.h"
using namespace std;

static int passed = 0;
static bool check(bool condition, const char* name)
{
    cout << name << ": " << (condition ? "PASSED" : "FAILED") << endl;
    if (condition) ++passed;
    return condition;
}

int main()
{
    cout << "========== RESQFLOW INVENTORY MODULE TESTS ==========\n";
    bool ok = true;
    Inventory item(1, 10, 100, 50, 10);
    ok &= check(item.getTotalStock() == 50 && item.getAvailableQuantity() == 50 && item.getReservedQuantity() == 0,
                "Initial stock accounting");
    ok &= check(item.addStock(20) && item.getTotalStock() == 70, "Stock addition");
    ok &= check(!item.addStock(0) && !item.addStock(-5), "Reject non-positive stock changes");
    ok &= check(item.reserveStock(30) && item.getAvailableQuantity() == 40 && item.getReservedQuantity() == 30 && item.getTotalStock() == 70,
                "Reservation preserves physical total");
    ok &= check(item.releaseReservedStock(10) && item.getAvailableQuantity() == 50 && item.getReservedQuantity() == 20,
                "Reservation release");
    ok &= check(item.dispatchStock(15) && item.getTotalStock() == 55 && item.getReservedQuantity() == 5,
                "Dispatch reduces physical stock");
    ok &= check(item.returnStock(8) && item.getTotalStock() == 63, "Returned stock increases available stock");
    ok &= check(!item.removeStock(1000) && !item.dispatchStock(99) && !item.releaseReservedStock(99),
                "Reject stock operations beyond available quantities");

    Inventory boundary(2, 10, 100, numeric_limits<int>::max() - 2, 0);
    ok &= check(boundary.addStock(2) && boundary.getTotalStock() == numeric_limits<int>::max(),
                "Allow stock up to integer maximum");
    ok &= check(!boundary.addStock(1) && !boundary.returnStock(1), "Prevent integer overflow");

    ResourceLinkedList resources;
    WarehouseLinkedList warehouses;
    Resource r1(10, "Water", "Essential", "Drinking water", "litre", 10);
    Resource r2(11, "Food", "Essential", "Food packets", "packet", 5);
    Warehouse w1(100, "North Depot", "Area A", 28.6, 77.2, "1111111111");
    Warehouse w2(101, "South Depot", "Area B", 28.5, 77.3, "2222222222");
    ok &= check(resources.addResource(r1) && resources.addResource(r2), "Register resources");
    ok &= check(warehouses.addWarehouse(w1) && warehouses.addWarehouse(w2), "Register warehouses");
    InventoryManager manager(resources, warehouses);
    ok &= check(manager.addInventory(10, 100, 100, 10) && manager.addInventory(10, 101, 20, 5),
                "Create inventory across warehouses");
    ok &= check(!manager.addInventory(10, 100, 1, 0) && !manager.addInventory(999, 100, 1, 0),
                "Reject duplicate and invalid resource inventory");
    Inventory* source = manager.findInventory(10, 100);
    Inventory* destination = manager.findInventory(10, 101);
    ok &= check(source && destination && manager.transferStock(100, 101, 10, 25) &&
                source->getAvailableQuantity() == 75 && destination->getAvailableQuantity() == 45,
                "Warehouse transfer updates both records");
    ok &= check(!manager.transferStock(100, 101, 10, 1000) && !manager.transferStock(100, 100, 10, 1),
                "Reject invalid warehouse transfers");
    ok &= check(manager.reserveStock(1, 20) && manager.dispatchStock(1, 5) && manager.returnStock(1, 2),
                "Manager reservation, dispatch and return operations");
    ok &= check(!manager.deleteInventory(1), "Prevent deletion while stock remains");
    ok &= check(manager.addInventory(11, 100, 0, 0), "Allow zero opening stock");
    Inventory* zero = manager.findInventory(11, 100);
    ok &= check(zero && zero->isLowStock(), "Low-stock threshold detection");
    warehouses.searchWarehouse(101)->setStatus(false);
    ok &= check(!manager.addInventory(11, 101, 1, 0) && !manager.transferStock(100, 101, 10, 1),
                "Reject inactive warehouse operations");
    ok &= check(!manager.addStock(999, 1) && !manager.reserveStock(999, 1), "Reject unknown inventory IDs");

    cout << "\n" << passed << " test groups passed." << endl;
    if (ok) cout << "ALL INVENTORY MODULE TESTS PASSED SUCCESSFULLY!\n";
    else cout << "INVENTORY MODULE TESTS FAILED.\n";
    return ok ? 0 : 1;
}
