#include "../include/ResponderModule.h"
#include <iostream>
#include <cmath>
#include <new>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <climits>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <iomanip>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

Responder::Responder()
    : responderId(0), responderName(""), role(""), contactNumber(""), specialization(""), latitude(0.0),
      longitude(0.0), status(ResponderStatus::AVAILABLE), assignedEmergencyId(-1)
{
}
Responder::Responder(int id, string name, string responderRole, string contact,
                     string responderSpecialization, double lat, double lon)
    : responderId(id), responderName(name), role(responderRole), contactNumber(contact),
      specialization(responderSpecialization), latitude(0.0), longitude(0.0),
      status(ResponderStatus::AVAILABLE), assignedEmergencyId(-1)
{
    if (!setCoordinates(lat, lon))
    {
        latitude = lat;
        longitude = lon;
    }
}
int Responder::getResponderId() const
{
    return responderId;
}
string Responder::getResponderName() const
{
    return responderName;
}
string Responder::getRole() const
{
    return role;
}
string Responder::getContactNumber() const
{
    return contactNumber;
}
string Responder::getSpecialization() const
{
    return specialization;
}
double Responder::getLatitude() const
{
    return latitude;
}
double Responder::getLongitude() const
{
    return longitude;
}
ResponderStatus Responder::getStatus() const
{
    return status;
}
int Responder::getAssignedEmergencyId() const
{
    return assignedEmergencyId;
}
void Responder::setResponderName(string name)
{
    if (!name.empty())
        responderName = name;
}
void Responder::setRole(string value)
{
    if (!value.empty())
        role = value;
}
void Responder::setContactNumber(string value)
{
    if (!value.empty())
        contactNumber = value;
}
void Responder::setSpecialization(string value)
{
    if (!value.empty())
        specialization = value;
}
bool Responder::setCoordinates(double lat, double lon)
{
    if (!isfinite(lat) || !isfinite(lon) || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0)
        return false;
    latitude = lat;
    longitude = lon;
    return true;
}
bool Responder::assignEmergency(int id)
{
    if (id <= 0 || status != ResponderStatus::AVAILABLE)
        return false;
    assignedEmergencyId = id;
    status = ResponderStatus::ASSIGNED;
    return true;
}
bool Responder::releaseResponder()
{
    if (status != ResponderStatus::ASSIGNED)
        return false;
    assignedEmergencyId = -1;
    status = ResponderStatus::AVAILABLE;
    return true;
}
bool Responder::setOffDuty()
{
    if (status == ResponderStatus::ASSIGNED)
        return false;
    status = ResponderStatus::OFF_DUTY;
    return true;
}
bool Responder::setAvailable()
{
    if (status == ResponderStatus::ASSIGNED)
        return false;
    status = ResponderStatus::AVAILABLE;
    return true;
}
string Responder::getStatusText() const
{
    switch (status)
    {
        case ResponderStatus::AVAILABLE:
            return "Available";
        case ResponderStatus::ASSIGNED:
            return "Assigned";
        case ResponderStatus::OFF_DUTY:
            return "Off Duty";
        default:
            return "Unknown";
    }
}
void Responder::displayResponder() const
{
    cout << "\nResponder ID: " << responderId << "\nName: " << responderName << "\nRole: " << role
         << "\nContact: " << contactNumber << "\nSpecialization: " << specialization
         << "\nLatitude: " << latitude << "\nLongitude: " << longitude << "\nStatus: " << getStatusText();
    if (status == ResponderStatus::ASSIGNED)
        cout << "\nAssigned Emergency ID: " << assignedEmergencyId;
    cout << "\n";
}

bool Responder::restoreState(ResponderStatus restoredStatus, int emergencyId)
{
    if (restoredStatus == ResponderStatus::ASSIGNED)
    {
        if (emergencyId <= 0)
            return false;
    }
    else
    {
        if (emergencyId != -1)
            return false;
    }

    if (restoredStatus != ResponderStatus::AVAILABLE && restoredStatus != ResponderStatus::ASSIGNED &&
        restoredStatus != ResponderStatus::OFF_DUTY)
        return false;

    status = restoredStatus;
    assignedEmergencyId = emergencyId;
    return true;
}

ResponderCircularList::ResponderCircularList() : tail(nullptr), cursor(nullptr), count(0)
{
}
ResponderCircularList::~ResponderCircularList()
{
    if (!tail)
        return;
    Node* current = tail->next;
    tail->next = nullptr;
    while (current)
    {
        Node* next = current->next;
        delete current;
        current = next;
    }
}
bool ResponderCircularList::contains(int id) const
{
    if (!tail)
        return false;
    Node* current = tail->next;
    do
    {
        if (current->responderId == id)
            return true;
        current = current->next;
    } while (current != tail->next);
    return false;
}
bool ResponderCircularList::addResponderId(int id)
{
    if (id <= 0 || contains(id))
        return false;
    Node* node = new (std::nothrow) Node(id);
    if (!node)
        return false;
    if (!tail)
    {
        node->next = node;
        tail = node;
        cursor = node;
    }
    else
    {
        node->next = tail->next;
        tail->next = node;
        tail = node;
    }
    ++count;
    return true;
}
bool ResponderCircularList::removeResponderId(int id)
{
    if (!tail)
        return false;
    Node* previous = tail;
    Node* current = tail->next;
    do
    {
        if (current->responderId == id)
        {
            if (current == previous)
                tail = nullptr;
            else
            {
                previous->next = current->next;
                if (current == tail)
                    tail = previous;
            }
            if (cursor == current)
                cursor = tail ? current->next : nullptr;
            delete current;
            --count;
            if (tail && !cursor)
                cursor = tail->next;
            return true;
        }
        previous = current;
        current = current->next;
    } while (current != tail->next);
    return false;
}
bool ResponderCircularList::getNextResponderId(int& id)
{
    if (!cursor)
        return false;
    id = cursor->responderId;
    cursor = cursor->next;
    return true;
}
int ResponderCircularList::getCount() const
{
    return count;
}
bool ResponderCircularList::isEmpty() const
{
    return tail == nullptr;
}

