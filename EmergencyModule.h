#ifndef EMERGENCY_MODULE_H
#define EMERGENCY_MODULE_H

#include <string>
#include <ctime>
#include "Database/DatabaseManager.h"

using namespace std;

enum class EmergencyStatus
{
    REPORTED,
    VERIFIED,
    IN_PROGRESS,
    RESOLVED,
    CANCELLED
};

const char* emergencyStatusToString(EmergencyStatus status);
bool stringToEmergencyStatus(const string& value, EmergencyStatus& status);
bool isValidStatusChange(EmergencyStatus oldStatus, EmergencyStatus newStatus);

class Emergency
{
private:
    int emergencyId;
    string emergencyType;
    string location;
    string description;
    int severity;
    int affectedPeople;
    double latitude;
    double longitude;
    EmergencyStatus status;
    long long registrationOrder;
    time_t registrationTime;

public:
    Emergency();

    Emergency(int id, string type, string location, string description, int severity, double latitude,
              double longitude, long long registrationOrder, int affectedPeople = 1,
              time_t registrationTime = 0);

    int getEmergencyId() const;
    string getEmergencyType() const;
    string getLocation() const;
    string getDescription() const;
    int getSeverity() const;
    int getAffectedPeople() const;
    double getLatitude() const;
    double getLongitude() const;
    EmergencyStatus getStatus() const;
    long long getRegistrationOrder() const;
    time_t getRegistrationTime() const;

    void setEmergencyType(string type);
    void setLocation(string location);
    void setDescription(string description);
    bool setSeverity(int severity);
    bool setAffectedPeople(int affectedPeople);
    bool setCoordinates(double latitude, double longitude);
    void setStatus(EmergencyStatus status);

    double calculatePopulationScore() const;
    double calculateWaitingTimeBonus() const;
    double calculatePriorityScore() const;

    string getStatusText() const;
    void displayEmergency() const;
};

class EmergencyMaxHeap
{
private:
    Emergency* heap;
    int capacity;
    int size;

    void resizeHeap();
    void swapEmergency(int index1, int index2);
    void heapifyUp(int index);
    void heapifyDown(int index);
    void rebuildHeap();

    bool hasHigherPriority(const Emergency& first, const Emergency& second) const;
    int findEmergencyIndex(int emergencyId) const;

public:
    EmergencyMaxHeap();
    ~EmergencyMaxHeap();

    EmergencyMaxHeap(const EmergencyMaxHeap&) = delete;
    EmergencyMaxHeap& operator=(const EmergencyMaxHeap&) = delete;

    bool isEmpty() const;
    int getSize() const;

    bool insertEmergency(const Emergency& emergency);
    bool extractMax(Emergency& emergency);
    bool peekMax(Emergency& emergency);

    bool updatePriority(int emergencyId, int newSeverity, int newAffectedPeople);
    bool removeEmergency(int emergencyId);

    bool refreshRecord(const Emergency& emergency);

    bool isValidHeap() const;

    void displayHeap();
};

class EmergencyManager
{
private:
    struct Node
    {
        Emergency data;
        Node* next;
        Node(const Emergency& emergency)
        {
            data = emergency;
            next = nullptr;
        }
    };
    Node* head;
    int count;
    int nextEmergencyId;
    long long nextRegistrationOrder;
    EmergencyMaxHeap* priorityHeap;

public:
    EmergencyManager();
    ~EmergencyManager();
    EmergencyManager(const EmergencyManager&) = delete;
    EmergencyManager& operator=(const EmergencyManager&) = delete;
    void connectPriorityHeap(EmergencyMaxHeap& heap);
    bool isEmpty() const;
    int getCount() const;

    int peekNextEmergencyId() const;
    void ensureNextIdAtLeast(int minimumId);
    bool displayPriorityQueue();
    bool peekHighestPriority(Emergency& emergency);

    bool canChangeStatus(int emergencyId, EmergencyStatus status);
    bool addEmergency(string type, string location, string description, int severity, double latitude,
                      double longitude, int affectedPeople = 1);
    bool deleteEmergency(int emergencyId);
    Emergency* searchEmergency(int emergencyId);
    bool updateEmergencyStatus(int emergencyId, EmergencyStatus status);
    bool updateEmergencySeverity(int emergencyId, int severity);
    bool updateEmergencyAffectedPeople(int emergencyId, int affectedPeople);
    bool updateEmergencyDetails(int emergencyId, string type, string location, string description);
    void displayAllEmergencies() const;
    void displayEmergenciesByStatus(EmergencyStatus status) const;
    bool loadEmergencyRecord(int emergencyId, string type, string location, string description, int severity,
                             double latitude, double longitude, long long registrationOrder,
                             int affectedPeople, time_t registrationTime, EmergencyStatus status);
};

class EmergencyRepository
{
private:
    DatabaseConnection& database;

public:
    explicit EmergencyRepository(DatabaseConnection& db);

    bool saveEmergency(const Emergency& emergency);
    bool loadEmergencies(EmergencyManager& emergencyManager);

    bool updateEmergencyDetails(const Emergency& emergency);
    bool updateEmergencySeverity(const Emergency& emergency);
    bool updateEmergencyAffectedPeople(const Emergency& emergency);

    bool updateEmergencyStatus(int emergencyId, EmergencyStatus newStatus, const char* notes = nullptr,
                               int changedBy = 0);

    bool emergencyExists(int emergencyId);

    bool displayStatusHistory(int emergencyId);

    int getMaxEmergencyId();
};

#endif
