#ifndef EMERGENCY_MANAGER_H
#define EMERGENCY_MANAGER_H
#include "Emergency.h"
#include "EmergencyMaxHeap.h"
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

    // ID that the next successful addEmergency() will use (never getCount()+1: IDs may have gaps).
    int peekNextEmergencyId() const;
    void ensureNextIdAtLeast(int minimumId);
    bool displayPriorityQueue();
    bool peekHighestPriority(Emergency& emergency);

    // True when the emergency exists and the lifecycle allows moving to 'status' (no side effects).
    bool canChangeStatus(int emergencyId, EmergencyStatus status);
    bool addEmergency(string type, string location, string description, int severity, double latitude, double longitude, int affectedPeople = 1);
    bool deleteEmergency(int emergencyId);
    Emergency* searchEmergency(int emergencyId);
    bool updateEmergencyStatus(int emergencyId, EmergencyStatus status);
    bool updateEmergencySeverity(int emergencyId, int severity);
    bool updateEmergencyAffectedPeople(int emergencyId, int affectedPeople);
    bool updateEmergencyDetails(int emergencyId, string type, string location, string description);
    void displayAllEmergencies() const;
    void displayEmergenciesByStatus(EmergencyStatus status) const;
    bool loadEmergencyRecord(
    int emergencyId,
    string type,
    string location,
    string description,
    int severity,
    double latitude,
    double longitude,
    long long registrationOrder,
    int affectedPeople,
    time_t registrationTime,
    EmergencyStatus status
);
};
#endif