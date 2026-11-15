#ifndef RESOURCE_HASH_TABLE_H
#define RESOURCE_HASH_TABLE_H

#include "Resource.h"

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

#endif