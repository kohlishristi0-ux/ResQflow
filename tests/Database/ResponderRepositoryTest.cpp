#include "Database/ResponderRepository.h"
#include "Database/DatabaseConnection.h"
#include "ResponderManager.h"

#include <mysql.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>

using namespace std;

int getDatabaseValue(MYSQL* connection, const char* query)
{
    if (mysql_query(connection, query) != 0)
        return 0;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return 0;

    MYSQL_ROW row = mysql_fetch_row(result);

    int value = (row && row[0]) ? atoi(row[0]) : 0;

    mysql_free_result(result);
    return value;
}

int main()
{
    DatabaseConnection database;

    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (!password)
    {
        cout << "Database password not configured.\n";
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow"))
    {
        cout << "Database connection failed.\n";
        return 1;
    }

    MYSQL* connection = database.getConnection();

    int emergencyId = getDatabaseValue(
        connection,
        "SELECT emergency_id FROM emergencies "
        "WHERE status NOT IN ('RESOLVED','CANCELLED') LIMIT 1"
    );

    int responderId = getDatabaseValue(
        connection,
        "SELECT COALESCE(MAX(responder_id),0)+1 FROM responders"
    );

    if (emergencyId <= 0 || responderId <= 0)
    {
        cout << "TEST PREREQUISITE: An active emergency is required.\n";
        return 1;
    }

    ResponderRepository repository(database);

    Responder responder(
        responderId,
        "Repository Test Responder",
        "Medical Team",
        "9999999999",
        "First Aid",
        30.3165,
        78.0322
    );

    cout << "\nTEST 1: Saving responder...\n";

    if (!repository.saveResponder(responder))
    {
        cout << "FAILED: Save responder.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 2: Checking responder existence...\n";

    if (!repository.responderExists(responderId))
    {
        cout << "FAILED: Responder existence check.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 3: Loading active responders...\n";

    ResponderManager manager;

    if (!repository.loadActiveResponders(manager))
    {
        cout << "FAILED: Loading responders.\n";
        return 1;
    }

    Responder* loaded = manager.findResponder(responderId);

    if (!loaded ||
        loaded->getStatus() != ResponderStatus::AVAILABLE)
    {
        cout << "FAILED: Responder restoration.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 4: Assigning responder to emergency...\n";

    if (!repository.assignResponder(
            responderId,
            emergencyId,
            "Repository assignment test"))
    {
        cout << "FAILED: Responder assignment.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 5: Restoring assigned responder...\n";

    if (!repository.loadActiveResponders(manager))
    {
        cout << "FAILED: Reloading assigned responder.\n";
        return 1;
    }

    loaded = manager.findResponder(responderId);

    if (!loaded ||
        loaded->getStatus() != ResponderStatus::ASSIGNED ||
        loaded->getAssignedEmergencyId() != emergencyId)
    {
        cout << "FAILED: Assigned responder restoration.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 6: Releasing responder...\n";

    if (!repository.releaseResponder(responderId))
    {
        cout << "FAILED: Responder release.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 7: Updating responder details...\n";

    Responder updatedResponder(
        responderId,
        "Updated Test Responder",
        "Rescue Team",
        "8888888888",
        "Disaster Management",
        30.3200,
        78.0400
    );

    if (!repository.updateResponder(updatedResponder))
    {
        cout << "FAILED: Responder update.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 8: Deactivating responder...\n";

    if (!repository.deactivateResponder(responderId))
    {
        cout << "FAILED: Responder deactivation.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nTEST 9: Verifying inactive responder is not loaded...\n";

    if (!repository.loadActiveResponders(manager))
    {
        cout << "FAILED: Loading active responders after deactivation.\n";
        return 1;
    }

    if (manager.findResponder(responderId) != nullptr)
    {
        cout << "FAILED: Inactive responder was loaded.\n";
        return 1;
    }

    cout << "PASSED\n";

    cout << "\nCleaning temporary test records...\n";

    ostringstream cleanupQuery;

    cleanupQuery
        << "DELETE FROM emergency_responder_assignments "
        << "WHERE responder_id = " << responderId;

    if (mysql_query(connection, cleanupQuery.str().c_str()) != 0)
    {
        cout << "WARNING: Assignment cleanup failed: "
             << mysql_error(connection) << "\n";
    }

    cleanupQuery.str("");
    cleanupQuery.clear();

    cleanupQuery
        << "DELETE FROM responders WHERE responder_id = "
        << responderId;

    if (mysql_query(connection, cleanupQuery.str().c_str()) != 0)
    {
        cout << "WARNING: Responder cleanup failed: "
             << mysql_error(connection) << "\n";
    }

    cout << "\nALL RESPONDER REPOSITORY TESTS PASSED SUCCESSFULLY!\n";

    return 0;
}