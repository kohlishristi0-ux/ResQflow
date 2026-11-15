#include "../include/WarehouseTransferManager.h"
#include <iostream>
#include <limits>
#include <new>
#include <ctime>

using namespace std;

WarehouseTransferManager::Node::Node(const WarehouseTransfer& transfer)
    : data(transfer), next(nullptr)
{
}

WarehouseTransferManager::WarehouseTransferManager(InventoryManager& inventory,
                                                     WarehouseLinkedList& warehouses,
                                                     ActivityHistory* history)
    : head(nullptr), tail(nullptr), count(0), nextTransferId(1),
      inventoryManager(&inventory), warehouseList(&warehouses), activityHistory(history)
{
}

WarehouseTransferManager::~WarehouseTransferManager()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* nextNode = current->next;
        delete current;
        current = nextNode;
    }
}

bool WarehouseTransferManager::isEmpty() const
{
    return head == nullptr;
}

int WarehouseTransferManager::getCount() const
{
    return count;
}

void WarehouseTransferManager::connectActivityHistory(ActivityHistory* history)
{
    activityHistory = history;
}

bool WarehouseTransferManager::areWarehousesValid(int sourceId, int destinationId) const
{
    if (sourceId <= 0 || destinationId <= 0 || sourceId == destinationId)
        return false;

    const Warehouse* source = warehouseList->searchWarehouse(sourceId);
    const Warehouse* destination = warehouseList->searchWarehouse(destinationId);

    return source != nullptr && destination != nullptr &&
           source->isActive() && destination->isActive();
}

bool WarehouseTransferManager::createTransfer(int sourceWarehouseId,
                                              int destinationWarehouseId,
                                              int resourceId, int quantity,
                                              const string& requestedBy,
                                              const string& remarks)
{
    if (!areWarehousesValid(sourceWarehouseId, destinationWarehouseId) ||
        resourceId <= 0 || quantity <= 0 || requestedBy.empty() ||
        count == numeric_limits<int>::max() || nextTransferId <= 0 ||
        nextTransferId == numeric_limits<int>::max())
    {
        return false;
    }

    const Inventory* sourceInventory = inventoryManager->findInventory(resourceId, sourceWarehouseId);
    const Inventory* destinationInventory = inventoryManager->findInventory(resourceId, destinationWarehouseId);

    if (sourceInventory == nullptr || destinationInventory == nullptr ||
        quantity > sourceInventory->getAvailableQuantity())
    {
        return false;
    }

    WarehouseTransfer transfer(nextTransferId, sourceWarehouseId, destinationWarehouseId,
                               resourceId, quantity, requestedBy, remarks, time(nullptr));
    Node* newNode = new (nothrow) Node(transfer);
    if (newNode == nullptr)
        return false;

    if (tail == nullptr)
        head = newNode;
    else
        tail->next = newNode;

    tail = newNode;
    ++count;
    ++nextTransferId;
    return true;
}

int WarehouseTransferManager::peekNextTransferId() const
{
    return nextTransferId;
}

bool WarehouseTransferManager::removeTransfer(int transferId)
{
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->data.getTransferId() != transferId)
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
    if (current == tail)
        tail = previous;
    delete current;
    --count;
    return true;
}

bool WarehouseTransferManager::hasPendingTransferForWarehouse(int warehouseId) const
{
    for (const Node* current = head; current != nullptr; current = current->next)
        if (current->data.getStatus() == TransferStatus::REQUESTED &&
            (current->data.getSourceWarehouseId() == warehouseId ||
             current->data.getDestinationWarehouseId() == warehouseId))
            return true;
    return false;
}

