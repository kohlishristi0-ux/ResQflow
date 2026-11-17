#include <iostream>
#include "../include/OperationsModule.h"

using namespace std;

static int passedGroups = 0;
static int failedGroups = 0;

void reportTest(const char* testName, bool passed)
{
    cout << testName << ": " << (passed ? "PASSED" : "FAILED") << '\n';

    if (passed)
        ++passedGroups;
    else
        ++failedGroups;
}

struct TestEnvironment
{
    ResourceLinkedList resources;
    WarehouseLinkedList warehouses;
    InventoryManager inventory;
    ActivityHistory history;
    WarehouseTransferManager transfers;

    TestEnvironment()
        : inventory(resources, warehouses),
          transfers(inventory, warehouses, &history)
    {
        resources.addResource(Resource(1, "Medical Kit", "Medical",
                                       "Emergency medical kit", "kit", 5));

        warehouses.addWarehouse(Warehouse(10, "North Warehouse", "North Zone",
                                           28.60, 77.20, "9876543210"));
        warehouses.addWarehouse(Warehouse(20, "South Warehouse", "South Zone",
                                           28.50, 77.10, "9876543211"));
        warehouses.addWarehouse(Warehouse(30, "Inactive Warehouse", "West Zone",
                                           28.40, 77.00, "9876543212"));
        warehouses.searchWarehouse(30)->setStatus(false);

        inventory.addInventory(1, 10, 100, 10);
        inventory.addInventory(1, 20, 20, 5);
    }
};

