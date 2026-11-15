#ifndef EMERGENCY_MAX_HEAP_H
#define EMERGENCY_MAX_HEAP_H

#include "Emergency.h"

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

    // Replace the stored copy of an emergency (details/status changed) and restore the heap property.
    bool refreshRecord(const Emergency& emergency);

    // Verifies the max-heap property for every parent/child pair (used by tests and diagnostics).
    bool isValidHeap() const;

    void displayHeap();
};

#endif