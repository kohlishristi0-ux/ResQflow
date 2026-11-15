#include "../include/WarehouseTransfer.h"
#include <iostream>
#include <ctime>

using namespace std;

WarehouseTransfer::WarehouseTransfer()
    : transferId(0), sourceWarehouseId(0), destinationWarehouseId(0),
      resourceId(0), quantity(0), status(TransferStatus::REQUESTED),
      requestedAt(0), completedAt(0)
{
}

WarehouseTransfer::WarehouseTransfer(int id, int sourceId, int destinationId,
                                     int resource, int transferQuantity,
                                     const string& actor, const string& transferRemarks,
                                     time_t requestTime)
    : transferId(id), sourceWarehouseId(sourceId), destinationWarehouseId(destinationId),
      resourceId(resource), quantity(transferQuantity), status(TransferStatus::REQUESTED),
      requestedBy(actor), remarks(transferRemarks), requestedAt(requestTime), completedAt(0)
{
}

int WarehouseTransfer::getTransferId() const { return transferId; }
int WarehouseTransfer::getSourceWarehouseId() const { return sourceWarehouseId; }
int WarehouseTransfer::getDestinationWarehouseId() const { return destinationWarehouseId; }
int WarehouseTransfer::getResourceId() const { return resourceId; }
int WarehouseTransfer::getQuantity() const { return quantity; }
TransferStatus WarehouseTransfer::getStatus() const { return status; }
string WarehouseTransfer::getRequestedBy() const { return requestedBy; }
string WarehouseTransfer::getRemarks() const { return remarks; }
time_t WarehouseTransfer::getRequestedAt() const { return requestedAt; }
time_t WarehouseTransfer::getCompletedAt() const { return completedAt; }

void WarehouseTransfer::markCompleted(time_t completionTime)
{
    if (status == TransferStatus::REQUESTED && completionTime >= 0)
    {
        status = TransferStatus::COMPLETED;
        completedAt = completionTime;
    }
}

void WarehouseTransfer::markCancelled()
{
    if (status == TransferStatus::REQUESTED)
        status = TransferStatus::CANCELLED;
}

bool WarehouseTransfer::restoreStatus(
    TransferStatus restoredStatus,
    time_t completionTime)
{
    if (restoredStatus == TransferStatus::REQUESTED)
    {
        status = restoredStatus;
        completedAt = 0;
        return true;
    }

    if (restoredStatus == TransferStatus::COMPLETED)
    {
        if (completionTime <= 0)
            return false;

        status = restoredStatus;
        completedAt = completionTime;
        return true;
    }

    if (restoredStatus == TransferStatus::CANCELLED)
    {
        status = restoredStatus;
        completedAt = 0;
        return true;
    }

    return false;
}

const char* transferStatusToString(TransferStatus transferStatus)
{
    switch (transferStatus)
    {
        case TransferStatus::REQUESTED: return "REQUESTED";
        case TransferStatus::COMPLETED: return "COMPLETED";
        case TransferStatus::CANCELLED: return "CANCELLED";
    }
    return "UNKNOWN";
}

void WarehouseTransfer::display() const
{
    cout << "\n----------------------------------------\n";
    cout << "Transfer ID: " << transferId << '\n';
    cout << "Source Warehouse ID: " << sourceWarehouseId << '\n';
    cout << "Destination Warehouse ID: " << destinationWarehouseId << '\n';
    cout << "Resource ID: " << resourceId << '\n';
    cout << "Quantity: " << quantity << '\n';
    cout << "Status: " << transferStatusToString(status) << '\n';
    cout << "Requested By: " << requestedBy << '\n';
    cout << "Remarks: " << remarks << '\n';
    cout << "Requested At: " << requestedAt << '\n';
    cout << "Completed At: " << completedAt << '\n';
    cout << "----------------------------------------\n";
}
