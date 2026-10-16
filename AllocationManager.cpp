#include <iostream>
#include "AllocationManager.h"

using namespace std;

AllocationManager::AllocationManager()
{
    head = nullptr;
    nextAllocationId = 1;
}

AllocationManager::~AllocationManager()
{
    AllocationNode* ptr = head;

    while (ptr != nullptr)
    {
        AllocationNode* temp = ptr;
        ptr = ptr->next;
        delete temp;
    }
}

void AllocationManager::allocateResource(
    ResourceLinkedList& resources,
    int emergencyId,
    int resourceId,
    int requestedQuantity
)
{
    if (emergencyId <= 0)
    {
        cout << "Invalid Emergency ID.\n";
        return;
    }

    if (requestedQuantity <= 0)
    {
        cout << "Requested quantity must be positive.\n";
        return;
    }

    string resourceName;
    int availableQuantity = 0;

    if (!resources.getResourceDetails(
            resourceId,
            resourceName,
            availableQuantity))
    {
        cout << "Resource not found.\n";
        return;
    }

    int allocatedQuantity;

    if (requestedQuantity < availableQuantity)
        allocatedQuantity = requestedQuantity;
    else
        allocatedQuantity = availableQuantity;

    int outstandingQuantity =
        requestedQuantity - allocatedQuantity;

    // Reduce the stock before recording the allocation.
    if (allocatedQuantity > 0)
    {
        if (!resources.reduceResourceQuantity(
                resourceId,
                allocatedQuantity))
        {
            cout << "Allocation failed. Please try again.\n";
            return;
        }
    }

    Allocation allocation;

    allocation.setDetails(
        nextAllocationId,
        emergencyId,
        resourceId,
        requestedQuantity,
        allocatedQuantity,
        outstandingQuantity
    );

    AllocationNode* newNode =
        new AllocationNode(allocation);

    if (head == nullptr)
    {
        head = newNode;
    }
    else
    {
        AllocationNode* ptr = head;

        while (ptr->next != nullptr)
            ptr = ptr->next;

        ptr->next = newNode;
    }

    nextAllocationId++;

    cout << "\nAllocation completed successfully!\n";
    cout << "Resource: " << resourceName << endl;
    cout << "Requested: " << requestedQuantity << endl;
    cout << "Allocated: " << allocatedQuantity << endl;
    cout << "Outstanding: " << outstandingQuantity << endl;
    cout << "Remaining Stock: "
         << availableQuantity - allocatedQuantity << endl;
}

void AllocationManager::displayAllocations() const
{
    if (head == nullptr)
    {
        cout << "No allocation records found.\n";
        return;
    }

    AllocationNode* ptr = head;

    while (ptr != nullptr)
    {
        ptr->data.displayAllocation();
        ptr = ptr->next;
    }
}