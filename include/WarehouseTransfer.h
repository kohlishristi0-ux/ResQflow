#ifndef WAREHOUSE_TRANSFER_H
#define WAREHOUSE_TRANSFER_H

#include <ctime>
#include <string>

enum class TransferStatus
{
    REQUESTED,
    COMPLETED,
    CANCELLED
};

class WarehouseTransfer
{
private:
    int transferId;
    int sourceWarehouseId;
    int destinationWarehouseId;
    int resourceId;
    int quantity;
    TransferStatus status;
    std::string requestedBy;
    std::string remarks;
    std::time_t requestedAt;
    std::time_t completedAt;

public:
    WarehouseTransfer();
    WarehouseTransfer(int id, int sourceId, int destinationId, int resource,
                      int transferQuantity, const std::string& actor,
                      const std::string& transferRemarks, std::time_t requestTime);

    int getTransferId() const;
    int getSourceWarehouseId() const;
    int getDestinationWarehouseId() const;
    int getResourceId() const;
    int getQuantity() const;
    TransferStatus getStatus() const;
    std::string getRequestedBy() const;
    std::string getRemarks() const;
    std::time_t getRequestedAt() const;
    std::time_t getCompletedAt() const;

    void markCompleted(std::time_t completionTime);
    void markCancelled();
    bool restoreStatus(TransferStatus restoredStatus, std::time_t completionTime);
    void display() const;
};

const char* transferStatusToString(TransferStatus status);

#endif
