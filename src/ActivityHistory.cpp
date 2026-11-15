#include "../include/ActivityHistory.h"
#include <iostream>
#include <limits>
#include <new>

using namespace std;

const char* activityTypeToString(ActivityType type)
{
    switch (type)
    {
        case ActivityType::EMERGENCY_REPORTED: return "EMERGENCY_REPORTED";
        case ActivityType::EMERGENCY_STATUS_CHANGED: return "EMERGENCY_STATUS_CHANGED";
        case ActivityType::ALLOCATION_CREATED: return "ALLOCATION_CREATED";
        case ActivityType::STOCK_RESERVED: return "STOCK_RESERVED";
        case ActivityType::STOCK_DISPATCHED: return "STOCK_DISPATCHED";
        case ActivityType::STOCK_RETURNED: return "STOCK_RETURNED";
        case ActivityType::ALLOCATION_CANCELLED: return "ALLOCATION_CANCELLED";
        case ActivityType::WAREHOUSE_TRANSFERRED: return "WAREHOUSE_TRANSFERRED";
        case ActivityType::STOCK_ADDED: return "STOCK_ADDED";
        case ActivityType::STOCK_REMOVED: return "STOCK_REMOVED";
        case ActivityType::RESPONDER_ASSIGNED: return "RESPONDER_ASSIGNED";
        case ActivityType::RESPONDER_RELEASED: return "RESPONDER_RELEASED";
    }
    return "UNKNOWN";
}

ActivityRecord::ActivityRecord()
    : activityId(-1), type(ActivityType::STOCK_ADDED), emergencyId(-1), allocationId(-1),
      resourceId(-1), warehouseId(-1), destinationWarehouseId(-1), responderId(-1),
      quantity(0), timestamp(0) {}

ActivityRecord::ActivityRecord(int id, ActivityType activityType, int emergency, int allocation,
                               int resource, int warehouse, int destinationWarehouse,
                               int responder, int activityQuantity, const string& details,
                               const string& actor, time_t activityTime)
    : activityId(id), type(activityType), emergencyId(emergency), allocationId(allocation),
      resourceId(resource), warehouseId(warehouse), destinationWarehouseId(destinationWarehouse),
      responderId(responder), quantity(activityQuantity), description(details),
      performedBy(actor), timestamp(activityTime) {}

int ActivityRecord::getActivityId() const { return activityId; }
ActivityType ActivityRecord::getType() const { return type; }
int ActivityRecord::getEmergencyId() const { return emergencyId; }
int ActivityRecord::getAllocationId() const { return allocationId; }
int ActivityRecord::getResourceId() const { return resourceId; }
int ActivityRecord::getWarehouseId() const { return warehouseId; }
int ActivityRecord::getDestinationWarehouseId() const { return destinationWarehouseId; }
int ActivityRecord::getResponderId() const { return responderId; }
int ActivityRecord::getQuantity() const { return quantity; }
string ActivityRecord::getDescription() const { return description; }
string ActivityRecord::getPerformedBy() const { return performedBy; }
time_t ActivityRecord::getTimestamp() const { return timestamp; }

void ActivityRecord::display() const
{
    tm* timeInfo = localtime(&timestamp);
    char buffer[32] = "Unknown time";
    if (timeInfo != nullptr)
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

    cout << "Activity ID: " << activityId
         << " | Type: " << activityTypeToString(type)
         << " | Time: " << buffer
         << " | Emergency ID: " << emergencyId
         << " | Allocation ID: " << allocationId
         << " | Resource ID: " << resourceId
         << " | Warehouse ID: " << warehouseId
         << " | Destination Warehouse ID: " << destinationWarehouseId
         << " | Responder ID: " << responderId
         << " | Quantity: " << quantity
         << " | By: " << performedBy
         << " | Details: " << description << endl;
}

ActivityHistory::Node::Node(const ActivityRecord& record)
    : data(record), previous(nullptr), next(nullptr) {}

ActivityHistory::ActivityHistory() : head(nullptr), tail(nullptr), count(0), nextActivityId(1) {}

ActivityHistory::~ActivityHistory() { clear(); }

bool ActivityHistory::isValidType(ActivityType type)
{
    return type >= ActivityType::EMERGENCY_REPORTED && type <= ActivityType::RESPONDER_RELEASED;
}

bool ActivityHistory::isEmpty() const { return head == nullptr; }
int ActivityHistory::getCount() const { return count; }

bool ActivityHistory::addActivity(ActivityType type, const string& description,
                                  const string& performedBy, int emergencyId,
                                  int allocationId, int resourceId, int warehouseId,
                                  int destinationWarehouseId, int responderId,
                                  int quantity, time_t timestamp)
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

    ActivityRecord record(nextActivityId, type, emergencyId, allocationId, resourceId,
                          warehouseId, destinationWarehouseId, responderId, quantity,
                          description, performedBy, timestamp);
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

bool ActivityHistory::loadActivityRecord(int id, ActivityType type,
                                         int emergencyId, int allocationId,
                                         int resourceId, int warehouseId,
                                         int destinationWarehouseId, int responderId,
                                         int quantity, const string& description,
                                         const string& performedBy, time_t timestamp)
{
    if (!isValidType(type) || id <= 0 ||
        id == numeric_limits<int>::max() ||
        description.empty() || performedBy.empty() ||
        quantity < 0 || timestamp <= 0 ||
        emergencyId < -1 || allocationId < -1 ||
        resourceId < -1 || warehouseId < -1 ||
        destinationWarehouseId < -1 || responderId < -1 ||
        count == numeric_limits<int>::max() ||
        searchByActivityId(id) != nullptr)
        return false;

    ActivityRecord record(id, type, emergencyId, allocationId,
                          resourceId, warehouseId, destinationWarehouseId,
                          responderId, quantity, description,
                          performedBy, timestamp);

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
    // nextActivityId is intentionally NOT reset: activity IDs are never reused, so a record that was
    // already written to MySQL can never collide with a new one.
}

void ActivityHistory::displayAll() const
{
    if (isEmpty()) { cout << "No activity history available." << endl; return; }
    const Node* current = head;
    while (current != nullptr) { current->data.display(); current = current->next; }
}

void ActivityHistory::displayByType(ActivityType type) const
{
    const Node* current = head;
    bool found = false;
    while (current != nullptr)
    {
        if (current->data.getType() == type) { current->data.display(); found = true; }
        current = current->next;
    }
    if (!found) cout << "No activities found for this type." << endl;
}

void ActivityHistory::displayByEmergency(int id) const
{
    const Node* current = head; bool found = false;
    while (current != nullptr) { if (current->data.getEmergencyId() == id) { current->data.display(); found = true; } current = current->next; }
    if (!found) cout << "No activities found for this emergency." << endl;
}

void ActivityHistory::displayByAllocation(int id) const
{
    const Node* current = head; bool found = false;
    while (current != nullptr) { if (current->data.getAllocationId() == id) { current->data.display(); found = true; } current = current->next; }
    if (!found) cout << "No activities found for this allocation." << endl;
}

void ActivityHistory::displayByWarehouse(int id) const
{
    const Node* current = head; bool found = false;
    while (current != nullptr)
    {
        if (current->data.getWarehouseId() == id || current->data.getDestinationWarehouseId() == id)
        { current->data.display(); found = true; }
        current = current->next;
    }
    if (!found) cout << "No activities found for this warehouse." << endl;
}

int ActivityHistory::getLastActivityId() const
{
    return tail != nullptr ? tail->data.getActivityId() : 0;
}