int main()
{
    cout << "========== RESQFLOW WAREHOUSE TRANSFER TESTS ==========\n\n";

    {
        TestEnvironment env;
        bool passed = env.transfers.isEmpty() && env.transfers.getCount() == 0;
        reportTest("Empty transfer manager boundary handling", passed);
    }

    {
        TestEnvironment env;
        bool created = env.transfers.createTransfer(10, 20, 1, 30, "ADMIN", "Relief stock movement");
        const WarehouseTransfer* transfer = env.transfers.searchTransfer(1);
        bool passed = created && transfer != nullptr &&
                      transfer->getStatus() == TransferStatus::REQUESTED &&
                      transfer->getQuantity() == 30 && env.transfers.getCount() == 1;
        reportTest("Valid transfer request creation and record storage", passed);
    }

    {
        TestEnvironment env;
        env.transfers.createTransfer(10, 20, 1, 30, "ADMIN");
        bool executed = env.transfers.executeTransfer(1);
        Inventory* source = env.inventory.findInventory(1, 10);
        Inventory* destination = env.inventory.findInventory(1, 20);
        const WarehouseTransfer* transfer = env.transfers.searchTransfer(1);
        bool passed = executed && source != nullptr && destination != nullptr &&
                      source->getAvailableQuantity() == 70 &&
                      destination->getAvailableQuantity() == 50 &&
                      transfer != nullptr && transfer->getStatus() == TransferStatus::COMPLETED &&
                      transfer->getCompletedAt() > 0;
        reportTest("Transfer execution updates both warehouse inventories", passed);
    }

    {
        TestEnvironment env;
        env.transfers.createTransfer(10, 20, 1, 15, "ADMIN");
        bool firstExecution = env.transfers.executeTransfer(1);
        bool secondExecution = env.transfers.executeTransfer(1);
        bool passed = firstExecution && !secondExecution &&
                      env.inventory.findInventory(1, 10)->getAvailableQuantity() == 85 &&
                      env.inventory.findInventory(1, 20)->getAvailableQuantity() == 35;
        reportTest("Completed transfers cannot execute twice", passed);
    }

    {
        TestEnvironment env;
        env.transfers.createTransfer(10, 20, 1, 25, "COORDINATOR");
        bool cancelled = env.transfers.cancelTransfer(1);
        bool executed = env.transfers.executeTransfer(1);
        bool passed = cancelled && !executed &&
                      env.transfers.searchTransfer(1)->getStatus() == TransferStatus::CANCELLED &&
                      env.inventory.findInventory(1, 10)->getAvailableQuantity() == 100 &&
                      env.inventory.findInventory(1, 20)->getAvailableQuantity() == 20;
        reportTest("Cancellation preserves stock and prevents execution", passed);
    }

    {
        TestEnvironment env;
        bool sameWarehouse = env.transfers.createTransfer(10, 10, 1, 5, "ADMIN");
        bool invalidSource = env.transfers.createTransfer(999, 20, 1, 5, "ADMIN");
        bool invalidDestination = env.transfers.createTransfer(10, 999, 1, 5, "ADMIN");
        bool passed = !sameWarehouse && !invalidSource && !invalidDestination &&
                      env.transfers.getCount() == 0;
        reportTest("Same and unknown warehouse validation", passed);
    }

    {
        TestEnvironment env;
        bool inactiveSource = env.transfers.createTransfer(30, 20, 1, 5, "ADMIN");
        bool inactiveDestination = env.transfers.createTransfer(10, 30, 1, 5, "ADMIN");
        bool passed = !inactiveSource && !inactiveDestination && env.transfers.getCount() == 0;
        reportTest("Inactive warehouse validation", passed);
    }

    {
        TestEnvironment env;
        bool zeroQuantity = env.transfers.createTransfer(10, 20, 1, 0, "ADMIN");
        bool negativeQuantity = env.transfers.createTransfer(10, 20, 1, -4, "ADMIN");
        bool unknownResource = env.transfers.createTransfer(10, 20, 999, 4, "ADMIN");
        bool emptyActor = env.transfers.createTransfer(10, 20, 1, 4, "");
        bool passed = !zeroQuantity && !negativeQuantity && !unknownResource && !emptyActor &&
                      env.transfers.getCount() == 0;
        reportTest("Invalid resource, quantity and requester validation", passed);
    }

    {
        TestEnvironment env;
        bool insufficient = env.transfers.createTransfer(10, 20, 1, 101, "ADMIN");
        bool passed = !insufficient && env.transfers.getCount() == 0 &&
                      env.inventory.findInventory(1, 10)->getAvailableQuantity() == 100;
        reportTest("Insufficient available stock is rejected", passed);
    }

    {
        TestEnvironment env;
        env.transfers.createTransfer(10, 20, 1, 12, "ADMIN");
        bool loggedTransfer = env.transfers.executeTransfer(1);
        const ActivityRecord* record = env.history.searchByActivityId(1);
        bool passed = loggedTransfer && env.history.getCount() == 1 && record != nullptr &&
                      record->getType() == ActivityType::WAREHOUSE_TRANSFERRED &&
                      record->getWarehouseId() == 10 &&
                      record->getDestinationWarehouseId() == 20 &&
                      record->getResourceId() == 1 && record->getQuantity() == 12;
        reportTest("Successful transfer is recorded in Activity History", passed);
    }

    {
        TestEnvironment env;
        env.transfers.createTransfer(10, 20, 1, 8, "ADMIN");
        bool missing = env.transfers.searchTransfer(999) == nullptr;
        bool invalidCancel = !env.transfers.cancelTransfer(999);
        bool passed = missing && invalidCancel;
        reportTest("Unknown transfer lookup and cancellation handling", passed);
    }

    {
        TestEnvironment env;
        env.transfers.createTransfer(10, 20, 1, 8, "ADMIN");
        env.transfers.createTransfer(10, 20, 1, 5, "ADMIN");
        bool first = env.transfers.cancelTransfer(1);
        bool second = env.transfers.executeTransfer(2);
        bool countCorrect = env.transfers.getCount() == 2;
        bool passed = first && second && countCorrect &&
                      env.transfers.searchTransfer(1)->getStatus() == TransferStatus::CANCELLED &&
                      env.transfers.searchTransfer(2)->getStatus() == TransferStatus::COMPLETED;
        reportTest("Multiple transfer records retain independent statuses", passed);
    }

    cout << "\nPassed test groups: " << passedGroups << "/"
         << (passedGroups + failedGroups) << '\n';

    if (failedGroups == 0)
    {
        cout << "\nALL WAREHOUSE TRANSFER TESTS PASSED SUCCESSFULLY!\n";
        return 0;
    }

    cout << "\nSOME WAREHOUSE TRANSFER TESTS FAILED.\n";
    return 1;
}
