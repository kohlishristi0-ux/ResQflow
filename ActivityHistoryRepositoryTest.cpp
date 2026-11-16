#include "ResponderModule.h"
#include <mysql.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>

using namespace std;

int main()
{
    DatabaseConnection database;

    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "ERROR: Database password environment variable not set." << endl;
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow", 3306))
    {
        cout << "Database connection failed: "
             << database.getLastError() << endl;
        return 1;
    }

    cout << "Database connected successfully." << endl;

    ActivityHistoryRepository repository(database);
    ActivityHistory history;

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection,
        "SELECT COALESCE(MAX(activity_id), 0) + 1 FROM activity_history") != 0)
    {
        cout << "Unable to generate test ID." << endl;
        return 1;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    MYSQL_ROW row = mysql_fetch_row(result);

    int testId = stoi(row[0]);

    mysql_free_result(result);

    ActivityRecord activity(
        testId,
        ActivityType::STOCK_ADDED,
        -1, -1, -1, -1, -1, -1,
        25,
        "Repository test: stock added",
        "TEST_USER",
        time(nullptr)
    );

    if (!repository.saveActivity(activity))
    {
        cout << "FAIL: Activity insertion failed." << endl;
        return 1;
    }

    cout << "PASS: Activity saved successfully." << endl;

    if (!repository.activityExists(testId))
    {
        cout << "FAIL: Activity existence check failed." << endl;
        return 1;
    }

    cout << "PASS: Activity existence verified." << endl;

    if (!repository.loadActivityHistory(history))
    {
        cout << "FAIL: Activity history loading failed." << endl;
        return 1;
    }

    const ActivityRecord* loaded = history.searchByActivityId(testId);

    if (loaded == nullptr ||
        loaded->getQuantity() != 25 ||
        loaded->getPerformedBy() != "TEST_USER" ||
        loaded->getDescription() != "Repository test: stock added")
    {
        cout << "FAIL: Loaded activity data does not match." << endl;
        return 1;
    }

    cout << "PASS: Activity loaded and verified successfully." << endl;

    string deleteQuery =
        "DELETE FROM activity_history WHERE activity_id = " +
        to_string(testId);

    if (!database.executeQuery(deleteQuery.c_str()))
    {
        cout << "WARNING: Test record cleanup failed." << endl;
    }
    else
    {
        cout << "PASS: Test record cleaned up." << endl;
    }

    cout << "\nALL ACTIVITY HISTORY REPOSITORY TESTS PASSED!"
         << endl;

    return 0;
}
