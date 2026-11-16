#include "../include/EmergencyModule.h"
#include <iostream>
#include <cmath>
#include <limits>
#include <new>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

using namespace std;

Emergency::Emergency()
{
    emergencyId = 0;
    emergencyType = "";
    location = "";
    description = "";
    severity = 1;
    affectedPeople = 1;
    latitude = 0.0;
    longitude = 0.0;
    status = EmergencyStatus::REPORTED;
    registrationOrder = 0;
    registrationTime = time(nullptr);
}

Emergency::Emergency(int id, string type, string location, string description, int severity, double latitude,
                     double longitude, long long registrationOrder, int affectedPeople,
                     time_t registrationTime)
{
    emergencyId = id;
    emergencyType = type;
    this->location = location;
    this->description = description;
    this->severity = (severity >= 1 && severity <= 5) ? severity : 1;
    this->affectedPeople = (affectedPeople > 0) ? affectedPeople : 1;

    if (isfinite(latitude) && isfinite(longitude) && latitude >= -90 && latitude <= 90 && longitude >= -180 &&
        longitude <= 180)
    {
        this->latitude = latitude;
        this->longitude = longitude;
    }
    else
    {
        this->latitude = 0.0;
        this->longitude = 0.0;
    }

    status = EmergencyStatus::REPORTED;
    this->registrationOrder = registrationOrder;
    this->registrationTime = (registrationTime > 0) ? registrationTime : time(nullptr);
}

int Emergency::getEmergencyId() const
{
    return emergencyId;
}

string Emergency::getEmergencyType() const
{
    return emergencyType;
}

string Emergency::getLocation() const
{
    return location;
}

string Emergency::getDescription() const
{
    return description;
}

int Emergency::getSeverity() const
{
    return severity;
}

int Emergency::getAffectedPeople() const
{
    return affectedPeople;
}

double Emergency::getLatitude() const
{
    return latitude;
}

double Emergency::getLongitude() const
{
    return longitude;
}

EmergencyStatus Emergency::getStatus() const
{
    return status;
}

long long Emergency::getRegistrationOrder() const
{
    return registrationOrder;
}

time_t Emergency::getRegistrationTime() const
{
    return registrationTime;
}

void Emergency::setEmergencyType(string type)
{
    emergencyType = type;
}

void Emergency::setLocation(string location)
{
    this->location = location;
}

void Emergency::setDescription(string description)
{
    this->description = description;
}

bool Emergency::setSeverity(int severity)
{
    if (severity < 1 || severity > 5)
        return false;

    this->severity = severity;
    return true;
}

bool Emergency::setAffectedPeople(int affectedPeople)
{
    if (affectedPeople <= 0)
        return false;

    this->affectedPeople = affectedPeople;
    return true;
}

bool Emergency::setCoordinates(double latitude, double longitude)
{
    if (!isfinite(latitude) || !isfinite(longitude) || latitude < -90 || latitude > 90 || longitude < -180 ||
        longitude > 180)
        return false;

    this->latitude = latitude;
    this->longitude = longitude;

    return true;
}

void Emergency::setStatus(EmergencyStatus status)
{
    this->status = status;
}

double Emergency::calculatePopulationScore() const
{
    double score = affectedPeople / 100.0;

    if (score > 10.0)
        score = 10.0;

    return score;
}

double Emergency::calculateWaitingTimeBonus() const
{
    time_t currentTime = time(nullptr);

    if (registrationTime <= 0 || currentTime <= registrationTime)
        return 0.0;

    double waitingSeconds = difftime(currentTime, registrationTime);
    double bonus = waitingSeconds / 1800.0;

    if (bonus > 10.0)
        bonus = 10.0;

    return bonus;
}

double Emergency::calculatePriorityScore() const
{
    double severityScore = severity * 30.0;
    double populationScore = calculatePopulationScore();
    double waitingBonus = calculateWaitingTimeBonus();

    return severityScore + populationScore + waitingBonus;
}

const char* emergencyStatusToString(EmergencyStatus status)
{
    switch (status)
    {
        case EmergencyStatus::REPORTED:
            return "REPORTED";
        case EmergencyStatus::VERIFIED:
            return "VERIFIED";
        case EmergencyStatus::IN_PROGRESS:
            return "IN_PROGRESS";
        case EmergencyStatus::RESOLVED:
            return "RESOLVED";
        case EmergencyStatus::CANCELLED:
            return "CANCELLED";
    }
    return "UNKNOWN";
}

