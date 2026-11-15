#include "Database/InventoryRepository.h"
#include <mysql.h>
#include <cstring>
#include <cstdlib>
#include <string>

InventoryRepository::InventoryRepository(DatabaseConnection& db)
    : database(db)
{
}

bool InventoryRepository::saveInventory(const Inventory& inventory)
{
    if (!database.isConnected() ||
        inventory.getInventoryId() <= 0 ||
        inventory.getResourceId() <= 0 ||
        inventory.getWarehouseId() <= 0 ||
        inventory.getAvailableQuantity() < 0 ||
        inventory.getReservedQuantity() < 0 ||
        inventory.getMinimumStockLevel() < 0)
    {
        return false;
    }

    const char* query =
        "INSERT INTO inventory "
        "(inventory_id, resource_id, warehouse_id, available_quantity, "
        "reserved_quantity, minimum_stock_level) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int inventoryId = inventory.getInventoryId();
    int resourceId = inventory.getResourceId();
    int warehouseId = inventory.getWarehouseId();
    int available = inventory.getAvailableQuantity();
    int reserved = inventory.getReservedQuantity();
    int minimum = inventory.getMinimumStockLevel();

    MYSQL_BIND params[6];
    memset(params, 0, sizeof(params));

    int* values[6] = {
        &inventoryId, &resourceId, &warehouseId,
        &available, &reserved, &minimum
    };

    for (int i = 0; i < 6; ++i)
    {
        params[i].buffer_type = MYSQL_TYPE_LONG;
        params[i].buffer = values[i];
    }

    bool success =
        mysql_stmt_bind_param(stmt, params) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool InventoryRepository::loadInventory(InventoryManager& inventoryManager)
{
    if (!database.isConnected())
        return false;

    const char* query =
        "SELECT i.inventory_id, i.resource_id, i.warehouse_id, "
        "i.available_quantity, i.reserved_quantity, i.minimum_stock_level "
        "FROM inventory i "
        "JOIN resources r ON r.resource_id = i.resource_id AND r.is_active = TRUE "
        "JOIN warehouses w ON w.warehouse_id = i.warehouse_id AND w.is_active = TRUE "
        "ORDER BY i.inventory_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    inventoryManager.clearInventory();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        if (row[0] == nullptr || row[1] == nullptr ||
            row[2] == nullptr || row[3] == nullptr ||
            row[4] == nullptr || row[5] == nullptr)
        {
            success = false;
            break;
        }

        int inventoryId = atoi(row[0]);
        int resourceId = atoi(row[1]);
        int warehouseId = atoi(row[2]);
        int available = atoi(row[3]);
        int reserved = atoi(row[4]);
        int minimum = atoi(row[5]);

        if (!inventoryManager.loadInventoryRecord(
                inventoryId,
                resourceId,
                warehouseId,
                available,
                reserved,
                minimum))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        inventoryManager.clearInventory();

    return success;
}

bool InventoryRepository::updateInventory(const Inventory& inventory)
{
    if (!database.isConnected() ||
        inventory.getInventoryId() <= 0 ||
        inventory.getAvailableQuantity() < 0 ||
        inventory.getReservedQuantity() < 0 ||
        inventory.getMinimumStockLevel() < 0)
    {
        return false;
    }

    const char* query =
        "UPDATE inventory SET available_quantity = ?, "
        "reserved_quantity = ?, minimum_stock_level = ? "
        "WHERE inventory_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int available = inventory.getAvailableQuantity();
    int reserved = inventory.getReservedQuantity();
    int minimum = inventory.getMinimumStockLevel();
    int inventoryId = inventory.getInventoryId();

    MYSQL_BIND params[4];
    memset(params, 0, sizeof(params));

    int* values[4] = {
        &available, &reserved, &minimum, &inventoryId
    };

    for (int i = 0; i < 4; ++i)
    {
        params[i].buffer_type = MYSQL_TYPE_LONG;
        params[i].buffer = values[i];
    }

    bool success =
        mysql_stmt_bind_param(stmt, params) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}

