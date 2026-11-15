#include "Database/WarehouseTransferRepository.h"
#include <mysql.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

WarehouseTransferRepository::WarehouseTransferRepository(DatabaseConnection& db)
    : database(db)
{
}

bool WarehouseTransferRepository::saveTransfer(
    const WarehouseTransfer& transfer)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr)
        return false;

    string notes = "REQUESTED_BY=" + transfer.getRequestedBy()
                 + "\nREMARKS=" + transfer.getRemarks();

    if (notes.length() > 500)
        return false;

    char* escapedNotes = new char[notes.length() * 2 + 1];

    unsigned long notesLength = mysql_real_escape_string(
        connection,
        escapedNotes,
        notes.c_str(),
        static_cast<unsigned long>(notes.length())
    );

    string safeNotes(escapedNotes, notesLength);
    delete[] escapedNotes;

    string query =
        "INSERT INTO warehouse_transfers "
        "(transfer_id, source_warehouse_id, destination_warehouse_id, "
        "status, requested_by, notes, created_at, completed_at) VALUES ("
        + to_string(transfer.getTransferId()) + ", "
        + to_string(transfer.getSourceWarehouseId()) + ", "
        + to_string(transfer.getDestinationWarehouseId()) + ", "
        "'PENDING', NULL, '" + safeNotes + "', "
        "FROM_UNIXTIME(" + to_string(
            static_cast<long long>(transfer.getRequestedAt())) + "), NULL)";

    if (!database.beginTransaction())
        return false;

    if (mysql_query(connection, query.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    string detailQuery =
        "INSERT INTO warehouse_transfer_details "
        "(transfer_id, resource_id, quantity) VALUES ("
        + to_string(transfer.getTransferId()) + ", "
        + to_string(transfer.getResourceId()) + ", "
        + to_string(transfer.getQuantity()) + ")";

    if (mysql_query(connection, detailQuery.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    if (!database.commitTransaction())
    {
        database.rollbackTransaction();
        return false;
    }

    return true;
}

bool WarehouseTransferRepository::loadTransfers(
    WarehouseTransferManager& transferManager)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr)
        return false;

    const char* query =
        "SELECT wt.transfer_id, wt.source_warehouse_id, "
        "wt.destination_warehouse_id, wd.resource_id, wd.quantity, "
        "wt.status, wt.notes, UNIX_TIMESTAMP(wt.created_at), "
        "UNIX_TIMESTAMP(wt.completed_at) "
        "FROM warehouse_transfers wt "
        "INNER JOIN warehouse_transfer_details wd "
        "ON wt.transfer_id = wd.transfer_id "
        "ORDER BY wt.transfer_id";

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    transferManager.clearTransfers();

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int transferId = atoi(row[0]);
        int sourceId = atoi(row[1]);
        int destinationId = atoi(row[2]);
        int resourceId = atoi(row[3]);
        int quantity = atoi(row[4]);

        TransferStatus status;

        if (string(row[5]) == "PENDING")
            status = TransferStatus::REQUESTED;
        else if (string(row[5]) == "COMPLETED")
            status = TransferStatus::COMPLETED;
        else if (string(row[5]) == "CANCELLED")
            status = TransferStatus::CANCELLED;
        else
        {
            mysql_free_result(result);
            transferManager.clearTransfers();
            return false;
        }

        string notes = row[6] != nullptr ? row[6] : "";

        string requestedBy;
        string remarks;

        const string requesterPrefix = "REQUESTED_BY=";
        const string remarksPrefix = "\nREMARKS=";

        size_t requesterPosition = notes.find(requesterPrefix);
        size_t remarksPosition = notes.find(remarksPrefix);

        if (requesterPosition != string::npos &&
            remarksPosition != string::npos &&
            remarksPosition > requesterPosition)
        {
            requestedBy = notes.substr(
                requesterPosition + requesterPrefix.length(),
                remarksPosition -
                (requesterPosition + requesterPrefix.length())
            );

            remarks = notes.substr(
                remarksPosition + remarksPrefix.length()
            );
        }
        else
        {
            mysql_free_result(result);
            transferManager.clearTransfers();
            return false;
        }

        time_t requestedAt = row[7] != nullptr
            ? static_cast<time_t>(atoll(row[7]))
            : 0;

        time_t completedAt = row[8] != nullptr
            ? static_cast<time_t>(atoll(row[8]))
            : 0;

        if (!transferManager.loadTransferRecord(
                transferId,
                sourceId,
                destinationId,
                resourceId,
                quantity,
                requestedBy,
                remarks,
                requestedAt,
                status,
                completedAt))
        {
            mysql_free_result(result);
            transferManager.clearTransfers();
            return false;
        }
    }

    mysql_free_result(result);
    return true;
}

bool WarehouseTransferRepository::updateTransfer(
    const WarehouseTransfer& transfer)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr)
        return false;

    string status;

    if (transfer.getStatus() == TransferStatus::REQUESTED)
        status = "PENDING";
    else if (transfer.getStatus() == TransferStatus::COMPLETED)
        status = "COMPLETED";
    else if (transfer.getStatus() == TransferStatus::CANCELLED)
        status = "CANCELLED";
    else
        return false;

    string query =
        "UPDATE warehouse_transfers SET "
        "status = '" + status + "', "
        "completed_at = ";

    if (transfer.getStatus() == TransferStatus::COMPLETED)
    {
        query += "FROM_UNIXTIME("
              + to_string(static_cast<long long>(
                    transfer.getCompletedAt())) + ")";
    }
    else
    {
        query += "NULL";
    }

    // Only a PENDING transfer may change state (guards against double execution / cancel-after-complete).
    query += " WHERE transfer_id = "
          + to_string(transfer.getTransferId())
          + " AND status = 'PENDING'";

    if (!database.beginTransaction())
        return false;

    if (mysql_query(connection, query.c_str()) != 0 ||
        mysql_affected_rows(connection) != 1)
    {
        cerr << "[WarehouseTransferRepository] status update failed: "
             << database.getLastError() << '\n';
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool WarehouseTransferRepository::transferExists(int transferId)
{
    MYSQL* connection = database.getConnection();

    if (connection == nullptr || transferId <= 0)
        return false;

    string query =
        "SELECT transfer_id FROM warehouse_transfers "
        "WHERE transfer_id = " + to_string(transferId);

    if (mysql_query(connection, query.c_str()) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    bool exists = mysql_num_rows(result) > 0;

    mysql_free_result(result);

    return exists;
}