ResponderManager::ResponderManager() : head(nullptr), tail(nullptr), responderCount(0), scheduler()
{
}
ResponderManager::~ResponderManager()
{
    Node* current = head;
    while (current)
    {
        Node* next = current->next;
        delete current;
        current = next;
    }
}
bool ResponderManager::addResponder(const Responder& responder)
{
    if (responder.getResponderId() <= 0 || responder.getResponderName().empty() ||
        responder.getRole().empty() || responder.getContactNumber().empty() ||
        responder.getSpecialization().empty() || !isfinite(responder.getLatitude()) ||
        !isfinite(responder.getLongitude()) || responder.getLatitude() < -90.0 ||
        responder.getLatitude() > 90.0 || responder.getLongitude() < -180.0 ||
        responder.getLongitude() > 180.0 || findResponder(responder.getResponderId()))
        return false;
    Node* node = nullptr;
    try
    {
        node = new (nothrow) Node(responder);
    }
    catch (...)
    {
        return false;
    }
    if (!node)
        return false;
    if (!scheduler.addResponderId(responder.getResponderId()))
    {
        delete node;
        return false;
    }
    if (!head)
        head = tail = node;
    else
    {
        tail->next = node;
        tail = node;
    }
    ++responderCount;
    return true;
}
Responder* ResponderManager::findResponder(int id)
{
    Node* current = head;
    while (current)
    {
        if (current->data.getResponderId() == id)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}
bool ResponderManager::removeResponder(int id)
{
    Node* current = head;
    Node* previous = nullptr;
    while (current && current->data.getResponderId() != id)
    {
        previous = current;
        current = current->next;
    }
    if (!current || current->data.getStatus() == ResponderStatus::ASSIGNED)
        return false;
    if (!scheduler.removeResponderId(id))
        return false;
    if (previous)
        previous->next = current->next;
    else
        head = current->next;
    if (tail == current)
        tail = previous;
    delete current;
    --responderCount;
    return true;
}
bool ResponderManager::assignResponder(int id, int emergencyId)
{
    Responder* responder = findResponder(id);
    return responder && responder->assignEmergency(emergencyId);
}
bool ResponderManager::assignNextAvailableResponder(int emergencyId, int& assignedId)
{
    if (emergencyId <= 0 || responderCount == 0)
        return false;
    for (int i = 0; i < responderCount; ++i)
    {
        int candidateId = -1;
        if (!scheduler.getNextResponderId(candidateId))
            return false;
        Responder* candidate = findResponder(candidateId);
        if (candidate && candidate->getStatus() == ResponderStatus::AVAILABLE &&
            candidate->assignEmergency(emergencyId))
        {
            assignedId = candidateId;
            return true;
        }
    }
    return false;
}
bool ResponderManager::releaseResponder(int id)
{
    Responder* r = findResponder(id);
    return r && r->releaseResponder();
}
bool ResponderManager::setResponderOffDuty(int id)
{
    Responder* r = findResponder(id);
    return r && r->setOffDuty();
}
bool ResponderManager::setResponderAvailable(int id)
{
    Responder* r = findResponder(id);
    return r && r->setAvailable();
}
void ResponderManager::displayAllResponders() const
{
    if (!head)
    {
        cout << "\nNo responders registered.\n";
        return;
    }
    Node* current = head;
    while (current)
    {
        current->data.displayResponder();
        current = current->next;
    }
}
void ResponderManager::displayAvailableResponders() const
{
    Node* current = head;
    bool found = false;
    while (current)
    {
        if (current->data.getStatus() == ResponderStatus::AVAILABLE)
        {
            current->data.displayResponder();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo available responders.\n";
}
void ResponderManager::displayAssignedResponders() const
{
    Node* current = head;
    bool found = false;
    while (current)
    {
        if (current->data.getStatus() == ResponderStatus::ASSIGNED)
        {
            current->data.displayResponder();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo assigned responders.\n";
}
int ResponderManager::getResponderCount() const
{
    return responderCount;
}
int ResponderManager::getAvailableCount() const
{
    int count = 0;
    Node* current = head;
    while (current)
    {
        if (current->data.getStatus() == ResponderStatus::AVAILABLE)
            ++count;
        current = current->next;
    }
    return count;
}
bool ResponderManager::isEmpty() const
{
    return head == nullptr;
}

void ResponderManager::clearResponders()
{
    Node* current = head;

    while (current != nullptr)
    {
        scheduler.removeResponderId(current->data.getResponderId());

        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    tail = nullptr;
    responderCount = 0;
}

bool ResponderManager::loadResponderRecord(int responderId, string name, string role, string contact,
                                           string specialization, double latitude, double longitude,
                                           ResponderStatus status, int assignedEmergencyId)
{
    Responder responder(responderId, name, role, contact, specialization, latitude, longitude);

    if (!responder.restoreState(status, assignedEmergencyId))
        return false;

    if (!addResponder(responder))
        return false;

    return true;
}

Responder* ResponderManager::findAssignedToEmergency(int emergencyId)
{
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getStatus() == ResponderStatus::ASSIGNED &&
            current->data.getAssignedEmergencyId() == emergencyId)
            return &current->data;
    return nullptr;
}

ResponderRepository::ResponderRepository(DatabaseConnection& db) : database(db)
{
}

static string escapeValue(MYSQL* connection, const string& value)
{
    string escaped;
    escaped.resize(value.length() * 2 + 1);
    unsigned long length = mysql_real_escape_string(connection, &escaped[0], value.c_str(),
                                                    static_cast<unsigned long>(value.length()));
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

    if (!isfinite(responder.getLatitude()) || !isfinite(responder.getLongitude()))
        return false;

    ostringstream query;

    query << "INSERT INTO responders "
          << "(responder_id, responder_name, role, contact_number, "
          << "specialization, latitude, longitude, status, is_active) "
          << "VALUES (" << responder.getResponderId() << ","
          << sqlValue(connection, responder.getResponderName()) << ","
          << sqlValue(connection, responder.getRole()) << ","
          << sqlValue(connection, responder.getContactNumber()) << ","
          << sqlValue(connection, responder.getSpecialization()) << "," << responder.getLatitude() << ","
          << responder.getLongitude() << "," << sqlValue(connection, status) << ",TRUE)";

    return database.executeQuery(query.str().c_str());
}

bool ResponderRepository::loadActiveResponders(ResponderManager& manager)
{
    MYSQL* connection = database.getConnection();

    if (!connection)
        return false;

    const char* query = "SELECT r.responder_id, r.responder_name, r.role, "
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
        if (!row[0] || !row[1] || !row[2] || !row[3] || !row[4] || !row[7] || !row[8])
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

        double latitude = row[5] ? atof(row[5]) : 0.0;
        double longitude = row[6] ? atof(row[6]) : 0.0;

        if (!manager.loadResponderRecord(atoi(row[0]), row[1], row[2], row[3], row[4], latitude, longitude,
                                         status, assignedEmergencyId))
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

    if (!isfinite(responder.getLatitude()) || !isfinite(responder.getLongitude()))
        return false;

    ostringstream query;

    query << "UPDATE responders SET "
          << "responder_name = " << sqlValue(connection, responder.getResponderName())
          << ", role = " << sqlValue(connection, responder.getRole())
          << ", contact_number = " << sqlValue(connection, responder.getContactNumber())
          << ", specialization = " << sqlValue(connection, responder.getSpecialization())
          << ", latitude = " << responder.getLatitude() << ", longitude = " << responder.getLongitude()
          << " WHERE responder_id = " << responder.getResponderId() << " AND is_active = TRUE";

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
          << "WHERE responder_id = " << responderId << " AND status <> 'ASSIGNED' "
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
          << "WHERE responder_id = " << responderId << " LIMIT 1";

    if (mysql_query(connection, query.str().c_str()) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return false;

    bool exists = mysql_num_rows(result) > 0;

    mysql_free_result(result);

    return exists;
}

bool ResponderRepository::assignResponder(int responderId, int emergencyId, const char* notes)
{
    MYSQL* connection = database.getConnection();

    if (!connection || responderId <= 0 || emergencyId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    ostringstream updateQuery;

    updateQuery << "UPDATE responders SET status = 'ASSIGNED' "
                << "WHERE responder_id = " << responderId << " AND status = 'AVAILABLE' "
                << "AND is_active = TRUE";

    if (!database.executeQuery(updateQuery.str().c_str()) || mysql_affected_rows(connection) != 1)
    {
        database.rollbackTransaction();
        return false;
    }

    string noteValue = notes ? sqlValue(connection, notes) : "NULL";

    ostringstream insertQuery;

    insertQuery << "INSERT INTO emergency_responder_assignments "
                << "(responder_id, emergency_id, assignment_status, notes) "
                << "VALUES (" << responderId << "," << emergencyId << ",'ACTIVE'," << noteValue << ")";

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

    assignmentQuery << "UPDATE emergency_responder_assignments "
                    << "SET assignment_status = 'RELEASED', "
                    << "released_at = NOW() "
                    << "WHERE responder_id = " << responderId << " AND assignment_status = 'ACTIVE'";

    if (!database.executeQuery(assignmentQuery.str().c_str()) || mysql_affected_rows(connection) != 1)
    {
        database.rollbackTransaction();
        return false;
    }

    ostringstream responderQuery;

    responderQuery << "UPDATE responders SET status = 'AVAILABLE' "
                   << "WHERE responder_id = " << responderId << " AND status = 'ASSIGNED' "
                   << "AND is_active = TRUE";

    if (!database.executeQuery(responderQuery.str().c_str()) || mysql_affected_rows(connection) != 1)
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

    const char* query = "UPDATE responders SET user_id = ? "
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

    bool ok = mysql_stmt_bind_param(stmt, b) == 0 && mysql_stmt_execute(stmt) == 0 &&
              mysql_stmt_affected_rows(stmt) >= 1;

    mysql_stmt_close(stmt);

    return ok;
}

int ResponderRepository::findResponderIdForUser(int userId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || userId <= 0)
        return -1;

    const char* query = "SELECT responder_id FROM responders "
                        "WHERE user_id = ? AND is_active = TRUE LIMIT 1";
    MYSQL_STMT* stmt = database.prepareStatement(query);
    if (!stmt)
        return -1;

    MYSQL_BIND in[1] = {};
    bindInt(in[0], userId);

    int id = -1;

    MYSQL_BIND out[1] = {};
    bindInt(out[0], id);

    bool ok = mysql_stmt_bind_param(stmt, in) == 0 && mysql_stmt_execute(stmt) == 0 &&
              mysql_stmt_store_result(stmt) == 0 && mysql_stmt_bind_result(stmt, out) == 0 &&
              mysql_stmt_fetch(stmt) == 0;

    mysql_stmt_close(stmt);

    return ok ? id : -1;
}

bool ResponderRepository::updateResponderStatus(int responderId, ResponderStatus status)
{
    MYSQL* connection = database.getConnection();

    const char* statusText = responderStatusText(status);

    if (!connection || responderId <= 0 || !statusText)
        return false;

    if (!responderExists(responderId))
        return false;

    const char* query = "UPDATE responders SET status = ? "
                        "WHERE responder_id = ? AND is_active = TRUE";
    MYSQL_STMT* stmt = database.prepareStatement(query);
    if (!stmt)
        return false;

    string value(statusText);
    unsigned long length = static_cast<unsigned long>(value.size());

    MYSQL_BIND b[2] = {};

    bindString(b[0], value, length);

    bindInt(b[1], responderId);

    bool ok = mysql_stmt_bind_param(stmt, b) == 0 && mysql_stmt_execute(stmt) == 0;

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
          << "WHERE user_id = " << userId << " AND is_active = FALSE";

    return database.executeQuery(query.str().c_str());
}

bool ResponderRepository::submitResponderRegistration(const string& username, const string& passwordHash,
                                                      const string& fullName, const string& email,
                                                      const string& contact, const string& responderRole,
                                                      const string& specialization, const string& city,
                                                      const string& area, const string& baseAddress)
{
    MYSQL* connection = database.getConnection();

    if (!connection || username.empty() || passwordHash.empty() || fullName.empty() || contact.empty() ||
        responderRole.empty() || specialization.empty() || city.empty() || area.empty() ||
        baseAddress.empty())
    {
        return false;
    }

    string existingQuery =
            "SELECT user_id FROM users WHERE username = " + sqlValue(connection, username) + " LIMIT 1";

    if (mysql_query(connection, existingQuery.c_str()) != 0)
        return false;

    MYSQL_RES* existingResult = mysql_store_result(connection);

    if (!existingResult)
        return false;

    bool usernameTaken = mysql_num_rows(existingResult) > 0;

    mysql_free_result(existingResult);

    if (usernameTaken)
        return false;

    existingQuery = "SELECT registration_id FROM responder_registration_requests "
                    "WHERE username = " +
                    sqlValue(connection, username) + " AND status = 'PENDING' LIMIT 1";

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

    query << "INSERT INTO responder_registration_requests "
          << "(username, password_hash, full_name, email, contact_number, "
          << "responder_role, specialization, city, area, base_address, status) "
          << "VALUES (" << sqlValue(connection, username) << "," << sqlValue(connection, passwordHash) << ","
          << sqlValue(connection, fullName) << "," << (email.empty() ? "NULL" : sqlValue(connection, email))
          << "," << sqlValue(connection, contact) << "," << sqlValue(connection, responderRole) << ","
          << sqlValue(connection, specialization) << "," << sqlValue(connection, city) << ","
          << sqlValue(connection, area) << "," << sqlValue(connection, baseAddress) << ","
          << "'PENDING')";

    return database.executeQuery(query.str().c_str());
}

void ResponderRepository::displayPendingResponderRegistrations()
{
    MYSQL* connection = database.getConnection();

    if (!connection)
    {
        cout << "Database connection unavailable.\n";
        return;
    }

    const char* query = "SELECT registration_id, full_name, username, contact_number, "
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

        cout << "\nRegistration ID: " << (row[0] ? row[0] : "N/A") << "\nName: " << (row[1] ? row[1] : "N/A")
             << "\nUsername: " << (row[2] ? row[2] : "N/A") << "\nContact: " << (row[3] ? row[3] : "N/A")
             << "\nOperational Role: " << (row[4] ? row[4] : "N/A")
             << "\nSpecialization: " << (row[5] ? row[5] : "N/A") << "\nCity: " << (row[6] ? row[6] : "N/A")
             << "\nArea: " << (row[7] ? row[7] : "N/A") << "\nBase Address: " << (row[8] ? row[8] : "N/A")
             << "\nSubmitted: " << (row[9] ? row[9] : "N/A")
             << "\n------------------------------------------\n";
    }

    if (!any)
        cout << "No pending registrations found.\n";

    mysql_free_result(result);
}

bool ResponderRepository::approveResponderRegistration(int registrationId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || registrationId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    string query = "SELECT username, password_hash, full_name, email, contact_number, "
                   "responder_role, specialization, city, area, base_address "
                   "FROM responder_registration_requests "
                   "WHERE registration_id = " +
                   to_string(registrationId) + " AND status = 'PENDING' FOR UPDATE";

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

    if (username.empty() || passwordHash.empty() || fullName.empty() || contact.empty() ||
        responderRole.empty() || city.empty() || area.empty() || baseAddress.empty())
    {
        database.rollbackTransaction();
        return false;
    }

    query = "INSERT INTO users "
            "(username, password_hash, full_name, email, contact_number, role, is_active) "
            "VALUES (" +
            sqlValue(connection, username) + "," + sqlValue(connection, passwordHash) + "," +
            sqlValue(connection, fullName) + "," + (email.empty() ? "NULL" : sqlValue(connection, email)) +
            "," + sqlValue(connection, contact) + ",'RESPONDER',TRUE)";

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

    if (mysql_query(connection, "SELECT COALESCE(MAX(responder_id),100000)+1 FROM responders") != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    result = mysql_store_result(connection);
    row = result ? mysql_fetch_row(result) : nullptr;

    int responderId = (row && row[0]) ? atoi(row[0]) : -1;

    if (result)
        mysql_free_result(result);

    if (responderId <= 0)
    {
        database.rollbackTransaction();
        return false;
    }

    query = "INSERT INTO responders "
            "(responder_id, responder_name, role, contact_number, "
            "specialization, city, area, base_address, latitude, longitude, "
            "status, user_id, is_active) "
            "VALUES (" +
            to_string(responderId) + "," + sqlValue(connection, fullName) + "," +
            sqlValue(connection, responderRole) + "," + sqlValue(connection, contact) + "," +
            sqlValue(connection, specialization) + "," + sqlValue(connection, city) + "," +
            sqlValue(connection, area) + "," + sqlValue(connection, baseAddress) + ",NULL,NULL,'AVAILABLE'," +
            to_string(userId) + ",TRUE)";

    if (mysql_query(connection, query.c_str()) != 0)
    {
        database.rollbackTransaction();
        return false;
    }

    query = "UPDATE responder_registration_requests "
            "SET status = 'APPROVED', reviewed_at = CURRENT_TIMESTAMP "
            "WHERE registration_id = " +
            to_string(registrationId) + " AND status = 'PENDING'";

    if (mysql_query(connection, query.c_str()) != 0 || mysql_affected_rows(connection) != 1)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool ResponderRepository::rejectResponderRegistration(int registrationId)
{
    MYSQL* connection = database.getConnection();

    if (!connection || registrationId <= 0)
        return false;

    string query = "UPDATE responder_registration_requests "
                   "SET status = 'REJECTED', reviewed_at = CURRENT_TIMESTAMP "
                   "WHERE registration_id = " +
                   to_string(registrationId) + " AND status = 'PENDING'";

    return database.executeQuery(query.c_str()) && mysql_affected_rows(connection) == 1;
}

Location::Location() : locationId(0), locationName(""), latitude(0.0), longitude(0.0), active(true)
{
}
Location::Location(int id, string name, double lat, double lon)
    : locationId(id), locationName(name), latitude(0.0), longitude(0.0), active(true)
{
    setCoordinates(lat, lon);
}
int Location::getLocationId() const
{
    return locationId;
}
string Location::getLocationName() const
{
    return locationName;
}
double Location::getLatitude() const
{
    return latitude;
}
double Location::getLongitude() const
{
    return longitude;
}
bool Location::isActive() const
{
    return active;
}
void Location::setLocationName(string name)
{
    if (!name.empty())
        locationName = name;
}
bool Location::setCoordinates(double lat, double lon)
{
    if (!isfinite(lat) || !isfinite(lon) || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0)
        return false;
    latitude = lat;
    longitude = lon;
    return true;
}
void Location::setActive(bool status)
{
    active = status;
}
void Location::displayLocation() const
{
    cout << "\nLocation ID: " << locationId << "\nName: " << locationName << "\nLatitude: " << latitude
         << "\nLongitude: " << longitude << "\nStatus: " << (active ? "Active" : "Inactive") << "\n";
}

double Location::distanceKmTo(const Location& other) const
{
    const double earthRadiusKm = 6371.0;
    const double toRadians = 3.14159265358979323846 / 180.0;

    double dLat = (other.latitude - latitude) * toRadians;
    double dLon = (other.longitude - longitude) * toRadians;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) + std::cos(latitude * toRadians) *
                                                                 std::cos(other.latitude * toRadians) *
                                                                 std::sin(dLon / 2) * std::sin(dLon / 2);

    return earthRadiusKm * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

Stack::Stack(int maxSize) : data(nullptr), capacity(maxSize > 0 ? maxSize : 0), topIndex(-1)
{
    if (capacity > 0)
        data = new (std::nothrow) int[capacity];
    if (data == nullptr)
        capacity = 0;
}
Stack::~Stack()
{
    delete[] data;
}
bool Stack::isEmpty() const
{
    return topIndex < 0;
}
bool Stack::isFull() const
{
    return capacity == 0 || topIndex + 1 >= capacity;
}
bool Stack::push(int value)
{
    if (isFull())
        return false;
    data[++topIndex] = value;
    return true;
}
bool Stack::pop(int& value)
{
    if (isEmpty())
        return false;
    value = data[topIndex--];
    return true;
}
bool Stack::peek(int& value) const
{
    if (isEmpty())
        return false;
    value = data[topIndex];
    return true;
}

LocationGraph::LocationGraph() : head(nullptr), tail(nullptr), vertexCount(0)
{
}
LocationGraph::~LocationGraph()
{
    Vertex* current = head;
    while (current != nullptr)
    {
        Vertex* next = current->next;
        clearEdges(current->adjacencyList);
        delete current;
        current = next;
    }
}
void LocationGraph::clearEdges(Edge* edge)
{
    while (edge != nullptr)
    {
        Edge* next = edge->next;
        delete edge;
        edge = next;
    }
}
LocationGraph::Vertex* LocationGraph::findVertex(int id) const
{
    Vertex* current = head;
    while (current != nullptr)
    {
        if (current->data.getLocationId() == id)
            return current;
        current = current->next;
    }
    return nullptr;
}
bool LocationGraph::hasEdge(int sourceId, int destinationId) const
{
    Vertex* source = findVertex(sourceId);
    if (!source)
        return false;
    Edge* edge = source->adjacencyList;
    while (edge)
    {
        if (edge->destinationId == destinationId)
            return true;
        edge = edge->next;
    }
    return false;
}
bool LocationGraph::addLocation(const Location& location)
{
    if (location.getLocationId() <= 0 || location.getLocationName().empty() || !location.isActive() ||
        !isfinite(location.getLatitude()) || !isfinite(location.getLongitude()) ||
        location.getLatitude() < -90.0 || location.getLatitude() > 90.0 || location.getLongitude() < -180.0 ||
        location.getLongitude() > 180.0 || findVertex(location.getLocationId()))
        return false;
    Vertex* node = new (nothrow) Vertex(location);
    if (!node)
        return false;
    if (!head)
        head = tail = node;
    else
    {
        tail->next = node;
        tail = node;
    }
    ++vertexCount;
    return true;
}
bool LocationGraph::removeLocation(int id)
{
    Vertex* current = head;
    Vertex* previous = nullptr;
    while (current && current->data.getLocationId() != id)
    {
        previous = current;
        current = current->next;
    }
    if (!current)
        return false;
    if (previous)
        previous->next = current->next;
    else
        head = current->next;
    if (tail == current)
        tail = previous;
    clearEdges(current->adjacencyList);
    delete current;
    for (Vertex* vertex = head; vertex; vertex = vertex->next)
    {
        Edge* edge = vertex->adjacencyList;
        Edge* before = nullptr;
        while (edge)
        {
            if (edge->destinationId == id)
            {
                Edge* doomed = edge;
                edge = edge->next;
                if (before)
                    before->next = edge;
                else
                    vertex->adjacencyList = edge;
                delete doomed;
            }
            else
            {
                before = edge;
                edge = edge->next;
            }
        }
    }
    --vertexCount;
    if (!head)
        tail = nullptr;
    return true;
}
Location* LocationGraph::findLocation(int id)
{
    Vertex* vertex = findVertex(id);
    return vertex ? &vertex->data : nullptr;
}
bool LocationGraph::addConnection(int sourceId, int destinationId, double distance)
{
    Vertex* source = findVertex(sourceId);
    Vertex* destination = findVertex(destinationId);
    if (sourceId == destinationId || !isfinite(distance) || distance <= 0.0 || !source || !destination ||
        !source->data.isActive() || !destination->data.isActive() || hasEdge(sourceId, destinationId))
        return false;
    Edge* first = new (nothrow) Edge(destinationId, distance);
    if (!first)
        return false;
    Edge* second = new (nothrow) Edge(sourceId, distance);
    if (!second)
    {
        delete first;
        return false;
    }
    first->next = source->adjacencyList;
    source->adjacencyList = first;
    second->next = destination->adjacencyList;
    destination->adjacencyList = second;
    return true;
}
bool LocationGraph::removeConnection(int sourceId, int destinationId)
{
    if (!hasEdge(sourceId, destinationId) || !hasEdge(destinationId, sourceId))
        return false;
    int ids[2] = {sourceId, destinationId};
    for (int i = 0; i < 2; ++i)
    {
        Vertex* vertex = findVertex(ids[i]);
        Edge* edge = vertex->adjacencyList;
        Edge* previous = nullptr;
        while (edge && edge->destinationId != ids[1 - i])
        {
            previous = edge;
            edge = edge->next;
        }
        if (!edge)
            return false;
        if (previous)
            previous->next = edge->next;
        else
            vertex->adjacencyList = edge->next;
        delete edge;
    }
    return true;
}
void LocationGraph::displayGraph() const
{
    if (!head)
    {
        cout << "\nGraph is empty.\n";
        return;
    }
    for (Vertex* vertex = head; vertex; vertex = vertex->next)
    {
        cout << "\n" << vertex->data.getLocationName() << " (" << vertex->data.getLocationId() << ") -> ";
        Edge* edge = vertex->adjacencyList;
        if (!edge)
            cout << "No connections";
        while (edge)
        {
            Vertex* destination = findVertex(edge->destinationId);
            if (destination)
                cout << destination->data.getLocationName() << " [" << edge->distance << " km]";
            if (edge->next)
                cout << " -> ";
            edge = edge->next;
        }
        cout << "\n";
    }
}
void LocationGraph::breadthFirstSearch(int startId) const
{
    if (!findVertex(startId))
    {
        cout << "\nStarting location not found.\n";
        return;
    }
    int* ids = new (nothrow) int[vertexCount];
    bool* visited = new (nothrow) bool[vertexCount];
    int* queue = new (nothrow) int[vertexCount];
    if (!ids || !visited || !queue)
    {
        delete[] ids;
        delete[] visited;
        delete[] queue;
        cout << "\nBFS memory allocation failed.\n";
        return;
    }
    int i = 0;
    for (Vertex* v = head; v; v = v->next)
    {
        ids[i] = v->data.getLocationId();
        visited[i++] = false;
    }
    int startIndex = 0;
    while (ids[startIndex] != startId)
        ++startIndex;
    int front = 0, rear = 0;
    queue[rear++] = startId;
    visited[startIndex] = true;
    cout << "\nBFS Traversal: ";
    while (front < rear)
    {
        int id = queue[front++];
        Vertex* vertex = findVertex(id);
        if (!vertex)
            continue;
        cout << vertex->data.getLocationName() << " ";
        for (Edge* edge = vertex->adjacencyList; edge; edge = edge->next)
        {
            int index = 0;
            while (index < vertexCount && ids[index] != edge->destinationId)
                ++index;
            Vertex* target = findVertex(edge->destinationId);
            if (index < vertexCount && target && target->data.isActive() && !visited[index])
            {
                visited[index] = true;
                queue[rear++] = edge->destinationId;
            }
        }
    }
    cout << "\n";
    delete[] ids;
    delete[] visited;
    delete[] queue;
}
void LocationGraph::depthFirstSearch(int startId) const
{
    if (!findVertex(startId))
    {
        cout << "\nStarting location not found.\n";
        return;
    }
    int* ids = new (nothrow) int[vertexCount];
    bool* visited = new (nothrow) bool[vertexCount];
    if (!ids || !visited)
    {
        delete[] ids;
        delete[] visited;
        cout << "\nDFS memory allocation failed.\n";
        return;
    }
    int i = 0;
    for (Vertex* v = head; v; v = v->next)
    {
        ids[i] = v->data.getLocationId();
        visited[i++] = false;
    }
    Stack stack(vertexCount);
    int startIndex = 0;
    while (ids[startIndex] != startId)
        ++startIndex;
    if (!stack.push(startId))
    {
        delete[] ids;
        delete[] visited;
        cout << "\nDFS stack allocation failed.\n";
        return;
    }
    visited[startIndex] = true;
    cout << "\nDFS Traversal: ";
    int id;
    while (stack.pop(id))
    {
        Vertex* vertex = findVertex(id);
        if (!vertex)
            continue;
        cout << vertex->data.getLocationName() << " ";
        for (Edge* edge = vertex->adjacencyList; edge; edge = edge->next)
        {
            int index = 0;
            while (index < vertexCount && ids[index] != edge->destinationId)
                ++index;
            Vertex* target = findVertex(edge->destinationId);
            if (index < vertexCount && target && target->data.isActive() && !visited[index])
            {
                visited[index] = true;
                if (!stack.push(edge->destinationId))
                {
                    cout << "\nDFS stack capacity error.\n";
                    break;
                }
            }
        }
    }
    cout << "\n";
    delete[] ids;
    delete[] visited;
}
void LocationGraph::findShortestPath(int sourceId, int destinationId) const
{
    if (!findVertex(sourceId) || !findVertex(destinationId))
    {
        cout << "\nSource or destination location not found.\n";
        return;
    }
    Vertex* sourceVertex = findVertex(sourceId);
    Vertex* destinationVertex = findVertex(destinationId);
    if (!sourceVertex->data.isActive() || !destinationVertex->data.isActive())
    {
        cout << "\nSource or destination location is inactive.\n";
        return;
    }
    int n = vertexCount;
    double* distances = new (nothrow) double[n];
    bool* visited = new (nothrow) bool[n];
    int* previous = new (nothrow) int[n];
    int* ids = new (nothrow) int[n];
    if (!distances || !visited || !previous || !ids)
    {
        delete[] distances;
        delete[] visited;
        delete[] previous;
        delete[] ids;
        cout << "\nRoute calculation memory allocation failed.\n";
        return;
    }
    int index = 0;
    for (Vertex* v = head; v; v = v->next)
    {
        ids[index] = v->data.getLocationId();
        distances[index] = numeric_limits<double>::max();
        visited[index] = false;
        previous[index] = -1;
        ++index;
    }
    int sourceIndex = 0;
    while (ids[sourceIndex] != sourceId)
        ++sourceIndex;
    distances[sourceIndex] = 0.0;
    for (int count = 0; count < n; ++count)
    {
        int minIndex = -1;
        for (int i = 0; i < n; ++i)
            if (!visited[i] && (minIndex == -1 || distances[i] < distances[minIndex]))
                minIndex = i;
        if (minIndex == -1 || distances[minIndex] == numeric_limits<double>::max())
            break;
        visited[minIndex] = true;
        Vertex* vertex = findVertex(ids[minIndex]);
        for (Edge* edge = vertex->adjacencyList; edge; edge = edge->next)
        {
            int targetIndex = 0;
            while (targetIndex < n && ids[targetIndex] != edge->destinationId)
                ++targetIndex;
            Vertex* target = findVertex(edge->destinationId);
            if (targetIndex < n && target && target->data.isActive() && !visited[targetIndex] &&
                distances[minIndex] <= numeric_limits<double>::max() - edge->distance)
            {
                double candidate = distances[minIndex] + edge->distance;
                if (candidate < distances[targetIndex])
                {
                    distances[targetIndex] = candidate;
                    previous[targetIndex] = minIndex;
                }
            }
        }
    }
    int destinationIndex = 0;
    while (ids[destinationIndex] != destinationId)
        ++destinationIndex;
    if (distances[destinationIndex] == numeric_limits<double>::max())
        cout << "\nNo path exists between these locations.\n";
    else
    {
        int* path = new (nothrow) int[n];
        if (!path)
            cout << "\nRoute path memory allocation failed.\n";
        else
        {
            int pathSize = 0, current = destinationIndex;
            while (current != -1 && pathSize < n)
            {
                path[pathSize++] = current;
                current = previous[current];
            }
            cout << "\nShortest Path: ";
            for (int i = pathSize - 1; i >= 0; --i)
            {
                Vertex* location = findVertex(ids[path[i]]);
                cout << (location ? location->data.getLocationName() : "Unknown");
                if (i > 0)
                    cout << " -> ";
            }
            cout << "\nTotal Distance: " << distances[destinationIndex] << " km\n";
            delete[] path;
        }
    }
    delete[] distances;
    delete[] visited;
    delete[] previous;
    delete[] ids;
}
int LocationGraph::getLocationCount() const
{
    return vertexCount;
}
bool LocationGraph::isEmpty() const
{
    return head == nullptr;
}

bool LocationGraph::hasConnection(int sourceId, int destinationId) const
{
    return hasEdge(sourceId, destinationId);
}

RoutePlanner::RoutePlanner(LocationGraph* locationGraph) : graph(locationGraph)
{
}
void RoutePlanner::setGraph(LocationGraph* locationGraph)
{
    graph = locationGraph;
}
void RoutePlanner::planRoute(int sourceId, int destinationId) const
{
    if (!graph)
    {
        cout << "\nLocation graph is not connected.\n";
        return;
    }
    Location* source = graph->findLocation(sourceId);
    Location* destination = graph->findLocation(destinationId);
    if (!source || !destination)
    {
        cout << "\nInvalid source or destination location.\n";
        return;
    }
    cout << "\n========== ROUTE PLANNING ==========\nSource: " << source->getLocationName()
         << "\nDestination: " << destination->getLocationName() << "\n";
    graph->findShortestPath(sourceId, destinationId);
    cout << "====================================\n";
}

LocationRepository::LocationRepository(DatabaseConnection& db) : database(db)
{
}

bool LocationRepository::saveLocation(const Location& location)
{
    if (!database.isConnected() || location.getLocationId() <= 0 || location.getLocationName().empty() ||
        !location.isActive())
        return false;

    const char* query = "INSERT INTO locations "
                        "(location_id, location_name, latitude, longitude, is_active) "
                        "VALUES (?, ?, ?, ?, TRUE)";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    int id = location.getLocationId();
    double latitude = location.getLatitude();
    double longitude = location.getLongitude();

    std::string name = location.getLocationName();
    unsigned long nameLength = static_cast<unsigned long>(name.length());

    MYSQL_BIND parameters[4]{};

    bindInt(parameters[0], id);

    bindString(parameters[1], name, nameLength);

    bindDouble(parameters[2], latitude);

    bindDouble(parameters[3], longitude);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool LocationRepository::loadLocations(LocationGraph& graph)
{
    if (!database.isConnected())
        return false;

    const char* query = "SELECT location_id, location_name, latitude, longitude "
                        "FROM locations WHERE is_active = TRUE ORDER BY location_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return false;

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        Location location(std::stoi(row[0]), row[1] ? row[1] : "", std::stod(row[2]), std::stod(row[3]));

        if (!graph.addLocation(location))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    return success;
}

bool LocationRepository::updateLocation(const Location& location)
{
    if (!database.isConnected() || location.getLocationId() <= 0 || location.getLocationName().empty())
        return false;

    const char* query = "UPDATE locations SET location_name = ?, "
                        "latitude = ?, longitude = ? "
                        "WHERE location_id = ?";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    std::string name = location.getLocationName();
    unsigned long nameLength = static_cast<unsigned long>(name.length());

    double latitude = location.getLatitude();
    double longitude = location.getLongitude();
    int id = location.getLocationId();

    MYSQL_BIND parameters[4]{};

    bindString(parameters[0], name, nameLength);

    bindDouble(parameters[1], latitude);

    bindDouble(parameters[2], longitude);

    bindInt(parameters[3], id);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool LocationRepository::deactivateLocation(int locationId)
{
    if (!database.isConnected() || locationId <= 0)
        return false;

    if (!database.beginTransaction())
        return false;

    MYSQL_STMT* edgeStatement =
            database.prepareStatement("UPDATE route_edges SET is_active = FALSE "
                                      "WHERE source_location_id = ? OR destination_location_id = ?");

    if (!edgeStatement)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND edgeParameters[2]{};
    bindInt(edgeParameters[0], locationId);
    bindInt(edgeParameters[1], locationId);

    bool edgesOk = mysql_stmt_bind_param(edgeStatement, edgeParameters) == 0 &&
                   mysql_stmt_execute(edgeStatement) == 0;

    mysql_stmt_close(edgeStatement);

    if (!edgesOk)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_STMT* statement = database.prepareStatement(
            "UPDATE locations SET is_active = FALSE WHERE location_id = ? AND is_active = TRUE");

    if (!statement)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND parameter{};
    bindInt(parameter, locationId);

    bool success = mysql_stmt_bind_param(statement, &parameter) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_affected_rows(statement) == 1;

    mysql_stmt_close(statement);

    if (!success)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool LocationRepository::locationExists(int locationId)
{
    if (!database.isConnected() || locationId <= 0)
        return false;

    const char* query = "SELECT location_id FROM locations "
                        "WHERE location_id = ? LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    MYSQL_BIND parameter{};

    bindInt(parameter, locationId);

    bool success = mysql_stmt_bind_param(statement, &parameter) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_store_result(statement) == 0;

    bool exists = success && mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);

    return exists;
}

bool LocationRepository::saveConnection(int sourceId, int destinationId, double distance)
{
    if (!database.isConnected() || sourceId <= 0 || destinationId <= 0 || sourceId == destinationId ||
        distance <= 0.0)
        return false;

    if (!database.beginTransaction())
        return false;

    MYSQL_STMT* find =
            database.prepareStatement("SELECT route_edge_id FROM route_edges "
                                      "WHERE (source_location_id = ? AND destination_location_id = ?) "
                                      "OR (source_location_id = ? AND destination_location_id = ?) LIMIT 1");

    if (!find)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND findParameters[4]{};
    int ids[4] = {sourceId, destinationId, destinationId, sourceId};

    for (int i = 0; i < 4; ++i)
    {
        findParameters[i].buffer_type = MYSQL_TYPE_LONG;
        findParameters[i].buffer = &ids[i];
    }

    bool findOk = mysql_stmt_bind_param(find, findParameters) == 0 && mysql_stmt_execute(find) == 0 &&
                  mysql_stmt_store_result(find) == 0;

    bool rowExists = findOk && mysql_stmt_num_rows(find) > 0;
    mysql_stmt_close(find);

    if (!findOk)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_STMT* statement = nullptr;
    bool success = false;

    if (rowExists)
    {
        statement =
                database.prepareStatement("UPDATE route_edges SET is_active = TRUE, distance_km = ? "
                                          "WHERE (source_location_id = ? AND destination_location_id = ?) "
                                          "OR (source_location_id = ? AND destination_location_id = ?)");

        if (statement)
        {
            MYSQL_BIND parameters[5]{};
            bindDouble(parameters[0], distance);

            for (int i = 0; i < 4; ++i)
            {
                parameters[i + 1].buffer_type = MYSQL_TYPE_LONG;
                parameters[i + 1].buffer = &ids[i];
            }

            success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;
        }
    }
    else
    {
        statement = database.prepareStatement(
                "INSERT INTO route_edges "
                "(source_location_id, destination_location_id, distance_km, is_active) "
                "VALUES (?, ?, ?, TRUE)");

        if (statement)
        {
            MYSQL_BIND parameters[3]{};
            bindInt(parameters[0], sourceId);
            bindInt(parameters[1], destinationId);
            bindDouble(parameters[2], distance);

            success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;
        }
    }

    if (statement)
        mysql_stmt_close(statement);

    if (!success)
    {
        database.rollbackTransaction();
        return false;
    }

    return database.commitTransaction();
}

bool LocationRepository::deactivateConnection(int sourceId, int destinationId)
{
    if (!database.isConnected() || sourceId <= 0 || destinationId <= 0)
        return false;

    MYSQL_STMT* statement =
            database.prepareStatement("UPDATE route_edges SET is_active = FALSE "
                                      "WHERE is_active = TRUE AND "
                                      "((source_location_id = ? AND destination_location_id = ?) "
                                      "OR (source_location_id = ? AND destination_location_id = ?))");

    if (!statement)
        return false;

    MYSQL_BIND parameters[4]{};
    int ids[4] = {sourceId, destinationId, destinationId, sourceId};

    for (int i = 0; i < 4; ++i)
    {
        parameters[i].buffer_type = MYSQL_TYPE_LONG;
        parameters[i].buffer = &ids[i];
    }

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_affected_rows(statement) >= 1;

    mysql_stmt_close(statement);
    return success;
}

bool LocationRepository::loadConnections(LocationGraph& graph)
{
    if (!database.isConnected())
        return false;

    const char* query =
            "SELECT e.source_location_id, e.destination_location_id, e.distance_km "
            "FROM route_edges e "
            "JOIN locations s ON s.location_id = e.source_location_id AND s.is_active = TRUE "
            "JOIN locations d ON d.location_id = e.destination_location_id AND d.is_active = TRUE "
            "WHERE e.is_active = TRUE ORDER BY e.route_edge_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return false;

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int sourceId = std::stoi(row[0]);
        int destinationId = std::stoi(row[1]);
        double distance = std::stod(row[2]);

        if (graph.hasConnection(sourceId, destinationId))
            continue;

        if (!graph.addConnection(sourceId, destinationId, distance))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    return success;
}

bool LocationRepository::connectionExists(int sourceId, int destinationId)
{
    if (!database.isConnected() || sourceId <= 0 || destinationId <= 0 || sourceId == destinationId)
        return false;

    const char* query = "SELECT route_edge_id FROM route_edges "
                        "WHERE source_location_id = ? "
                        "AND destination_location_id = ? "
                        "AND is_active = TRUE LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (!statement)
        return false;

    MYSQL_BIND parameters[2]{};

    bindInt(parameters[0], sourceId);

    bindInt(parameters[1], destinationId);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_store_result(statement) == 0;

    bool exists = success && mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);

    return exists;
}

const char* activityTypeToString(ActivityType type)
{
    switch (type)
    {
        case ActivityType::EMERGENCY_REPORTED:
            return "EMERGENCY_REPORTED";
        case ActivityType::EMERGENCY_STATUS_CHANGED:
            return "EMERGENCY_STATUS_CHANGED";
        case ActivityType::ALLOCATION_CREATED:
            return "ALLOCATION_CREATED";
        case ActivityType::STOCK_RESERVED:
            return "STOCK_RESERVED";
        case ActivityType::STOCK_DISPATCHED:
            return "STOCK_DISPATCHED";
        case ActivityType::STOCK_RETURNED:
            return "STOCK_RETURNED";
        case ActivityType::ALLOCATION_CANCELLED:
            return "ALLOCATION_CANCELLED";
        case ActivityType::WAREHOUSE_TRANSFERRED:
            return "WAREHOUSE_TRANSFERRED";
        case ActivityType::STOCK_ADDED:
            return "STOCK_ADDED";
        case ActivityType::STOCK_REMOVED:
            return "STOCK_REMOVED";
        case ActivityType::RESPONDER_ASSIGNED:
            return "RESPONDER_ASSIGNED";
        case ActivityType::RESPONDER_RELEASED:
            return "RESPONDER_RELEASED";
    }
    return "UNKNOWN";
}

ActivityRecord::ActivityRecord()
    : activityId(-1), type(ActivityType::STOCK_ADDED), emergencyId(-1), allocationId(-1), resourceId(-1),
      warehouseId(-1), destinationWarehouseId(-1), responderId(-1), quantity(0), timestamp(0)
{
}

ActivityRecord::ActivityRecord(int id, ActivityType activityType, int emergency, int allocation, int resource,
                               int warehouse, int destinationWarehouse, int responder, int activityQuantity,
                               const string& details, const string& actor, time_t activityTime)
    : activityId(id), type(activityType), emergencyId(emergency), allocationId(allocation),
      resourceId(resource), warehouseId(warehouse), destinationWarehouseId(destinationWarehouse),
      responderId(responder), quantity(activityQuantity), description(details), performedBy(actor),
      timestamp(activityTime)
{
}

int ActivityRecord::getActivityId() const
{
    return activityId;
}
ActivityType ActivityRecord::getType() const
{
    return type;
}
int ActivityRecord::getEmergencyId() const
{
    return emergencyId;
}
int ActivityRecord::getAllocationId() const
{
    return allocationId;
}
int ActivityRecord::getResourceId() const
{
    return resourceId;
}
int ActivityRecord::getWarehouseId() const
{
    return warehouseId;
}
int ActivityRecord::getDestinationWarehouseId() const
{
    return destinationWarehouseId;
}
int ActivityRecord::getResponderId() const
{
    return responderId;
}
int ActivityRecord::getQuantity() const
{
    return quantity;
}
string ActivityRecord::getDescription() const
{
    return description;
}
string ActivityRecord::getPerformedBy() const
{
    return performedBy;
}
time_t ActivityRecord::getTimestamp() const
{
    return timestamp;
}

void ActivityRecord::display() const
{
    tm* timeInfo = localtime(&timestamp);
    char buffer[32] = "Unknown time";
    if (timeInfo != nullptr)
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

    cout << "Activity ID: " << activityId << " | Type: " << activityTypeToString(type)
         << " | Time: " << buffer << " | Emergency ID: " << emergencyId
         << " | Allocation ID: " << allocationId << " | Resource ID: " << resourceId
         << " | Warehouse ID: " << warehouseId << " | Destination Warehouse ID: " << destinationWarehouseId
         << " | Responder ID: " << responderId << " | Quantity: " << quantity << " | By: " << performedBy
         << " | Details: " << description << endl;
}

ActivityHistory::Node::Node(const ActivityRecord& record) : data(record), previous(nullptr), next(nullptr)
{
}

ActivityHistory::ActivityHistory() : head(nullptr), tail(nullptr), count(0), nextActivityId(1)
{
}

ActivityHistory::~ActivityHistory()
{
    clear();
}

bool ActivityHistory::isValidType(ActivityType type)
{
    return type >= ActivityType::EMERGENCY_REPORTED && type <= ActivityType::RESPONDER_RELEASED;
}

bool ActivityHistory::isEmpty() const
{
    return head == nullptr;
}
int ActivityHistory::getCount() const
{
    return count;
}

bool ActivityHistory::addActivity(ActivityType type, const string& description, const string& performedBy,
                                  int emergencyId, int allocationId, int resourceId, int warehouseId,
                                  int destinationWarehouseId, int responderId, int quantity, time_t timestamp)
{
    if (!isValidType(type) || description.empty() || performedBy.empty() || quantity < 0 ||
        emergencyId < -1 || allocationId < -1 || resourceId < -1 || warehouseId < -1 ||
        destinationWarehouseId < -1 || responderId < -1 || count == numeric_limits<int>::max() ||
        nextActivityId <= 0 || nextActivityId == numeric_limits<int>::max())
        return false;

    if (timestamp == 0)
        timestamp = time(nullptr);
    if (timestamp < 0)
        return false;

    ActivityRecord record(nextActivityId, type, emergencyId, allocationId, resourceId, warehouseId,
                          destinationWarehouseId, responderId, quantity, description, performedBy, timestamp);
    Node* newNode = new (nothrow) Node(record);
    if (newNode == nullptr)
        return false;

    newNode->previous = tail;
    if (tail != nullptr)
        tail->next = newNode;
    else
        head = newNode;
    tail = newNode;
    ++count;
    ++nextActivityId;
    return true;
}

bool ActivityHistory::loadActivityRecord(int id, ActivityType type, int emergencyId, int allocationId,
                                         int resourceId, int warehouseId, int destinationWarehouseId,
                                         int responderId, int quantity, const string& description,
                                         const string& performedBy, time_t timestamp)
{
    if (!isValidType(type) || id <= 0 || id == numeric_limits<int>::max() || description.empty() ||
        performedBy.empty() || quantity < 0 || timestamp <= 0 || emergencyId < -1 || allocationId < -1 ||
        resourceId < -1 || warehouseId < -1 || destinationWarehouseId < -1 || responderId < -1 ||
        count == numeric_limits<int>::max() || searchByActivityId(id) != nullptr)
        return false;

    ActivityRecord record(id, type, emergencyId, allocationId, resourceId, warehouseId,
                          destinationWarehouseId, responderId, quantity, description, performedBy, timestamp);

    Node* newNode = new (nothrow) Node(record);

    if (newNode == nullptr)
        return false;

    newNode->previous = tail;

    if (tail != nullptr)
        tail->next = newNode;
    else
        head = newNode;

    tail = newNode;
    ++count;

    if (id >= nextActivityId)
        nextActivityId = id + 1;

    return true;
}

ActivityRecord* ActivityHistory::searchByActivityId(int id)
{
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getActivityId() == id)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

const ActivityRecord* ActivityHistory::searchByActivityId(int id) const
{
    const Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getActivityId() == id)
            return &current->data;
        current = current->next;
    }
    return nullptr;
}

bool ActivityHistory::deleteActivity(int id)
{
    Node* current = head;
    while (current != nullptr && current->data.getActivityId() != id)
        current = current->next;
    if (current == nullptr)
        return false;

    if (current->previous != nullptr)
        current->previous->next = current->next;
    else
        head = current->next;
    if (current->next != nullptr)
        current->next->previous = current->previous;
    else
        tail = current->previous;

    delete current;
    --count;
    return true;
}

void ActivityHistory::clear()
{
    Node* current = head;

    while (current != nullptr)
    {
        Node* next = current->next;
        delete current;
        current = next;
    }

    head = nullptr;
    tail = nullptr;
    count = 0;
}

void ActivityHistory::displayAll() const
{
    if (isEmpty())
    {
        cout << "No activity history available." << endl;
        return;
    }
    const Node* current = head;
    while (current != nullptr)
    {
        current->data.display();
        current = current->next;
    }
}

void ActivityHistory::displayByType(ActivityType type) const
{
    const Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getType() == type)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "No activities found for this type." << endl;
}

void ActivityHistory::displayByEmergency(int id) const
{
    const Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == id)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "No activities found for this emergency." << endl;
}

void ActivityHistory::displayByAllocation(int id) const
{
    const Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getAllocationId() == id)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "No activities found for this allocation." << endl;
}

void ActivityHistory::displayByWarehouse(int id) const
{
    const Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getWarehouseId() == id || current->data.getDestinationWarehouseId() == id)
        {
            current->data.display();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "No activities found for this warehouse." << endl;
}

int ActivityHistory::getLastActivityId() const
{
    return tail != nullptr ? tail->data.getActivityId() : 0;
}

ActivityHistoryRepository::ActivityHistoryRepository(DatabaseConnection& db) : database(db)
{
}

bool ActivityHistoryRepository::saveActivity(const ActivityRecord& activity)
{
    if (!database.isConnected() || activity.getActivityId() <= 0 || activity.getDescription().empty() ||
        activity.getPerformedBy().empty() || activity.getQuantity() < 0 || activity.getTimestamp() <= 0)
        return false;

    const char* query = "INSERT INTO activity_history "
                        "(activity_id, activity_type, description, emergency_id, "
                        "allocation_id, responder_id, warehouse_id, "
                        "destination_warehouse_id, resource_id, quantity, "
                        "performed_by, created_at) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, FROM_UNIXTIME(?))";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    int activityId = activity.getActivityId();
    std::string type = activityTypeToString(activity.getType());
    std::string description = activity.getDescription();
    std::string performedBy = activity.getPerformedBy();

    int emergencyId = activity.getEmergencyId();
    int allocationId = activity.getAllocationId();
    int responderId = activity.getResponderId();
    int warehouseId = activity.getWarehouseId();
    int destinationWarehouseId = activity.getDestinationWarehouseId();
    int resourceId = activity.getResourceId();
    int quantity = activity.getQuantity();

    DbBool emergencyNull = emergencyId == -1;
    DbBool allocationNull = allocationId == -1;
    DbBool responderNull = responderId == -1;
    DbBool warehouseNull = warehouseId == -1;
    DbBool destinationNull = destinationWarehouseId == -1;
    DbBool resourceNull = resourceId == -1;

    long long timestamp = static_cast<long long>(activity.getTimestamp());

    unsigned long typeLength = static_cast<unsigned long>(type.length());
    unsigned long descriptionLength = static_cast<unsigned long>(description.length());
    unsigned long performedByLength = static_cast<unsigned long>(performedBy.length());

    MYSQL_BIND parameters[12]{};

    bindInt(parameters[0], activityId);

    bindString(parameters[1], type, typeLength);

    bindString(parameters[2], description, descriptionLength);

    bindInt(parameters[3], emergencyId);
    parameters[3].is_null = &emergencyNull;

    bindInt(parameters[4], allocationId);
    parameters[4].is_null = &allocationNull;

    bindInt(parameters[5], responderId);
    parameters[5].is_null = &responderNull;

    bindInt(parameters[6], warehouseId);
    parameters[6].is_null = &warehouseNull;

    bindInt(parameters[7], destinationWarehouseId);
    parameters[7].is_null = &destinationNull;

    bindInt(parameters[8], resourceId);
    parameters[8].is_null = &resourceNull;

    bindInt(parameters[9], quantity);

    bindString(parameters[10], performedBy, performedByLength);

    bindLongLong(parameters[11], timestamp);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool ActivityHistoryRepository::loadActivityHistory(ActivityHistory& history)
{
    if (!database.isConnected())
        return false;

    const char* query = "SELECT activity_id, activity_type, emergency_id, allocation_id, "
                        "resource_id, warehouse_id, destination_warehouse_id, responder_id, "
                        "quantity, description, performed_by, UNIX_TIMESTAMP(created_at) "
                        "FROM activity_history "
                        "ORDER BY created_at, activity_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    history.clear();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        try
        {
            if (row[0] == nullptr || row[1] == nullptr || row[8] == nullptr || row[9] == nullptr ||
                row[10] == nullptr || row[11] == nullptr)
            {
                success = false;
                break;
            }

            long long idValue = std::stoll(row[0]);
            long long timestampValue = std::stoll(row[11]);

            if (idValue <= 0 || idValue >= INT_MAX || timestampValue <= 0)
            {
                success = false;
                break;
            }

            ActivityType type;
            bool validType = false;

            for (int i = 0; i <= static_cast<int>(ActivityType::RESPONDER_RELEASED); ++i)
            {
                ActivityType candidate = static_cast<ActivityType>(i);

                if (std::string(activityTypeToString(candidate)) == row[1])
                {
                    type = candidate;
                    validType = true;
                    break;
                }
            }

            if (!validType)
            {
                success = false;
                break;
            }

            int emergencyId = row[2] ? std::stoi(row[2]) : -1;
            int allocationId = row[3] ? std::stoi(row[3]) : -1;
            int resourceId = row[4] ? std::stoi(row[4]) : -1;
            int warehouseId = row[5] ? std::stoi(row[5]) : -1;
            int destinationWarehouseId = row[6] ? std::stoi(row[6]) : -1;
            int responderId = row[7] ? std::stoi(row[7]) : -1;
            int quantity = std::stoi(row[8]);

            if (!history.loadActivityRecord(static_cast<int>(idValue), type, emergencyId, allocationId,
                                            resourceId, warehouseId, destinationWarehouseId, responderId,
                                            quantity, row[9], row[10],
                                            static_cast<std::time_t>(timestampValue)))
            {
                success = false;
                break;
            }
        }
        catch (...)
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        history.clear();

    return success;
}

bool ActivityHistoryRepository::activityExists(int activityId)
{
    if (!database.isConnected() || activityId <= 0)
        return false;

    const char* query = "SELECT activity_id FROM activity_history "
                        "WHERE activity_id = ? LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};

    bindInt(parameter, activityId);

    bool success = mysql_stmt_bind_param(statement, &parameter) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_store_result(statement) == 0;

    bool exists = false;

    if (success)
        exists = mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);

    return exists;
}

User::User()
    : userId(0), username(""), passwordHash(""), fullName(""), email(""), contactNumber(""),
      role(UserRole::COORDINATOR), active(true)
{
}

User::User(int id, const std::string& user, const std::string& hash, const std::string& name,
           const std::string& emailAddress, const std::string& number, UserRole userRole)
    : userId(id), username(user), passwordHash(hash), fullName(name), email(emailAddress),
      contactNumber(number), role(userRole), active(true)
{
}

int User::getUserId() const
{
    return userId;
}
std::string User::getUsername() const
{
    return username;
}
std::string User::getPasswordHash() const
{
    return passwordHash;
}
std::string User::getFullName() const
{
    return fullName;
}
std::string User::getEmail() const
{
    return email;
}
std::string User::getContactNumber() const
{
    return contactNumber;
}
UserRole User::getRole() const
{
    return role;
}
bool User::isActive() const
{
    return active;
}

void User::setFullName(const std::string& name)
{
    if (!name.empty())
        fullName = name;
}

void User::setEmail(const std::string& emailAddress)
{
    email = emailAddress;
}

void User::setContactNumber(const std::string& number)
{
    contactNumber = number;
}

void User::setRole(UserRole userRole)
{
    role = userRole;
}

void User::setActive(bool status)
{
    active = status;
}

const char* userRoleToString(UserRole role)
{
    switch (role)
    {
        case UserRole::ADMIN:
            return "ADMIN";

        case UserRole::COORDINATOR:
            return "COORDINATOR";

        case UserRole::RESPONDER:
            return "RESPONDER";

        default:
            return "COORDINATOR";
    }
}

bool stringToUserRole(const std::string& value, UserRole& role)
{
    if (value == "ADMIN")
        role = UserRole::ADMIN;
    else if (value == "COORDINATOR")
        role = UserRole::COORDINATOR;
    else if (value == "RESPONDER")
        role = UserRole::RESPONDER;
    else
        return false;

    return true;
}

namespace
{
const int ITERATIONS = 210000;
const int SALT_SIZE = 16;
const int HASH_SIZE = 32;

std::string toHex(const unsigned char* data, int length)
{
    std::ostringstream output;
    output << std::hex << std::setfill('0');

    for (int i = 0; i < length; i++)
        output << std::setw(2) << static_cast<int>(data[i]);

    return output.str();
}

bool fromHex(const std::string& hex, unsigned char* output, int length)
{
    if (hex.length() != static_cast<size_t>(length * 2))
        return false;

    for (int i = 0; i < length; i++)
    {
        char* end = nullptr;
        std::string byte = hex.substr(i * 2, 2);
        long value = std::strtol(byte.c_str(), &end, 16);

        if (end == nullptr || *end != '\0')
            return false;

        output[i] = static_cast<unsigned char>(value);
    }

    return true;
}
}

std::string PasswordHasher::hashPassword(const std::string& password)
{
    if (password.length() < 10)
        return "";

    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];

    if (RAND_bytes(salt, SALT_SIZE) != 1)
        return "";

    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()), salt, SALT_SIZE, ITERATIONS,
                          EVP_sha256(), HASH_SIZE, hash) != 1)
        return "";

    return "pbkdf2_sha256$" + std::to_string(ITERATIONS) + "$" + toHex(salt, SALT_SIZE) + "$" +
           toHex(hash, HASH_SIZE);
}