bool stringToEmergencyStatus(const string& value, EmergencyStatus& status)
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

bool isValidStatusChange(EmergencyStatus oldStatus, EmergencyStatus newStatus)
{
    switch (oldStatus)
    {
        case EmergencyStatus::REPORTED:
            return newStatus == EmergencyStatus::VERIFIED || newStatus == EmergencyStatus::CANCELLED;
        case EmergencyStatus::VERIFIED:
            return newStatus == EmergencyStatus::IN_PROGRESS || newStatus == EmergencyStatus::CANCELLED;
        case EmergencyStatus::IN_PROGRESS:
            return newStatus == EmergencyStatus::RESOLVED || newStatus == EmergencyStatus::CANCELLED;
        default:
            return false;
    }
}

string Emergency::getStatusText() const
{
    switch (status)
    {
        case EmergencyStatus::REPORTED:
            return "Reported";
        case EmergencyStatus::VERIFIED:
            return "Verified";
        case EmergencyStatus::IN_PROGRESS:
            return "In Progress";
        case EmergencyStatus::RESOLVED:
            return "Resolved";
        case EmergencyStatus::CANCELLED:
            return "Cancelled";
        default:
            return "Unknown";
    }
}

void Emergency::displayEmergency() const
{
    cout << "\n--------------------------------";
    cout << "\nEmergency ID: " << emergencyId;
    cout << "\nType: " << emergencyType;
    cout << "\nLocation: " << location;
    cout << "\nDescription: " << description;
    cout << "\nSeverity: " << severity << "/5";
    cout << "\nAffected People: " << affectedPeople;
    cout << "\nLatitude: " << latitude;
    cout << "\nLongitude: " << longitude;
    cout << "\nStatus: " << getStatusText();
    cout << "\nRegistration Order: " << registrationOrder;
    cout << "\nRegistration Time: ";

    if (registrationTime > 0)
        cout << ctime(&registrationTime);
    else
        cout << "Unavailable\n";

    cout << "Population Score: " << calculatePopulationScore();
    cout << "\nWaiting Time Bonus: " << calculateWaitingTimeBonus();
    cout << "\nTotal Priority Score: " << calculatePriorityScore();
    cout << "\n--------------------------------\n";
}

EmergencyMaxHeap::EmergencyMaxHeap()
{
    capacity = 10;
    size = 0;
    heap = new Emergency[capacity];
}

EmergencyMaxHeap::~EmergencyMaxHeap()
{
    delete[] heap;
}

void EmergencyMaxHeap::resizeHeap()
{
    int newCapacity = capacity * 2;
    Emergency* newHeap = new Emergency[newCapacity];

    for (int i = 0; i < size; i++)
        newHeap[i] = heap[i];

    delete[] heap;
    heap = newHeap;
    capacity = newCapacity;
}

void EmergencyMaxHeap::swapEmergency(int index1, int index2)
{
    Emergency temp = heap[index1];
    heap[index1] = heap[index2];
    heap[index2] = temp;
}

bool EmergencyMaxHeap::hasHigherPriority(const Emergency& first, const Emergency& second) const
{
    double firstScore = first.calculatePriorityScore();
    double secondScore = second.calculatePriorityScore();

    if (firstScore != secondScore)
        return firstScore > secondScore;

    if (first.getRegistrationOrder() != second.getRegistrationOrder())
        return first.getRegistrationOrder() < second.getRegistrationOrder();

    return first.getEmergencyId() < second.getEmergencyId();
}

int EmergencyMaxHeap::findEmergencyIndex(int emergencyId) const
{
    for (int i = 0; i < size; i++)
    {
        if (heap[i].getEmergencyId() == emergencyId)
            return i;
    }

    return -1;
}

void EmergencyMaxHeap::heapifyUp(int index)
{
    while (index > 0)
    {
        int parent = (index - 1) / 2;

        if (hasHigherPriority(heap[index], heap[parent]))
        {
            swapEmergency(index, parent);
            index = parent;
        }
        else
            break;
    }
}

