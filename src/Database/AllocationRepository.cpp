#include "Database/AllocationRepository.h"
#include <mysql.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>

AllocationRepository::AllocationRepository(DatabaseConnection& db)
    : database(db)
{
}

bool AllocationRepository::saveAllocation(const Allocation& allocation)
{
    if (!database.isConnected() ||
        allocation.getAllocationId() <= 0 ||
        allocation.getEmergencyId() <= 0 ||
        allocation.getResourceId() <= 0 ||
        allocation.getWarehouseId() <= 0 ||
        allocation.getRequestedQuantity() <= 0 ||
        allocation.getAllocatedQuantity() < 0 ||
        allocation.getAllocatedQuantity() > allocation.getRequestedQuantity() ||
        allocation.getDispatchedQuantity() < 0 ||
        allocation.getDispatchedQuantity() > allocation.getAllocatedQuantity() ||
        allocation.getReturnedQuantity() < 0 ||
        allocation.getReturnedQuantity() > allocation.getDispatchedQuantity())
        return false;

    const char* query =
        "INSERT INTO allocations "
        "(allocation_id, emergency_id, resource_id, warehouse_id, "
        "requested_quantity, allocated_quantity, dispatched_quantity, "
        "returned_quantity, status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int id = allocation.getAllocationId();
    int emergency = allocation.getEmergencyId();
    int resource = allocation.getResourceId();
    int warehouse = allocation.getWarehouseId();
    int requested = allocation.getRequestedQuantity();
    int allocated = allocation.getAllocatedQuantity();
    int dispatched = allocation.getDispatchedQuantity();
    int returned = allocation.getReturnedQuantity();

    const char* status = nullptr;

    switch (allocation.getStatus())
    {
        case AllocationStatus::PENDING: status = "PENDING"; break;
        case AllocationStatus::PARTIALLY_ALLOCATED: status = "PARTIALLY_ALLOCATED"; break;
        case AllocationStatus::ALLOCATED: status = "ALLOCATED"; break;
        case AllocationStatus::DISPATCHED: status = "DISPATCHED"; break;
        case AllocationStatus::COMPLETED: status = "COMPLETED"; break;
        case AllocationStatus::CANCELLED: status = "CANCELLED"; break;
    }

    if (status == nullptr)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    unsigned long statusLength = static_cast<unsigned long>(strlen(status));

    MYSQL_BIND params[9];
    memset(params, 0, sizeof(params));

    int* values[8] = {
        &id, &emergency, &resource, &warehouse,
        &requested, &allocated, &dispatched, &returned
    };

    for (int i = 0; i < 8; i++)
    {
        params[i].buffer_type = MYSQL_TYPE_LONG;
        params[i].buffer = values[i];
    }

    params[8].buffer_type = MYSQL_TYPE_STRING;
    params[8].buffer = const_cast<char*>(status);
    params[8].buffer_length = statusLength;
    params[8].length = &statusLength;

    bool success =
        mysql_stmt_bind_param(stmt, params) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool AllocationRepository::loadAllocations(AllocationManager& allocationManager)
{
    if (!database.isConnected())
        return false;

    const char* query =
        "SELECT allocation_id, emergency_id, resource_id, warehouse_id, "
        "requested_quantity, allocated_quantity, dispatched_quantity, "
        "returned_quantity, status "
        "FROM allocations ORDER BY allocation_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    allocationManager.clearAllocations();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        for (int i = 0; i < 9; i++)
        {
            if (row[i] == nullptr)
            {
                success = false;
                break;
            }
        }

        if (!success)
            break;

        AllocationStatus status;

        if (strcmp(row[8], "PENDING") == 0)
            status = AllocationStatus::PENDING;
        else if (strcmp(row[8], "PARTIALLY_ALLOCATED") == 0)
            status = AllocationStatus::PARTIALLY_ALLOCATED;
        else if (strcmp(row[8], "ALLOCATED") == 0)
            status = AllocationStatus::ALLOCATED;
        else if (strcmp(row[8], "DISPATCHED") == 0)
            status = AllocationStatus::DISPATCHED;
        else if (strcmp(row[8], "COMPLETED") == 0)
            status = AllocationStatus::COMPLETED;
        else if (strcmp(row[8], "CANCELLED") == 0)
            status = AllocationStatus::CANCELLED;
        else
        {
            success = false;
            break;
        }

        if (!allocationManager.loadAllocationRecord(
                atoi(row[0]),
                atoi(row[1]),
                atoi(row[2]),
                atoi(row[3]),
                atoi(row[4]),
                atoi(row[5]),
                atoi(row[6]),
                atoi(row[7]),
                status))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        allocationManager.clearAllocations();

    return success;
}

bool AllocationRepository::updateAllocation(const Allocation& allocation)
{
    if (!database.isConnected() ||
        allocation.getAllocationId() <= 0 ||
        allocation.getAllocatedQuantity() < 0 ||
        allocation.getAllocatedQuantity() > allocation.getRequestedQuantity() ||
        allocation.getDispatchedQuantity() < 0 ||
        allocation.getDispatchedQuantity() > allocation.getAllocatedQuantity() ||
        allocation.getReturnedQuantity() < 0 ||
        allocation.getReturnedQuantity() > allocation.getDispatchedQuantity())
        return false;

    // MySQL reports 0 affected rows for an UPDATE that changes nothing, so existence is verified explicitly.
    if (!allocationExists(allocation.getAllocationId()))
        return false;

    const char* query =
        "UPDATE allocations SET allocated_quantity = ?, "
        "dispatched_quantity = ?, returned_quantity = ?, status = ? "
        "WHERE allocation_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int allocated = allocation.getAllocatedQuantity();
    int dispatched = allocation.getDispatchedQuantity();
    int returned = allocation.getReturnedQuantity();
    int id = allocation.getAllocationId();

    const char* status = nullptr;

    switch (allocation.getStatus())
    {
        case AllocationStatus::PENDING: status = "PENDING"; break;
        case AllocationStatus::PARTIALLY_ALLOCATED: status = "PARTIALLY_ALLOCATED"; break;
        case AllocationStatus::ALLOCATED: status = "ALLOCATED"; break;
        case AllocationStatus::DISPATCHED: status = "DISPATCHED"; break;
        case AllocationStatus::COMPLETED: status = "COMPLETED"; break;
        case AllocationStatus::CANCELLED: status = "CANCELLED"; break;
    }

    if (status == nullptr)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    unsigned long statusLength = static_cast<unsigned long>(strlen(status));

    MYSQL_BIND params[5];
    memset(params, 0, sizeof(params));

    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &allocated;

    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &dispatched;

    params[2].buffer_type = MYSQL_TYPE_LONG;
    params[2].buffer = &returned;

    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = const_cast<char*>(status);
    params[3].buffer_length = statusLength;
    params[3].length = &statusLength;

    params[4].buffer_type = MYSQL_TYPE_LONG;
    params[4].buffer = &id;

    bool success =
        mysql_stmt_bind_param(stmt, params) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool AllocationRepository::allocationExists(int allocationId)
{
    if (!database.isConnected() || allocationId <= 0)
        return false;

    const char* query =
        "SELECT allocation_id FROM allocations WHERE allocation_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND input;
    memset(&input, 0, sizeof(input));

    input.buffer_type = MYSQL_TYPE_LONG;
    input.buffer = &allocationId;

    int returnedId = 0;

    MYSQL_BIND output;
    memset(&output, 0, sizeof(output));

    output.buffer_type = MYSQL_TYPE_LONG;
    output.buffer = &returnedId;

    bool exists = false;

    if (mysql_stmt_bind_param(stmt, &input) == 0 &&
        mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_bind_result(stmt, &output) == 0 &&
        mysql_stmt_store_result(stmt) == 0)
    {
        exists = mysql_stmt_fetch(stmt) == 0;
    }

    mysql_stmt_close(stmt);
    return exists;
}

bool AllocationRepository::syncRequirement(int emergencyId, int resourceId)
{
    if (!database.isConnected() || emergencyId <= 0 || resourceId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    MYSQL_BIND parameters[2];
    memset(parameters, 0, sizeof(parameters));
    parameters[0].buffer_type = MYSQL_TYPE_LONG;
    parameters[0].buffer = &emergencyId;
    parameters[1].buffer_type = MYSQL_TYPE_LONG;
    parameters[1].buffer = &resourceId;

    MYSQL_STMT* deleteStatement = database.prepareStatement(
        "DELETE FROM emergency_requirements WHERE emergency_id = ? AND resource_id = ?");

    bool ok = deleteStatement != nullptr &&
              mysql_stmt_bind_param(deleteStatement, parameters) == 0 &&
              mysql_stmt_execute(deleteStatement) == 0;

    if (deleteStatement)
        mysql_stmt_close(deleteStatement);

    if (!ok)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND insertParameters[2];
    memset(insertParameters, 0, sizeof(insertParameters));
    insertParameters[0].buffer_type = MYSQL_TYPE_LONG;
    insertParameters[0].buffer = &emergencyId;
    insertParameters[1].buffer_type = MYSQL_TYPE_LONG;
    insertParameters[1].buffer = &resourceId;

    // No rows are inserted when every allocation of the pair was cancelled.
    MYSQL_STMT* insertStatement = database.prepareStatement(
        "INSERT INTO emergency_requirements "
        "(emergency_id, resource_id, required_quantity, fulfilled_quantity) "
        "SELECT emergency_id, resource_id, SUM(requested_quantity), SUM(allocated_quantity) "
        "FROM allocations WHERE emergency_id = ? AND resource_id = ? AND status <> 'CANCELLED' "
        "GROUP BY emergency_id, resource_id");

    ok = insertStatement != nullptr &&
         mysql_stmt_bind_param(insertStatement, insertParameters) == 0 &&
         mysql_stmt_execute(insertStatement) == 0;

    if (insertStatement)
        mysql_stmt_close(insertStatement);

    if (!ok)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool AllocationRepository::displayRequirements(int emergencyId)
{
    if (!database.isConnected() || emergencyId <= 0)
        return false;

    MYSQL* connection = database.getConnection();
    std::string query =
        "SELECT r.resource_id, res.resource_name, r.required_quantity, r.fulfilled_quantity "
        "FROM emergency_requirements r JOIN resources res ON res.resource_id = r.resource_id "
        "WHERE r.emergency_id = " + std::to_string(emergencyId) + " ORDER BY r.resource_id";

    if (mysql_query(connection, query.c_str()) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    std::cout << "\n===== RESOURCE REQUIREMENTS FOR EMERGENCY " << emergencyId << " =====\n";

    MYSQL_ROW row;
    int rows = 0;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        std::cout << "Resource " << row[0] << " (" << row[1] << "): required "
                  << row[2] << ", fulfilled " << row[3] << '\n';
        ++rows;
    }

    if (rows == 0)
        std::cout << "No requirements recorded.\n";

    mysql_free_result(result);
    return true;
}
