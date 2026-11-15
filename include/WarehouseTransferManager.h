#ifndef WAREHOUSE_TRANSFER_MANAGER_H
#define WAREHOUSE_TRANSFER_MANAGER_H

#include "WarehouseTransfer.h"
#include "InventoryManager.h"
#include "ActivityHistory.h"

class WarehouseTransferManager
{
private:
    struct Node
    {
        WarehouseTransfer data;
        Node* next;

        explicit Node(const WarehouseTransfer& transfer);
    };

    Node* head;
    Node* tail;
    int count;
    int nextTransferId;

    InventoryManager* inventoryManager;
    WarehouseLinkedList* warehouseList;
    ActivityHistory* activityHistory;

    bool areWarehousesValid(int sourceId, int destinationId) const;

public:
    WarehouseTransferManager(InventoryManager& inventory,
                             WarehouseLinkedList& warehouses,
                             ActivityHistory* history = nullptr);
    ~WarehouseTransferManager();

    WarehouseTransferManager(const WarehouseTransferManager&) = delete;
    WarehouseTransferManager& operator=(const WarehouseTransferManager&) = delete;

    bool isEmpty() const;
    int getCount() const;

    void connectActivityHistory(ActivityHistory* history);

    bool createTransfer(int sourceWarehouseId, int destinationWarehouseId,
                        int resourceId, int quantity,
                        const std::string& requestedBy,
                        const std::string& remarks = "");

    int peekNextTransferId() const;
    bool removeTransfer(int transferId);   // rollback of a transfer whose database insert failed
    bool hasPendingTransferForWarehouse(int warehouseId) const;

    bool executeTransfer(int transferId);
    bool cancelTransfer(int transferId);

    WarehouseTransfer* searchTransfer(int transferId);
    const WarehouseTransfer* searchTransfer(int transferId) const;

    void displayAllTransfers() const;
    void displayTransfersByWarehouse(int warehouseId) const;
    void displayTransfersByStatus(TransferStatus status) const;
    void clearTransfers();

    bool loadTransferRecord(
    int transferId,
    int sourceWarehouseId,
    int destinationWarehouseId,
    int resourceId,
    int quantity,
    const std::string& requestedBy,
    const std::string& remarks,
    std::time_t requestedAt,
    TransferStatus status,
    std::time_t completedAt
);
};

#endif