bool PasswordHasher::verifyPassword(const std::string& password, const std::string& storedHash)
{
    std::istringstream input(storedHash);
    std::string algorithm;
    std::string iterationText;
    std::string saltHex;
    std::string hashHex;

    if (!std::getline(input, algorithm, '$') || !std::getline(input, iterationText, '$') ||
        !std::getline(input, saltHex, '$') || !std::getline(input, hashHex, '$'))
        return false;

    if (algorithm != "pbkdf2_sha256")
        return false;

    int iterations = 0;

    try
    {
        size_t position = 0;
        iterations = std::stoi(iterationText, &position);

        if (position != iterationText.length() || iterations < 100000 || iterations > 1000000)
            return false;
    }
    catch (...)
    {
        return false;
    }

    unsigned char salt[SALT_SIZE];
    unsigned char expectedHash[HASH_SIZE];
    unsigned char actualHash[HASH_SIZE];

    if (!fromHex(saltHex, salt, SALT_SIZE) || !fromHex(hashHex, expectedHash, HASH_SIZE))
        return false;

    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()), salt, SALT_SIZE, iterations,
                          EVP_sha256(), HASH_SIZE, actualHash) != 1)
        return false;

    return CRYPTO_memcmp(actualHash, expectedHash, HASH_SIZE) == 0;
}