void EmergencyMaxHeap::heapifyDown(int index)
{
    while (true)
    {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int highest = index;

        if (left < size && hasHigherPriority(heap[left], heap[highest]))
            highest = left;

        if (right < size && hasHigherPriority(heap[right], heap[highest]))
            highest = right;

        if (highest == index)
            break;

        swapEmergency(index, highest);
        index = highest;
    }
}

void EmergencyMaxHeap::rebuildHeap()
{
    for (int i = size / 2 - 1; i >= 0; i--)
        heapifyDown(i);
}

bool EmergencyMaxHeap::isEmpty() const
{
    return size == 0;
}

int EmergencyMaxHeap::getSize() const
{
    return size;
}

bool EmergencyMaxHeap::insertEmergency(const Emergency& emergency)
{
    if (emergency.getEmergencyId() <= 0 || emergency.getSeverity() < 1 || emergency.getSeverity() > 5 ||
        emergency.getAffectedPeople() <= 0 || emergency.getRegistrationTime() <= 0 ||
        emergency.getStatus() == EmergencyStatus::RESOLVED ||
        emergency.getStatus() == EmergencyStatus::CANCELLED)
        return false;

    if (findEmergencyIndex(emergency.getEmergencyId()) != -1)
        return false;

    if (size == capacity)
        resizeHeap();

    heap[size] = emergency;
    size++;

    rebuildHeap();

    return true;
}

bool EmergencyMaxHeap::extractMax(Emergency& emergency)
{
    if (isEmpty())
        return false;

    rebuildHeap();

    emergency = heap[0];
    heap[0] = heap[size - 1];
    size--;

    if (size > 0)
        heapifyDown(0);

    return true;
}

bool EmergencyMaxHeap::peekMax(Emergency& emergency)
{
    if (isEmpty())
        return false;

    rebuildHeap();
    emergency = heap[0];

    return true;
}

bool EmergencyMaxHeap::updatePriority(int emergencyId, int newSeverity, int newAffectedPeople)
{
    if (newSeverity < 1 || newSeverity > 5 || newAffectedPeople <= 0)
        return false;

    int index = findEmergencyIndex(emergencyId);

    if (index == -1)
        return false;

    if (heap[index].getStatus() == EmergencyStatus::RESOLVED ||
        heap[index].getStatus() == EmergencyStatus::CANCELLED)
        return false;

    heap[index].setSeverity(newSeverity);
    heap[index].setAffectedPeople(newAffectedPeople);

    if (index > 0 && hasHigherPriority(heap[index], heap[(index - 1) / 2]))
        heapifyUp(index);
    else
        heapifyDown(index);

    return true;
}

bool EmergencyMaxHeap::removeEmergency(int emergencyId)
{
    rebuildHeap();

    int index = findEmergencyIndex(emergencyId);

    if (index == -1)
        return false;

    heap[index] = heap[size - 1];
    size--;

    if (index < size)
    {
        if (index > 0 && hasHigherPriority(heap[index], heap[(index - 1) / 2]))
            heapifyUp(index);
        else
            heapifyDown(index);
    }

    return true;
}

bool EmergencyMaxHeap::refreshRecord(const Emergency& emergency)
{
    int index = findEmergencyIndex(emergency.getEmergencyId());

    if (index == -1)
        return false;

    heap[index] = emergency;

    if (index > 0 && hasHigherPriority(heap[index], heap[(index - 1) / 2]))
        heapifyUp(index);
    else
        heapifyDown(index);

    return true;
}

bool EmergencyMaxHeap::isValidHeap() const
{
    for (int i = 1; i < size; i++)
    {
        if (hasHigherPriority(heap[i], heap[(i - 1) / 2]))
            return false;
    }

    return true;
}

void EmergencyMaxHeap::displayHeap()
{
    if (isEmpty())
    {
        cout << "\nEmergency priority heap is empty.\n";
        return;
    }

    rebuildHeap();

    cout << "\n========== EMERGENCY PRIORITY HEAP ==========\n";

    for (int i = 0; i < size; i++)
    {
        cout << "\nHeap Position: " << i;
        cout << "\nEmergency ID: " << heap[i].getEmergencyId();
        cout << "\nType: " << heap[i].getEmergencyType();
        cout << "\nSeverity: " << heap[i].getSeverity();
        cout << "\nAffected People: " << heap[i].getAffectedPeople();
        cout << "\nPopulation Score: " << heap[i].calculatePopulationScore();
        cout << "\nWaiting Time Bonus: " << heap[i].calculateWaitingTimeBonus();
        cout << "\nTotal Priority Score: " << heap[i].calculatePriorityScore();
        cout << "\nLocation: " << heap[i].getLocation();
        cout << "\nRegistration Order: " << heap[i].getRegistrationOrder();
        cout << "\n---------------------------------------------\n";
    }
}

