#ifndef RESOURCE_LINKED_LIST_H
#define RESOURCE_LINKED_LIST_H

#include "Resource.h"

struct ResourceNode
{
    Resource data;
    ResourceNode* next;
};

class ResourceLinkedList
{
private:
    ResourceNode* head;

public:
    ResourceLinkedList();
    ~ResourceLinkedList();

    bool insertResource();
    void displayResources() const;

    ResourceNode* searchResource(int id) const;
    bool updateQuantity(int id, int newQuantity);

    bool getResourceDetails(
    int resourceId,
    string& resourceName,
    int& availableQuantity
);

bool reduceResourceQuantity(
    int resourceId,
    int quantity
);
};

#endif