UserRepository::UserRepository(DatabaseConnection& db) : database(db)
{
}

bool UserRepository::saveUser(const User& user)
{
    MYSQL* connection = database.getConnection();
    if (!connection)
        return false;

    std::string username = user.getUsername();
    std::string passwordHash = user.getPasswordHash();
    std::string fullName = user.getFullName();
    std::string email = user.getEmail();
    std::string contact = user.getContactNumber();
    std::string role = userRoleToString(user.getRole());

    if (username.empty() || passwordHash.empty() || fullName.empty() || role.empty())
        return false;

    const char* query = "INSERT INTO users "
                        "(username, password_hash, full_name, email, contact_number, role, is_active) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?)";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    MYSQL_BIND parameters[7] = {};
    unsigned long lengths[6] = {
            static_cast<unsigned long>(username.length()), static_cast<unsigned long>(passwordHash.length()),
            static_cast<unsigned long>(fullName.length()), static_cast<unsigned long>(email.length()),
            static_cast<unsigned long>(contact.length()),  static_cast<unsigned long>(role.length())};

    DbBool emailNull = email.empty();
    DbBool contactNull = contact.empty();
    unsigned char active = user.isActive() ? 1 : 0;

    bindString(parameters[0], username, lengths[0]);

    bindString(parameters[1], passwordHash, lengths[1]);

    bindString(parameters[2], fullName, lengths[2]);

    parameters[3].buffer_type = MYSQL_TYPE_STRING;
    parameters[3].buffer = email.empty() ? nullptr : &email[0];
    parameters[3].buffer_length = lengths[3];
    parameters[3].length = &lengths[3];
    parameters[3].is_null = &emailNull;

    parameters[4].buffer_type = MYSQL_TYPE_STRING;
    parameters[4].buffer = contact.empty() ? nullptr : &contact[0];
    parameters[4].buffer_length = lengths[4];
    parameters[4].length = &lengths[4];
    parameters[4].is_null = &contactNull;

    bindString(parameters[5], role, lengths[5]);

    parameters[6].buffer_type = MYSQL_TYPE_TINY;
    parameters[6].buffer = &active;

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);
    return success;
}