bool InventoryRepository::deleteInventory(int inventoryId)
{
    if (!database.isConnected() || inventoryId <= 0)
        return false;

    const char* query =
        "DELETE FROM inventory "
        "WHERE inventory_id = ? "
        "AND available_quantity = 0 "
        "AND reserved_quantity = 0";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND param;
    memset(&param, 0, sizeof(param));

    param.buffer_type = MYSQL_TYPE_LONG;
    param.buffer = &inventoryId;

    bool success =
        mysql_stmt_bind_param(stmt, &param) == 0 &&
        mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_affected_rows(stmt) == 1;

    mysql_stmt_close(stmt);
    return success;
}

bool InventoryRepository::inventoryExists(int inventoryId)
{
    if (!database.isConnected() || inventoryId <= 0)
        return false;

    const char* query =
        "SELECT inventory_id FROM inventory WHERE inventory_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND input;
    memset(&input, 0, sizeof(input));

    input.buffer_type = MYSQL_TYPE_LONG;
    input.buffer = &inventoryId;

    bool exists = false;
    int returnedId = 0;

    MYSQL_BIND output;
    memset(&output, 0, sizeof(output));

    output.buffer_type = MYSQL_TYPE_LONG;
    output.buffer = &returnedId;

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

bool InventoryRepository::recordMovement(
    int resourceId,
    int warehouseId,
    const char* transactionType,
    int quantity,
    const char* referenceType,
    long long referenceId,
    int performedBy,
    const char* notes)
{
    if (!database.isConnected() || resourceId <= 0 || warehouseId <= 0 ||
        transactionType == nullptr || quantity <= 0)
        return false;

    MYSQL_STMT* stmt = database.prepareStatement(
        "INSERT INTO resource_transactions "
        "(resource_id, warehouse_id, transaction_type, quantity, "
        "reference_type, reference_id, performed_by, notes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    if (stmt == nullptr)
        return false;

    std::string type = transactionType;
    std::string refType = referenceType ? referenceType : "";
    std::string note = notes ? notes : "";
    unsigned long typeLength = static_cast<unsigned long>(type.length());
    unsigned long refTypeLength = static_cast<unsigned long>(refType.length());
    unsigned long noteLength = static_cast<unsigned long>(note.length());

    DbBool refTypeNull = refType.empty() ? 1 : 0;
    DbBool refIdNull = referenceId <= 0 ? 1 : 0;
    DbBool userNull = performedBy <= 0 ? 1 : 0;
    DbBool noteNull = note.empty() ? 1 : 0;

    MYSQL_BIND params[8];
    memset(params, 0, sizeof(params));

    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &resourceId;
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &warehouseId;
    params[2].buffer_type = MYSQL_TYPE_STRING;
    params[2].buffer = type.data();
    params[2].buffer_length = typeLength;
    params[2].length = &typeLength;
    params[3].buffer_type = MYSQL_TYPE_LONG;
    params[3].buffer = &quantity;
    params[4].buffer_type = MYSQL_TYPE_STRING;
    params[4].buffer = refType.data();
    params[4].buffer_length = refTypeLength;
    params[4].length = &refTypeLength;
    params[4].is_null = &refTypeNull;
    params[5].buffer_type = MYSQL_TYPE_LONGLONG;
    params[5].buffer = &referenceId;
    params[5].is_null = &refIdNull;
    params[6].buffer_type = MYSQL_TYPE_LONG;
    params[6].buffer = &performedBy;
    params[6].is_null = &userNull;
    params[7].buffer_type = MYSQL_TYPE_STRING;
    params[7].buffer = note.data();
    params[7].buffer_length = noteLength;
    params[7].length = &noteLength;
    params[7].is_null = &noteNull;

    bool success =
        mysql_stmt_bind_param(stmt, params) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);
    return success;
}
