#ifndef EMERGENCY_PRIORITY_QUEUE_H
#define EMERGENCY_PRIORITY_QUEUE_H

#include "Emergency.h"

const int MAX_EMERGENCIES = 100;

class EmergencyPriorityQueue
{
private:
    Emergency emergencies[MAX_EMERGENCIES];
    int count;

    bool hasHigherPriority(
        const Emergency& a,
        const Emergency& b
    ) const;

public:
    EmergencyPriorityQueue();

    bool enqueue(Emergency emergency);
    bool dequeue(Emergency& result);

    void displayQueue() const;

    bool isEmpty() const;
    bool isFull() const;
};

#endif