#include <iostream>
#include "Allocation.h"

using namespace std;

Allocation::Allocation()
{
    allocationId = 0;
    emergencyId = 0;
    resourceId = 0;
    requestedQuantity = 0;
    allocatedQuantity = 0;
    outstandingQuantity = 0;
}

void Allocation::setDetails(
    int aId,
    int eId,
    int rId,
    int requested,
    int allocated,
    int outstanding
)
{
    allocationId = aId;
    emergencyId = eId;
    resourceId = rId;
    requestedQuantity = requested;
    allocatedQuantity = allocated;
    outstandingQuantity = outstanding;
}

void Allocation::displayAllocation() const
{
    cout << "\nAllocation ID: " << allocationId;
    cout << "\nEmergency ID: " << emergencyId;
    cout << "\nResource ID: " << resourceId;
    cout << "\nRequested Quantity: " << requestedQuantity;
    cout << "\nAllocated Quantity: " << allocatedQuantity;
    cout << "\nOutstanding Quantity: " << outstandingQuantity;
    cout << "\n-----------------------------\n";
}