bool UserRepository::loadUserByUsername(const std::string& username, User& user)
{
    MYSQL* connection = database.getConnection();
    if (!connection || username.empty())
        return false;

    const char* query = "SELECT user_id, username, password_hash, full_name, "
                        "email, contact_number, role, is_active "
                        "FROM users WHERE username = ?";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    MYSQL_BIND input[1] = {};
    unsigned long usernameLength = static_cast<unsigned long>(username.length());

    bindString(input[0], username, usernameLength);

    if (mysql_stmt_bind_param(statement, input) != 0 || mysql_stmt_execute(statement) != 0 ||
        mysql_stmt_store_result(statement) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    int id = 0;
    char usernameBuffer[61] = {};
    char hashBuffer[256] = {};
    char nameBuffer[121] = {};
    char emailBuffer[151] = {};
    char contactBuffer[26] = {};
    char roleBuffer[20] = {};
    unsigned char active = 0;

    unsigned long lengths[7] = {};
    DbBool emailNull = 0;
    DbBool contactNull = 0;

    MYSQL_BIND results[8] = {};

    bindInt(results[0], id);

    results[1].buffer_type = MYSQL_TYPE_STRING;
    results[1].buffer = usernameBuffer;
    results[1].buffer_length = sizeof(usernameBuffer);
    results[1].length = &lengths[0];

    results[2].buffer_type = MYSQL_TYPE_STRING;
    results[2].buffer = hashBuffer;
    results[2].buffer_length = sizeof(hashBuffer);
    results[2].length = &lengths[1];

    results[3].buffer_type = MYSQL_TYPE_STRING;
    results[3].buffer = nameBuffer;
    results[3].buffer_length = sizeof(nameBuffer);
    results[3].length = &lengths[2];

    results[4].buffer_type = MYSQL_TYPE_STRING;
    results[4].buffer = emailBuffer;
    results[4].buffer_length = sizeof(emailBuffer);
    results[4].length = &lengths[3];
    results[4].is_null = &emailNull;

    results[5].buffer_type = MYSQL_TYPE_STRING;
    results[5].buffer = contactBuffer;
    results[5].buffer_length = sizeof(contactBuffer);
    results[5].length = &lengths[4];
    results[5].is_null = &contactNull;

    results[6].buffer_type = MYSQL_TYPE_STRING;
    results[6].buffer = roleBuffer;
    results[6].buffer_length = sizeof(roleBuffer);
    results[6].length = &lengths[5];

    results[7].buffer_type = MYSQL_TYPE_TINY;
    results[7].buffer = &active;

    if (mysql_stmt_bind_result(statement, results) != 0 || mysql_stmt_fetch(statement) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    UserRole role;
    std::string roleValue(roleBuffer, lengths[5]);

    if (!stringToUserRole(roleValue, role))
    {
        mysql_stmt_close(statement);
        return false;
    }

    User loadedUser(id, std::string(usernameBuffer, lengths[0]), std::string(hashBuffer, lengths[1]),
                    std::string(nameBuffer, lengths[2]),
                    emailNull ? "" : std::string(emailBuffer, lengths[3]),
                    contactNull ? "" : std::string(contactBuffer, lengths[4]), role);

    loadedUser.setActive(active != 0);
    user = loadedUser;

    mysql_stmt_close(statement);
    return true;
}

bool UserRepository::updateUserProfile(const User& user)
{
    MYSQL* connection = database.getConnection();
    if (!connection || user.getUserId() <= 0 || user.getFullName().empty())
        return false;

    std::string name = user.getFullName();
    std::string email = user.getEmail();
    std::string contact = user.getContactNumber();
    std::string role = userRoleToString(user.getRole());

    const char* query = "UPDATE users SET full_name = ?, email = ?, "
                        "contact_number = ?, role = ? WHERE user_id = ?";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    MYSQL_BIND parameters[5] = {};
    unsigned long lengths[4] = {
            static_cast<unsigned long>(name.length()), static_cast<unsigned long>(email.length()),
            static_cast<unsigned long>(contact.length()), static_cast<unsigned long>(role.length())};

    DbBool emailNull = email.empty();
    DbBool contactNull = contact.empty();
    int id = user.getUserId();

    bindString(parameters[0], name, lengths[0]);

    parameters[1].buffer_type = MYSQL_TYPE_STRING;
    parameters[1].buffer = email.empty() ? nullptr : &email[0];
    parameters[1].buffer_length = lengths[1];
    parameters[1].length = &lengths[1];
    parameters[1].is_null = &emailNull;

    parameters[2].buffer_type = MYSQL_TYPE_STRING;
    parameters[2].buffer = contact.empty() ? nullptr : &contact[0];
    parameters[2].buffer_length = lengths[2];
    parameters[2].length = &lengths[2];
    parameters[2].is_null = &contactNull;

    bindString(parameters[3], role, lengths[3]);

    bindInt(parameters[4], id);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);
    return success;
}

bool UserRepository::updatePasswordHash(int userId, const std::string& passwordHash)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0 || passwordHash.empty())
        return false;

    const char* query = "UPDATE users SET password_hash = ? WHERE user_id = ?";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    int id = userId;
    unsigned long hashLength = static_cast<unsigned long>(passwordHash.length());

    MYSQL_BIND parameters[2] = {};

    bindString(parameters[0], passwordHash, hashLength);

    bindInt(parameters[1], id);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);
    return success;
}

