#include "Database/ResourceRepository.h"
#include <mysql.h>
#include <string>

ResourceRepository::ResourceRepository(DatabaseConnection& db)
    : database(db)
{
}

bool ResourceRepository::saveResource(const Resource& resource)
{
    if (!database.isConnected() ||
        resource.getResourceId() <= 0 ||
        !resource.isActive() ||
        resource.getMinimumStockLevel() < 0)
        return false;

    const char* query =
        "INSERT INTO resources "
        "(resource_id, resource_name, category, description, unit, minimum_stock_level) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    int id = resource.getResourceId();
    int minimumStock = resource.getMinimumStockLevel();

    std::string name = resource.getResourceName();
    std::string category = resource.getCategory();
    std::string description = resource.getDescription();
    std::string unit = resource.getUnit();

    unsigned long nameLength = static_cast<unsigned long>(name.length());
    unsigned long categoryLength = static_cast<unsigned long>(category.length());
    unsigned long descriptionLength = static_cast<unsigned long>(description.length());
    unsigned long unitLength = static_cast<unsigned long>(unit.length());

    MYSQL_BIND parameters[6]{};

    parameters[0].buffer_type = MYSQL_TYPE_LONG;
    parameters[0].buffer = &id;

    parameters[1].buffer_type = MYSQL_TYPE_STRING;
    parameters[1].buffer = name.data();
    parameters[1].buffer_length = nameLength;
    parameters[1].length = &nameLength;

    parameters[2].buffer_type = MYSQL_TYPE_STRING;
    parameters[2].buffer = category.data();
    parameters[2].buffer_length = categoryLength;
    parameters[2].length = &categoryLength;

    parameters[3].buffer_type = MYSQL_TYPE_STRING;
    parameters[3].buffer = description.data();
    parameters[3].buffer_length = descriptionLength;
    parameters[3].length = &descriptionLength;

    parameters[4].buffer_type = MYSQL_TYPE_STRING;
    parameters[4].buffer = unit.data();
    parameters[4].buffer_length = unitLength;
    parameters[4].length = &unitLength;

    parameters[5].buffer_type = MYSQL_TYPE_LONG;
    parameters[5].buffer = &minimumStock;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool ResourceRepository::loadActiveResources(ResourceLinkedList& resourceList)
{
    if (!database.isConnected())
        return false;

    const char* query =
        "SELECT resource_id, resource_name, category, description, "
        "unit, minimum_stock_level "
        "FROM resources WHERE is_active = TRUE ORDER BY resource_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    resourceList.clear();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int id = std::stoi(row[0]);
        std::string name = row[1] ? row[1] : "";
        std::string category = row[2] ? row[2] : "";
        std::string description = row[3] ? row[3] : "";
        std::string unit = row[4] ? row[4] : "";
        int minimumStock = std::stoi(row[5]);

        Resource resource(
            id, name, category, description, unit, minimumStock
        );

        if (!resourceList.addResource(resource))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        resourceList.clear();

    return success;
}

bool ResourceRepository::updateResource(const Resource& resource)
{
    if (!database.isConnected() ||
        resource.getResourceId() <= 0 ||
        resource.getMinimumStockLevel() < 0)
        return false;

    const char* query =
        "UPDATE resources SET resource_name = ?, category = ?, "
        "description = ?, unit = ?, minimum_stock_level = ? "
        "WHERE resource_id = ?";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    std::string name = resource.getResourceName();
    std::string category = resource.getCategory();
    std::string description = resource.getDescription();
    std::string unit = resource.getUnit();

    int minimumStock = resource.getMinimumStockLevel();
    int id = resource.getResourceId();

    unsigned long nameLength = static_cast<unsigned long>(name.length());
    unsigned long categoryLength = static_cast<unsigned long>(category.length());
    unsigned long descriptionLength = static_cast<unsigned long>(description.length());
    unsigned long unitLength = static_cast<unsigned long>(unit.length());

    MYSQL_BIND parameters[6]{};

        parameters[0].buffer_type = MYSQL_TYPE_STRING;
    parameters[0].buffer = name.data();
    parameters[0].buffer_length = nameLength;
    parameters[0].length = &nameLength;

    parameters[1].buffer_type = MYSQL_TYPE_STRING;
    parameters[1].buffer = category.data();
    parameters[1].buffer_length = categoryLength;
    parameters[1].length = &categoryLength;

    parameters[2].buffer_type = MYSQL_TYPE_STRING;
    parameters[2].buffer = description.data();
    parameters[2].buffer_length = descriptionLength;
    parameters[2].length = &descriptionLength;

    parameters[3].buffer_type = MYSQL_TYPE_STRING;
    parameters[3].buffer = unit.data();
    parameters[3].buffer_length = unitLength;
    parameters[3].length = &unitLength;

    parameters[4].buffer_type = MYSQL_TYPE_LONG;
    parameters[4].buffer = &minimumStock;

    parameters[5].buffer_type = MYSQL_TYPE_LONG;
    parameters[5].buffer = &id;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool ResourceRepository::deactivateResource(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    const char* query =
        "UPDATE resources SET is_active = FALSE WHERE resource_id = ?";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    parameter.buffer_type = MYSQL_TYPE_LONG;
    parameter.buffer = &resourceId;

    bool success =
        mysql_stmt_bind_param(statement, &parameter) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_affected_rows(statement) == 1;

    mysql_stmt_close(statement);

    return success;
}

bool ResourceRepository::resourceExists(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    const char* query =
        "SELECT resource_id FROM resources WHERE resource_id = ? LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    parameter.buffer_type = MYSQL_TYPE_LONG;
    parameter.buffer = &resourceId;

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

bool ResourceRepository::reactivateResource(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    MYSQL_STMT* statement = database.prepareStatement(
        "UPDATE resources SET is_active = TRUE WHERE resource_id = ? AND is_active = FALSE");

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    parameter.buffer_type = MYSQL_TYPE_LONG;
    parameter.buffer = &resourceId;

    bool success =
        mysql_stmt_bind_param(statement, &parameter) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_affected_rows(statement) == 1;

    mysql_stmt_close(statement);
    return success;
}

bool ResourceRepository::isResourceActive(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    MYSQL_STMT* statement = database.prepareStatement(
        "SELECT resource_id FROM resources WHERE resource_id = ? AND is_active = TRUE");

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    parameter.buffer_type = MYSQL_TYPE_LONG;
    parameter.buffer = &resourceId;

    bool ok =
        mysql_stmt_bind_param(statement, &parameter) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_store_result(statement) == 0;

    bool active = ok && mysql_stmt_num_rows(statement) > 0;
    mysql_stmt_close(statement);
    return active;
}
