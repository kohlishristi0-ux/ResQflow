#include <iostream>
#include "../include/OperationsModule.h"

using namespace std;

int main()
{
    ResourceLinkedList resources;
    WarehouseLinkedList warehouses;
    EmergencyMaxHeap priorityHeap;
    EmergencyManager emergencies;
    emergencies.connectPriorityHeap(priorityHeap);

    if (!resources.addResource(Resource(1, "Water", "Essential", "Drinking water", "litre", 5)))
    {
        cout << "SETUP FAILED: resource\n";
        return 1;
    }
    if (!warehouses.addWarehouse(Warehouse(1, "Central", "Main Road", 28.6, 77.2, "9999999999")))
    {
        cout << "SETUP FAILED: warehouse\n";
        return 1;
    }
    if (!emergencies.addEmergency("Flood", "Sector 1", "Flood response", 5, 28.6, 77.2, 100))
    {
        cout << "SETUP FAILED: emergency\n";
        return 1;
    }

    InventoryManager inventory(resources, warehouses);
    if (!inventory.addInventory(1, 1, 8, 2))
    {
        cout << "SETUP FAILED: inventory\n";
        return 1;
    }
    AllocationManager allocations(inventory, emergencies, resources);

    if (!allocations.createAllocation(1, 1, 1, 10)) return 2;
    Allocation* allocation = allocations.searchAllocation(1);
    Inventory* stock = inventory.findInventory(1, 1);
    if (allocation == nullptr || stock == nullptr || allocation->getAllocatedQuantity() != 8 ||
        allocation->getOutstandingQuantity() != 2 || stock->getReservedQuantity() != 8) return 3;
    cout << "Partial allocation and reservation: PASSED\n";

    if (!inventory.addStock(stock->getInventoryId(), 5)) return 4;
    if (!allocations.allocateAdditionalStock(1, 2)) return 5;
    if (allocation->getAllocatedQuantity() != 10 || allocation->getStatus() != AllocationStatus::ALLOCATED) return 6;
    cout << "Additional allocation: PASSED\n";

    if (!allocations.dispatchAllocation(1, 3)) return 7;
    if (allocation->getDispatchedQuantity() != 3 || stock->getReservedQuantity() != 7) return 8;
    if (!allocations.returnDispatchedStock(1, 1)) return 9;
    if (allocation->getReturnedQuantity() != 1 || allocation->getNetDispatchedQuantity() != 2 ||
        stock->getAvailableQuantity() != 4) return 10;
    cout << "Dispatch and partial return: PASSED\n";

    if (allocations.returnDispatchedStock(1, 3)) return 11;
    if (allocations.dispatchAllocation(1, 8)) return 12;
    if (!allocations.cancelAllocation(1)) return 13;
    if (stock->getReservedQuantity() != 0 || stock->getAvailableQuantity() != 11 ||
        allocation->getStatus() != AllocationStatus::CANCELLED || allocation->getDispatchedQuantity() != 3 ||
        allocation->getReturnedQuantity() != 1) return 14;
    if (allocations.allocateAdditionalStock(1, 1) || allocations.dispatchAllocation(1, 1)) return 15;
    cout << "Cancellation, reservation release and history preservation: PASSED\n";

    warehouses.searchWarehouse(1)->setStatus(false);
    if (allocations.createAllocation(1, 1, 1, 1)) return 16;
    cout << "Inactive warehouse validation: PASSED\n";

    if (allocations.createAllocation(999, 1, 1, 1) || allocations.createAllocation(1, 999, 1, 1) ||
        allocations.createAllocation(1, 1, 999, 1) || allocations.createAllocation(1, 1, 1, 0)) return 17;
    cout << "Invalid reference and quantity validation: PASSED\n";
    cout << "\nALL ALLOCATION MODULE TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}