EmergencyManager::EmergencyManager()
{
    head = nullptr;
    count = 0;
    nextEmergencyId = 1;
    nextRegistrationOrder = 1;
    priorityHeap = nullptr;
}
EmergencyManager::~EmergencyManager()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }
}
void EmergencyManager::connectPriorityHeap(EmergencyMaxHeap& heap)
{
    if (head == nullptr)
        priorityHeap = &heap;
}

bool EmergencyManager::loadEmergencyRecord(int emergencyId, string type, string location, string description,
                                           int severity, double latitude, double longitude,
                                           long long registrationOrder, int affectedPeople,
                                           time_t registrationTime, EmergencyStatus status)
{
    if (priorityHeap == nullptr || emergencyId <= 0 || emergencyId == numeric_limits<int>::max() ||
        type.empty() || location.empty() || severity < 1 || severity > 5 || affectedPeople < 1 ||
        !isfinite(latitude) || !isfinite(longitude) || latitude < -90 || latitude > 90 || longitude < -180 ||
        longitude > 180 || registrationOrder <= 0 || registrationOrder == numeric_limits<long long>::max() ||
        registrationTime <= 0 || count == numeric_limits<int>::max())
    {
        return false;
    }

    switch (status)
    {
        case EmergencyStatus::REPORTED:
        case EmergencyStatus::VERIFIED:
        case EmergencyStatus::IN_PROGRESS:
        case EmergencyStatus::RESOLVED:
        case EmergencyStatus::CANCELLED:
            break;
        default:
            return false;
    }

    Node* current = head;

    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == emergencyId)
        {
            return false;
        }
        current = current->next;
    }

    Emergency emergency(emergencyId, type, location, description, severity, latitude, longitude,
                        registrationOrder, affectedPeople, registrationTime);

    emergency.setStatus(status);

    Node* newNode = new (nothrow) Node(emergency);

    if (newNode == nullptr)
    {
        return false;
    }

    bool isActive = status != EmergencyStatus::RESOLVED && status != EmergencyStatus::CANCELLED;

    if (isActive && !priorityHeap->insertEmergency(emergency))
    {
        delete newNode;
        return false;
    }

    if (head == nullptr)
    {
        head = newNode;
    }
    else
    {
        current = head;

        while (current->next != nullptr)
        {
            current = current->next;
        }

        current->next = newNode;
    }

    count++;

    if (emergencyId >= nextEmergencyId)
    {
        nextEmergencyId = emergencyId + 1;
    }

    if (registrationOrder >= nextRegistrationOrder)
    {
        nextRegistrationOrder = registrationOrder + 1;
    }

    return true;
}

int EmergencyManager::peekNextEmergencyId() const
{
    return nextEmergencyId;
}

void EmergencyManager::ensureNextIdAtLeast(int minimumId)
{
    if (minimumId > nextEmergencyId)
        nextEmergencyId = minimumId;
}

bool EmergencyManager::displayPriorityQueue()
{
    if (priorityHeap == nullptr)
        return false;

    priorityHeap->displayHeap();
    return true;
}

bool EmergencyManager::peekHighestPriority(Emergency& emergency)
{
    return priorityHeap != nullptr && priorityHeap->peekMax(emergency);
}

bool EmergencyManager::canChangeStatus(int emergencyId, EmergencyStatus status)
{
    Emergency* emergency = searchEmergency(emergencyId);
    if (emergency == nullptr)
        return false;

    return isValidStatusChange(emergency->getStatus(), status);
}

