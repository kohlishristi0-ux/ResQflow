#ifndef RESPONDER_MODULE_H
#define RESPONDER_MODULE_H

#include <string>
#include <ctime>
#include "Database/DatabaseManager.h"

using namespace std;

enum class ResponderStatus
{
    AVAILABLE,
    ASSIGNED,
    OFF_DUTY
};
class Responder
{
private:
    int responderId;
    string responderName;
    string role;
    string contactNumber;
    string specialization;
    double latitude;
    double longitude;
    ResponderStatus status;
    int assignedEmergencyId;

public:
    Responder();
    Responder(int id, string name, string role, string contact, string specialization, double latitude,
              double longitude);
    int getResponderId() const;
    string getResponderName() const;
    string getRole() const;
    string getContactNumber() const;
    string getSpecialization() const;
    double getLatitude() const;
    double getLongitude() const;
    ResponderStatus getStatus() const;
    int getAssignedEmergencyId() const;
    void setResponderName(string name);
    void setRole(string role);
    void setContactNumber(string contact);
    void setSpecialization(string specialization);
    bool setCoordinates(double latitude, double longitude);
    bool assignEmergency(int emergencyId);
    bool releaseResponder();
    bool setOffDuty();
    bool setAvailable();
    bool restoreState(ResponderStatus restoredStatus, int emergencyId);
    string getStatusText() const;
    void displayResponder() const;
};

class ResponderCircularList
{
private:
    struct Node
    {
        int responderId;
        Node* next;
        explicit Node(int id) : responderId(id), next(nullptr)
        {
        }
    };
    Node* tail;
    Node* cursor;
    int count;

public:
    ResponderCircularList();
    ~ResponderCircularList();
    ResponderCircularList(const ResponderCircularList&) = delete;
    ResponderCircularList& operator=(const ResponderCircularList&) = delete;
    bool addResponderId(int responderId);
    bool removeResponderId(int responderId);
    bool getNextResponderId(int& responderId);
    bool contains(int responderId) const;
    int getCount() const;
    bool isEmpty() const;
};

class ResponderManager
{
private:
    struct Node
    {
        Responder data;
        Node* next;
        explicit Node(const Responder& responder) : data(responder), next(nullptr)
        {
        }
    };
    Node* head;
    Node* tail;
    int responderCount;
    ResponderCircularList scheduler;

public:
    ResponderManager();
    ~ResponderManager();
    ResponderManager(const ResponderManager&) = delete;
    ResponderManager& operator=(const ResponderManager&) = delete;
    bool addResponder(const Responder& responder);
    Responder* findResponder(int responderId);
    bool removeResponder(int responderId);
    bool assignResponder(int responderId, int emergencyId);
    bool assignNextAvailableResponder(int emergencyId, int& assignedResponderId);
    bool releaseResponder(int responderId);
    bool setResponderOffDuty(int responderId);
    bool setResponderAvailable(int responderId);
    void displayAllResponders() const;
    void displayAvailableResponders() const;
    void displayAssignedResponders() const;
    int getResponderCount() const;
    int getAvailableCount() const;
    bool isEmpty() const;
    void clearResponders();

    bool loadResponderRecord(int responderId, string name, string role, string contact, string specialization,
                             double latitude, double longitude, ResponderStatus status,
                             int assignedEmergencyId);

    Responder* findAssignedToEmergency(int emergencyId);
};

class ResponderRepository
{
private:
    DatabaseConnection& database;

public:
    explicit ResponderRepository(DatabaseConnection& db);

    bool saveResponder(const Responder& responder);

    bool loadActiveResponders(ResponderManager& manager);

    bool updateResponder(const Responder& responder);

    bool deactivateResponder(int responderId);

    bool responderExists(int responderId);

    bool linkUserToResponder(int userId, int responderId);

    bool submitResponderRegistration(const std::string& username, const std::string& passwordHash,
                                     const std::string& fullName, const std::string& email,
                                     const std::string& contact, const std::string& responderRole,
                                     const std::string& specialization, const std::string& city,
                                     const std::string& area, const std::string& baseAddress);
    void displayPendingResponderRegistrations();
    bool approveResponderRegistration(int registrationId);
    bool rejectResponderRegistration(int registrationId);
    int findResponderIdForUser(int userId);
    bool activateResponderForUser(int userId);
    bool updateResponderStatus(int responderId, ResponderStatus status);

    bool assignResponder(int responderId, int emergencyId, const char* notes = nullptr);

    bool releaseResponder(int responderId);
};

class Location
{
private:
    int locationId;
    string locationName;
    double latitude;
    double longitude;
    bool active;

public:
    Location();
    Location(int id, string name, double latitude, double longitude);
    int getLocationId() const;
    string getLocationName() const;
    double getLatitude() const;
    double getLongitude() const;
    bool isActive() const;
    void setLocationName(string name);
    bool setCoordinates(double latitude, double longitude);
    void setActive(bool status);
    void displayLocation() const;

    double distanceKmTo(const Location& other) const;
};

class Stack
{
private:
    int* data;
    int capacity;
    int topIndex;

public:
    explicit Stack(int maxSize);
    ~Stack();
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;
    bool isEmpty() const;
    bool isFull() const;
    bool push(int value);
    bool pop(int& value);
    bool peek(int& value) const;
};