bool UserRepository::setUserActive(int userId, bool active)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0)
        return false;

    const char* query = "UPDATE users SET is_active = ? WHERE user_id = ?";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    unsigned char activeValue = active ? 1 : 0;
    int id = userId;

    MYSQL_BIND parameters[2] = {};

    parameters[0].buffer_type = MYSQL_TYPE_TINY;
    parameters[0].buffer = &activeValue;

    bindInt(parameters[1], id);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    if (!success)
        return false;

    const char* checkQuery = "SELECT user_id FROM users WHERE user_id = ?";
    MYSQL_STMT* check = database.prepareStatement(checkQuery);
    if (!check)
        return false;

    MYSQL_BIND checkInput[1] = {};
    bindInt(checkInput[0], id);

    bool exists = mysql_stmt_bind_param(check, checkInput) == 0 && mysql_stmt_execute(check) == 0 &&
                  mysql_stmt_store_result(check) == 0 && mysql_stmt_num_rows(check) > 0;

    mysql_stmt_close(check);
    return exists;
}

bool UserRepository::userExists(const std::string& username)
{
    MYSQL* connection = database.getConnection();
    if (!connection || username.empty())
        return false;

    const char* query = "SELECT user_id FROM users WHERE username = ?";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    unsigned long usernameLength = static_cast<unsigned long>(username.length());

    MYSQL_BIND parameters[1] = {};
    bindString(parameters[0], username, usernameLength);

    bool exists = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0 &&
                  mysql_stmt_store_result(statement) == 0 && mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);
    return exists;
}

