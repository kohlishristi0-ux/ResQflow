#include "Database/EmergencyRepository.h"
#include <mysql.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace
{
    void reportStatementError(MYSQL_STMT* stmt, const char* context)
    {
        std::cerr << "[EmergencyRepository] " << context << ": "
                  << (stmt != nullptr ? mysql_stmt_error(stmt) : "no statement") << '\n';
    }

    const char* statusToString(EmergencyStatus status)
    {
        switch (status)
        {
            case EmergencyStatus::REPORTED: return "REPORTED";
            case EmergencyStatus::VERIFIED: return "VERIFIED";
            case EmergencyStatus::IN_PROGRESS: return "IN_PROGRESS";
            case EmergencyStatus::RESOLVED: return "RESOLVED";
            case EmergencyStatus::CANCELLED: return "CANCELLED";
        }
        return nullptr;
    }

    bool stringToStatus(const std::string& value, EmergencyStatus& status)
    {
        if (value == "REPORTED")
            status = EmergencyStatus::REPORTED;
        else if (value == "VERIFIED")
            status = EmergencyStatus::VERIFIED;
        else if (value == "IN_PROGRESS")
            status = EmergencyStatus::IN_PROGRESS;
        else if (value == "RESOLVED")
            status = EmergencyStatus::RESOLVED;
        else if (value == "CANCELLED")
            status = EmergencyStatus::CANCELLED;
        else
            return false;

        return true;
    }

    bool validTransition(EmergencyStatus oldStatus, EmergencyStatus newStatus)
    {
        switch (oldStatus)
        {
            case EmergencyStatus::REPORTED:
                return newStatus == EmergencyStatus::VERIFIED ||
                       newStatus == EmergencyStatus::CANCELLED;

            case EmergencyStatus::VERIFIED:
                return newStatus == EmergencyStatus::IN_PROGRESS ||
                       newStatus == EmergencyStatus::CANCELLED;

            case EmergencyStatus::IN_PROGRESS:
                return newStatus == EmergencyStatus::RESOLVED ||
                       newStatus == EmergencyStatus::CANCELLED;

            default:
                return false;
        }
    }
}

EmergencyRepository::EmergencyRepository(DatabaseConnection& db)
    : database(db)
{
}

