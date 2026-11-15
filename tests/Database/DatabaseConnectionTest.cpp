#include <iostream>
#include <cstdlib>
#include <mysql.h>
#include "Database/DatabaseConnection.h"

using namespace std;

int main()
{
    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "ERROR: Database password not found." << endl;
        return 1;
    }

    DatabaseConnection db;

    if (!db.connect("127.0.0.1", "root", password, "resqflow", 3306))
    {
        cout << "CONNECTION FAILED: " << db.getLastError() << endl;
        return 1;
    }

    cout << "SUCCESS: Connected to ResQflow database!" << endl;

    // TEST 1: Prepared Statement
    MYSQL_STMT* statement = db.prepareStatement("SELECT ?");

    if (statement == nullptr)
    {
        cout << "FAILED: Prepared statement creation." << endl;
        db.disconnect();
        return 1;
    }

    int inputValue = 100;
    int outputValue = 0;

    MYSQL_BIND parameters[1]{};
    parameters[0].buffer_type = MYSQL_TYPE_LONG;
    parameters[0].buffer = &inputValue;

    if (mysql_stmt_bind_param(statement, parameters) != 0 ||
        mysql_stmt_execute(statement) != 0)
    {
        cout << "FAILED: Prepared statement execution." << endl;
        mysql_stmt_close(statement);
        db.disconnect();
        return 1;
    }

    MYSQL_BIND results[1]{};
    results[0].buffer_type = MYSQL_TYPE_LONG;
    results[0].buffer = &outputValue;

    if (mysql_stmt_bind_result(statement, results) != 0 ||
        mysql_stmt_store_result(statement) != 0 ||
        mysql_stmt_fetch(statement) != 0)
    {
        cout << "FAILED: Prepared statement result." << endl;
        mysql_stmt_close(statement);
        db.disconnect();
        return 1;
    }

    if (outputValue != 100)
    {
        cout << "FAILED: Incorrect prepared statement result." << endl;
        mysql_stmt_close(statement);
        db.disconnect();
        return 1;
    }

    cout << "SUCCESS: Prepared statement test passed!" << endl;
    mysql_stmt_close(statement);

    // TEST 2: Transaction and Rollback
    if (!db.beginTransaction())
    {
        cout << "FAILED: Transaction start." << endl;
        db.disconnect();
        return 1;
    }

    cout << "SUCCESS: Transaction started!" << endl;

    if (!db.rollbackTransaction())
    {
        cout << "FAILED: Transaction rollback." << endl;
        db.disconnect();
        return 1;
    }

    cout << "SUCCESS: Transaction rollback passed!" << endl;

    db.disconnect();

    cout << "\nALL DATABASE CONNECTION TESTS PASSED!" << endl;

    return 0;
}