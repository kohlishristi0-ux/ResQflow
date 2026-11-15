#include "../../include/Database/ResponderRepository.h"
#include <mysql.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <cstring>
#include <cstdlib>

using namespace std;

ResponderRepository::ResponderRepository(DatabaseConnection& db)
    : database(db)
{
}

static string escapeValue(MYSQL* connection, const string& value)
{
    string escaped;
    escaped.resize(value.length() * 2 + 1);
    unsigned long length = mysql_real_escape_string(
        connection,
        &escaped[0],
        value.c_str(),
        static_cast<unsigned long>(value.length())
    );
    escaped.resize(length);
    return escaped;
}

static string sqlValue(MYSQL* connection, const string& value)
{
    return "'" + escapeValue(connection, value) + "'";
}

static const char* responderStatusText(ResponderStatus status)
{
    switch (status)
    {
        case ResponderStatus::AVAILABLE:
            return "AVAILABLE";
        case ResponderStatus::ASSIGNED:
            return "ASSIGNED";
        case ResponderStatus::OFF_DUTY:
            return "OFF_DUTY";
        default:
            return nullptr;
    }
}

static bool parseResponderStatus(const string& value, ResponderStatus& status)
{
    if (value == "AVAILABLE")
        status = ResponderStatus::AVAILABLE;
    else if (value == "ASSIGNED")
        status = ResponderStatus::ASSIGNED;
    else if (value == "OFF_DUTY")
        status = ResponderStatus::OFF_DUTY;
    else
        return false;

    return true;
}

bool ResponderRepository::saveResponder(const Responder& responder)
{
    MYSQL* connection = database.getConnection();

    if (!connection || responder.getResponderId() <= 0)
        return false;

    const char* status = responderStatusText(responder.getStatus());

    if (!status || responder.getStatus() == ResponderStatus::ASSIGNED)
        return false;

    if (!isfinite(responder.getLatitude()) ||
        !isfinite(responder.getLongitude()))
        return false;

    ostringstream query;

    query << "INSERT INTO responders "
          << "(responder_id, responder_name, role, contact_number, "
          << "specialization, latitude, longitude, status, is_active) "
          << "VALUES ("
          << responder.getResponderId() << ","
          << sqlValue(connection, responder.getResponderName()) << ","
          << sqlValue(connection, responder.getRole()) << ","
          << sqlValue(connection, responder.getContactNumber()) << ","
          << sqlValue(connection, responder.getSpecialization()) << ","
          << responder.getLatitude() << ","
          << responder.getLongitude() << ","
          << sqlValue(connection, status) << ",TRUE)";

    return database.executeQuery(query.str().c_str());
}

bool ResponderRepository::loadActiveResponders(ResponderManager& manager)
{
    MYSQL* connection = database.getConnection();

    if (!connection)
        return false;

    const char* query =
        "SELECT r.responder_id, r.responder_name, r.role, "
        "r.contact_number, r.specialization, r.latitude, "
        "r.longitude, r.status, "
        "COALESCE(a.emergency_id, -1) "
        "FROM responders r "
        "LEFT JOIN emergency_responder_assignments a "
        "ON r.responder_id = a.responder_id "
        "AND a.assignment_status = 'ACTIVE' "
        "WHERE r.is_active = TRUE "
        "ORDER BY r.responder_id";

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return false;

    manager.clearResponders();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        if (!row[0] || !row[1] || !row[2] ||
            !row[3] || !row[4] || !row[7] || !row[8])
        {
            success = false;
            break;
        }

        ResponderStatus status;

        if (!parseResponderStatus(row[7], status))
        {
            success = false;
            break;
        }

        int assignedEmergencyId = atoi(row[8]);

        // Coordinates may be NULL for newly registered responders.
        double latitude = row[5] ? atof(row[5]) : 0.0;
        double longitude = row[6] ? atof(row[6]) : 0.0;

        if (!manager.loadResponderRecord(
                atoi(row[0]),
                row[1],
                row[2],
                row[3],
                row[4],
                latitude,
                longitude,
                status,
                assignedEmergencyId))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        manager.clearResponders();

    return success;
}

