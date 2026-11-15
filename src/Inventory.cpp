#include <iostream>
#include <limits>
#include "../include/Inventory.h"

using namespace std;

Inventory::Inventory()
{
    inventoryId = 0;
    resourceId = 0;
    warehouseId = 0;
    availableQuantity = 0;
    reservedQuantity = 0;
    minimumStockLevel = 0;
}

Inventory::Inventory(int inventoryId, int resourceId, int warehouseId,
                     int availableQuantity, int minimumStockLevel)
    : Inventory(inventoryId, resourceId, warehouseId,
                availableQuantity, 0, minimumStockLevel)
{
}

Inventory::Inventory(int inventoryId, int resourceId, int warehouseId,
                     int availableQuantity, int reservedQuantity, int minimumStockLevel)
{
    this->inventoryId = inventoryId;
    this->resourceId = resourceId;
    this->warehouseId = warehouseId;
    this->availableQuantity = availableQuantity >= 0 ? availableQuantity : 0;
    this->reservedQuantity = reservedQuantity >= 0 ? reservedQuantity : 0;
    this->minimumStockLevel = minimumStockLevel >= 0 ? minimumStockLevel : 0;
}

int Inventory::getInventoryId() const {
    return inventoryId;
}
int Inventory::getResourceId() const {
    return resourceId;
}
int Inventory::getWarehouseId() const { return warehouseId; }
int Inventory::getAvailableQuantity() const { return availableQuantity; }
int Inventory::getReservedQuantity() const { return reservedQuantity; }
int Inventory::getMinimumStockLevel() const { return minimumStockLevel; }

int Inventory::getTotalStock() const
{
    // All mutating operations preserve this invariant: total stock never exceeds INT_MAX.
    return availableQuantity + reservedQuantity;
}

bool Inventory::addStock(int quantity)
{
    if (quantity <= 0 || quantity > numeric_limits<int>::max() - getTotalStock())
        return false;
    availableQuantity += quantity;
    return true;
}

bool Inventory::removeStock(int quantity)
{
    if (quantity <= 0 || quantity > availableQuantity)
        return false;
    availableQuantity -= quantity;
    return true;
}

bool Inventory::reserveStock(int quantity)
{
    if (quantity <= 0 || quantity > availableQuantity)
        return false;
    availableQuantity -= quantity;
    reservedQuantity += quantity;
    return true;
}

bool Inventory::releaseReservedStock(int quantity)
{
    if (quantity <= 0 || quantity > reservedQuantity)
        return false;
    reservedQuantity -= quantity;
    availableQuantity += quantity;
    return true;
}

bool Inventory::dispatchStock(int quantity)
{
    if (quantity <= 0 || quantity > reservedQuantity)
        return false;
    reservedQuantity -= quantity;
    return true;
}

bool Inventory::returnStock(int quantity)
{
    if (quantity <= 0 || quantity > numeric_limits<int>::max() - getTotalStock())
        return false;
    availableQuantity += quantity;
    return true;
}

void Inventory::setMinimumStockLevel(int level)
{
    if (level >= 0)
        minimumStockLevel = level;
}

bool Inventory::isLowStock() const
{
    return availableQuantity <= minimumStockLevel;
}

void Inventory::displayInventory() const
{
    cout << "\n--------------------------------";
    cout << "\nInventory ID: " << inventoryId;
    cout << "\nResource ID: " << resourceId;
    cout << "\nWarehouse ID: " << warehouseId;
    cout << "\nAvailable Quantity: " << availableQuantity;
    cout << "\nReserved Quantity: " << reservedQuantity;
    cout << "\nTotal Physical Stock: " << getTotalStock();
    cout << "\nMinimum Stock Level: " << minimumStockLevel;
    cout << "\nStock Status: " << (isLowStock() ? "Low Stock" : "Sufficient Stock");
    cout << "\n--------------------------------\n";
}
