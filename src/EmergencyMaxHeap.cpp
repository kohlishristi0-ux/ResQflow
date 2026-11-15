#include <iostream>
#include "../include/EmergencyMaxHeap.h"

using namespace std;

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
    if (emergency.getEmergencyId() <= 0 ||
        emergency.getSeverity() < 1 ||
        emergency.getSeverity() > 5 ||
        emergency.getAffectedPeople() <= 0 ||
        emergency.getRegistrationTime() <= 0 ||
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