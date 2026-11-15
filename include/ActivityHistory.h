#ifndef ACTIVITY_HISTORY_H
#define ACTIVITY_HISTORY_H

#include <ctime>
#include <string>

enum class ActivityType
{
    EMERGENCY_REPORTED,
    EMERGENCY_STATUS_CHANGED,
    ALLOCATION_CREATED,
    STOCK_RESERVED,
    STOCK_DISPATCHED,
    STOCK_RETURNED,
    ALLOCATION_CANCELLED,
    WAREHOUSE_TRANSFERRED,
    STOCK_ADDED,
    STOCK_REMOVED,
    RESPONDER_ASSIGNED,
    RESPONDER_RELEASED
};

class ActivityRecord
{
private:
    int activityId;
    ActivityType type;
    int emergencyId;
    int allocationId;
    int resourceId;
    int warehouseId;
    int destinationWarehouseId;
    int responderId;
    int quantity;
    std::string description;
    std::string performedBy;
    std::time_t timestamp;

public:
    ActivityRecord();
    ActivityRecord(int id, ActivityType activityType, int emergency, int allocation,
                   int resource, int warehouse, int destinationWarehouse,
                   int responder, int activityQuantity, const std::string& details,
                   const std::string& actor, std::time_t activityTime);

    int getActivityId() const;
    ActivityType getType() const;
    int getEmergencyId() const;
    int getAllocationId() const;
    int getResourceId() const;
    int getWarehouseId() const;
    int getDestinationWarehouseId() const;
    int getResponderId() const;
    int getQuantity() const;
    std::string getDescription() const;
    std::string getPerformedBy() const;
    std::time_t getTimestamp() const;
    void display() const;
};

class ActivityHistory
{
private:
    struct Node
    {
        ActivityRecord data;
        Node* previous;
        Node* next;
        explicit Node(const ActivityRecord& record);
    };

    Node* head;
    Node* tail;
    int count;
    int nextActivityId;

    static bool isValidType(ActivityType type);

public:
    ActivityHistory();
    ~ActivityHistory();

    ActivityHistory(const ActivityHistory&) = delete;
    ActivityHistory& operator=(const ActivityHistory&) = delete;

    bool isEmpty() const;
    int getCount() const;
    int getLastActivityId() const;   // 0 when empty

    // Pass -1 for an ID that does not apply to this activity.
    // A timestamp of 0 requests the current system time.
    bool addActivity(ActivityType type, const std::string& description,
                     const std::string& performedBy = "SYSTEM",
                     int emergencyId = -1, int allocationId = -1,
                     int resourceId = -1, int warehouseId = -1,
                     int destinationWarehouseId = -1, int responderId = -1,
                     int quantity = 0, std::time_t timestamp = 0);

    ActivityRecord* searchByActivityId(int activityId);

    bool loadActivityRecord(int activityId, ActivityType type,
                        int emergencyId, int allocationId,
                        int resourceId, int warehouseId,
                        int destinationWarehouseId, int responderId,
                        int quantity, const std::string& description,
                        const std::string& performedBy,
                        std::time_t timestamp);

    const ActivityRecord* searchByActivityId(int activityId) const;
    bool deleteActivity(int activityId);
    void clear();

    void displayAll() const;
    void displayByType(ActivityType type) const;
    void displayByEmergency(int emergencyId) const;
    void displayByAllocation(int allocationId) const;
    void displayByWarehouse(int warehouseId) const;
};

const char* activityTypeToString(ActivityType type);

#endif
