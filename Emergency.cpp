#include <iostream>
#include "Emergency.h"

using namespace std;

Emergency::Emergency()
{
    emergencyId = 0;
    emergencyType = "";
    location = "";
    description = "";
    severity = 0;
    registrationOrder = 0;
}

void Emergency::inputEmergency()
{
    cout << "Enter Emergency Type: ";
    cin >> ws;
    getline(cin, emergencyType);

    cout << "Enter Location: ";
    cin >> ws;
    getline(cin, location);

    cout << "Enter Description: ";
    cin >> ws;
    getline(cin, description);

    cout << "\nSelect Severity:\n";
    cout << "1. Low\n";
    cout << "2. Medium\n";
    cout << "3. High\n";
    cout << "4. Critical\n";

    cout << "Enter Severity: ";
    cin >> severity;

    while (severity < 1 || severity > 4)
    {
        cout << "Invalid severity. Enter 1-4: ";
        cin >> severity;
    }
}

void Emergency::displayEmergency() const
{
    cout << "\nEmergency ID: " << emergencyId;
    cout << "\nType: " << emergencyType;
    cout << "\nLocation: " << location;
    cout << "\nDescription: " << description;

    cout << "\nSeverity: ";

    switch (severity)
    {
        case 1:
            cout << "Low";
            break;

        case 2:
            cout << "Medium";
            break;

        case 3:
            cout << "High";
            break;

        case 4:
            cout << "Critical";
            break;
    }

    cout << "\nRegistration Order: "
         << registrationOrder << endl;
}

int Emergency::getEmergencyId() const
{
    return emergencyId;
}

string Emergency::getEmergencyType() const
{
    return emergencyType;
}

string Emergency::getLocation() const
{
    return location;
}

string Emergency::getDescription() const
{
    return description;
}

int Emergency::getSeverity() const
{
    return severity;
}

int Emergency::getRegistrationOrder() const
{
    return registrationOrder;
}

void Emergency::setEmergencyId(int id)
{
    emergencyId = id;
}

void Emergency::setRegistrationOrder(int order)
{
    registrationOrder = order;
}