bool ResponderRepository::updateResponder(const Responder& responder)
{
    MYSQL* connection = database.getConnection();

    if (!connection || responder.getResponderId() <= 0)
        return false;

    if (!isfinite(responder.getLatitude()) ||
        !isfinite(responder.getLongitude()))
        return false;

    ostringstream query;

    query << "UPDATE responders SET "
          << "responder_name = "
          << sqlValue(connection, responder.getResponderName())
          << ", role = "
          << sqlValue(connection, responder.getRole())
          << ", contact_number = "
          << sqlValue(connection, responder.getContactNumber())
          << ", specialization = "
          << sqlValue(connection, responder.getSpecialization())
          << ", latitude = " << responder.getLatitude()
          << ", longitude = " << responder.getLongitude()
          << " WHERE responder_id = "
          << responder.getResponderId()
          << " AND is_active = TRUE";

    if (!database.executeQuery(query.str().c_str()))
        return false;

    return mysql_affected_rows(connection) == 1;
}

bool ResponderRepository::deactivateResponder(int responderId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || responderId <= 0)
        return false;

    ostringstream query;

    query << "UPDATE responders SET is_active = FALSE "
          << "WHERE responder_id = " << responderId
          << " AND status <> 'ASSIGNED' "
          << "AND is_active = TRUE";

    if (!database.executeQuery(query.str().c_str()))
        return false;

    return mysql_affected_rows(connection) == 1;
}

bool ResponderRepository::responderExists(int responderId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || responderId <= 0)
        return false;

    ostringstream query;

    query << "SELECT responder_id FROM responders "
          << "WHERE responder_id = " << responderId
          << " LIMIT 1";

    if (mysql_query(connection, query.str().c_str()) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return false;

    bool exists = mysql_num_rows(result) > 0;

    mysql_free_result(result);

    return exists;
}

