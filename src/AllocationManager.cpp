#include <iostream>
#include <limits>
#include <new>
#include "../include/AllocationManager.h"

using namespace std;

AllocationManager::AllocationManager(InventoryManager& inventory, EmergencyManager& emergency, ResourceLinkedList& resources)
{
    head = nullptr;
    count = 0;
    nextAllocationId = 1;
    inventoryManager = &inventory;
    emergencyManager = &emergency;
    resourceList = &resources;
}

AllocationManager::~AllocationManager()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }
}

bool AllocationManager::isEmpty() const { return head == nullptr; }
int AllocationManager::getCount() const { return count; }

bool AllocationManager::createAllocation(int emergencyId, int resourceId, int warehouseId, int requestedQuantity)
{
    if (requestedQuantity <= 0 || nextAllocationId <= 0 || nextAllocationId == numeric_limits<int>::max())
        return false;
    Emergency* emergency = emergencyManager->searchEmergency(emergencyId);
    Resource* resource = resourceList->searchResource(resourceId);
    Inventory* inventory = inventoryManager->findInventory(resourceId, warehouseId);
    if (emergency == nullptr || resource == nullptr || inventory == nullptr ||
        !resource->isActive() || !inventoryManager->isWarehouseActive(warehouseId) ||
        emergency->getStatus() == EmergencyStatus::RESOLVED || emergency->getStatus() == EmergencyStatus::CANCELLED)
        return false;

    int quantityToAllocate = inventory->getAvailableQuantity() < requestedQuantity
                           ? inventory->getAvailableQuantity() : requestedQuantity;
    Allocation allocation(nextAllocationId, emergencyId, resourceId, warehouseId, requestedQuantity);
    if (quantityToAllocate > 0 && !allocation.addAllocatedQuantity(quantityToAllocate))
        return false;

    Node* newNode = new (nothrow) Node(allocation);
    if (newNode == nullptr)
        return false;
    if (quantityToAllocate > 0 && !inventoryManager->reserveStock(inventory->getInventoryId(), quantityToAllocate))
    {
        delete newNode;
        return false;
    }
    if (head == nullptr)
        head = newNode;
    else
    {
        Node* current = head;
        while (current->next != nullptr) current = current->next;
        current->next = newNode;
    }
    count++;
    nextAllocationId++;
    return true;
}

int AllocationManager::peekNextAllocationId() const
{
    return nextAllocationId;
}

Allocation* AllocationManager::findActiveAllocationForEmergency(int emergencyId)
{
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == emergencyId &&
            current->data.getStatus() != AllocationStatus::CANCELLED &&
            current->data.getStatus() != AllocationStatus::COMPLETED)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

bool AllocationManager::removeAllocation(int allocationId)
{
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getAllocationId() != allocationId)
    {
        previous = current;
        current = current->next;
    }
    if (current == nullptr)
        return false;

    int reserved = current->data.getAllocatedQuantity() - current->data.getDispatchedQuantity();
    if (reserved > 0)
    {
        Inventory* inventory = inventoryManager->findInventory(current->data.getResourceId(), current->data.getWarehouseId());
        if (inventory != nullptr)
            inventoryManager->releaseReservedStock(inventory->getInventoryId(), reserved);
    }

    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    delete current;
    --count;
    return true;
}

bool AllocationManager::hasActiveAllocationForResource(int resourceId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getResourceId() == resourceId &&
            current->data.getStatus() != AllocationStatus::CANCELLED &&
            current->data.getStatus() != AllocationStatus::COMPLETED)
            return true;
    return false;
}

bool AllocationManager::hasActiveAllocationForWarehouse(int warehouseId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getWarehouseId() == warehouseId &&
            current->data.getStatus() != AllocationStatus::CANCELLED &&
            current->data.getStatus() != AllocationStatus::COMPLETED)
            return true;
    return false;
}

Allocation* AllocationManager::searchAllocation(int allocationId)
{
    if (allocationId <= 0) return nullptr;
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getAllocationId() == allocationId) return &current->data;
        current = current->next;
    }
    return nullptr;
}

bool AllocationManager::allocateAdditionalStock(int allocationId, int quantity)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || quantity <= 0 ||
        allocation->getStatus() == AllocationStatus::CANCELLED ||
        allocation->getStatus() == AllocationStatus::COMPLETED ||
        quantity > allocation->getOutstandingQuantity()) return false;
    Emergency* emergency = emergencyManager->searchEmergency(allocation->getEmergencyId());
    Resource* resource = resourceList->searchResource(allocation->getResourceId());
    if (emergency == nullptr || resource == nullptr || !resource->isActive() ||
        !inventoryManager->isWarehouseActive(allocation->getWarehouseId()) ||
        emergency->getStatus() == EmergencyStatus::RESOLVED || emergency->getStatus() == EmergencyStatus::CANCELLED)
        return false;
    Inventory* inventory = inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
    if (inventory == nullptr || quantity > inventory->getAvailableQuantity()) return false;
    if (!inventoryManager->reserveStock(inventory->getInventoryId(), quantity)) return false;
    if (!allocation->addAllocatedQuantity(quantity))
    {
        inventoryManager->releaseReservedStock(inventory->getInventoryId(), quantity);
        return false;
    }
    return true;
}

