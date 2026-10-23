#include <iostream>

#include "ResourceLinkedList.h"
#include "EmergencyPriorityQueue.h"
#include "AllocationManager.h"

using namespace std;

// RESOURCE MANAGEMENT MENU
void resourceMenu(ResourceLinkedList& resources)
{
    int choice;
    int id;
    int quantity;

    do
    {
        cout << "\n===== RESOURCE MANAGEMENT =====\n";
        cout << "1. Add Resource\n";
        cout << "2. Display All Resources\n";
        cout << "3. Search Resource\n";
        cout << "4. Update Resource Quantity\n";
        cout << "5. Return to Main Menu\n";

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
                resources.insertResource();
                break;

            case 2:
                resources.displayResources();
                break;

            case 3:
            {
                cout << "Enter Resource ID: ";
                cin >> id;

                ResourceNode* result =
                    resources.searchResource(id);

                if (result != nullptr)
                {
                    result->data.displayResource();
                }
                else
                {
                    cout << "Resource not found.\n";
                }

                break;
            }

            case 4:
                cout << "Enter Resource ID: ";
                cin >> id;

                cout << "Enter New Quantity: ";
                cin >> quantity;

                resources.updateQuantity(id, quantity);
                break;

            case 5:
                cout << "Returning to Main Menu.\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 5);
}


// EMERGENCY MANAGEMENT MENU
void emergencyMenu(
    EmergencyPriorityQueue& queue,
    int& emergencyId,
    int& registrationOrder)
{
    int choice;

    do
    {
        cout << "\n===== EMERGENCY MANAGEMENT =====\n";
        cout << "1. Register Emergency\n";
        cout << "2. Display Emergencies\n";
        cout << "3. Process Next Emergency\n";
        cout << "4. Return to Main Menu\n";

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
            {
                Emergency emergency;

                emergency.inputEmergency();

                emergency.setEmergencyId(emergencyId);
                emergency.setRegistrationOrder(
                    registrationOrder
                );

                if (queue.enqueue(emergency))
                {
                    cout << "Emergency registered successfully!\n";

                    emergencyId++;
                    registrationOrder++;
                }

                break;
            }

            case 2:
                queue.displayQueue();
                break;

            case 3:
            {
                Emergency nextEmergency;

                if (queue.dequeue(nextEmergency))
                {
                    cout << "\nProcessing highest-priority emergency:\n";
                    nextEmergency.displayEmergency();
                }

                break;
            }

            case 4:
                cout << "Returning to Main Menu.\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 4);
}


// ALLOCATION MENU
void allocationMenu(
    ResourceLinkedList& resources,
    AllocationManager& allocationManager)
{
    int emergencyId;
    int resourceId;
    int requestedQuantity;

    cout << "\n===== RESOURCE ALLOCATION =====\n";

    cout << "Enter Emergency ID: ";
    cin >> emergencyId;

    cout << "Enter Resource ID: ";
    cin >> resourceId;

    cout << "Enter Requested Quantity: ";
    cin >> requestedQuantity;

    allocationManager.allocateResource(
        resources,
        emergencyId,
        resourceId,
        requestedQuantity
    );
}


// MAIN FUNCTION
int main()
{
    // Shared objects: create them only once.
    ResourceLinkedList resources;
    EmergencyPriorityQueue emergencyQueue;
    AllocationManager allocationManager;

    int emergencyId = 101;
    int registrationOrder = 1;
    int choice;

    do
    {
        cout << "\n========================================\n";
        cout << " RESOURCE PLANNING AND EMERGENCY\n";
        cout << "       COORDINATION SYSTEM\n";
        cout << "========================================\n";

        cout << "1. Resource Management\n";
        cout << "2. Emergency Management\n";
        cout << "3. Allocate Resources\n";
        cout << "4. Display Allocation Records\n";
        cout << "5. Exit\n";

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
                resourceMenu(resources);
                break;

            case 2:
                emergencyMenu(
                    emergencyQueue,
                    emergencyId,
                    registrationOrder
                );
                break;

            case 3:
                allocationMenu(
                    resources,
                    allocationManager
                );
                break;

            case 4:
                allocationManager.displayAllocations();
                break;

            case 5:
                cout << "Exiting Resource Planning System.\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 5);

    return 0;
}