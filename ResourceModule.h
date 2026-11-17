#ifndef RESOURCE_MODULE_H
#define RESOURCE_MODULE_H

#include <string>
#include "Database/DatabaseManager.h"

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

class ResourceHashTable
{
private:
    struct Entry
    {
        int resourceId;
        Resource* resource;
        Entry* next;

        Entry(int id, Resource* ptr)
        {
            resourceId = id;
            resource = ptr;
            next = nullptr;
        }
    };

    Entry** table;
    int capacity;
    int count;
    double loadFactor;

    int hashFunction(int resourceId) const;
    bool resizeTable();

public:
    ResourceHashTable(int initialCapacity = 10);
    ~ResourceHashTable();

    ResourceHashTable(const ResourceHashTable&) = delete;
    ResourceHashTable& operator=(const ResourceHashTable&) = delete;

    bool insert(int resourceId, Resource* resource);
    Resource* search(int resourceId) const;
    bool remove(int resourceId);

    bool contains(int resourceId) const;
    int getCount() const;
    int getCapacity() const;
    double getLoadFactor() const;

    void displayTable() const;
    void clear();
};

class ResourceLinkedList
{
private:
    struct Node
    {
        Resource data;
        Node* next;

        Node(const Resource& resource) : data(resource), next(nullptr)
        {
        }
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

    bool updateResource(int resourceId, std::string name, std::string category, std::string description,
                        std::string unit, int minimumStockLevel);

    void displayAllResources() const;

    void clear();
    int getCount() const;

    int displayMatchingName(const std::string& fragment) const;
};

class ResourceRepository
{
private:
    DatabaseConnection& database;

public:
    explicit ResourceRepository(DatabaseConnection& db);

    bool saveResource(const Resource& resource);

    bool loadActiveResources(ResourceLinkedList& resourceList);

    bool updateResource(const Resource& resource);

    bool deactivateResource(int resourceId);

    bool resourceExists(int resourceId);

    bool reactivateResource(int resourceId);
    bool isResourceActive(int resourceId);
};

#endif
