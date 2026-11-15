#include <iostream>
#include <cassert>
#include <ctime>
#include "../include/Emergency.h"
#include "../include/EmergencyMaxHeap.h"
#include "../include/EmergencyManager.h"

using namespace std;

void testEmergencyScoring()
{
    Emergency emergency(1, "Flood", "Delhi", "Heavy flooding", 5, 28.6, 77.2, 1, 500, time(nullptr) - 3600);
    assert(emergency.getAffectedPeople() == 500);
    assert(emergency.calculatePopulationScore() == 5.0);
    assert(emergency.calculateWaitingTimeBonus() >= 2.0);
    assert(emergency.calculateWaitingTimeBonus() <= 10.0);
    assert(emergency.calculatePriorityScore() > 150.0);
    assert(!emergency.setAffectedPeople(0));
    assert(!emergency.setSeverity(6));
    assert(!emergency.setCoordinates(100.0, 200.0));
    assert(emergency.setAffectedPeople(800));
    assert(emergency.setSeverity(4));
    cout << "Emergency scoring and validation: PASSED\n";
}

void testHeapInsertionAndOrdering()
{
    EmergencyMaxHeap heap;
    time_t registrationTime = time(nullptr);

    Emergency first(1, "Flood", "Delhi", "Emergency 1", 3, 28.6, 77.2, 1, 100, registrationTime);
    Emergency second(2, "Fire", "Mumbai", "Emergency 2", 4, 19.0, 72.8, 2, 100, registrationTime);
    Emergency third(3, "Accident", "Pune", "Emergency 3", 3, 18.5, 73.8, 3, 500, registrationTime);

    assert(heap.insertEmergency(first));
    assert(heap.insertEmergency(second));
    assert(heap.insertEmergency(third));
    assert(!heap.insertEmergency(first));
    assert(heap.getSize() == 3);

    Emergency highest;
    assert(heap.peekMax(highest));
    assert(highest.getEmergencyId() == 2);

    assert(heap.updatePriority(1, 5, 100));
    assert(heap.peekMax(highest));
    assert(highest.getEmergencyId() == 1);

    assert(heap.updatePriority(3, 5, 1000));
    assert(heap.peekMax(highest));
    assert(highest.getEmergencyId() == 3);

    cout << "Heap insertion, ordering and priority updates: PASSED\n";
}

void testHeapRemovalAndExtraction()
{
    EmergencyMaxHeap heap;

    for (int i = 1; i <= 15; i++)
    {
        Emergency emergency(i, "Test", "Location", "Testing", i % 5 + 1, 20.0, 70.0, i, 100);
        assert(heap.insertEmergency(emergency));
    }

    assert(heap.getSize() == 15);
    assert(!heap.removeEmergency(100));
    assert(heap.removeEmergency(5));
    assert(heap.getSize() == 14);

    Emergency previous;
    assert(heap.extractMax(previous));

    while (!heap.isEmpty())
    {
        Emergency current;
        assert(heap.extractMax(current));
        assert(previous.calculatePriorityScore() >= current.calculatePriorityScore());
        previous = current;
    }

    assert(!heap.extractMax(previous));
    assert(!heap.peekMax(previous));

    cout << "Heap resizing, removal and extraction: PASSED\n";
}

void testFIFOOrdering()
{
    EmergencyMaxHeap heap;
    time_t registrationTime = time(nullptr);

    Emergency first(1, "Flood", "Delhi", "First", 3, 28.6, 77.2, 1, 100, registrationTime);
    Emergency second(2, "Fire", "Mumbai", "Second", 3, 19.0, 72.8, 2, 100, registrationTime);

    assert(heap.insertEmergency(first));
    assert(heap.insertEmergency(second));

    Emergency highest;
    assert(heap.peekMax(highest));
    assert(highest.getEmergencyId() == 1);

    cout << "FIFO ordering for equal priority: PASSED\n";
}

void testEmergencyManager()
{
    EmergencyMaxHeap heap;
    EmergencyManager manager;

    manager.connectPriorityHeap(heap);

    assert(manager.isEmpty());
    assert(manager.getCount() == 0);

    assert(!manager.addEmergency("", "Delhi", "Test", 3, 28.6, 77.2, 100));
    assert(!manager.addEmergency("Flood", "Delhi", "Test", 6, 28.6, 77.2, 100));
    assert(!manager.addEmergency("Flood", "Delhi", "Test", 3, 28.6, 77.2, 0));
    assert(!manager.addEmergency("Flood", "Delhi", "Test", 3, 100.0, 77.2, 100));

    assert(manager.addEmergency("Flood", "Delhi", "Heavy flooding", 3, 28.6, 77.2, 100));
    assert(manager.addEmergency("Fire", "Mumbai", "Building fire", 4, 19.0, 72.8, 500));
    assert(manager.getCount() == 2);
    assert(heap.getSize() == 2);

    Emergency* first = manager.searchEmergency(1);
    assert(first != nullptr);
    assert(first->getAffectedPeople() == 100);

    assert(manager.updateEmergencySeverity(1, 5));
    assert(first->getSeverity() == 5);

    assert(manager.updateEmergencyAffectedPeople(1, 1000));
    assert(first->getAffectedPeople() == 1000);

    assert(!manager.updateEmergencySeverity(1, 6));
    assert(!manager.updateEmergencyAffectedPeople(1, 0));

    assert(manager.updateEmergencyDetails(1, "Major Flood", "Delhi", "Updated description"));
    assert(first->getEmergencyType() == "Major Flood");

    assert(!manager.updateEmergencyDetails(1, "", "Delhi", "Invalid"));

    assert(manager.updateEmergencyStatus(1, EmergencyStatus::VERIFIED));
    assert(!manager.updateEmergencyStatus(1, EmergencyStatus::REPORTED));
    assert(manager.updateEmergencyStatus(1, EmergencyStatus::IN_PROGRESS));
    assert(manager.updateEmergencyStatus(1, EmergencyStatus::RESOLVED));

    assert(first->getStatus() == EmergencyStatus::RESOLVED);
    assert(heap.getSize() == 1);

    assert(!manager.updateEmergencySeverity(1, 3));
    assert(!manager.updateEmergencyAffectedPeople(1, 500));
    assert(!manager.updateEmergencyStatus(1, EmergencyStatus::CANCELLED));

    assert(manager.deleteEmergency(1));
    assert(manager.searchEmergency(1) == nullptr);
    assert(manager.getCount() == 1);
    assert(heap.getSize() == 1);

    assert(manager.updateEmergencyStatus(2, EmergencyStatus::CANCELLED));
    assert(heap.getSize() == 0);
    assert(manager.getCount() == 1);

    assert(!manager.deleteEmergency(100));
    assert(manager.deleteEmergency(2));
    assert(manager.isEmpty());

    cout << "Emergency Manager, synchronization and lifecycle: PASSED\n";
}

int main()
{
    cout << "\n========== RESQFLOW EMERGENCY MODULE TESTS ==========\n";

    testEmergencyScoring();
    testHeapInsertionAndOrdering();
    testHeapRemovalAndExtraction();
    testFIFOOrdering();
    testEmergencyManager();

    cout << "\nALL EMERGENCY MODULE TESTS PASSED SUCCESSFULLY!\n";

    return 0;
}