bool UserRepository::hasAnyUsers()
{
    MYSQL* connection = database.getConnection();
    if (!connection)
        return false;

    const char* query = "SELECT user_id FROM users LIMIT 1";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    bool success = mysql_stmt_execute(statement) == 0 && mysql_stmt_store_result(statement) == 0;

    bool found = success && mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);
    return found;
}

void UserRepository::displayAllUsers()
{
    MYSQL* connection = database.getConnection();
    if (!connection)
    {
        std::cout << "Database connection unavailable.\n";
        return;
    }

    const char* query = "SELECT user_id, username, full_name, role, is_active "
                        "FROM users ORDER BY user_id";

    if (mysql_query(connection, query) != 0)
    {
        std::cout << "Could not retrieve users: " << mysql_error(connection) << "\n";
        return;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    if (!result)
    {
        std::cout << "Could not read user records.\n";
        return;
    }

    std::cout << "\n---------------- REGISTERED USERS ----------------\n";
    std::cout << "ID\tUSERNAME\t\tFULL NAME\t\tROLE\tSTATUS\n";
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        std::cout << (row[0] ? row[0] : "-") << "\t" << (row[1] ? row[1] : "-") << "\t\t"
                  << (row[2] ? row[2] : "-") << "\t\t" << (row[3] ? row[3] : "-") << "\t"
                  << ((row[4] && std::string(row[4]) == "1") ? "ACTIVE" : "INACTIVE") << "\n";
    }
    std::cout << "--------------------------------------------------\n";
    mysql_free_result(result);
}

