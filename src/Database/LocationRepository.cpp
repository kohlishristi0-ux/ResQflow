#include "Database/LocationRepository.h"
#include <mysql.h>
#include <string>

LocationRepository::LocationRepository(DatabaseConnection& db)
    : database(db)
{
}

bool LocationRepository::saveLocation(const Location& location)
{
    if (!database.isConnected() ||
        location.getLocationId() <= 0 ||
        location.getLocationName().empty() ||
        !location.isActive())
        return false;

    const char* query =
        "INSERT INTO locations "
        "(location_id, location_name, latitude, longitude, is_active) "
        "VALUES (?, ?, ?, ?, TRUE)";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    int id = location.getLocationId();
    double latitude = location.getLatitude();
    double longitude = location.getLongitude();

    std::string name = location.getLocationName();
    unsigned long nameLength = static_cast<unsigned long>(name.length());

    MYSQL_BIND parameters[4]{};

    parameters[0].buffer_type = MYSQL_TYPE_LONG;
    parameters[0].buffer = &id;

    parameters[1].buffer_type = MYSQL_TYPE_STRING;
    parameters[1].buffer = name.data();
    parameters[1].buffer_length = nameLength;
    parameters[1].length = &nameLength;

    parameters[2].buffer_type = MYSQL_TYPE_DOUBLE;
    parameters[2].buffer = &latitude;

    parameters[3].buffer_type = MYSQL_TYPE_DOUBLE;
    parameters[3].buffer = &longitude;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool LocationRepository::loadLocations(LocationGraph& graph)
{
    if (!database.isConnected())
        return false;

    const char* query =
        "SELECT location_id, location_name, latitude, longitude "
        "FROM locations WHERE is_active = TRUE ORDER BY location_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return false;

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        Location location(
            std::stoi(row[0]),
            row[1] ? row[1] : "",
            std::stod(row[2]),
            std::stod(row[3])
        );

        if (!graph.addLocation(location))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    return success;
}

bool LocationRepository::updateLocation(const Location& location)
{
    if (!database.isConnected() ||
        location.getLocationId() <= 0 ||
        location.getLocationName().empty())
        return false;

    const char* query =
        "UPDATE locations SET location_name = ?, "
        "latitude = ?, longitude = ? "
        "WHERE location_id = ?";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    std::string name = location.getLocationName();
    unsigned long nameLength = static_cast<unsigned long>(name.length());

    double latitude = location.getLatitude();
    double longitude = location.getLongitude();
    int id = location.getLocationId();

    MYSQL_BIND parameters[4]{};

    parameters[0].buffer_type = MYSQL_TYPE_STRING;
    parameters[0].buffer = name.data();
    parameters[0].buffer_length = nameLength;
    parameters[0].length = &nameLength;

    parameters[1].buffer_type = MYSQL_TYPE_DOUBLE;
    parameters[1].buffer = &latitude;

    parameters[2].buffer_type = MYSQL_TYPE_DOUBLE;
    parameters[2].buffer = &longitude;

    parameters[3].buffer_type = MYSQL_TYPE_LONG;
    parameters[3].buffer = &id;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool LocationRepository::deactivateLocation(int locationId)
{
    if (!database.isConnected() || locationId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    // Roads touching the location are switched off together with it (keeps loadConnections consistent).
    MYSQL_STMT* edgeStatement = database.prepareStatement(
        "UPDATE route_edges SET is_active = FALSE "
        "WHERE source_location_id = ? OR destination_location_id = ?");

    if (!edgeStatement)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND edgeParameters[2]{};
    edgeParameters[0].buffer_type = MYSQL_TYPE_LONG;
    edgeParameters[0].buffer = &locationId;
    edgeParameters[1].buffer_type = MYSQL_TYPE_LONG;
    edgeParameters[1].buffer = &locationId;

    bool edgesOk =
        mysql_stmt_bind_param(edgeStatement, edgeParameters) == 0 &&
        mysql_stmt_execute(edgeStatement) == 0;

    mysql_stmt_close(edgeStatement);

    if (!edgesOk)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_STMT* statement = database.prepareStatement(
        "UPDATE locations SET is_active = FALSE WHERE location_id = ? AND is_active = TRUE");

    if (!statement)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND parameter{};
    parameter.buffer_type = MYSQL_TYPE_LONG;
    parameter.buffer = &locationId;

    bool success =
        mysql_stmt_bind_param(statement, &parameter) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_affected_rows(statement) == 1;

    mysql_stmt_close(statement);

    if (!success)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool LocationRepository::locationExists(int locationId)
{
    if (!database.isConnected() || locationId <= 0)
        return false;

    const char* query =
        "SELECT location_id FROM locations "
        "WHERE location_id = ? LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    MYSQL_BIND parameter{};

    parameter.buffer_type = MYSQL_TYPE_LONG;
    parameter.buffer = &locationId;

    bool success =
        mysql_stmt_bind_param(statement, &parameter) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_store_result(statement) == 0;

    bool exists = success && mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);

    return exists;
}

bool LocationRepository::saveConnection(
    int sourceId,
    int destinationId,
    double distance)
{
    if (!database.isConnected() ||
        sourceId <= 0 ||
        destinationId <= 0 ||
        sourceId == destinationId ||
        distance <= 0.0)
        return false;

    if (!database.beginTransaction())
        return false;

    // 1. Is there already a row (active or not) for this pair in either direction?
    MYSQL_STMT* find = database.prepareStatement(
        "SELECT route_edge_id FROM route_edges "
        "WHERE (source_location_id = ? AND destination_location_id = ?) "
        "OR (source_location_id = ? AND destination_location_id = ?) LIMIT 1");

    if (!find)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND findParameters[4]{};
    int ids[4] = {sourceId, destinationId, destinationId, sourceId};

    for (int i = 0; i < 4; ++i)
    {
        findParameters[i].buffer_type = MYSQL_TYPE_LONG;
        findParameters[i].buffer = &ids[i];
    }

    bool findOk =
        mysql_stmt_bind_param(find, findParameters) == 0 &&
        mysql_stmt_execute(find) == 0 &&
        mysql_stmt_store_result(find) == 0;

    bool rowExists = findOk && mysql_stmt_num_rows(find) > 0;
    mysql_stmt_close(find);

    if (!findOk)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_STMT* statement = nullptr;
    bool success = false;

    if (rowExists)
    {
        statement = database.prepareStatement(
            "UPDATE route_edges SET is_active = TRUE, distance_km = ? "
            "WHERE (source_location_id = ? AND destination_location_id = ?) "
            "OR (source_location_id = ? AND destination_location_id = ?)");

        if (statement)
        {
            MYSQL_BIND parameters[5]{};
            parameters[0].buffer_type = MYSQL_TYPE_DOUBLE;
            parameters[0].buffer = &distance;

            for (int i = 0; i < 4; ++i)
            {
                parameters[i + 1].buffer_type = MYSQL_TYPE_LONG;
                parameters[i + 1].buffer = &ids[i];
            }

            success =
                mysql_stmt_bind_param(statement, parameters) == 0 &&
                mysql_stmt_execute(statement) == 0;
        }
    }
    else
    {
        statement = database.prepareStatement(
            "INSERT INTO route_edges "
            "(source_location_id, destination_location_id, distance_km, is_active) "
            "VALUES (?, ?, ?, TRUE)");

        if (statement)
        {
            MYSQL_BIND parameters[3]{};
            parameters[0].buffer_type = MYSQL_TYPE_LONG;
            parameters[0].buffer = &sourceId;
            parameters[1].buffer_type = MYSQL_TYPE_LONG;
            parameters[1].buffer = &destinationId;
            parameters[2].buffer_type = MYSQL_TYPE_DOUBLE;
            parameters[2].buffer = &distance;

            success =
                mysql_stmt_bind_param(statement, parameters) == 0 &&
                mysql_stmt_execute(statement) == 0;
        }
    }

    if (statement)
        mysql_stmt_close(statement);

    if (!success)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool LocationRepository::deactivateConnection(int sourceId, int destinationId)
{
    if (!database.isConnected() || sourceId <= 0 || destinationId <= 0)
        return false;

    MYSQL_STMT* statement = database.prepareStatement(
        "UPDATE route_edges SET is_active = FALSE "
        "WHERE is_active = TRUE AND "
        "((source_location_id = ? AND destination_location_id = ?) "
        "OR (source_location_id = ? AND destination_location_id = ?))");

    if (!statement)
        return false;

    MYSQL_BIND parameters[4]{};
    int ids[4] = {sourceId, destinationId, destinationId, sourceId};

    for (int i = 0; i < 4; ++i)
    {
        parameters[i].buffer_type = MYSQL_TYPE_LONG;
        parameters[i].buffer = &ids[i];
    }

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_affected_rows(statement) >= 1;

    mysql_stmt_close(statement);
    return success;
}

bool LocationRepository::loadConnections(LocationGraph& graph)
{
    if (!database.isConnected())
        return false;

    const char* query =
        "SELECT e.source_location_id, e.destination_location_id, e.distance_km "
        "FROM route_edges e "
        "JOIN locations s ON s.location_id = e.source_location_id AND s.is_active = TRUE "
        "JOIN locations d ON d.location_id = e.destination_location_id AND d.is_active = TRUE "
        "WHERE e.is_active = TRUE ORDER BY e.route_edge_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return false;

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int sourceId = std::stoi(row[0]);
        int destinationId = std::stoi(row[1]);
        double distance = std::stod(row[2]);

        // roads are undirected in the graph: a pair stored in both directions is only added once
        if (graph.hasConnection(sourceId, destinationId))
            continue;

        if (!graph.addConnection(sourceId, destinationId, distance))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    return success;
}

bool LocationRepository::connectionExists(
    int sourceId,
    int destinationId)
{
    if (!database.isConnected() ||
        sourceId <= 0 ||
        destinationId <= 0 ||
        sourceId == destinationId)
        return false;

    const char* query =
        "SELECT route_edge_id FROM route_edges "
        "WHERE source_location_id = ? "
        "AND destination_location_id = ? "
        "AND is_active = TRUE LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    MYSQL_BIND parameters[2]{};

    parameters[0].buffer_type = MYSQL_TYPE_LONG;
    parameters[0].buffer = &sourceId;

    parameters[1].buffer_type = MYSQL_TYPE_LONG;
    parameters[1].buffer = &destinationId;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_store_result(statement) == 0;

    bool exists = success && mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);

    return exists;
}