bool AllocationManager::dispatchAllocation(int allocationId, int quantity)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || quantity <= 0 ||
        allocation->getStatus() == AllocationStatus::CANCELLED ||
        allocation->getStatus() == AllocationStatus::COMPLETED ||
        quantity > allocation->getAllocatedQuantity() - allocation->getDispatchedQuantity()) return false;
    Emergency* emergency = emergencyManager->searchEmergency(allocation->getEmergencyId());
    if (emergency == nullptr || emergency->getStatus() == EmergencyStatus::CANCELLED ||
        emergency->getStatus() == EmergencyStatus::RESOLVED ||
        !inventoryManager->isWarehouseActive(allocation->getWarehouseId())) return false;
    Inventory* inventory = inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
    if (inventory == nullptr || quantity > inventory->getReservedQuantity()) return false;
    // All model checks are completed before inventory is mutated.
    if (quantity > allocation->getAllocatedQuantity() - allocation->getDispatchedQuantity()) return false;
    if (!inventoryManager->dispatchStock(inventory->getInventoryId(), quantity)) return false;
    if (!allocation->dispatchQuantity(quantity)) return false;
    return true;
}

bool AllocationManager::returnDispatchedStock(int allocationId, int quantity)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || quantity <= 0 || allocation->getStatus() == AllocationStatus::CANCELLED ||
        quantity > allocation->getDispatchedQuantity() - allocation->getReturnedQuantity()) return false;
    Inventory* inventory = inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
    if (inventory == nullptr) return false;
    if (!inventoryManager->returnStock(inventory->getInventoryId(), quantity)) return false;
    if (!allocation->returnQuantity(quantity)) return false;
    return true;
}

bool AllocationManager::completeAllocation(int allocationId)
{
    Allocation* allocation = searchAllocation(allocationId);
    return allocation != nullptr && allocation->completeAllocation();
}

bool AllocationManager::cancelAllocation(int allocationId)
{
    Allocation* allocation = searchAllocation(allocationId);
    if (allocation == nullptr || allocation->getStatus() == AllocationStatus::CANCELLED ||
        allocation->getStatus() == AllocationStatus::COMPLETED) return false;
    int remainingReserved = allocation->getAllocatedQuantity() - allocation->getDispatchedQuantity();
    Inventory* inventory = nullptr;
    if (remainingReserved > 0)
    {
        inventory = inventoryManager->findInventory(allocation->getResourceId(), allocation->getWarehouseId());
        if (inventory == nullptr || remainingReserved > inventory->getReservedQuantity()) return false;
    }
    if (remainingReserved > 0 && !inventoryManager->releaseReservedStock(inventory->getInventoryId(), remainingReserved))
        return false;
    return allocation->cancelAllocation();
}

void AllocationManager::displayAllAllocations() const
{
    if (isEmpty()) { cout << "\nNo allocation records available.\n"; return; }
    Node* current = head;
    while (current != nullptr) { current->data.displayAllocation(); current = current->next; }
    cout << "\nTotal Allocations: " << count << endl;
}

void AllocationManager::displayAllocationsByEmergency(int emergencyId) const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == emergencyId) { current->data.displayAllocation(); found = true; }
        current = current->next;
    }
    if (!found) cout << "\nNo allocations found for this emergency.\n";
}

void AllocationManager::displayAllocationsByStatus(AllocationStatus status) const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getStatus() == status) { current->data.displayAllocation(); found = true; }
        current = current->next;
    }
    if (!found) cout << "\nNo allocations found with this status.\n";
}

void AllocationManager::clearAllocations()
{
    Node* current = head;

    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    count = 0;
    nextAllocationId = 1;
}

bool AllocationManager::loadAllocationRecord(
    int allocationId,
    int emergencyId,
    int resourceId,
    int warehouseId,
    int requestedQuantity,
    int allocatedQuantity,
    int dispatchedQuantity,
    int returnedQuantity,
    AllocationStatus status)
{
    if (allocationId <= 0 || emergencyId <= 0 ||
        resourceId <= 0 || warehouseId <= 0 ||
        requestedQuantity <= 0 ||
        allocatedQuantity < 0 ||
        allocatedQuantity > requestedQuantity ||
        dispatchedQuantity < 0 ||
        dispatchedQuantity > allocatedQuantity ||
        returnedQuantity < 0 ||
        returnedQuantity > dispatchedQuantity ||
        count == numeric_limits<int>::max())
        return false;

    if (status != AllocationStatus::PENDING &&
        status != AllocationStatus::PARTIALLY_ALLOCATED &&
        status != AllocationStatus::ALLOCATED &&
        status != AllocationStatus::DISPATCHED &&
        status != AllocationStatus::COMPLETED &&
        status != AllocationStatus::CANCELLED)
        return false;

    Node* current = head;

    while (current != nullptr)
    {
        if (current->data.getAllocationId() == allocationId)
            return false;

        current = current->next;
    }

    Allocation allocation(
        allocationId, emergencyId, resourceId, warehouseId,
        requestedQuantity, allocatedQuantity, dispatchedQuantity,
        returnedQuantity, status
    );

    Node* newNode = new (nothrow) Node(allocation);

    if (newNode == nullptr)
        return false;

    if (head == nullptr)
        head = newNode;
    else
    {
        current = head;

        while (current->next != nullptr)
            current = current->next;

        current->next = newNode;
    }

    count++;

    if (allocationId >= nextAllocationId)
    {
        if (allocationId == numeric_limits<int>::max())
            nextAllocationId = 0;
        else
            nextAllocationId = allocationId + 1;
    }

    return true;
}