#include <iostream>
#include <limits>
#include <new>
#include "../include/InventoryManager.h"

using namespace std;

InventoryManager::InventoryManager(ResourceLinkedList& resources, WarehouseLinkedList& warehouses)
{
    head = nullptr;
    count = 0;
    nextInventoryId = 1;
    resourceList = &resources;
    warehouseList = &warehouses;
}

InventoryManager::~InventoryManager()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }
}

bool InventoryManager::isEmpty() const { return head == nullptr; }

bool InventoryManager::isWarehouseActive(int warehouseId) const
{
    const Warehouse* warehouse = warehouseList->searchWarehouse(warehouseId);
    return warehouse != nullptr && warehouse->isActive();
}

int InventoryManager::getCount() const { return count; }

bool InventoryManager::addInventory(int resourceId, int warehouseId, int quantity, int minimumStockLevel)
{
    Resource* resource = resourceList->searchResource(resourceId);
    Warehouse* warehouse = warehouseList->searchWarehouse(warehouseId);
    if (resource == nullptr || warehouse == nullptr || !resource->isActive() || !warehouse->isActive())
        return false;
    if (resourceId <= 0 || warehouseId <= 0 || quantity < 0 || minimumStockLevel < 0 ||
        nextInventoryId <= 0 || nextInventoryId == numeric_limits<int>::max() || count == numeric_limits<int>::max())
        return false;
    if (findInventory(resourceId, warehouseId) != nullptr)
        return false;

    Inventory inventory(nextInventoryId, resourceId, warehouseId, quantity, minimumStockLevel);
    Node* newNode = new (nothrow) Node(inventory);
    if (newNode == nullptr)
        return false;

    if (head == nullptr)
        head = newNode;
    else
    {
        Node* current = head;
        while (current->next != nullptr)
            current = current->next;
        current->next = newNode;
    }
    ++count;
    ++nextInventoryId;
    return true;
}

bool InventoryManager::loadInventoryRecord(
    int inventoryId, int resourceId, int warehouseId,
    int availableQuantity, int reservedQuantity,
    int minimumStockLevel)
{
    Resource* resource = resourceList->searchResource(resourceId);
    Warehouse* warehouse = warehouseList->searchWarehouse(warehouseId);

    if (inventoryId <= 0 ||
        inventoryId == numeric_limits<int>::max() ||
        resourceId <= 0 || warehouseId <= 0 ||
        availableQuantity < 0 || reservedQuantity < 0 ||
        minimumStockLevel < 0 ||
        reservedQuantity > numeric_limits<int>::max() - availableQuantity ||
        resource == nullptr || warehouse == nullptr ||
        !resource->isActive() || !warehouse->isActive() ||
        searchInventory(inventoryId) != nullptr ||
        findInventory(resourceId, warehouseId) != nullptr ||
        count == numeric_limits<int>::max())
    {
        return false;
    }

    Inventory inventory(
        inventoryId,
        resourceId,
        warehouseId,
        availableQuantity,
        reservedQuantity,
        minimumStockLevel
    );

    Node* newNode = new (nothrow) Node(inventory);

    if (newNode == nullptr)
        return false;

    if (head == nullptr)
        head = newNode;
    else
    {
        Node* current = head;

        while (current->next != nullptr)
            current = current->next;

        current->next = newNode;
    }

    ++count;

    if (inventoryId >= nextInventoryId)
        nextInventoryId = inventoryId + 1;

    return true;
}

bool InventoryManager::deleteInventory(int inventoryId)
{
    if (inventoryId <= 0 || head == nullptr)
        return false;
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getInventoryId() != inventoryId)
    {
        previous = current;
        current = current->next;
    }
    if (current == nullptr || current->data.getTotalStock() != 0)
        return false;
    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    delete current;
    --count;
    return true;
}

bool InventoryManager::discardInventory(int inventoryId)
{
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getInventoryId() != inventoryId)
    {
        previous = current;
        current = current->next;
    }
    if (current == nullptr)
        return false;
    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    delete current;
    --count;
    return true;
}

bool InventoryManager::hasStockForResource(int resourceId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getResourceId() == resourceId && current->data.getTotalStock() > 0)
            return true;
    return false;
}

