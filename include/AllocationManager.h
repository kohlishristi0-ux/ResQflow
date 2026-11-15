#ifndef ALLOCATION_MANAGER_H
#define ALLOCATION_MANAGER_H

#include "Allocation.h"
#include "InventoryManager.h"
#include "EmergencyManager.h"
#include "ResourceLinkedList.h"

class AllocationManager
{
private:
    struct Node
    {
        Allocation data;
        Node* next;

        Node(const Allocation& allocation)
        {
            data = allocation;
            next = nullptr;
        }
    };

    Node* head;
    int count;
    int nextAllocationId;

    InventoryManager* inventoryManager;
    EmergencyManager* emergencyManager;
    ResourceLinkedList* resourceList;

public:
    AllocationManager(InventoryManager& inventory, EmergencyManager& emergency, ResourceLinkedList& resources);
    ~AllocationManager();

    AllocationManager(const AllocationManager&) = delete;
    AllocationManager& operator=(const AllocationManager&) = delete;

    bool isEmpty() const;
    int getCount() const;

    bool createAllocation(int emergencyId, int resourceId, int warehouseId, int requestedQuantity);

    Allocation* searchAllocation(int allocationId);
    int peekNextAllocationId() const;

    // First allocation of an emergency that is not CANCELLED/COMPLETED (nullptr when none).
    Allocation* findActiveAllocationForEmergency(int emergencyId);

    // Roll back a freshly created allocation: returns its reserved stock and unlinks it.
    bool removeAllocation(int allocationId);

    bool hasActiveAllocationForResource(int resourceId) const;
    bool hasActiveAllocationForWarehouse(int warehouseId) const;

    bool allocateAdditionalStock(int allocationId, int quantity);
    bool dispatchAllocation(int allocationId, int quantity);
    bool returnDispatchedStock(int allocationId, int quantity);
    bool completeAllocation(int allocationId);
    bool cancelAllocation(int allocationId);

    void displayAllAllocations() const;
    void displayAllocationsByEmergency(int emergencyId) const;
    void displayAllocationsByStatus(AllocationStatus status) const;
    void clearAllocations();
    bool loadAllocationRecord(
    int allocationId,
    int emergencyId,
    int resourceId,
    int warehouseId,
    int requestedQuantity,
    int allocatedQuantity,
    int dispatchedQuantity,
    int returnedQuantity,
    AllocationStatus status
);
};

#endif