#include "../include/Resource.h"

Resource::Resource()
{
    resourceId = 0;
    resourceName = "";
    category = "";
    description = "";
    unit = "";
    minimumStockLevel = 0;
    active = true;
}

Resource::Resource(int id, string name, string category, string description, string unit, int minimumStockLevel)
{
    resourceId = id;
    resourceName = name;
    this->category = category;
    this->description = description;
    this->unit = unit;
    this->minimumStockLevel = minimumStockLevel;
    active = true;
}

int Resource::getResourceId() const
{
    return resourceId;
}

string Resource::getResourceName() const
{
    return resourceName;
}

string Resource::getCategory() const
{
    return category;
}

string Resource::getDescription() const
{
    return description;
}

string Resource::getUnit() const
{
    return unit;
}

int Resource::getMinimumStockLevel() const
{
    return minimumStockLevel;
}

bool Resource::isActive() const
{
    return active;
}

void Resource::setResourceName(string name)
{
    resourceName = name;
}

void Resource::setCategory(string category)
{
    this->category = category;
}

void Resource::setDescription(string description)
{
    this->description = description;
}

void Resource::setUnit(string unit)
{
    this->unit = unit;
}

void Resource::setMinimumStockLevel(int level)
{
    if (level >= 0)
        minimumStockLevel = level;
}

void Resource::setStatus(bool status)
{
    active = status;
}