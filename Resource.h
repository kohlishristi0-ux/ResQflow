#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
using namespace std;

class Resource
{
private:
    int resourceId;
    string resourceName;
    int quantity;
    string location;

public:
    Resource();

    void inputResource();
    void displayResource() const;

    int getResourceId() const;
    string getResourceName() const;
    int getQuantity() const;
    string getLocation() const;

    void setQuantity(int quantity);
};

#endif