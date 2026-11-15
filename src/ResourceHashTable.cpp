#include <iostream>
#include <new>
#include <limits>
#include "../include/ResourceHashTable.h"

using namespace std;

ResourceHashTable::ResourceHashTable(int initialCapacity)
{
    if (initialCapacity < 1)
        initialCapacity = 10;

    capacity = initialCapacity;
    count = 0;
    loadFactor = 0.0;
    table = new Entry*[capacity]();
}

ResourceHashTable::~ResourceHashTable()
{
    clear();
    delete[] table;
}

int ResourceHashTable::hashFunction(int resourceId) const
{
    return resourceId % capacity;
}

bool ResourceHashTable::resizeTable()
{
    if (capacity > (numeric_limits<int>::max() - 1) / 2)
        return false;

    int newCapacity = capacity * 2 + 1;

    Entry** newTable = new (nothrow) Entry*[newCapacity]();

    if (newTable == nullptr)
        return false;

    for (int i = 0; i < capacity; i++)
    {
        Entry* current = table[i];

        while (current != nullptr)
        {
            Entry* nextNode = current->next;
            int newIndex = current->resourceId % newCapacity;

            current->next = newTable[newIndex];
            newTable[newIndex] = current;

            current = nextNode;
        }
    }

    delete[] table;
    table = newTable;
    capacity = newCapacity;
    loadFactor = static_cast<double>(count) / capacity;

    return true;
}

bool ResourceHashTable::insert(int resourceId, Resource* resource)
{
    if (resourceId <= 0 || resource == nullptr)
        return false;

    if (resource->getResourceId() != resourceId)
        return false;

    if (search(resourceId) != nullptr)
        return false;

    if (static_cast<double>(count + 1) / capacity > 0.75)
    {
        if (!resizeTable())
            return false;
    }

    int index = hashFunction(resourceId);

    Entry* newEntry = new (nothrow) Entry(resourceId, resource);

    if (newEntry == nullptr)
        return false;

    newEntry->next = table[index];
    table[index] = newEntry;

    count++;
    loadFactor = static_cast<double>(count) / capacity;

    return true;
}

Resource* ResourceHashTable::search(int resourceId) const
{
    if (resourceId <= 0)
        return nullptr;

    int index = hashFunction(resourceId);
    Entry* current = table[index];

    while (current != nullptr)
    {
        if (current->resourceId == resourceId)
            return current->resource;

        current = current->next;
    }

    return nullptr;
}

bool ResourceHashTable::remove(int resourceId)
{
    if (resourceId <= 0)
        return false;

    int index = hashFunction(resourceId);
    Entry* current = table[index];
    Entry* previous = nullptr;

    while (current != nullptr)
    {
        if (current->resourceId == resourceId)
        {
            if (previous == nullptr)
                table[index] = current->next;
            else
                previous->next = current->next;

            delete current;
            count--;
            loadFactor = static_cast<double>(count) / capacity;

            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

bool ResourceHashTable::contains(int resourceId) const
{
    return search(resourceId) != nullptr;
}

int ResourceHashTable::getCount() const
{
    return count;
}

int ResourceHashTable::getCapacity() const
{
    return capacity;
}

double ResourceHashTable::getLoadFactor() const
{
    return loadFactor;
}

void ResourceHashTable::displayTable() const
{
    cout << "\n========== RESOURCE HASH TABLE ==========\n";

    for (int i = 0; i < capacity; i++)
    {
        cout << "Index " << i << ": ";

        Entry* current = table[i];

        if (current == nullptr)
        {
            cout << "EMPTY";
        }
        else
        {
            while (current != nullptr)
            {
                cout << "[" << current->resourceId << "]";

                if (current->next != nullptr)
                    cout << " -> ";

                current = current->next;
            }
        }

        cout << endl;
    }

    cout << "\nTotal Entries: " << count;
    cout << "\nCapacity: " << capacity;
    cout << "\nLoad Factor: " << loadFactor;
    cout << "\n=========================================\n";
}

void ResourceHashTable::clear()
{
    for (int i = 0; i < capacity; i++)
    {
        Entry* current = table[i];

        while (current != nullptr)
        {
            Entry* temp = current;
            current = current->next;
            delete temp;
        }

        table[i] = nullptr;
    }

    count = 0;
    loadFactor = 0.0;
}