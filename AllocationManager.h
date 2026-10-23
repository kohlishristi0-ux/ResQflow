#ifndef ALLOCATION_MANAGER_H
#define ALLOCATION_MANAGER_H

#include "Allocation.h"
#include "ResourceLinkedList.h"

struct AllocationNode
{
    Allocation data;
    AllocationNode* next;

    AllocationNode(Allocation a)
    {
        data = a;
        next = nullptr;
    }
};

class AllocationManager
{
private:
    AllocationNode* head;
    int nextAllocationId;

public:
    AllocationManager();

    ~AllocationManager();

    void allocateResource(
        ResourceLinkedList& resources,
        int emergencyId,
        int resourceId,
        int requestedQuantity
    );

    void displayAllocations() const;
};

#endif