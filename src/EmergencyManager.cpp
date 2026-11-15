#include <iostream>
#include <cmath>
#include <limits>
#include <new>
#include "../include/EmergencyManager.h"
using namespace std;
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

bool EmergencyManager::loadEmergencyRecord(
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
    EmergencyStatus status)
{
    if (priorityHeap == nullptr ||
        emergencyId <= 0 ||
        emergencyId == numeric_limits<int>::max() ||
        type.empty() ||
        location.empty() ||
        severity < 1 || severity > 5 ||
        affectedPeople < 1 ||
        !isfinite(latitude) || !isfinite(longitude) ||
        latitude < -90 || latitude > 90 ||
        longitude < -180 || longitude > 180 ||
        registrationOrder <= 0 ||
        registrationOrder == numeric_limits<long long>::max() ||
        registrationTime <= 0 ||
        count == numeric_limits<int>::max())
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

    Emergency emergency(
        emergencyId,
        type,
        location,
        description,
        severity,
        latitude,
        longitude,
        registrationOrder,
        affectedPeople,
        registrationTime
    );

    emergency.setStatus(status);

    Node* newNode = new (nothrow) Node(emergency);

    if (newNode == nullptr)
    {
        return false;
    }

    bool isActive =
        status != EmergencyStatus::RESOLVED &&
        status != EmergencyStatus::CANCELLED;

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

    switch (emergency->getStatus())
    {
        case EmergencyStatus::REPORTED:
            return status == EmergencyStatus::VERIFIED || status == EmergencyStatus::CANCELLED;
        case EmergencyStatus::VERIFIED:
            return status == EmergencyStatus::IN_PROGRESS || status == EmergencyStatus::CANCELLED;
        case EmergencyStatus::IN_PROGRESS:
            return status == EmergencyStatus::RESOLVED || status == EmergencyStatus::CANCELLED;
        default:
            return false;   // RESOLVED and CANCELLED are final
    }
}

bool EmergencyManager::isEmpty() const
{
    return head == nullptr;
}
int EmergencyManager::getCount() const
{
    return count;
}
bool EmergencyManager::addEmergency(string type, string location, string description, int severity, double latitude, double longitude, int affectedPeople)
{
    if (priorityHeap == nullptr ||
        type.empty() ||
        location.empty() ||
        severity < 1 ||
        severity > 5 ||
        affectedPeople <= 0 ||
        !isfinite(latitude) ||
        !isfinite(longitude) ||
        latitude < -90.0 ||
        latitude > 90.0 ||
        longitude < -180.0 ||
        longitude > 180.0 ||
        nextEmergencyId <= 0 ||
        nextEmergencyId == numeric_limits<int>::max() ||
        nextRegistrationOrder <= 0 ||
        nextRegistrationOrder == numeric_limits<long long>::max())
        return false;
    Emergency emergency(
        nextEmergencyId,
        type,
        location,
        description,
        severity,
        latitude,
        longitude,
        nextRegistrationOrder,
        affectedPeople
    );
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
    if (status != EmergencyStatus::RESOLVED &&
        status != EmergencyStatus::CANCELLED)
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
    EmergencyStatus currentStatus = emergency->getStatus();
    bool validTransition = false;
    if (currentStatus == EmergencyStatus::REPORTED)
        validTransition = (status == EmergencyStatus::VERIFIED || status == EmergencyStatus::CANCELLED);
    else if (currentStatus == EmergencyStatus::VERIFIED)
        validTransition = (status == EmergencyStatus::IN_PROGRESS || status == EmergencyStatus::CANCELLED);
    else if (currentStatus == EmergencyStatus::IN_PROGRESS)
        validTransition = (status == EmergencyStatus::RESOLVED || status == EmergencyStatus::CANCELLED);
    if (!validTransition)
        return false;
    if (status == EmergencyStatus::RESOLVED || status == EmergencyStatus::CANCELLED)
    {
        if (!priorityHeap->removeEmergency(emergencyId))
            return false;
    }
    emergency->setStatus(status);

    // keep the heap's copy of still-active emergencies in sync (status shown in the queue)
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
    if (!priorityHeap->updatePriority(
        emergencyId,
        severity,
        emergency->getAffectedPeople()))
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
    if (!priorityHeap->updatePriority(
        emergencyId,
        emergency->getSeverity(),
        affectedPeople))
        return false;
    emergency->setAffectedPeople(affectedPeople);
    return true;
}
bool EmergencyManager::updateEmergencyDetails(int emergencyId, string type, string location, string description)
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