bool EmergencyRepository::saveEmergency(const Emergency& emergency)
{
    if (!database.isConnected() ||
        emergency.getEmergencyId() <= 0 ||
        emergency.getEmergencyType().empty() ||
        emergency.getLocation().empty() ||
        emergency.getSeverity() < 1 ||
        emergency.getSeverity() > 5 ||
        emergency.getAffectedPeople() < 1 ||
        emergency.getRegistrationOrder() <= 0 ||
        emergency.getRegistrationTime() <= 0)
    {
        return false;
    }

    const char* status = statusToString(emergency.getStatus());

    if (status == nullptr)
        return false;

    const char* query =
        "INSERT INTO emergencies "
        "(emergency_id, emergency_type, location_name, description, "
        "severity, affected_people, latitude, longitude, status, "
        "registration_order, reported_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, FROM_UNIXTIME(?))";

    if (!database.beginTransaction())
        return false;

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
    {
        std::cerr << "[EmergencyRepository] prepare INSERT failed: "
                  << database.getLastError() << '\n';
        database.rollbackTransaction();
        return false;
    }

    int id = emergency.getEmergencyId();
    int severity = emergency.getSeverity();
    int affectedPeople = emergency.getAffectedPeople();
    double latitude = emergency.getLatitude();
    double longitude = emergency.getLongitude();
    long long registrationOrder = emergency.getRegistrationOrder();
    long long registrationTime =
        static_cast<long long>(emergency.getRegistrationTime());

    std::string type = emergency.getEmergencyType();
    std::string location = emergency.getLocation();
    std::string description = emergency.getDescription();
    std::string statusText = status;

    unsigned long typeLength = type.length();
    unsigned long locationLength = location.length();
    unsigned long descriptionLength = description.length();
    unsigned long statusLength = statusText.length();

    MYSQL_BIND bind[11] = {};

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &id;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = type.data();
    bind[1].buffer_length = typeLength;
    bind[1].length = &typeLength;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = location.data();
    bind[2].buffer_length = locationLength;
    bind[2].length = &locationLength;

    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = description.data();
    bind[3].buffer_length = descriptionLength;
    bind[3].length = &descriptionLength;

    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = &severity;

    bind[5].buffer_type = MYSQL_TYPE_LONG;
    bind[5].buffer = &affectedPeople;

    bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[6].buffer = &latitude;

    bind[7].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[7].buffer = &longitude;

    bind[8].buffer_type = MYSQL_TYPE_STRING;
    bind[8].buffer = statusText.data();
    bind[8].buffer_length = statusLength;
    bind[8].length = &statusLength;

    bind[9].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[9].buffer = &registrationOrder;

    bind[10].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[10].buffer = &registrationTime;

    bool success = mysql_stmt_bind_param(stmt, bind) == 0;

    if (!success)
        reportStatementError(stmt, "bind INSERT");
    else if (mysql_stmt_execute(stmt) != 0)
    {
        reportStatementError(stmt, "execute INSERT");
        success = false;
    }

    mysql_stmt_close(stmt);

    if (!success)
    {
        database.rollbackTransaction();
        return false;
    }

    // First entry of the lifecycle: NULL -> current status (history is preserved from registration).
    const char* historyQuery =
        "INSERT INTO emergency_status_history "
        "(emergency_id, old_status, new_status, changed_by, notes) "
        "VALUES (?, NULL, ?, NULL, 'Emergency registered')";

    MYSQL_STMT* historyStmt = database.prepareStatement(historyQuery);

    if (historyStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND historyBind[2] = {};
    historyBind[0].buffer_type = MYSQL_TYPE_LONG;
    historyBind[0].buffer = &id;
    historyBind[1].buffer_type = MYSQL_TYPE_STRING;
    historyBind[1].buffer = statusText.data();
    historyBind[1].buffer_length = statusLength;
    historyBind[1].length = &statusLength;

    bool historySuccess =
        mysql_stmt_bind_param(historyStmt, historyBind) == 0 &&
        mysql_stmt_execute(historyStmt) == 0;

    if (!historySuccess)
        reportStatementError(historyStmt, "INSERT initial status history");

    mysql_stmt_close(historyStmt);

    if (!historySuccess)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}



bool EmergencyRepository::loadEmergencies(
    EmergencyManager& emergencyManager)
{
    if (!database.isConnected())
        return false;

    const char* query =
        "SELECT emergency_id, emergency_type, location_name, description, "
        "severity, latitude, longitude, registration_order, "
        "affected_people, UNIX_TIMESTAMP(reported_at), status "
        "FROM emergencies ORDER BY registration_order";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    MYSQL_ROW row;
    bool success = true;

    try
    {
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        if (row[0] == nullptr || row[1] == nullptr ||
            row[2] == nullptr || row[4] == nullptr ||
            row[5] == nullptr || row[6] == nullptr ||
            row[7] == nullptr || row[8] == nullptr ||
            row[9] == nullptr || row[10] == nullptr)
        {
            success = false;
            break;
        }

        int id = std::stoi(row[0]);
        std::string type = row[1];
        std::string location = row[2];
        std::string description = row[3] ? row[3] : "";

        int severity = std::stoi(row[4]);
        double latitude = std::stod(row[5]);
        double longitude = std::stod(row[6]);
        long long registrationOrder = std::stoll(row[7]);
        int affectedPeople = std::stoi(row[8]);
        time_t registrationTime =
            static_cast<time_t>(std::stoll(row[9]));

        EmergencyStatus status;

        if (!stringToStatus(row[10], status))
        {
            success = false;
            break;
        }

        if (!emergencyManager.loadEmergencyRecord(
                id,
                type,
                location,
                description,
                severity,
                latitude,
                longitude,
                registrationOrder,
                affectedPeople,
                registrationTime,
                status))
        {
            success = false;
            break;
        }
    }
    }
    catch (const std::exception& error)
    {
        std::cerr << "[EmergencyRepository] corrupt row while loading: "
                  << error.what() << '\n';
        success = false;
    }

    mysql_free_result(result);
    return success;
}

