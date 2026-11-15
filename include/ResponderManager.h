#ifndef RESPONDER_MANAGER_H
#define RESPONDER_MANAGER_H
#include "Responder.h"
#include "ResponderCircularList.h"
class ResponderManager
{
private:
    struct Node
    {
        Responder data;
        Node* next;
        explicit Node(const Responder& responder) : data(responder), next(nullptr) {}
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

    bool loadResponderRecord(
    int responderId,
    string name,
    string role,
    string contact,
    string specialization,
    double latitude,
    double longitude,
    ResponderStatus status,
    int assignedEmergencyId
);

    // First responder currently assigned to the given emergency (nullptr when none).
    Responder* findAssignedToEmergency(int emergencyId);
};
#endif