bool UserRepository::deactivateUserSafely(int userId, int currentUserId)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0 || userId == currentUserId)
        return false;

    const char* lookupQuery = "SELECT role, is_active FROM users WHERE user_id = ?";
    MYSQL_STMT* lookup = database.prepareStatement(lookupQuery);
    if (!lookup)
        return false;
    MYSQL_BIND in[1] = {};
    bindInt(in[0], userId);
    if (mysql_stmt_bind_param(lookup, in) != 0 || mysql_stmt_execute(lookup) != 0 ||
        mysql_stmt_store_result(lookup) != 0)
    {
        mysql_stmt_close(lookup);
        return false;
    }
    char role[32] = {};
    unsigned char active = 0;
    unsigned long roleLength = 0;
    MYSQL_BIND out[2] = {};
    out[0].buffer_type = MYSQL_TYPE_STRING;
    out[0].buffer = role;
    out[0].buffer_length = sizeof(role) - 1;
    out[0].length = &roleLength;
    out[1].buffer_type = MYSQL_TYPE_TINY;
    out[1].buffer = &active;
    bool fetched = mysql_stmt_bind_result(lookup, out) == 0 && mysql_stmt_fetch(lookup) == 0;
    mysql_stmt_close(lookup);
    if (!fetched || !active)
        return false;

    if (std::string(role) == "ADMIN")
    {
        if (mysql_query(connection, "SELECT COUNT(*) FROM users WHERE role='ADMIN' AND is_active=1") != 0)
            return false;
        MYSQL_RES* countResult = mysql_store_result(connection);
        if (!countResult)
            return false;
        MYSQL_ROW countRow = mysql_fetch_row(countResult);
        unsigned long activeAdmins = countRow && countRow[0] ? std::strtoul(countRow[0], nullptr, 10) : 0;
        mysql_free_result(countResult);
        if (activeAdmins <= 1)
            return false;
    }
    return setUserActive(userId, false);
}

bool UserRepository::reactivateUserSafely(int userId)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0)
        return false;

    const char* query = "SELECT is_active FROM users WHERE user_id = ?";
    MYSQL_STMT* statement = database.prepareStatement(query);
    if (!statement)
        return false;

    MYSQL_BIND input[1] = {};
    bindInt(input[0], userId);

    unsigned char active = 0;
    MYSQL_BIND output[1] = {};
    output[0].buffer_type = MYSQL_TYPE_TINY;
    output[0].buffer = &active;

    bool fetched = mysql_stmt_bind_param(statement, input) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_bind_result(statement, output) == 0 && mysql_stmt_fetch(statement) == 0;
    mysql_stmt_close(statement);

    if (!fetched || active != 0)
        return false;

    return setUserActive(userId, true);
}

namespace
{
string readPasswordImpl()
{
    string password;

#ifdef _WIN32
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n')
    {
        if (ch == '\b')
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else if (ch >= 32 && ch <= 126)
        {
            password += static_cast<char>(ch);
            cout << '*';
        }
    }
    cout << '\n';
#else
    termios oldSettings{};
    bool isTerminal = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &oldSettings) == 0;

    if (isTerminal)
    {
        termios hidden = oldSettings;
        hidden.c_lflag &= static_cast<tcflag_t>(~ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
    }

    getline(cin, password);

    if (isTerminal)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
        cout << '\n';
    }
#endif
    return password;
}

bool createFirstAdmin(UserRepository& repository)
{
    string username;
    string fullName;
    string password;
    string confirmPassword;

    cout << "\n====================================\n";
    cout << "       RESQFLOW FIRST-TIME SETUP\n";
    cout << "====================================\n";
    cout << "No user accounts found.\n";
    cout << "Create the first ADMIN account.\n\n";

    cout << "Full name: ";
    getline(cin, fullName);

    cout << "Username: ";
    getline(cin, username);

    if (fullName.empty() || username.empty())
    {
        cout << "Name and username cannot be empty.\n";
        return false;
    }

    cout << "Create password (minimum 10 characters): ";
    password = readPasswordImpl();

    cout << "Confirm password: ";
    confirmPassword = readPasswordImpl();

    if (password.length() < 10)
    {
        cout << "Password must contain at least 10 characters.\n";
        return false;
    }

    if (password != confirmPassword)
    {
        cout << "Passwords do not match.\n";
        return false;
    }

    string passwordHash = PasswordHasher::hashPassword(password);

    if (passwordHash.empty())
    {
        cout << "Password hashing failed. Admin account not created.\n";
        return false;
    }

    User admin(0, username, passwordHash, fullName, "", "", UserRole::ADMIN);

    if (!repository.saveUser(admin))
    {
        cout << "Could not create the administrator account.\n";
        cout << "Check the database and username.\n";
        return false;
    }

    cout << "\nFirst ADMIN account created successfully.\n";
    return true;
}
}

bool Authentication::authenticate(UserRepository& repository, User& loggedInUser)
{
    if (!repository.hasAnyUsers())
    {
        if (!createFirstAdmin(repository))
            return false;
    }

    cout << "\n====================================\n";
    cout << "           RESQFLOW LOGIN\n";
    cout << "====================================\n";

    for (int attempt = 1; attempt <= 3; attempt++)
    {
        string username;
        string password;

        cout << "\nUsername: ";
        getline(cin, username);

        cout << "Password: ";
        password = readPasswordImpl();

        User user;

        if (!repository.loadUserByUsername(username, user) || !user.isActive() ||
            !PasswordHasher::verifyPassword(password, user.getPasswordHash()))
        {
            cout << "\nInvalid username or password.\n";
            cout << "Attempts remaining: " << 3 - attempt << "\n";
            continue;
        }

        loggedInUser = user;

        cout << "\nLogin successful. Welcome, " << user.getFullName() << "!\n";

        cout << "Role: " << userRoleToString(user.getRole()) << "\n";
        return true;
    }

    cout << "\nMaximum login attempts exceeded.\n";
    return false;
}

std::string Authentication::promptPassword()
{
    return readPasswordImpl();
}
