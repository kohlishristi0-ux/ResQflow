#include <iostream>
#include "../include/Allocation.h"

using namespace std;

Allocation::Allocation()
{
    allocationId = 0;
    emergencyId = 0;
    resourceId = 0;
    warehouseId = 0;
    requestedQuantity = 0;
    allocatedQuantity = 0;
    dispatchedQuantity = 0;
    returnedQuantity = 0;
    status = AllocationStatus::PENDING;
}

Allocation::Allocation(int allocationId, int emergencyId, int resourceId, int warehouseId, int requestedQuantity)
{
    this->allocationId = allocationId;
    this->emergencyId = emergencyId;
    this->resourceId = resourceId;
    this->warehouseId = warehouseId;
    this->requestedQuantity = requestedQuantity > 0 ? requestedQuantity : 0;
    allocatedQuantity = 0;
    dispatchedQuantity = 0;
    returnedQuantity = 0;
    status = AllocationStatus::PENDING;
}

Allocation::Allocation(int allocationId, int emergencyId, int resourceId,
                       int warehouseId, int requestedQuantity,
                       int allocatedQuantity, int dispatchedQuantity,
                       int returnedQuantity, AllocationStatus status)
{
    this->allocationId = allocationId;
    this->emergencyId = emergencyId;
    this->resourceId = resourceId;
    this->warehouseId = warehouseId;
    this->requestedQuantity = requestedQuantity;
    this->allocatedQuantity = allocatedQuantity;
    this->dispatchedQuantity = dispatchedQuantity;
    this->returnedQuantity = returnedQuantity;
    this->status = status;
}

int Allocation::getAllocationId() const
{
    return allocationId;
}

int Allocation::getEmergencyId() const
{
    return emergencyId;
}

int Allocation::getResourceId() const
{
    return resourceId;
}

int Allocation::getWarehouseId() const
{
    return warehouseId;
}

int Allocation::getRequestedQuantity() const
{
    return requestedQuantity;
}

int Allocation::getAllocatedQuantity() const
{
    return allocatedQuantity;
}

int Allocation::getDispatchedQuantity() const
{
    return dispatchedQuantity;
}

int Allocation::getReturnedQuantity() const
{
    return returnedQuantity;
}

int Allocation::getNetDispatchedQuantity() const
{
    return dispatchedQuantity - returnedQuantity;
}

int Allocation::getOutstandingQuantity() const
{
    return requestedQuantity - allocatedQuantity;
}

AllocationStatus Allocation::getStatus() const
{
    return status;
}

bool Allocation::addAllocatedQuantity(int quantity)
{
    if (quantity <= 0 || quantity > getOutstandingQuantity())
        return false;

    if (status == AllocationStatus::CANCELLED ||
        status == AllocationStatus::COMPLETED)
        return false;

    allocatedQuantity += quantity;

    if (allocatedQuantity == requestedQuantity)
        status = AllocationStatus::ALLOCATED;
    else
        status = AllocationStatus::PARTIALLY_ALLOCATED;

    return true;
}

bool Allocation::dispatchQuantity(int quantity)
{
    if (quantity <= 0 || quantity > allocatedQuantity - dispatchedQuantity)
        return false;

    if (status == AllocationStatus::CANCELLED ||
        status == AllocationStatus::COMPLETED)
        return false;

    dispatchedQuantity += quantity;
    status = AllocationStatus::DISPATCHED;

    return true;
}

bool Allocation::returnQuantity(int quantity)
{
    if (quantity <= 0 || quantity > dispatchedQuantity - returnedQuantity)
        return false;
    if (status == AllocationStatus::CANCELLED)
        return false;
    returnedQuantity += quantity;
    return true;
}

bool Allocation::completeAllocation()
{
    if (status == AllocationStatus::CANCELLED ||
        dispatchedQuantity != requestedQuantity)
        return false;

    status = AllocationStatus::COMPLETED;
    return true;
}

bool Allocation::cancelAllocation()
{
    if (status == AllocationStatus::COMPLETED ||
        status == AllocationStatus::CANCELLED)
        return false;

    status = AllocationStatus::CANCELLED;
    return true;
}

void Allocation::displayAllocation() const
{
    cout << "\n--------------------------------";
    cout << "\nAllocation ID: " << allocationId;
    cout << "\nEmergency ID: " << emergencyId;
    cout << "\nResource ID: " << resourceId;
    cout << "\nWarehouse ID: " << warehouseId;
    cout << "\nRequested Quantity: " << requestedQuantity;
    cout << "\nAllocated Quantity: " << allocatedQuantity;
    cout << "\nDispatched Quantity: " << dispatchedQuantity;
    cout << "\nReturned Quantity: " << returnedQuantity;
    cout << "\nNet Dispatched Quantity: " << getNetDispatchedQuantity();
    cout << "\nOutstanding Quantity: " << getOutstandingQuantity();

    cout << "\nStatus: ";

    switch (status)
    {
        case AllocationStatus::PENDING:
            cout << "Pending";
            break;

        case AllocationStatus::PARTIALLY_ALLOCATED:
            cout << "Partially Allocated";
            break;

        case AllocationStatus::ALLOCATED:
            cout << "Allocated";
            break;

        case AllocationStatus::DISPATCHED:
            cout << "Dispatched";
            break;

        case AllocationStatus::COMPLETED:
            cout << "Completed";
            break;

        case AllocationStatus::CANCELLED:
            cout << "Cancelled";
            break;
    }

    cout << "\n--------------------------------\n";
}