bool EmergencyManager::isEmpty() const
{
    return head == nullptr;
}
int EmergencyManager::getCount() const
{
    return count;
}
bool EmergencyManager::addEmergency(string type, string location, string description, int severity,
                                    double latitude, double longitude, int affectedPeople)
{
    if (priorityHeap == nullptr || type.empty() || location.empty() || severity < 1 || severity > 5 ||
        affectedPeople <= 0 || !isfinite(latitude) || !isfinite(longitude) || latitude < -90.0 ||
        latitude > 90.0 || longitude < -180.0 || longitude > 180.0 || nextEmergencyId <= 0 ||
        nextEmergencyId == numeric_limits<int>::max() || nextRegistrationOrder <= 0 ||
        nextRegistrationOrder == numeric_limits<long long>::max())
        return false;
    Emergency emergency(nextEmergencyId, type, location, description, severity, latitude, longitude,
                        nextRegistrationOrder, affectedPeople);
    Node* newNode = new (nothrow) Node(emergency);
    if (newNode == nullptr)
        return false;
    if (!priorityHeap->insertEmergency(emergency))
    {
        delete newNode;
        return false;
    }
    if (head == nullptr)
        head = newNode;
    else
    {
        Node* current = head;
        while (current->next != nullptr)
            current = current->next;
        current->next = newNode;
    }
    count++;
    nextEmergencyId++;
    nextRegistrationOrder++;
    return true;
}
bool EmergencyManager::deleteEmergency(int emergencyId)
{
    if (priorityHeap == nullptr || emergencyId <= 0)
        return false;
    Node* current = head;
    Node* previous = nullptr;
    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == emergencyId)
            break;
        previous = current;
        current = current->next;
    }
    if (current == nullptr)
        return false;
    EmergencyStatus status = current->data.getStatus();
    if (status != EmergencyStatus::RESOLVED && status != EmergencyStatus::CANCELLED)
    {
        if (!priorityHeap->removeEmergency(emergencyId))
            return false;
    }
    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;
    delete current;
    count--;
    return true;
}
Emergency* EmergencyManager::searchEmergency(int emergencyId)
{
    if (emergencyId <= 0)
        return nullptr;
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getEmergencyId() == emergencyId)
            return &(current->data);
        current = current->next;
    }
    return nullptr;
}
bool EmergencyManager::updateEmergencyStatus(int emergencyId, EmergencyStatus status)
{
    if (priorityHeap == nullptr)
        return false;
    Emergency* emergency = searchEmergency(emergencyId);
    if (emergency == nullptr)
        return false;
    if (!isValidStatusChange(emergency->getStatus(), status))
        return false;
    if (status == EmergencyStatus::RESOLVED || status == EmergencyStatus::CANCELLED)
    {
        if (!priorityHeap->removeEmergency(emergencyId))
            return false;
    }
    emergency->setStatus(status);

    if (status != EmergencyStatus::RESOLVED && status != EmergencyStatus::CANCELLED)
        priorityHeap->refreshRecord(*emergency);

    return true;
}
bool EmergencyManager::updateEmergencySeverity(int emergencyId, int severity)
{
    if (priorityHeap == nullptr || severity < 1 || severity > 5)
        return false;
    Emergency* emergency = searchEmergency(emergencyId);
    if (emergency == nullptr)
        return false;
    if (emergency->getStatus() == EmergencyStatus::RESOLVED ||
        emergency->getStatus() == EmergencyStatus::CANCELLED)
        return false;
    if (!priorityHeap->updatePriority(emergencyId, severity, emergency->getAffectedPeople()))
        return false;
    emergency->setSeverity(severity);
    return true;
}
bool EmergencyManager::updateEmergencyAffectedPeople(int emergencyId, int affectedPeople)
{
    if (priorityHeap == nullptr || affectedPeople <= 0)
        return false;
    Emergency* emergency = searchEmergency(emergencyId);
    if (emergency == nullptr)
        return false;
    if (emergency->getStatus() == EmergencyStatus::RESOLVED ||
        emergency->getStatus() == EmergencyStatus::CANCELLED)
        return false;
    if (!priorityHeap->updatePriority(emergencyId, emergency->getSeverity(), affectedPeople))
        return false;
    emergency->setAffectedPeople(affectedPeople);
    return true;
}
bool EmergencyManager::updateEmergencyDetails(int emergencyId, string type, string location,
                                              string description)
{
    if (type.empty() || location.empty())
        return false;
    Emergency* emergency = searchEmergency(emergencyId);
    if (emergency == nullptr)
        return false;
    if (emergency->getStatus() == EmergencyStatus::RESOLVED ||
        emergency->getStatus() == EmergencyStatus::CANCELLED)
        return false;
    emergency->setEmergencyType(type);
    emergency->setLocation(location);
    emergency->setDescription(description);

    if (priorityHeap != nullptr)
        priorityHeap->refreshRecord(*emergency);

    return true;
}
void EmergencyManager::displayAllEmergencies() const
{
    if (head == nullptr)
    {
        cout << "\nNo emergencies registered.\n";
        return;
    }
    cout << "\n========== ALL EMERGENCIES ==========\n";
    Node* current = head;
    while (current != nullptr)
    {
        current->data.displayEmergency();
        current = current->next;
    }
}
void EmergencyManager::displayEmergenciesByStatus(EmergencyStatus status) const
{
    Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getStatus() == status)
        {
            current->data.displayEmergency();
            found = true;
        }
        current = current->next;
    }
    if (!found)
        cout << "\nNo emergencies found with the requested status.\n";
}