bool WarehouseTransferManager::executeTransfer(int transferId)
{
    WarehouseTransfer* transfer = searchTransfer(transferId);
    if (transfer == nullptr || transfer->getStatus() != TransferStatus::REQUESTED)
        return false;

    if (!areWarehousesValid(transfer->getSourceWarehouseId(),
                            transfer->getDestinationWarehouseId()))
    {
        return false;
    }

    if (!inventoryManager->transferStock(transfer->getSourceWarehouseId(),
                                         transfer->getDestinationWarehouseId(),
                                         transfer->getResourceId(),
                                         transfer->getQuantity()))
    {
        return false;
    }

    transfer->markCompleted(time(nullptr));

    if (activityHistory != nullptr)
    {
        activityHistory->addActivity(
            ActivityType::WAREHOUSE_TRANSFERRED,
            "Warehouse stock transfer completed. Transfer ID: " + to_string(transferId),
            transfer->getRequestedBy(), -1, -1, transfer->getResourceId(),
            transfer->getSourceWarehouseId(), transfer->getDestinationWarehouseId(),
            -1, transfer->getQuantity());
    }

    return true;
}

bool WarehouseTransferManager::cancelTransfer(int transferId)
{
    WarehouseTransfer* transfer = searchTransfer(transferId);
    if (transfer == nullptr || transfer->getStatus() != TransferStatus::REQUESTED)
        return false;

    transfer->markCancelled();
    return transfer->getStatus() == TransferStatus::CANCELLED;
}

WarehouseTransfer* WarehouseTransferManager::searchTransfer(int transferId)
{
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getTransferId() == transferId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

const WarehouseTransfer* WarehouseTransferManager::searchTransfer(int transferId) const
{
    const Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getTransferId() == transferId)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

void WarehouseTransferManager::displayAllTransfers() const
{
    if (isEmpty())
    {
        cout << "No warehouse transfers recorded.\n";
        return;
    }

    const Node* current = head;
    while (current != nullptr)
    {
        current->data.display();
        current = current->next;
    }
}

void WarehouseTransferManager::displayTransfersByWarehouse(int warehouseId) const
{
    const Node* current = head;
    bool found = false;

    while (current != nullptr)
    {
        if (current->data.getSourceWarehouseId() == warehouseId ||
            current->data.getDestinationWarehouseId() == warehouseId)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }

    if (!found)
        cout << "No transfers found for this warehouse.\n";
}

void WarehouseTransferManager::displayTransfersByStatus(TransferStatus status) const
{
    const Node* current = head;
    bool found = false;

    while (current != nullptr)
    {
        if (current->data.getStatus() == status)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }

    if (!found)
        cout << "No transfers found for this status.\n";
}

void WarehouseTransferManager::clearTransfers()
{
    Node* current = head;

    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    tail = nullptr;
    count = 0;
    nextTransferId = 1;
}

bool WarehouseTransferManager::loadTransferRecord(
    int transferId,
    int sourceWarehouseId,
    int destinationWarehouseId,
    int resourceId,
    int quantity,
    const string& requestedBy,
    const string& remarks,
    time_t requestedAt,
    TransferStatus status,
    time_t completedAt)
{
    if (transferId <= 0 ||
        sourceWarehouseId <= 0 ||
        destinationWarehouseId <= 0 ||
        sourceWarehouseId == destinationWarehouseId ||
        resourceId <= 0 ||
        quantity <= 0 ||
        requestedBy.empty() ||
        requestedAt <= 0 ||
        count == numeric_limits<int>::max())
        return false;

    if (searchTransfer(transferId) != nullptr)
        return false;

    WarehouseTransfer transfer(
        transferId,
        sourceWarehouseId,
        destinationWarehouseId,
        resourceId,
        quantity,
        requestedBy,
        remarks,
        requestedAt
    );

    if (!transfer.restoreStatus(status, completedAt))
        return false;

    Node* node = new (nothrow) Node(transfer);

    if (!node)
        return false;

    if (!head)
        head = node;
    else
        tail->next = node;

    tail = node;
    ++count;

    if (transferId >= nextTransferId)
    {
        if (transferId == numeric_limits<int>::max())
            nextTransferId = 0;
        else
            nextTransferId = transferId + 1;
    }

    return true;
}