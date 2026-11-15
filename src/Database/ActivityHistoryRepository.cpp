#include "Database/ActivityHistoryRepository.h"
#include <mysql.h>
#include <string>
#include <ctime>
#include <climits>

ActivityHistoryRepository::ActivityHistoryRepository(DatabaseConnection& db)
    : database(db)
{
}

bool ActivityHistoryRepository::saveActivity(const ActivityRecord& activity)
{
    if (!database.isConnected() ||
        activity.getActivityId() <= 0 ||
        activity.getDescription().empty() ||
        activity.getPerformedBy().empty() ||
        activity.getQuantity() < 0 ||
        activity.getTimestamp() <= 0)
        return false;

    const char* query =
        "INSERT INTO activity_history "
        "(activity_id, activity_type, description, emergency_id, "
        "allocation_id, responder_id, warehouse_id, "
        "destination_warehouse_id, resource_id, quantity, "
        "performed_by, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, FROM_UNIXTIME(?))";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    int activityId = activity.getActivityId();
    std::string type = activityTypeToString(activity.getType());
    std::string description = activity.getDescription();
    std::string performedBy = activity.getPerformedBy();

    int emergencyId = activity.getEmergencyId();
    int allocationId = activity.getAllocationId();
    int responderId = activity.getResponderId();
    int warehouseId = activity.getWarehouseId();
    int destinationWarehouseId = activity.getDestinationWarehouseId();
    int resourceId = activity.getResourceId();
    int quantity = activity.getQuantity();

   DbBool emergencyNull = emergencyId == -1;
   DbBool allocationNull = allocationId == -1;
   DbBool responderNull = responderId == -1;
   DbBool warehouseNull = warehouseId == -1;
   DbBool destinationNull = destinationWarehouseId == -1;
   DbBool resourceNull = resourceId == -1;

    long long timestamp =
        static_cast<long long>(activity.getTimestamp());

    unsigned long typeLength = static_cast<unsigned long>(type.length());
    unsigned long descriptionLength =
        static_cast<unsigned long>(description.length());
    unsigned long performedByLength =
        static_cast<unsigned long>(performedBy.length());

    MYSQL_BIND parameters[12]{};

    parameters[0].buffer_type = MYSQL_TYPE_LONG;
    parameters[0].buffer = &activityId;

    parameters[1].buffer_type = MYSQL_TYPE_STRING;
    parameters[1].buffer = type.data();
    parameters[1].buffer_length = typeLength;
    parameters[1].length = &typeLength;

    parameters[2].buffer_type = MYSQL_TYPE_STRING;
    parameters[2].buffer = description.data();
    parameters[2].buffer_length = descriptionLength;
    parameters[2].length = &descriptionLength;

    parameters[3].buffer_type = MYSQL_TYPE_LONG;
    parameters[3].buffer = &emergencyId;
    parameters[3].is_null = &emergencyNull;

    parameters[4].buffer_type = MYSQL_TYPE_LONG;
    parameters[4].buffer = &allocationId;
    parameters[4].is_null = &allocationNull;

    parameters[5].buffer_type = MYSQL_TYPE_LONG;
    parameters[5].buffer = &responderId;
    parameters[5].is_null = &responderNull;

    parameters[6].buffer_type = MYSQL_TYPE_LONG;
    parameters[6].buffer = &warehouseId;
    parameters[6].is_null = &warehouseNull;

    parameters[7].buffer_type = MYSQL_TYPE_LONG;
    parameters[7].buffer = &destinationWarehouseId;
    parameters[7].is_null = &destinationNull;

    parameters[8].buffer_type = MYSQL_TYPE_LONG;
    parameters[8].buffer = &resourceId;
    parameters[8].is_null = &resourceNull;

    parameters[9].buffer_type = MYSQL_TYPE_LONG;
    parameters[9].buffer = &quantity;

    parameters[10].buffer_type = MYSQL_TYPE_STRING;
    parameters[10].buffer = performedBy.data();
    parameters[10].buffer_length = performedByLength;
    parameters[10].length = &performedByLength;

    parameters[11].buffer_type = MYSQL_TYPE_LONGLONG;
    parameters[11].buffer = &timestamp;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool ActivityHistoryRepository::loadActivityHistory(ActivityHistory& history)
{
    if (!database.isConnected())
        return false;

    const char* query =
        "SELECT activity_id, activity_type, emergency_id, allocation_id, "
        "resource_id, warehouse_id, destination_warehouse_id, responder_id, "
        "quantity, description, performed_by, UNIX_TIMESTAMP(created_at) "
        "FROM activity_history "
        "ORDER BY created_at, activity_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    history.clear();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        try
        {
            if (row[0] == nullptr || row[1] == nullptr ||
                row[8] == nullptr || row[9] == nullptr ||
                row[10] == nullptr || row[11] == nullptr)
            {
                success = false;
                break;
            }

            long long idValue = std::stoll(row[0]);
            long long timestampValue = std::stoll(row[11]);

            if (idValue <= 0 || idValue >= INT_MAX ||
                timestampValue <= 0)
            {
                success = false;
                break;
            }

            ActivityType type;
            bool validType = false;

            for (int i = 0;
                 i <= static_cast<int>(ActivityType::RESPONDER_RELEASED);
                 ++i)
            {
                ActivityType candidate = static_cast<ActivityType>(i);

                if (std::string(activityTypeToString(candidate)) == row[1])
                {
                    type = candidate;
                    validType = true;
                    break;
                }
            }

            if (!validType)
            {
                success = false;
                break;
            }

            int emergencyId = row[2] ? std::stoi(row[2]) : -1;
            int allocationId = row[3] ? std::stoi(row[3]) : -1;
            int resourceId = row[4] ? std::stoi(row[4]) : -1;
            int warehouseId = row[5] ? std::stoi(row[5]) : -1;
            int destinationWarehouseId = row[6] ? std::stoi(row[6]) : -1;
            int responderId = row[7] ? std::stoi(row[7]) : -1;
            int quantity = std::stoi(row[8]);

            if (!history.loadActivityRecord(
                    static_cast<int>(idValue),
                    type,
                    emergencyId,
                    allocationId,
                    resourceId,
                    warehouseId,
                    destinationWarehouseId,
                    responderId,
                    quantity,
                    row[9],
                    row[10],
                    static_cast<std::time_t>(timestampValue)))
            {
                success = false;
                break;
            }
        }
        catch (...)
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        history.clear();

    return success;
}

bool ActivityHistoryRepository::activityExists(int activityId)
{
    if (!database.isConnected() || activityId <= 0)
        return false;

    const char* query =
        "SELECT activity_id FROM activity_history "
        "WHERE activity_id = ? LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};

    parameter.buffer_type = MYSQL_TYPE_LONG;
    parameter.buffer = &activityId;

    bool success =
        mysql_stmt_bind_param(statement, &parameter) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_store_result(statement) == 0;

    bool exists = false;

    if (success)
        exists = mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);

    return exists;
}