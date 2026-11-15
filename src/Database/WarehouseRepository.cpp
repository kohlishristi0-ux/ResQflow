#include "Database/WarehouseRepository.h"
#include <mysql.h>
#include <cstring>
#include <string>

WarehouseRepository::WarehouseRepository(DatabaseConnection& db) : database(db) {}

bool WarehouseRepository::saveWarehouse(const Warehouse& warehouse)
{
    if (!database.isConnected() || warehouse.getWarehouseId() <= 0 ||
        warehouse.getWarehouseName().empty() || warehouse.getAddress().empty() ||
        !warehouse.isActive() || warehouseExists(warehouse.getWarehouseId()))
        return false;

    // The existing schema still has NOT NULL legacy latitude/longitude columns.
    // Store neutral zero values there; coordinates are no longer collected,
    // stored in the Warehouse object, displayed, or used by warehouse logic.
    const char* query =
        "INSERT INTO warehouses "
        "(warehouse_id, warehouse_name, address, latitude, longitude, contact_number) "
        "VALUES (?, ?, ?, 0, 0, ?)";

    MYSQL_STMT* stmt = mysql_stmt_init(database.getConnection());
    if (!stmt) return false;
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    { mysql_stmt_close(stmt); return false; }

    int id = warehouse.getWarehouseId();
    const std::string& name = warehouse.getWarehouseName();
    const std::string& address = warehouse.getAddress();
    const std::string& contact = warehouse.getContactNumber();
    unsigned long nameLength = name.length(), addressLength = address.length(), contactLength = contact.length();

    MYSQL_BIND bind[4] = {};
    bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = &id;
    bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = const_cast<char*>(name.c_str()); bind[1].buffer_length = nameLength; bind[1].length = &nameLength;
    bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = const_cast<char*>(address.c_str()); bind[2].buffer_length = addressLength; bind[2].length = &addressLength;
    bind[3].buffer_type = MYSQL_TYPE_STRING; bind[3].buffer = const_cast<char*>(contact.c_str()); bind[3].buffer_length = contactLength; bind[3].length = &contactLength;

    bool ok = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;
    mysql_stmt_close(stmt);
    return ok;
}

bool WarehouseRepository::loadActiveWarehouses(WarehouseLinkedList& warehouseList)
{
    if (!database.isConnected()) return false;
    const char* query =
        "SELECT warehouse_id, warehouse_name, address, contact_number "
        "FROM warehouses WHERE is_active = TRUE ORDER BY warehouse_id";
    if (mysql_query(database.getConnection(), query) != 0) return false;
    MYSQL_RES* result = mysql_store_result(database.getConnection());
    if (!result) return false;

    warehouseList.clear();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int id = std::stoi(row[0]);
        std::string name = row[1] ? row[1] : "";
        std::string address = row[2] ? row[2] : "";
        std::string contact = row[3] ? row[3] : "";
        Warehouse warehouse(id, name, address, contact);
        if (!warehouseList.addWarehouse(warehouse))
        { warehouseList.clear(); mysql_free_result(result); return false; }
    }
    mysql_free_result(result);
    return true;
}

bool WarehouseRepository::warehouseExists(int warehouseId)
{
    if (!database.isConnected() || warehouseId <= 0) return false;
    const char* query = "SELECT warehouse_id FROM warehouses WHERE warehouse_id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(database.getConnection());
    if (!stmt) return false;
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    { mysql_stmt_close(stmt); return false; }
    MYSQL_BIND param[1] = {};
    param[0].buffer_type = MYSQL_TYPE_LONG; param[0].buffer = &warehouseId;
    bool exists = false;
    if (mysql_stmt_bind_param(stmt, param) == 0 && mysql_stmt_execute(stmt) == 0 && mysql_stmt_store_result(stmt) == 0)
        exists = mysql_stmt_num_rows(stmt) > 0;
    mysql_stmt_close(stmt);
    return exists;
}

bool WarehouseRepository::updateWarehouse(const Warehouse& warehouse)
{
    if (!database.isConnected() || warehouse.getWarehouseId() <= 0 ||
        warehouse.getWarehouseName().empty() || warehouse.getAddress().empty() ||
        !warehouseExists(warehouse.getWarehouseId())) return false;

    const char* query =
        "UPDATE warehouses SET warehouse_name = ?, address = ?, contact_number = ? "
        "WHERE warehouse_id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(database.getConnection());
    if (!stmt) return false;
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    { mysql_stmt_close(stmt); return false; }

    const std::string& name = warehouse.getWarehouseName();
    const std::string& address = warehouse.getAddress();
    const std::string& contact = warehouse.getContactNumber();
    unsigned long nameLength = name.length(), addressLength = address.length(), contactLength = contact.length();
    int id = warehouse.getWarehouseId();

    MYSQL_BIND bind[4] = {};
    bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = const_cast<char*>(name.c_str()); bind[0].buffer_length = nameLength; bind[0].length = &nameLength;
    bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = const_cast<char*>(address.c_str()); bind[1].buffer_length = addressLength; bind[1].length = &addressLength;
    bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = const_cast<char*>(contact.c_str()); bind[2].buffer_length = contactLength; bind[2].length = &contactLength;
    bind[3].buffer_type = MYSQL_TYPE_LONG; bind[3].buffer = &id;

    bool ok = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;
    mysql_stmt_close(stmt);
    return ok;
}

bool WarehouseRepository::deactivateWarehouse(int warehouseId)
{
    if (!database.isConnected() || warehouseId <= 0) return false;
    const char* query = "UPDATE warehouses SET is_active = FALSE WHERE warehouse_id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(database.getConnection());
    if (!stmt) return false;
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    { mysql_stmt_close(stmt); return false; }
    MYSQL_BIND bind[1] = {};
    bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = &warehouseId;
    bool ok = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0 && mysql_stmt_affected_rows(stmt) == 1;
    mysql_stmt_close(stmt);
    return ok;
}
