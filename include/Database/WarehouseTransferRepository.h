#ifndef WAREHOUSE_TRANSFER_REPOSITORY_H
#define WAREHOUSE_TRANSFER_REPOSITORY_H

#include "Database/DatabaseConnection.h"
#include "WarehouseTransferManager.h"
#include "WarehouseTransfer.h"

class WarehouseTransferRepository
{
private:
    DatabaseConnection& database;

public:
    explicit WarehouseTransferRepository(DatabaseConnection& db);

    bool saveTransfer(const WarehouseTransfer& transfer);

    bool loadTransfers(WarehouseTransferManager& transferManager);

    bool updateTransfer(const WarehouseTransfer& transfer);

    bool transferExists(int transferId);
};

#endif