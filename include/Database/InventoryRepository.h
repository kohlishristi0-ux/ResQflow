#ifndef INVENTORY_REPOSITORY_H
#define INVENTORY_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../Inventory.h"
#include "../InventoryManager.h"

class InventoryRepository
{
private:
    DatabaseConnection& database;

public:
    explicit InventoryRepository(DatabaseConnection& db);

    bool saveInventory(const Inventory& inventory);
    bool loadInventory(InventoryManager& inventoryManager);
    bool updateInventory(const Inventory& inventory);
    bool deleteInventory(int inventoryId);
    bool inventoryExists(int inventoryId);

    // Immutable stock-movement ledger (resource_transactions). transactionType must be one of
    // STOCK_IN, RESERVE, RELEASE, DISPATCH, RETURN, TRANSFER_OUT, TRANSFER_IN, ADJUSTMENT.
    bool recordMovement(int resourceId, int warehouseId, const char* transactionType,
                        int quantity, const char* referenceType = nullptr,
                        long long referenceId = 0, int performedBy = 0,
                        const char* notes = nullptr);
};

#endif