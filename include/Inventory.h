#ifndef INVENTORY_H
#define INVENTORY_H

class Inventory
{
private:
    int inventoryId;
    int resourceId;
    int warehouseId;
    int availableQuantity;
    int reservedQuantity;
    int minimumStockLevel;

public:
    Inventory();
    Inventory(int inventoryId, int resourceId, int warehouseId, int availableQuantity, int minimumStockLevel);

    Inventory(int inventoryId, int resourceId, int warehouseId,
              int availableQuantity, int reservedQuantity, int minimumStockLevel);
    int getInventoryId() const;
    int getResourceId() const;
    int getWarehouseId() const;
    int getAvailableQuantity() const;
    int getReservedQuantity() const;
    int getMinimumStockLevel() const;
    int getTotalStock() const;

    bool addStock(int quantity);
    bool removeStock(int quantity);
    bool reserveStock(int quantity);
    bool releaseReservedStock(int quantity);
    bool dispatchStock(int quantity);
    bool returnStock(int quantity);

    void setMinimumStockLevel(int level);
    bool isLowStock() const;
    void displayInventory() const;
};

#endif
