#include "Database/EmergencyRepository.h"
#include "Database/DatabaseConnection.h"
#include "EmergencyManager.h"
#include "EmergencyMaxHeap.h"
#include <mysql.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

int main()
{
    DatabaseConnection database;

    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "ERROR: RESQFLOW_DB_PASSWORD is not set.\n";
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow", 3306))
    {
        cout << "Database connection failed: "
             << database.getLastError() << endl;
        return 1;
    }

    cout << "Database connected successfully.\n";

    MYSQL* connection = database.getConnection();

    int testId = 0;
    long long registrationOrder = 0;

    if (mysql_query(connection,
        "SELECT COALESCE(MAX(emergency_id), 900000) + 1 FROM emergencies") != 0)
    {
        cout << "FAILED: Could not generate emergency ID.\n";
        return 1;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    MYSQL_ROW row = result ? mysql_fetch_row(result) : nullptr;

    if (row == nullptr)
    {
        cout << "FAILED: Could not retrieve emergency ID.\n";
        if (result) mysql_free_result(result);
        return 1;
    }

    testId = atoi(row[0]);
    mysql_free_result(result);

    if (mysql_query(connection,
        "SELECT COALESCE(MAX(registration_order), 0) + 1 FROM emergencies") != 0)
    {
        cout << "FAILED: Could not generate registration order.\n";
        return 1;
    }

    result = mysql_store_result(connection);
    row = result ? mysql_fetch_row(result) : nullptr;

    if (row == nullptr)
    {
        cout << "FAILED: Could not retrieve registration order.\n";
        if (result) mysql_free_result(result);
        return 1;
    }

    registrationOrder = stoll(row[0]);
    mysql_free_result(result);

    EmergencyRepository repository(database);

    Emergency testEmergency(
        testId,
        "Flood",
        "Dehradun Test Location",
        "Emergency repository integration test",
        4,
        30.3165,
        78.0322,
        registrationOrder,
        150,
        time(nullptr)
    );

    cout << "\nTEST 1: Saving emergency...\n";

    if (!repository.saveEmergency(testEmergency))
    {
        cout << "FAILED: " << database.getLastError() << endl;
        return 1;
    }

    cout << "PASSED: Emergency saved.\n";

    cout << "\nTEST 2: Checking emergency existence...\n";

    if (!repository.emergencyExists(testId))
    {
        cout << "FAILED: Emergency not found.\n";
        return 1;
    }

    cout << "PASSED: Emergency exists.\n";

    cout << "\nTEST 3: Updating severity...\n";

    Emergency updatedEmergency(
        testId,
        "Flood",
        "Dehradun Test Location",
        "Emergency repository integration test",
        5,
        30.3165,
        78.0322,
        registrationOrder,
        150,
        testEmergency.getRegistrationTime()
    );

    if (!repository.updateEmergencySeverity(updatedEmergency))
    {
        cout << "FAILED: Severity update failed.\n";
        return 1;
    }

    cout << "PASSED: Severity updated.\n";

    cout << "\nTEST 4: Updating affected population...\n";

    Emergency populationUpdate(
        testId,
        "Flood",
        "Dehradun Test Location",
        "Emergency repository integration test",
        5,
        30.3165,
        78.0322,
        registrationOrder,
        200,
        testEmergency.getRegistrationTime()
    );

    if (!repository.updateEmergencyAffectedPeople(populationUpdate))
    {
        cout << "FAILED: Population update failed.\n";
        return 1;
    }

    cout << "PASSED: Affected population updated.\n";

    cout << "\nTEST 5: Updating emergency details...\n";

    Emergency detailsUpdate(
        testId,
        "Flash Flood",
        "Updated Dehradun Location",
        "Updated emergency test description",
        5,
        30.3165,
        78.0322,
        registrationOrder,
        200,
        testEmergency.getRegistrationTime()
    );

    if (!repository.updateEmergencyDetails(detailsUpdate))
    {
        cout << "FAILED: Details update failed.\n";
        return 1;
    }

    cout << "PASSED: Emergency details updated.\n";

    cout << "\nTEST 6: Testing valid status transitions...\n";

    if (!repository.updateEmergencyStatus(
            testId, EmergencyStatus::VERIFIED, "Test verification"))
    {
        cout << "FAILED: REPORTED to VERIFIED transition failed.\n";
        return 1;
    }

    if (!repository.updateEmergencyStatus(
            testId, EmergencyStatus::IN_PROGRESS, "Test work started"))
    {
        cout << "FAILED: VERIFIED to IN_PROGRESS transition failed.\n";
        return 1;
    }

    if (!repository.updateEmergencyStatus(
            testId, EmergencyStatus::RESOLVED, "Test resolved"))
    {
        cout << "FAILED: IN_PROGRESS to RESOLVED transition failed.\n";
        return 1;
    }

    cout << "PASSED: Valid status transitions recorded.\n";

    cout << "\nTEST 7: Rejecting invalid status transition...\n";

    if (repository.updateEmergencyStatus(
            testId, EmergencyStatus::IN_PROGRESS))
    {
        cout << "FAILED: Invalid transition was accepted.\n";
        return 1;
    }

    cout << "PASSED: Invalid transition rejected.\n";

    cout << "\nTEST 8: Verifying status history...\n";

    string historyQuery =
        "SELECT COUNT(*) FROM emergency_status_history WHERE emergency_id = "
        + to_string(testId);

    if (mysql_query(connection, historyQuery.c_str()) != 0)
    {
        cout << "FAILED: Could not query status history.\n";
        return 1;
    }

    result = mysql_store_result(connection);
    row = result ? mysql_fetch_row(result) : nullptr;

    if (row == nullptr || atoi(row[0]) != 4)
    {
        cout << "FAILED: Expected four status history records (registration + three transitions).\n";
        if (result) mysql_free_result(result);
        return 1;
    }

    mysql_free_result(result);

    cout << "PASSED: Four status history records found (registration + three transitions).\n";

    cout << "\nTEST 9: Loading emergencies into memory...\n";

    EmergencyMaxHeap priorityHeap;
    EmergencyManager emergencyManager;

    emergencyManager.connectPriorityHeap(priorityHeap);

    if (!repository.loadEmergencies(emergencyManager))
    {
        cout << "FAILED: Emergency loading failed.\n";
        return 1;
    }

    Emergency* loadedEmergency =
        emergencyManager.searchEmergency(testId);

    if (loadedEmergency == nullptr ||
        loadedEmergency->getEmergencyId() != testId ||
        loadedEmergency->getSeverity() != 5 ||
        loadedEmergency->getAffectedPeople() != 200 ||
        loadedEmergency->getStatus() != EmergencyStatus::RESOLVED)
    {
        cout << "FAILED: Loaded emergency data does not match.\n";
        return 1;
    }

    cout << "PASSED: Emergency restored with original data and status.\n";

    if (priorityHeap.getSize() != emergencyManager.getCount())
    {
        // Resolved and cancelled emergencies are intentionally
        // excluded from the priority heap.
        cout << "NOTE: Terminal emergencies are excluded from the heap.\n";
    }

    cout << "\n====================================\n";
    cout << "ALL EMERGENCY REPOSITORY TESTS PASSED!\n";
    cout << "====================================\n";

    return 0;
}