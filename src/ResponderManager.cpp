#include "../include/ResponderManager.h"
#include <iostream>
#include <cmath>
#include <new>
using namespace std;
ResponderManager::ResponderManager() : head(nullptr), tail(nullptr), responderCount(0), scheduler() {}
ResponderManager::~ResponderManager()
{
    Node* current = head;
    while (current) { Node* next = current->next; delete current; current = next; }
}
bool ResponderManager::addResponder(const Responder& responder)
{
    if (responder.getResponderId() <= 0 || responder.getResponderName().empty() || responder.getRole().empty() || responder.getContactNumber().empty() || responder.getSpecialization().empty() || !isfinite(responder.getLatitude()) || !isfinite(responder.getLongitude()) || responder.getLatitude() < -90.0 || responder.getLatitude() > 90.0 || responder.getLongitude() < -180.0 || responder.getLongitude() > 180.0 || findResponder(responder.getResponderId())) return false;
    Node* node = nullptr;
    try { node = new (nothrow) Node(responder); } catch (...) { return false; }
    if (!node) return false;
    if (!scheduler.addResponderId(responder.getResponderId())) { delete node; return false; }
    if (!head) head = tail = node;
    else { tail->next = node; tail = node; }
    ++responderCount;
    return true;
}
Responder* ResponderManager::findResponder(int id)
{
    Node* current = head;
    while (current) { if (current->data.getResponderId() == id) return &current->data; current = current->next; }
    return nullptr;
}
bool ResponderManager::removeResponder(int id)
{
    Node* current = head; Node* previous = nullptr;
    while (current && current->data.getResponderId() != id) { previous = current; current = current->next; }
    if (!current || current->data.getStatus() == ResponderStatus::ASSIGNED) return false;
    if (!scheduler.removeResponderId(id)) return false;
    if (previous) previous->next = current->next; else head = current->next;
    if (tail == current) tail = previous;
    delete current; --responderCount; return true;
}
bool ResponderManager::assignResponder(int id, int emergencyId)
{
    Responder* responder = findResponder(id);
    return responder && responder->assignEmergency(emergencyId);
}
bool ResponderManager::assignNextAvailableResponder(int emergencyId, int& assignedId)
{
    if (emergencyId <= 0 || responderCount == 0) return false;
    for (int i = 0; i < responderCount; ++i)
    {
        int candidateId = -1;
        if (!scheduler.getNextResponderId(candidateId)) return false;
        Responder* candidate = findResponder(candidateId);
        if (candidate && candidate->getStatus() == ResponderStatus::AVAILABLE && candidate->assignEmergency(emergencyId)) { assignedId = candidateId; return true; }
    }
    return false;
}
bool ResponderManager::releaseResponder(int id) { Responder* r = findResponder(id); return r && r->releaseResponder(); }
bool ResponderManager::setResponderOffDuty(int id) { Responder* r = findResponder(id); return r && r->setOffDuty(); }
bool ResponderManager::setResponderAvailable(int id) { Responder* r = findResponder(id); return r && r->setAvailable(); }
void ResponderManager::displayAllResponders() const
{
    if (!head) { cout << "\nNo responders registered.\n"; return; }
    Node* current = head; while (current) { current->data.displayResponder(); current = current->next; }
}
void ResponderManager::displayAvailableResponders() const
{
    Node* current = head; bool found = false;
    while (current) { if (current->data.getStatus() == ResponderStatus::AVAILABLE) { current->data.displayResponder(); found = true; } current = current->next; }
    if (!found) cout << "\nNo available responders.\n";
}
void ResponderManager::displayAssignedResponders() const
{
    Node* current = head; bool found = false;
    while (current) { if (current->data.getStatus() == ResponderStatus::ASSIGNED) { current->data.displayResponder(); found = true; } current = current->next; }
    if (!found) cout << "\nNo assigned responders.\n";
}
int ResponderManager::getResponderCount() const { return responderCount; }
int ResponderManager::getAvailableCount() const
{
    int count = 0; Node* current = head;
    while (current) { if (current->data.getStatus() == ResponderStatus::AVAILABLE) ++count; current = current->next; }
    return count;
}
bool ResponderManager::isEmpty() const { return head == nullptr; }

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

bool ResponderManager::loadResponderRecord(
    int responderId,
    string name,
    string role,
    string contact,
    string specialization,
    double latitude,
    double longitude,
    ResponderStatus status,
    int assignedEmergencyId)
{
    Responder responder(
        responderId,
        name,
        role,
        contact,
        specialization,
        latitude,
        longitude
    );

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
