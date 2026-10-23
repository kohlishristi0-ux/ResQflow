#include <iostream>
#include "Resource.h"

using namespace std;

Resource::Resource()
{
    resourceId = 0;
    resourceName = "";
    quantity = 0;
    location = "";
}

void Resource::inputResource()
{
    cout << "Enter Resource ID: ";
    cin >> resourceId;

    cout << "Enter Resource Name: ";
    cin >> ws;
    getline(cin, resourceName);

    cout << "Enter Available Quantity: ";
    cin >> quantity;

    cout << "Enter Location: ";
    cin >> ws;
    getline(cin, location);
}

void Resource::displayResource() const
{
    cout << "\nResource ID: " << resourceId;
    cout << "\nResource Name: " << resourceName;
    cout << "\nAvailable Quantity: " << quantity;
    cout << "\nLocation: " << location << endl;
}

int Resource::getResourceId() const
{
    return resourceId;
}

string Resource::getResourceName() const
{
    return resourceName;
}

int Resource::getQuantity() const
{
    return quantity;
}

string Resource::getLocation() const
{
    return location;
}

void Resource::setQuantity(int quantity)
{
    this->quantity = quantity;
}