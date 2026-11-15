#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
using namespace std;

class Resource
{
private:
    int resourceId;
    string resourceName;
    string category;
    string description;
    string unit;
    int minimumStockLevel;
    bool active;

public:
    Resource();
    Resource(int id, string name, string category, string description, string unit, int minimumStockLevel);

    int getResourceId() const;
    string getResourceName() const;
    string getCategory() const;
    string getDescription() const;
    string getUnit() const;
    int getMinimumStockLevel() const;
    bool isActive() const;

    void setResourceName(string name);
    void setCategory(string category);
    void setDescription(string description);
    void setUnit(string unit);
    void setMinimumStockLevel(int level);
    void setStatus(bool status);
};

#endif