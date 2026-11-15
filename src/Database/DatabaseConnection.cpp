#include "Database/DatabaseConnection.h"
#include <cstring>

DatabaseConnection::DatabaseConnection()
{
    connection = nullptr;
    connected = false;
    transactionDepth = 0;
    transactionFailed = false;
}

DatabaseConnection::~DatabaseConnection()
{
    disconnect();
}

bool DatabaseConnection::connect(const char* host,
                                 const char* username,
                                 const char* password,
                                 const char* database,
                                 unsigned int port)
{
    if (connected)
        disconnect();

    connection = mysql_init(nullptr);

    if (connection == nullptr)
        return false;

    if (mysql_real_connect(connection, host, username, password,
                           database, port, nullptr, 0) == nullptr)
    {
        mysql_close(connection);
        connection = nullptr;
        connected = false;
        return false;
    }

    mysql_set_character_set(connection, "utf8mb4");
    connected = true;
    transactionDepth = 0;
    transactionFailed = false;
    return true;
}

void DatabaseConnection::disconnect()
{
    if (connection != nullptr)
    {
        mysql_close(connection);
        connection = nullptr;
    }

    connected = false;
    transactionDepth = 0;
    transactionFailed = false;
}

bool DatabaseConnection::isConnected() const
{
    return connected;
}

bool DatabaseConnection::executeQuery(const char* query)
{
    if (!connected || query == nullptr)
        return false;

    return mysql_query(connection, query) == 0;
}

bool DatabaseConnection::beginTransaction()
{
    if (!connected)
        return false;

    // Nested call: join the transaction that is already open.
    if (transactionDepth > 0)
    {
        ++transactionDepth;
        return true;
    }

    if (mysql_query(connection, "START TRANSACTION") != 0)
        return false;

    transactionDepth = 1;
    transactionFailed = false;
    return true;
}

bool DatabaseConnection::commitTransaction()
{
    if (!connected || transactionDepth <= 0)
        return false;

    --transactionDepth;

    if (transactionDepth > 0)
        return !transactionFailed;   // inner scope: the outermost scope decides

    bool failed = transactionFailed;
    transactionFailed = false;

    if (failed)
    {
        mysql_rollback(connection);
        return false;
    }

    return mysql_commit(connection) == 0;
}

bool DatabaseConnection::rollbackTransaction()
{
    if (!connected || transactionDepth <= 0)
        return false;

    --transactionDepth;
    transactionFailed = true;        // poison: the whole outer transaction must roll back

    if (transactionDepth > 0)
        return true;

    transactionFailed = false;
    return mysql_rollback(connection) == 0;
}

bool DatabaseConnection::inTransaction() const
{
    return transactionDepth > 0;
}

const char* DatabaseConnection::getLastError() const
{
    if (connection == nullptr)
        return "No active database connection";

    return mysql_error(connection);
}

MYSQL_STMT* DatabaseConnection::prepareStatement(const char* query)
{
    if (!connected || query == nullptr)
        return nullptr;

    MYSQL_STMT* statement = mysql_stmt_init(connection);

    if (statement == nullptr)
        return nullptr;

    if (mysql_stmt_prepare(statement, query,
                           static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return nullptr;
    }

    return statement;
}

MYSQL* DatabaseConnection::getConnection() const
{
    return connection;
}

unsigned long long DatabaseConnection::lastAffectedRows() const
{
    if (connection == nullptr)
        return 0;

    return static_cast<unsigned long long>(mysql_affected_rows(connection));
}
