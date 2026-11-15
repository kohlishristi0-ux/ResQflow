#include <iostream>
#include <new>
#include <limits>
#include "../include/WarehouseHashTable.h"

using namespace std;

WarehouseHashTable::WarehouseHashTable(int initialCapacity)
{
    if (initialCapacity < 1)
        initialCapacity = 10;

    capacity = initialCapacity;
    count = 0;
    loadFactor = 0.0;
    table = new Entry*[capacity]();
}

WarehouseHashTable::~WarehouseHashTable()
{
    clear();
    delete[] table;
}

int WarehouseHashTable::hashFunction(int warehouseId) const
{
    return warehouseId % capacity;
}

bool WarehouseHashTable::resizeTable()
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
            int newIndex = current->warehouseId % newCapacity;

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

bool WarehouseHashTable::insert(int warehouseId, Warehouse* warehouse)
{
    if (warehouseId <= 0 || warehouse == nullptr)
        return false;

    if (warehouse->getWarehouseId() != warehouseId)
        return false;

    if (search(warehouseId) != nullptr)
        return false;

    if (static_cast<double>(count + 1) / capacity > 0.75)
    {
        if (!resizeTable())
            return false;
    }

    int index = hashFunction(warehouseId);

    Entry* newEntry = new (nothrow) Entry(warehouseId, warehouse);

    if (newEntry == nullptr)
        return false;

    newEntry->next = table[index];
    table[index] = newEntry;

    count++;
    loadFactor = static_cast<double>(count) / capacity;

    return true;
}

Warehouse* WarehouseHashTable::search(int warehouseId) const
{
    if (warehouseId <= 0)
        return nullptr;

    int index = hashFunction(warehouseId);
    Entry* current = table[index];

    while (current != nullptr)
    {
        if (current->warehouseId == warehouseId)
            return current->warehouse;

        current = current->next;
    }

    return nullptr;
}

bool WarehouseHashTable::remove(int warehouseId)
{
    if (warehouseId <= 0)
        return false;

    int index = hashFunction(warehouseId);
    Entry* current = table[index];
    Entry* previous = nullptr;

    while (current != nullptr)
    {
        if (current->warehouseId == warehouseId)
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

bool WarehouseHashTable::contains(int warehouseId) const
{
    return search(warehouseId) != nullptr;
}

int WarehouseHashTable::getCount() const
{
    return count;
}

int WarehouseHashTable::getCapacity() const
{
    return capacity;
}

double WarehouseHashTable::getLoadFactor() const
{
    return loadFactor;
}

void WarehouseHashTable::displayTable() const
{
    cout << "\n========== WAREHOUSE HASH TABLE ==========\n";

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
                cout << "[" << current->warehouseId << "]";

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
    cout << "\n==========================================\n";
}

void WarehouseHashTable::clear()
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