bool ResponderRepository::assignResponder(
    int responderId,
    int emergencyId,
    const char* notes
)
{
    MYSQL* connection = database.getConnection();

    if (!connection || responderId <= 0 || emergencyId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    ostringstream updateQuery;

    updateQuery
        << "UPDATE responders SET status = 'ASSIGNED' "
        << "WHERE responder_id = " << responderId
        << " AND status = 'AVAILABLE' "
        << "AND is_active = TRUE";

    if (!database.executeQuery(updateQuery.str().c_str()) ||
        mysql_affected_rows(connection) != 1)
    {
        database.rollbackTransaction();
        return false;
    }

    string noteValue = notes ? sqlValue(connection, notes) : "NULL";

    ostringstream insertQuery;

    insertQuery
        << "INSERT INTO emergency_responder_assignments "
        << "(responder_id, emergency_id, assignment_status, notes) "
        << "VALUES ("
        << responderId << ","
        << emergencyId << ",'ACTIVE',"
        << noteValue << ")";

    if (!database.executeQuery(insertQuery.str().c_str()))
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool ResponderRepository::releaseResponder(int responderId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || responderId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    ostringstream assignmentQuery;

    assignmentQuery
        << "UPDATE emergency_responder_assignments "
        << "SET assignment_status = 'RELEASED', "
        << "released_at = NOW() "
        << "WHERE responder_id = " << responderId
        << " AND assignment_status = 'ACTIVE'";

    if (!database.executeQuery(assignmentQuery.str().c_str()) ||
        mysql_affected_rows(connection) != 1)
    {
        database.rollbackTransaction();
        return false;
    }

    ostringstream responderQuery;

    responderQuery
        << "UPDATE responders SET status = 'AVAILABLE' "
        << "WHERE responder_id = " << responderId
        << " AND status = 'ASSIGNED' "
        << "AND is_active = TRUE";

    if (!database.executeQuery(responderQuery.str().c_str()) ||
        mysql_affected_rows(connection) != 1)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool ResponderRepository::linkUserToResponder(int userId, int responderId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || userId <= 0 || responderId <= 0)
        return false;

    const char* query =
        "UPDATE responders SET user_id = ? "
        "WHERE responder_id = ? AND is_active = TRUE "
        "AND (user_id IS NULL OR user_id = ?) "
        "AND EXISTS (SELECT 1 FROM users u WHERE u.user_id = ? "
        "AND u.role = 'RESPONDER' AND u.is_active = TRUE)";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (!stmt)
        return false;

    MYSQL_BIND b[4] = {};
    int values[4] = {userId, responderId, userId, userId};

    for (int i = 0; i < 4; ++i)
    {
        b[i].buffer_type = MYSQL_TYPE_LONG;
        b[i].buffer = &values[i];
    }

    bool ok =
        mysql_stmt_bind_param(stmt, b) == 0 &&
        mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_affected_rows(stmt) >= 1;

    mysql_stmt_close(stmt);

    return ok;
}

int ResponderRepository::findResponderIdForUser(int userId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || userId <= 0)
        return -1;

    MYSQL_STMT* stmt = mysql_stmt_init(connection);

    if (!stmt)
        return -1;

    const char* query =
        "SELECT responder_id FROM responders "
        "WHERE user_id = ? AND is_active = TRUE LIMIT 1";

    if (mysql_stmt_prepare(stmt, query,
        static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(stmt);
        return -1;
    }

    MYSQL_BIND in[1] = {};
    in[0].buffer_type = MYSQL_TYPE_LONG;
    in[0].buffer = &userId;

    int id = -1;

    MYSQL_BIND out[1] = {};
    out[0].buffer_type = MYSQL_TYPE_LONG;
    out[0].buffer = &id;

    bool ok =
        mysql_stmt_bind_param(stmt, in) == 0 &&
        mysql_stmt_execute(stmt) == 0 &&
        mysql_stmt_store_result(stmt) == 0 &&
        mysql_stmt_bind_result(stmt, out) == 0 &&
        mysql_stmt_fetch(stmt) == 0;

    mysql_stmt_close(stmt);

    return ok ? id : -1;
}

bool ResponderRepository::updateResponderStatus(
    int responderId,
    ResponderStatus status
)
{
    MYSQL* connection = database.getConnection();

    const char* statusText = responderStatusText(status);

    if (!connection || responderId <= 0 || !statusText)
        return false;

    // MySQL reports 0 affected rows when the status is unchanged, so existence is verified explicitly.
    if (!responderExists(responderId))
        return false;

    MYSQL_STMT* stmt = mysql_stmt_init(connection);

    if (!stmt)
        return false;

    const char* query =
        "UPDATE responders SET status = ? "
        "WHERE responder_id = ? AND is_active = TRUE";

    if (mysql_stmt_prepare(stmt, query,
        static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    string value(statusText);
    unsigned long length = static_cast<unsigned long>(value.size());

    MYSQL_BIND b[2] = {};

    b[0].buffer_type = MYSQL_TYPE_STRING;
    b[0].buffer = &value[0];
    b[0].buffer_length = length;
    b[0].length = &length;

    b[1].buffer_type = MYSQL_TYPE_LONG;
    b[1].buffer = &responderId;

    bool ok =
        mysql_stmt_bind_param(stmt, b) == 0 &&
        mysql_stmt_execute(stmt) == 0;

    mysql_stmt_close(stmt);

    return ok;
}

bool ResponderRepository::activateResponderForUser(int userId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || userId <= 0)
        return false;

    ostringstream query;

    query << "UPDATE responders SET is_active = TRUE "
          << "WHERE user_id = " << userId
          << " AND is_active = FALSE";

    return database.executeQuery(query.str().c_str());
}

// ======================================================
// RESPONDER SELF-REGISTRATION
// ======================================================

bool ResponderRepository::submitResponderRegistration(
    const string& username,
    const string& passwordHash,
    const string& fullName,
    const string& email,
    const string& contact,
    const string& responderRole,
    const string& specialization,
    const string& city,
    const string& area,
    const string& baseAddress
)
{
    MYSQL* connection = database.getConnection();

    if (!connection ||
        username.empty() ||
        passwordHash.empty() ||
        fullName.empty() ||
        contact.empty() ||
        responderRole.empty() ||
        specialization.empty() ||
        city.empty() ||
        area.empty() ||
        baseAddress.empty())
    {
        return false;
    }

    // Check whether the username is already registered.
    string existingQuery =
        "SELECT user_id FROM users WHERE username = " +
        sqlValue(connection, username) + " LIMIT 1";

    if (mysql_query(connection, existingQuery.c_str()) != 0)
        return false;

    MYSQL_RES* existingResult = mysql_store_result(connection);

    if (!existingResult)
        return false;

    bool usernameTaken = mysql_num_rows(existingResult) > 0;

    mysql_free_result(existingResult);

    if (usernameTaken)
        return false;

    // Also prevent duplicate pending registrations.
    existingQuery =
        "SELECT registration_id FROM responder_registration_requests "
        "WHERE username = " + sqlValue(connection, username) +
        " AND status = 'PENDING' LIMIT 1";

    if (mysql_query(connection, existingQuery.c_str()) != 0)
        return false;

    existingResult = mysql_store_result(connection);

    if (!existingResult)
        return false;

    bool pendingExists = mysql_num_rows(existingResult) > 0;

    mysql_free_result(existingResult);

    if (pendingExists)
        return false;

    ostringstream query;

    query
        << "INSERT INTO responder_registration_requests "
        << "(username, password_hash, full_name, email, contact_number, "
        << "responder_role, specialization, city, area, base_address, status) "
        << "VALUES ("
        << sqlValue(connection, username) << ","
        << sqlValue(connection, passwordHash) << ","
        << sqlValue(connection, fullName) << ","
        << (email.empty() ? "NULL" : sqlValue(connection, email)) << ","
        << sqlValue(connection, contact) << ","
        << sqlValue(connection, responderRole) << ","
        << sqlValue(connection, specialization) << ","
        << sqlValue(connection, city) << ","
        << sqlValue(connection, area) << ","
        << sqlValue(connection, baseAddress) << ","
        << "'PENDING')";

    return database.executeQuery(query.str().c_str());
}

// ======================================================
// DISPLAY PENDING RESPONDER REGISTRATIONS
// ======================================================

void ResponderRepository::displayPendingResponderRegistrations()
{
    MYSQL* connection = database.getConnection();

    if (!connection)
    {
        cout << "Database connection unavailable.\n";
        return;
    }

    const char* query =
        "SELECT registration_id, full_name, username, contact_number, "
        "responder_role, specialization, city, area, base_address, created_at "
        "FROM responder_registration_requests "
        "WHERE status = 'PENDING' "
        "ORDER BY created_at";

    if (mysql_query(connection, query) != 0)
    {
        cout << "Could not load pending registrations. "
             << "Check the responder registration database migration.\n";
        return;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
    {
        cout << "No pending registrations found.\n";
        return;
    }

    MYSQL_ROW row;
    bool any = false;

    cout << "\n========== PENDING RESPONDER REGISTRATIONS ==========\n";

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        any = true;

        cout << "\nRegistration ID: "
             << (row[0] ? row[0] : "N/A")
             << "\nName: "
             << (row[1] ? row[1] : "N/A")
             << "\nUsername: "
             << (row[2] ? row[2] : "N/A")
             << "\nContact: "
             << (row[3] ? row[3] : "N/A")
             << "\nOperational Role: "
             << (row[4] ? row[4] : "N/A")
             << "\nSpecialization: "
             << (row[5] ? row[5] : "N/A")
             << "\nCity: "
             << (row[6] ? row[6] : "N/A")
             << "\nArea: "
             << (row[7] ? row[7] : "N/A")
             << "\nBase Address: "
             << (row[8] ? row[8] : "N/A")
             << "\nSubmitted: "
             << (row[9] ? row[9] : "N/A")
             << "\n------------------------------------------\n";
    }

    if (!any)
        cout << "No pending registrations found.\n";

    mysql_free_result(result);
}

// ======================================================
// APPROVE RESPONDER REGISTRATION
// Automatically creates and links the responder profile.
// ======================================================

bool ResponderRepository::approveResponderRegistration(int registrationId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || registrationId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    string query =
        "SELECT username, password_hash, full_name, email, contact_number, "
        "responder_role, specialization, city, area, base_address "
        "FROM responder_registration_requests "
        "WHERE registration_id = " + to_string(registrationId) +
        " AND status = 'PENDING' FOR UPDATE";

    if (mysql_query(connection, query.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    MYSQL_ROW row = result ? mysql_fetch_row(result) : nullptr;

    if (!row)
    {
        if (result)
            mysql_free_result(result);

        database.rollbackTransaction();
        return false;
    }

    // Copy values before releasing the result.
    string username = row[0] ? row[0] : "";
    string passwordHash = row[1] ? row[1] : "";
    string fullName = row[2] ? row[2] : "";
    string email = row[3] ? row[3] : "";
    string contact = row[4] ? row[4] : "";
    string responderRole = row[5] ? row[5] : "";
    string specialization = row[6] ? row[6] : "";
    string city = row[7] ? row[7] : "";
    string area = row[8] ? row[8] : "";
    string baseAddress = row[9] ? row[9] : "";

    mysql_free_result(result);

    if (username.empty() || passwordHash.empty() || fullName.empty() ||
        contact.empty() || responderRole.empty() || city.empty() ||
        area.empty() || baseAddress.empty())
    {
        database.rollbackTransaction();
        return false;
    }

    // Create the login account.
    query =
        "INSERT INTO users "
        "(username, password_hash, full_name, email, contact_number, role, is_active) "
        "VALUES (" +
        sqlValue(connection, username) + "," +
        sqlValue(connection, passwordHash) + "," +
        sqlValue(connection, fullName) + "," +
        (email.empty() ? "NULL" : sqlValue(connection, email)) + "," +
        sqlValue(connection, contact) +
        ",'RESPONDER',TRUE)";

    if (mysql_query(connection, query.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    int userId = static_cast<int>(mysql_insert_id(connection));

    if (userId <= 0)
    {
        database.rollbackTransaction();
        return false;
    }

    // Generate a responder ID.
    if (mysql_query(
        connection,
        "SELECT COALESCE(MAX(responder_id),100000)+1 FROM responders"
    ) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    result = mysql_store_result(connection);
    row = result ? mysql_fetch_row(result) : nullptr;

    int responderId =
        (row && row[0]) ? atoi(row[0]) : -1;

    if (result)
        mysql_free_result(result);

    if (responderId <= 0)
    {
        database.rollbackTransaction();
        return false;
    }

    // Create the responder profile and link it to the new user.
    // Coordinates remain NULL until a location is selected later.
    query =
        "INSERT INTO responders "
        "(responder_id, responder_name, role, contact_number, "
        "specialization, city, area, base_address, latitude, longitude, "
        "status, user_id, is_active) "
        "VALUES (" +
        to_string(responderId) + "," +
        sqlValue(connection, fullName) + "," +
        sqlValue(connection, responderRole) + "," +
        sqlValue(connection, contact) + "," +
        sqlValue(connection, specialization) + "," +
        sqlValue(connection, city) + "," +
        sqlValue(connection, area) + "," +
        sqlValue(connection, baseAddress) +
        ",NULL,NULL,'AVAILABLE'," +
        to_string(userId) + ",TRUE)";

    if (mysql_query(connection, query.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    // Mark the registration as approved.
    query =
        "UPDATE responder_registration_requests "
        "SET status = 'APPROVED', reviewed_at = CURRENT_TIMESTAMP "
        "WHERE registration_id = " + to_string(registrationId) +
        " AND status = 'PENDING'";

    if (mysql_query(connection, query.c_str()) != 0 ||
        mysql_affected_rows(connection) != 1)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

// ======================================================
// REJECT RESPONDER REGISTRATION
// ======================================================

bool ResponderRepository::rejectResponderRegistration(int registrationId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || registrationId <= 0)
        return false;

    string query =
        "UPDATE responder_registration_requests "
        "SET status = 'REJECTED', reviewed_at = CURRENT_TIMESTAMP "
        "WHERE registration_id = " + to_string(registrationId) +
        " AND status = 'PENDING'";

    return database.executeQuery(query.c_str()) &&
           mysql_affected_rows(connection) == 1;
}