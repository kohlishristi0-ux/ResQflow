#include <iostream>
#include "EmergencyPriorityQueue.h"

using namespace std;

EmergencyPriorityQueue::EmergencyPriorityQueue()
{
    count = 0;
}

bool EmergencyPriorityQueue::hasHigherPriority(
    const Emergency& a,
    const Emergency& b
) const
{
    if (a.getSeverity() != b.getSeverity())
    {
        return a.getSeverity() > b.getSeverity();
    }

    return a.getRegistrationOrder()
           < b.getRegistrationOrder();
}

bool EmergencyPriorityQueue::enqueue(Emergency emergency)
{
    if (isFull())
    {
        cout << "Priority queue is full.\n";
        return false;
    }

    int i = count - 1;

    while (i >= 0 &&
           hasHigherPriority(emergency, emergencies[i]))
    {
        emergencies[i + 1] = emergencies[i];
        i--;
    }

    emergencies[i + 1] = emergency;
    count++;

    return true;
}

bool EmergencyPriorityQueue::dequeue(Emergency& result)
{
    if (isEmpty())
    {
        cout << "No emergencies to process.\n";
        return false;
    }

    result = emergencies[0];

    for (int i = 0; i < count - 1; i++)
    {
        emergencies[i] = emergencies[i + 1];
    }

    count--;

    return true;
}

void EmergencyPriorityQueue::displayQueue() const
{
    if (isEmpty())
    {
        cout << "No emergencies registered.\n";
        return;
    }

    for (int i = 0; i < count; i++)
    {
        cout << "\nPriority Position: " << i + 1;
        emergencies[i].displayEmergency();
    }
}

bool EmergencyPriorityQueue::isEmpty() const
{
    return count == 0;
}

bool EmergencyPriorityQueue::isFull() const
{
    return count == MAX_EMERGENCIES;
}