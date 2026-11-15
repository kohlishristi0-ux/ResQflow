#ifndef RESOURCE_LINKED_LIST_H
#define RESOURCE_LINKED_LIST_H

#include <string>
#include "Resource.h"
#include "ResourceHashTable.h"

class ResourceLinkedList
{
private:
    struct Node
    {
        Resource data;
        Node* next;

        Node(const Resource& resource) : data(resource), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int count;
    ResourceHashTable resourceIndex;

public:
    ResourceLinkedList();
    ~ResourceLinkedList();

    ResourceLinkedList(const ResourceLinkedList&) = delete;
    ResourceLinkedList& operator=(const ResourceLinkedList&) = delete;

    bool isEmpty() const;
    bool isDuplicate(int resourceId) const;

    bool addResource(const Resource& resource);
    bool deleteResource(int resourceId);

    Resource* searchResource(int resourceId);
    const Resource* searchResource(int resourceId) const;

    bool updateResource(int resourceId, std::string name, std::string category, std::string description, std::string unit, int minimumStockLevel);

    void displayAllResources() const;

    void clear();
    int getCount() const;

    // Case-insensitive substring search; prints every match and returns how many were found.
    int displayMatchingName(const std::string& fragment) const;
};

#endif