void reportStatementError(MYSQL_STMT* stmt, const char* context)
{
    std::cerr << "[EmergencyRepository] " << context << ": "
              << (stmt != nullptr ? mysql_stmt_error(stmt) : "no statement") << '\n';
}

EmergencyRepository::EmergencyRepository(DatabaseConnection& db)
    : database(db)
{
}

bool EmergencyRepository::saveEmergency(const Emergency& emergency)
{
    if (!database.isConnected() || emergency.getEmergencyId() <= 0 || emergency.getEmergencyType().empty() ||
        emergency.getLocation().empty() || emergency.getSeverity() < 1 || emergency.getSeverity() > 5 ||
        emergency.getAffectedPeople() < 1 || emergency.getRegistrationOrder() <= 0 ||
        emergency.getRegistrationTime() <= 0)
    {
        return false;
    }

    const char* status = emergencyStatusToString(emergency.getStatus());

    const char* query = "INSERT INTO emergencies "
                        "(emergency_id, emergency_type, location_name, description, "
                        "severity, affected_people, latitude, longitude, status, "
                        "registration_order, reported_at) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, FROM_UNIXTIME(?))";

    if (!database.beginTransaction())
        return false;

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
    {
        std::cerr << "[EmergencyRepository] prepare INSERT failed: " << database.getLastError() << '\n';
        database.rollbackTransaction();
        return false;
    }

    int id = emergency.getEmergencyId();
    int severity = emergency.getSeverity();
    int affectedPeople = emergency.getAffectedPeople();
    double latitude = emergency.getLatitude();
    double longitude = emergency.getLongitude();
    long long registrationOrder = emergency.getRegistrationOrder();
    long long registrationTime = static_cast<long long>(emergency.getRegistrationTime());

    std::string type = emergency.getEmergencyType();
    std::string location = emergency.getLocation();
    std::string description = emergency.getDescription();
    std::string statusText = status;

    unsigned long typeLength = type.length();
    unsigned long locationLength = location.length();
    unsigned long descriptionLength = description.length();
    unsigned long statusLength = statusText.length();

    MYSQL_BIND bind[11] = {};

    bindInt(bind[0], id);

    bindString(bind[1], type, typeLength);

    bindString(bind[2], location, locationLength);

    bindString(bind[3], description, descriptionLength);

    bindInt(bind[4], severity);

    bindInt(bind[5], affectedPeople);

    bindDouble(bind[6], latitude);

    bindDouble(bind[7], longitude);

    bindString(bind[8], statusText, statusLength);

    bindLongLong(bind[9], registrationOrder);

    bindLongLong(bind[10], registrationTime);

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

    const char* historyQuery = "INSERT INTO emergency_status_history "
                               "(emergency_id, old_status, new_status, changed_by, notes) "
                               "VALUES (?, NULL, ?, NULL, 'Emergency registered')";

    MYSQL_STMT* historyStmt = database.prepareStatement(historyQuery);

    if (historyStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND historyBind[2] = {};
    bindInt(historyBind[0], id);
    bindString(historyBind[1], statusText, statusLength);

    bool historySuccess =
            mysql_stmt_bind_param(historyStmt, historyBind) == 0 && mysql_stmt_execute(historyStmt) == 0;

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

bool EmergencyRepository::loadEmergencies(EmergencyManager& emergencyManager)
{
    if (!database.isConnected())
        return false;

    const char* query = "SELECT emergency_id, emergency_type, location_name, description, "
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
            if (row[0] == nullptr || row[1] == nullptr || row[2] == nullptr || row[4] == nullptr ||
                row[5] == nullptr || row[6] == nullptr || row[7] == nullptr || row[8] == nullptr ||
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
            time_t registrationTime = static_cast<time_t>(std::stoll(row[9]));

            EmergencyStatus status;

            if (!stringToEmergencyStatus(row[10], status))
            {
                success = false;
                break;
            }

            if (!emergencyManager.loadEmergencyRecord(id, type, location, description, severity, latitude,
                                                      longitude, registrationOrder, affectedPeople,
                                                      registrationTime, status))
            {
                success = false;
                break;
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "[EmergencyRepository] corrupt row while loading: " << error.what() << '\n';
        success = false;
    }

    mysql_free_result(result);
    return success;
}

bool EmergencyRepository::updateEmergencyDetails(const Emergency& emergency)
{
    if (!database.isConnected() || emergency.getEmergencyId() <= 0 || emergency.getEmergencyType().empty() ||
        emergency.getLocation().empty())
        return false;

    const char* query = "UPDATE emergencies SET emergency_type = ?, "
                        "location_name = ?, description = ? "
                        "WHERE emergency_id = ?";

    if (!emergencyExists(emergency.getEmergencyId()))
    {
        std::cerr << "[EmergencyRepository] emergency " << emergency.getEmergencyId()
                  << " does not exist in MySQL\n";
        return false;
    }

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
    {
        std::cerr << "[EmergencyRepository] prepare UPDATE details failed: " << database.getLastError()
                  << '\n';
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

    bindString(bind[0], type, typeLength);

    bindString(bind[1], location, locationLength);

    bindString(bind[2], description, descriptionLength);

    bindInt(bind[3], id);

    bool success = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;

    if (!success)
        reportStatementError(stmt, "UPDATE details");

    mysql_stmt_close(stmt);
    return success;
}

bool EmergencyRepository::updateEmergencySeverity(const Emergency& emergency)
{
    if (!database.isConnected() || emergency.getEmergencyId() <= 0 || emergency.getSeverity() < 1 ||
        emergency.getSeverity() > 5)
        return false;

    if (!emergencyExists(emergency.getEmergencyId()))
        return false;

    const char* query = "UPDATE emergencies SET severity = ? WHERE emergency_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int severity = emergency.getSeverity();
    int id = emergency.getEmergencyId();

    MYSQL_BIND bind[2] = {};

    bindInt(bind[0], severity);

    bindInt(bind[1], id);

    bool success = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;

    if (!success)
        reportStatementError(stmt, "UPDATE severity");

    mysql_stmt_close(stmt);
    return success;
}

bool EmergencyRepository::updateEmergencyAffectedPeople(const Emergency& emergency)
{
    if (!database.isConnected() || emergency.getEmergencyId() <= 0 || emergency.getAffectedPeople() < 1)
        return false;

    if (!emergencyExists(emergency.getEmergencyId()))
        return false;

    const char* query = "UPDATE emergencies SET affected_people = ? WHERE emergency_id = ?";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    int affectedPeople = emergency.getAffectedPeople();
    int id = emergency.getEmergencyId();

    MYSQL_BIND bind[2] = {};

    bindInt(bind[0], affectedPeople);

    bindInt(bind[1], id);

    bool success = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;

    if (!success)
        reportStatementError(stmt, "UPDATE affected people");

    mysql_stmt_close(stmt);
    return success;
}

bool EmergencyRepository::updateEmergencyStatus(int emergencyId, EmergencyStatus newStatus, const char* notes,
                                                int changedBy)
{
    if (!database.isConnected() || emergencyId <= 0)
        return false;

    const char* newStatusText = emergencyStatusToString(newStatus);

    if (!database.beginTransaction())
        return false;

    const char* selectQuery = "SELECT status FROM emergencies WHERE emergency_id = ? FOR UPDATE";

    MYSQL_STMT* selectStmt = database.prepareStatement(selectQuery);

    if (selectStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    MYSQL_BIND selectBind[1] = {};
    bindInt(selectBind[0], emergencyId);

    bool selectSuccess = mysql_stmt_bind_param(selectStmt, selectBind) == 0 &&
                         mysql_stmt_execute(selectStmt) == 0 && mysql_stmt_store_result(selectStmt) == 0;

    char oldStatusBuffer[30] = {};
    unsigned long oldStatusLength = 0;

    MYSQL_BIND resultBind[1] = {};
    resultBind[0].buffer_type = MYSQL_TYPE_STRING;
    resultBind[0].buffer = oldStatusBuffer;
    resultBind[0].buffer_length = sizeof(oldStatusBuffer);
    resultBind[0].length = &oldStatusLength;

    if (!selectSuccess || mysql_stmt_num_rows(selectStmt) != 1 ||
        mysql_stmt_bind_result(selectStmt, resultBind) != 0 || mysql_stmt_fetch(selectStmt) != 0)
    {
        mysql_stmt_close(selectStmt);
        database.rollbackTransaction();
        return false;
    }

    mysql_stmt_close(selectStmt);

    EmergencyStatus oldStatus;

    if (!stringToEmergencyStatus(std::string(oldStatusBuffer, oldStatusLength), oldStatus) ||
        !isValidStatusChange(oldStatus, newStatus))
    {
        database.rollbackTransaction();
        return false;
    }

    const char* updateQuery = "UPDATE emergencies SET status = ? WHERE emergency_id = ?";
    MYSQL_STMT* updateStmt = database.prepareStatement(updateQuery);

    if (updateStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    std::string newStatusString = newStatusText;
    unsigned long newStatusLength = newStatusString.length();

    MYSQL_BIND updateBind[2] = {};

    bindString(updateBind[0], newStatusString, newStatusLength);

    bindInt(updateBind[1], emergencyId);

    bool updateSuccess = mysql_stmt_bind_param(updateStmt, updateBind) == 0 &&
                         mysql_stmt_execute(updateStmt) == 0 && mysql_stmt_affected_rows(updateStmt) == 1;

    if (!updateSuccess)
        reportStatementError(updateStmt, "UPDATE status");

    mysql_stmt_close(updateStmt);

    if (!updateSuccess)
    {
        database.rollbackTransaction();
        return false;
    }

    const char* historyQuery = "INSERT INTO emergency_status_history "
                               "(emergency_id, old_status, new_status, changed_by, notes) "
                               "VALUES (?, ?, ?, ?, ?)";

    MYSQL_STMT* historyStmt = database.prepareStatement(historyQuery);

    if (historyStmt == nullptr)
    {
        database.rollbackTransaction();
        return false;
    }

    std::string oldStatusString = emergencyStatusToString(oldStatus);
    unsigned long historyOldStatusLength = oldStatusString.length();

    std::string notesString = notes ? notes : "";
    unsigned long notesLength = notesString.length();

    DbBool changedByIsNull = (changedBy <= 0) ? 1 : 0;

    MYSQL_BIND historyBind[5] = {};

    bindInt(historyBind[0], emergencyId);

    bindString(historyBind[1], oldStatusString, historyOldStatusLength);

    bindString(historyBind[2], newStatusString, newStatusLength);

    bindInt(historyBind[3], changedBy);
    historyBind[3].is_null = &changedByIsNull;

    bindString(historyBind[4], notesString, notesLength);

    bool historySuccess =
            mysql_stmt_bind_param(historyStmt, historyBind) == 0 && mysql_stmt_execute(historyStmt) == 0;

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

    const char* query = "SELECT emergency_id FROM emergencies "
                        "WHERE emergency_id = ? LIMIT 1";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND bind[1] = {};
    bindInt(bind[0], emergencyId);

    bool success = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0 &&
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

    const char* query = "SELECT COALESCE(old_status, '-'), new_status, "
                        "COALESCE(notes, ''), changed_at "
                        "FROM emergency_status_history "
                        "WHERE emergency_id = ? ORDER BY status_history_id";

    MYSQL_STMT* stmt = database.prepareStatement(query);

    if (stmt == nullptr)
        return false;

    MYSQL_BIND input[1] = {};
    bindInt(input[0], emergencyId);

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

    bool ok = mysql_stmt_bind_param(stmt, input) == 0 && mysql_stmt_execute(stmt) == 0 &&
              mysql_stmt_bind_result(stmt, output) == 0 && mysql_stmt_store_result(stmt) == 0;

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
        std::cout << changedAt << " | " << oldStatus << " -> " << newStatus;

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