bool InventoryManager::hasStockInWarehouse(int warehouseId) const
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getWarehouseId() == warehouseId && current->data.getTotalStock() > 0)
            return true;
    return false;
}

int InventoryManager::collectByResource(int resourceId, Inventory** out, int capacity)
{
    int found = 0;
    for (Node* current = head; current != nullptr && found < capacity; current = current->next)
        if (current->data.getResourceId() == resourceId)
            out[found++] = &current->data;
    return found;
}

Inventory* InventoryManager::searchInventory(int inventoryId)
{
    if (inventoryId <= 0) return nullptr;
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getInventoryId() == inventoryId) return &current->data;
        current = current->next;
    }
    return nullptr;
}

Inventory* InventoryManager::findInventory(int resourceId, int warehouseId)
{
    if (resourceId <= 0 || warehouseId <= 0) return nullptr;
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getResourceId() == resourceId && current->data.getWarehouseId() == warehouseId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

bool InventoryManager::addStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->addStock(quantity);
}
bool InventoryManager::removeStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->removeStock(quantity);
}
bool InventoryManager::reserveStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->reserveStock(quantity);
}
bool InventoryManager::releaseReservedStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->releaseReservedStock(quantity);
}
bool InventoryManager::dispatchStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->dispatchStock(quantity);
}
bool InventoryManager::returnStock(int inventoryId, int quantity)
{
    Inventory* inventory = searchInventory(inventoryId);
    return inventory != nullptr && inventory->returnStock(quantity);
}

bool InventoryManager::transferStock(int sourceWarehouseId, int destinationWarehouseId, int resourceId, int quantity)
{
    if (quantity <= 0 || sourceWarehouseId <= 0 || destinationWarehouseId <= 0 || resourceId <= 0 ||
        sourceWarehouseId == destinationWarehouseId)
        return false;
    if (!isWarehouseActive(sourceWarehouseId) || !isWarehouseActive(destinationWarehouseId))
        return false;
    Inventory* source = findInventory(resourceId, sourceWarehouseId);
    Inventory* destination = findInventory(resourceId, destinationWarehouseId);
    if (source == nullptr || destination == nullptr || quantity > source->getAvailableQuantity())
        return false;
    if (quantity > numeric_limits<int>::max() - destination->getTotalStock())
        return false;

    // Validate both sides before mutation. Roll back the source if the destination update fails.
    if (!source->removeStock(quantity))
        return false;
    if (!destination->addStock(quantity))
    {
        source->addStock(quantity);
        return false;
    }
    return true;
}

void InventoryManager::displayAllInventory() const
{
    if (isEmpty()) { cout << "\nNo inventory records available.\n"; return; }
    Node* current = head;
    while (current != nullptr) { current->data.displayInventory(); current = current->next; }
    cout << "\nTotal Inventory Records: " << count << endl;
}

void InventoryManager::displayInventoryByWarehouse(int warehouseId) const
{
    Node* current = head; bool found = false;
    while (current != nullptr)
    {
        if (current->data.getWarehouseId() == warehouseId) { current->data.displayInventory(); found = true; }
        current = current->next;
    }
    if (!found) cout << "\nNo inventory found for this warehouse.\n";
}

void InventoryManager::displayInventoryByResource(int resourceId) const
{
    Node* current = head; bool found = false;
    while (current != nullptr)
    {
        if (current->data.getResourceId() == resourceId) { current->data.displayInventory(); found = true; }
        current = current->next;
    }
    if (!found) cout << "\nNo inventory found for this resource.\n";
}

void InventoryManager::displayLowStock() const
{
    Node* current = head; bool found = false;
    while (current != nullptr)
    {
        if (current->data.isLowStock()) { current->data.displayInventory(); found = true; }
        current = current->next;
    }
    if (!found) cout << "\nNo low-stock records found.\n";
}

void InventoryManager::clearInventory()
{
    while (head != nullptr)
    {
        Node* temp = head;
        head = head->next;
        delete temp;
    }

    count = 0;
    nextInventoryId = 1;
}