class LocationGraph
{
private:
    struct Edge
    {
        int destinationId;
        double distance;
        Edge* next;
        Edge(int id, double dist) : destinationId(id), distance(dist), next(nullptr)
        {
        }
    };
    struct Vertex
    {
        Location data;
        Edge* adjacencyList;
        Vertex* next;
        explicit Vertex(const Location& location) : data(location), adjacencyList(nullptr), next(nullptr)
        {
        }
    };
    Vertex* head;
    Vertex* tail;
    int vertexCount;
    Vertex* findVertex(int locationId) const;
    bool hasEdge(int sourceId, int destinationId) const;
    void clearEdges(Edge* edge);

public:
    LocationGraph();
    ~LocationGraph();
    LocationGraph(const LocationGraph&) = delete;
    LocationGraph& operator=(const LocationGraph&) = delete;
    bool addLocation(const Location& location);
    bool removeLocation(int locationId);
    Location* findLocation(int locationId);
    bool addConnection(int sourceId, int destinationId, double distance);
    bool removeConnection(int sourceId, int destinationId);
    bool hasConnection(int sourceId, int destinationId) const;
    void displayGraph() const;
    void breadthFirstSearch(int startLocationId) const;
    void depthFirstSearch(int startLocationId) const;
    void findShortestPath(int sourceId, int destinationId) const;
    int getLocationCount() const;
    bool isEmpty() const;
};

class RoutePlanner
{
private:
    LocationGraph* graph;

public:
    explicit RoutePlanner(LocationGraph* locationGraph);
    void setGraph(LocationGraph* locationGraph);
    void planRoute(int sourceId, int destinationId) const;
};

class LocationRepository
{
private:
    DatabaseConnection& database;

public:
    explicit LocationRepository(DatabaseConnection& db);

    bool saveLocation(const Location& location);

    bool loadLocations(LocationGraph& graph);

    bool updateLocation(const Location& location);

    bool deactivateLocation(int locationId);

    bool locationExists(int locationId);

    bool saveConnection(int sourceId, int destinationId, double distance);

    bool loadConnections(LocationGraph& graph);

    bool connectionExists(int sourceId, int destinationId);

    bool deactivateConnection(int sourceId, int destinationId);
};

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
    ActivityRecord(int id, ActivityType activityType, int emergency, int allocation, int resource,
                   int warehouse, int destinationWarehouse, int responder, int activityQuantity,
                   const std::string& details, const std::string& actor, std::time_t activityTime);

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
    int getLastActivityId() const;

    bool addActivity(ActivityType type, const std::string& description,
                     const std::string& performedBy = "SYSTEM", int emergencyId = -1, int allocationId = -1,
                     int resourceId = -1, int warehouseId = -1, int destinationWarehouseId = -1,
                     int responderId = -1, int quantity = 0, std::time_t timestamp = 0);

    ActivityRecord* searchByActivityId(int activityId);

    bool loadActivityRecord(int activityId, ActivityType type, int emergencyId, int allocationId,
                            int resourceId, int warehouseId, int destinationWarehouseId, int responderId,
                            int quantity, const std::string& description, const std::string& performedBy,
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

class ActivityHistoryRepository
{
private:
    DatabaseConnection& database;

public:
    explicit ActivityHistoryRepository(DatabaseConnection& db);

    bool saveActivity(const ActivityRecord& activity);

    bool loadActivityHistory(ActivityHistory& history);

    bool activityExists(int activityId);
};

enum class UserRole
{
    ADMIN,
    COORDINATOR,
    RESPONDER
};

class User
{
private:
    int userId;
    std::string username;
    std::string passwordHash;
    std::string fullName;
    std::string email;
    std::string contactNumber;
    UserRole role;
    bool active;

public:
    User();

    User(int id, const std::string& username, const std::string& passwordHash, const std::string& fullName,
         const std::string& email, const std::string& contactNumber, UserRole role);

    int getUserId() const;
    std::string getUsername() const;
    std::string getPasswordHash() const;
    std::string getFullName() const;
    std::string getEmail() const;
    std::string getContactNumber() const;
    UserRole getRole() const;
    bool isActive() const;

    void setFullName(const std::string& name);
    void setEmail(const std::string& emailAddress);
    void setContactNumber(const std::string& number);
    void setRole(UserRole userRole);
    void setActive(bool status);
};

const char* userRoleToString(UserRole role);
bool stringToUserRole(const std::string& value, UserRole& role);

class PasswordHasher
{
public:
    static std::string hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& storedHash);
};

class UserRepository
{
private:
    DatabaseConnection& database;

public:
    explicit UserRepository(DatabaseConnection& db);

    bool saveUser(const User& user);

    bool loadUserByUsername(const std::string& username, User& user);

    bool updateUserProfile(const User& user);

    bool updatePasswordHash(int userId, const std::string& passwordHash);

    bool setUserActive(int userId, bool active);

    bool userExists(const std::string& username);

    bool hasAnyUsers();
    void displayAllUsers();
    bool deactivateUserSafely(int userId, int currentUserId);
    bool reactivateUserSafely(int userId);
};

class Authentication
{
public:
    static bool authenticate(UserRepository& repository, User& loggedInUser);

    static std::string promptPassword();
};

#endif