bool EmergencyRepository::updateEmergencyDetails(
    const Emergency& emergency)
{
    if (!database.isConnected() ||
        emergency.getEmergencyId() <= 0 ||
        emergency.getEmergencyType().empty() ||
        emergency.getLocation().empty())
        return false;

    const char* query =
        "UPDATE emergencies SET emergency_type = ?, "
        "location_name = ?, description = ? "
        "WHERE emergency_id = ?";

    // UPDATE reports 0 changed rows when the values are identical, so existence is checked separately.
    if (!emergencyExists(emergency.getEmergencyId()))
    {
        std::cerr << "[EmergencyRepository] emergency "
                  << emergency.getEmergencyId() << " does not exist in MySQL\n";
        return false;
    }

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
    {
        std::cerr << "[EmergencyRepository] prepare UPDATE details failed: "
                  << database.getLastError() << '\n';
        return false;
    }

    std::string type = emergency.getEmergencyType();
    std::string location = emergency.getLocation();
    std::string description = emergency.getDescription();

    unsigned long typeLength = type.length();
    unsigned long locationLength = location.length();
    unsigned long descriptionLength = description.length();

    int id = emergency.getEmergencyId();

    MYSQL_BIND bind[4] = {};

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = type.data();
    bind[0].buffer_length = typeLength;
    bind[0].length = &typeLength;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = location.data();
    bind[1].buffer_length = locationLength;
    bind[1].length = &locationLength;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = description.data();
    bind[2].buffer_length = descriptionLength;
    bind[2].length = &descriptionLength;

    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = &id;

    bool success =
        mysql_stmt_bind_param(stmt, bind) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    if (!success)
        reportStatementError(stmt, "UPDATE details");

    mysql_stmt_close(stmt);
    return success;
}

bool EmergencyRepository::updateEmergencySeverity(
    const Emergency& emergency)
{
    if (!database.isConnected() ||
        emergency.getEmergencyId() <= 0 ||
        emergency.getSeverity() < 1 ||
        emergency.getSeverity() > 5)
        return false;

    if (!emergencyExists(emergency.getEmergencyId()))
        return false;

    const char* query =
        "UPDATE emergencies SET severity = ? WHERE emergency_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int severity = emergency.getSeverity();
    int id = emergency.getEmergencyId();

    MYSQL_BIND bind[2] = {};

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &severity;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &id;

    bool success =
        mysql_stmt_bind_param(stmt, bind) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    if (!success)
        reportStatementError(stmt, "UPDATE severity");

    mysql_stmt_close(stmt);
    return success;
}

bool EmergencyRepository::updateEmergencyAffectedPeople(
    const Emergency& emergency)
{
    if (!database.isConnected() ||
        emergency.getEmergencyId() <= 0 ||
        emergency.getAffectedPeople() < 1)
        return false;

    if (!emergencyExists(emergency.getEmergencyId()))
        return false;

    const char* query =
        "UPDATE emergencies SET affected_people = ? WHERE emergency_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int affectedPeople = emergency.getAffectedPeople();
    int id = emergency.getEmergencyId();

    MYSQL_BIND bind[2] = {};

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &affectedPeople;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &id;

    bool success =
        mysql_stmt_bind_param(stmt, bind) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    if (!success)
        reportStatementError(stmt, "UPDATE affected people");

    mysql_stmt_close(stmt);
    return success;
}

