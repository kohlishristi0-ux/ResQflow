#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <mysql.h>
#include <type_traits>

// MySQL C API uses 'bool' and MariaDB Connector/C uses 'char' (my_bool) for MYSQL_BIND::is_null.
// Deriving the type from the struct keeps the code correct with either library.
using DbBool = std::remove_pointer<decltype(MYSQL_BIND::is_null)>::type;

class DatabaseConnection
{
private:
    MYSQL* connection;
    bool connected;
    int transactionDepth;      // supports nested begin/commit (inner calls join the outer transaction)
    bool transactionFailed;    // set when any nested scope rolled back

public:
    DatabaseConnection();
    ~DatabaseConnection();

    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    bool connect(const char* host,
                 const char* username,
                 const char* password,
                 const char* database,
                 unsigned int port = 3306);

    void disconnect();
    bool isConnected() const;

    bool executeQuery(const char* query);

    MYSQL_STMT* prepareStatement(const char* query);

    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool inTransaction() const;

    const char* getLastError() const;

    // Number of rows changed by the last statement on this connection.
    unsigned long long lastAffectedRows() const;
    MYSQL* getConnection() const;
};

#endif