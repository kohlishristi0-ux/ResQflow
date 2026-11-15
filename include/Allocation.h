#ifndef ALLOCATION_H
#define ALLOCATION_H

#include <string>
using namespace std;

enum class AllocationStatus
{
    PENDING,
    PARTIALLY_ALLOCATED,
    ALLOCATED,
    DISPATCHED,
    COMPLETED,
    CANCELLED
};

class Allocation
{
private:
    int allocationId;
    int emergencyId;
    int resourceId;
    int warehouseId;
    int requestedQuantity;
    int allocatedQuantity;
    int dispatchedQuantity;
    int returnedQuantity;
    AllocationStatus status;

public:
    Allocation();
    Allocation(int allocationId, int emergencyId, int resourceId, int warehouseId, int requestedQuantity);
    Allocation(int allocationId, int emergencyId, int resourceId,
           int warehouseId, int requestedQuantity,
           int allocatedQuantity, int dispatchedQuantity,
           int returnedQuantity, AllocationStatus status);
    int getAllocationId() const;
    int getEmergencyId() const;
    int getResourceId() const;
    int getWarehouseId() const;
    int getRequestedQuantity() const;
    int getAllocatedQuantity() const;
    int getDispatchedQuantity() const;
    int getReturnedQuantity() const;
    int getNetDispatchedQuantity() const;
    int getOutstandingQuantity() const;
    AllocationStatus getStatus() const;

    bool addAllocatedQuantity(int quantity);
    bool dispatchQuantity(int quantity);
    bool returnQuantity(int quantity);
    bool completeAllocation();
    bool cancelAllocation();

    void displayAllocation() const;
};

#endif