bool EmergencyRepository::updateEmergencyStatus(
    int emergencyId,
    EmergencyStatus newStatus,
    const char* notes,
    int changedBy)
{
    if (!database.isConnected() || emergencyId <= 0)
        return false;

    const char* newStatusText = statusToString(newStatus);

    if (newStatusText == nullptr)
        return false;

    if (!database.beginTransaction())
        return false;

    const char* selectQuery =
        "SELECT status FROM emergencies WHERE emergency_id = ? FOR UPDATE";

    MYSQL_STMT* selectStmt = database.prepareStatement(selectQuery);

    if (selectStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND selectBind[1] = {};
    selectBind[0].buffer_type = MYSQL_TYPE_LONG;
    selectBind[0].buffer = &emergencyId;

    bool selectSuccess =
        mysql_stmt_bind_param(selectStmt, selectBind) == 0 &&
        mysql_stmt_execute(selectStmt) == 0 &&
        mysql_stmt_store_result(selectStmt) == 0;

    char oldStatusBuffer[30] = {};
    unsigned long oldStatusLength = 0;

    MYSQL_BIND resultBind[1] = {};
    resultBind[0].buffer_type = MYSQL_TYPE_STRING;
    resultBind[0].buffer = oldStatusBuffer;
    resultBind[0].buffer_length = sizeof(oldStatusBuffer);
    resultBind[0].length = &oldStatusLength;

    if (!selectSuccess ||
        mysql_stmt_num_rows(selectStmt) != 1 ||
        mysql_stmt_bind_result(selectStmt, resultBind) != 0 ||
        mysql_stmt_fetch(selectStmt) != 0)
    {
        mysql_stmt_close(selectStmt);
        database.rollbackTransaction();
        return false;
    }

    mysql_stmt_close(selectStmt);

    EmergencyStatus oldStatus;

    if (!stringToStatus(std::string(oldStatusBuffer, oldStatusLength), oldStatus) ||
        !validTransition(oldStatus, newStatus))
    {
        database.rollbackTransaction();
        return false;
    }

    const char* updateQuery =
        "UPDATE emergencies SET status = ? WHERE emergency_id = ?";
    MYSQL_STMT* updateStmt = database.prepareStatement(updateQuery);

    if (updateStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    std::string newStatusString = newStatusText;
    unsigned long newStatusLength = newStatusString.length();

    MYSQL_BIND updateBind[2] = {};

    updateBind[0].buffer_type = MYSQL_TYPE_STRING;
    updateBind[0].buffer = newStatusString.data();
    updateBind[0].buffer_length = newStatusLength;
    updateBind[0].length = &newStatusLength;

    updateBind[1].buffer_type = MYSQL_TYPE_LONG;
    updateBind[1].buffer = &emergencyId;

    bool updateSuccess =
        mysql_stmt_bind_param(updateStmt, updateBind) == 0 &&
        mysql_stmt_execute(updateStmt) == 0 &&
        mysql_stmt_affected_rows(updateStmt) == 1;

    if (!updateSuccess)
        reportStatementError(updateStmt, "UPDATE status");

    mysql_stmt_close(updateStmt);

    if (!updateSuccess)
    {
        database.rollbackTransaction();
        return false;
    }

    const char* historyQuery =
        "INSERT INTO emergency_status_history "
        "(emergency_id, old_status, new_status, changed_by, notes) "
        "VALUES (?, ?, ?, ?, ?)";

    MYSQL_STMT* historyStmt = database.prepareStatement(historyQuery);

    if (historyStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    std::string oldStatusString = statusToString(oldStatus);
    unsigned long historyOldStatusLength  = oldStatusString.length();

    std::string notesString = notes ? notes : "";
    unsigned long notesLength = notesString.length();

    DbBool changedByIsNull = (changedBy <= 0) ? 1 : 0;

    MYSQL_BIND historyBind[5] = {};

    historyBind[0].buffer_type = MYSQL_TYPE_LONG;
    historyBind[0].buffer = &emergencyId;

    historyBind[1].buffer_type = MYSQL_TYPE_STRING;
    historyBind[1].buffer = oldStatusString.data();
    historyBind[1].buffer_length = historyOldStatusLength;
    historyBind[1].length = &historyOldStatusLength;

    historyBind[2].buffer_type = MYSQL_TYPE_STRING;
    historyBind[2].buffer = newStatusString.data();
    historyBind[2].buffer_length = newStatusLength;
    historyBind[2].length = &newStatusLength;

    historyBind[3].buffer_type = MYSQL_TYPE_LONG;
    historyBind[3].buffer = &changedBy;
    historyBind[3].is_null = &changedByIsNull;

    historyBind[4].buffer_type = MYSQL_TYPE_STRING;
    historyBind[4].buffer = notesString.data();
    historyBind[4].buffer_length = notesLength;
    historyBind[4].length = &notesLength;

    bool historySuccess =
        mysql_stmt_bind_param(historyStmt, historyBind) == 0 &&
        mysql_stmt_execute(historyStmt) == 0;

    if (!historySuccess)
        reportStatementError(historyStmt, "INSERT status history");

    mysql_stmt_close(historyStmt);

    if (!historySuccess)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool EmergencyRepository::emergencyExists(int emergencyId)
{
    if (!database.isConnected() || emergencyId <= 0)
        return false;

    const char* query =
        "SELECT emergency_id FROM emergencies "
        "WHERE emergency_id = ? LIMIT 1";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND bind[1] = {};
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &emergencyId;

    bool success =
        mysql_stmt_bind_param(stmt, bind) == 0 &&
        mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_store_result(stmt) == 0;

    bool exists = success && mysql_stmt_num_rows(stmt) > 0;

    mysql_stmt_close(stmt);
    return exists;
}

int EmergencyRepository::getMaxEmergencyId()
{
    if (!database.isConnected())
        return 0;

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, "SELECT COALESCE(MAX(emergency_id), 0) FROM emergencies") != 0)
        return 0;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return 0;

    int maxId = 0;
    MYSQL_ROW row = mysql_fetch_row(result);

    if (row != nullptr && row[0] != nullptr)
        maxId = std::atoi(row[0]);

    mysql_free_result(result);
    return maxId;
}

bool EmergencyRepository::displayStatusHistory(int emergencyId)
{
    if (!database.isConnected() || emergencyId <= 0)
        return false;

    const char* query =
        "SELECT COALESCE(old_status, '-'), new_status, "
        "COALESCE(notes, ''), changed_at "
        "FROM emergency_status_history "
        "WHERE emergency_id = ? ORDER BY status_history_id";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND input[1] = {};
    input[0].buffer_type = MYSQL_TYPE_LONG;
    input[0].buffer = &emergencyId;

    char oldStatus[32] = {};
    char newStatus[32] = {};
    char notes[512] = {};
    char changedAt[40] = {};
    unsigned long oldLength = 0, newLength = 0, notesLength = 0, changedLength = 0;

    MYSQL_BIND output[4] = {};
    output[0].buffer_type = MYSQL_TYPE_STRING;
    output[0].buffer = oldStatus;
    output[0].buffer_length = sizeof(oldStatus) - 1;
    output[0].length = &oldLength;
    output[1].buffer_type = MYSQL_TYPE_STRING;
    output[1].buffer = newStatus;
    output[1].buffer_length = sizeof(newStatus) - 1;
    output[1].length = &newLength;
    output[2].buffer_type = MYSQL_TYPE_STRING;
    output[2].buffer = notes;
    output[2].buffer_length = sizeof(notes) - 1;
    output[2].length = &notesLength;
    output[3].buffer_type = MYSQL_TYPE_STRING;
    output[3].buffer = changedAt;
    output[3].buffer_length = sizeof(changedAt) - 1;
    output[3].length = &changedLength;

    bool ok =
        mysql_stmt_bind_param(stmt, input) == 0 &&
        mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_bind_result(stmt, output) == 0 &&
        mysql_stmt_store_result(stmt) == 0;

    if (!ok)
    {
        reportStatementError(stmt, "SELECT status history");
        mysql_stmt_close(stmt);
        return false;
    }

    std::cout << "\n===== STATUS HISTORY FOR EMERGENCY " << emergencyId << " =====\n";

    int rows = 0;

    while (mysql_stmt_fetch(stmt) == 0)
    {
        std::cout << changedAt << " | "
                  << oldStatus << " -> " << newStatus;

        if (notes[0] != '\0')
            std::cout << " | " << notes;

        std::cout << '\n';
        ++rows;
        std::memset(oldStatus, 0, sizeof(oldStatus));
        std::memset(newStatus, 0, sizeof(newStatus));
        std::memset(notes, 0, sizeof(notes));
        std::memset(changedAt, 0, sizeof(changedAt));
    }

    if (rows == 0)
        std::cout << "No history recorded.\n";

    mysql_stmt_close(stmt);
    return true;
}
