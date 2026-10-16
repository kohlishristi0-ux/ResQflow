#include <iostream>

#include "ResourceLinkedList.h"
#include "AllocationManager.h"

using namespace std;

int main()
{
    ResourceLinkedList resources;
    AllocationManager manager;

    cout << "===== ADD RESOURCES =====\n";

    // Add resource 101: Ambulance, quantity 5.
    // Add resource 102: First Aid Kit, quantity 100.
    // Use your existing resource insertion function.

    resources.insertResource();
    resources.insertResource();

    cout << "\n===== ALLOCATE RESOURCES =====\n";

    int emergencyId;
    int resourceId;
    int requestedQuantity;

    cout << "Enter Emergency ID: ";
    cin >> emergencyId;

    cout << "Enter Resource ID: ";
    cin >> resourceId;

    cout << "Enter Requested Quantity: ";
    cin >> requestedQuantity;

    manager.allocateResource(
        resources,
        emergencyId,
        resourceId,
        requestedQuantity
    );

    cout << "\n===== UPDATED INVENTORY =====\n";
    resources.displayResources();

    cout << "\n===== ALLOCATION RECORDS =====\n";
    manager.displayAllocations();

    return 0;
}