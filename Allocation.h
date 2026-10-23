#ifndef ALLOCATION_H
#define ALLOCATION_H

class Allocation
{
private:
    int allocationId;
    int emergencyId;
    int resourceId;

    int requestedQuantity;
    int allocatedQuantity;
    int outstandingQuantity;

public:
    Allocation();

    void setDetails(
        int allocationId,
        int emergencyId,
        int resourceId,
        int requestedQuantity,
        int allocatedQuantity,
        int outstandingQuantity
    );

    void displayAllocation() const;
};

#endif