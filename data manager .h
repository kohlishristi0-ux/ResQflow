#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <mysql.h>
#include <string>
#include <type_traits>

using namespace std;

using DbBool = std::remove_pointer<decltype(MYSQL_BIND::is_null)>::type;

void bindInt(MYSQL_BIND& bind, int& value);
void bindLongLong(MYSQL_BIND& bind, long long& value);
void bindDouble(MYSQL_BIND& bind, double& value);
void bindString(MYSQL_BIND& bind, const string& value, unsigned long& length);

class DatabaseConnection
{
private:
    MYSQL* connection;
    bool connected;
    int transactionDepth;
    bool transactionFailed;

public:
    DatabaseConnection();
    ~DatabaseConnection();

    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    bool connect(const char* host, const char* username, const char* password, const char* database,
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

    unsigned long long lastAffectedRows() const;
    MYSQL* getConnection() const;
};

#endif
