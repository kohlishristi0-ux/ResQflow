#include <iostream>
#include "../include/ResourceLinkedList.h"

using namespace std;

ResourceLinkedList::ResourceLinkedList()
{
    head = nullptr;
    tail = nullptr;
    count = 0;
}

void ResourceLinkedList::clear()
{
    resourceIndex.clear();

    Node* current = head;

    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    tail = nullptr;
    count = 0;
}

ResourceLinkedList::~ResourceLinkedList()
{
    resourceIndex.clear();

    Node* current = head;
    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    tail = nullptr;
    count = 0;
}

bool ResourceLinkedList::isEmpty() const
{
    return head == nullptr;
}

bool ResourceLinkedList::isDuplicate(int resourceId) const
{
    return resourceIndex.contains(resourceId);
}

int ResourceLinkedList::getCount() const
{
    return count;
}

bool ResourceLinkedList::addResource(const Resource& resource)
{
    int resourceId = resource.getResourceId();

    if (resourceId <= 0 || isDuplicate(resourceId) || !resource.isActive())
        return false;

    Node* newNode = new Node(resource);

    if (!resourceIndex.insert(resourceId, &newNode->data))
    {
        delete newNode;
        return false;
    }

    if (head == nullptr)
    {
        head = newNode;
        tail = newNode;
    }
    else
    {
        tail->next = newNode;
        tail = newNode;
    }

    count++;
    return true;
}

bool ResourceLinkedList::deleteResource(int resourceId)
{
    if (head == nullptr)
        return false;

    Node* current = head;
    Node* previous = nullptr;

    while (current != nullptr && current->data.getResourceId() != resourceId)
    {
        previous = current;
        current = current->next;
    }

    if (current == nullptr)
        return false;

    if (!resourceIndex.remove(resourceId))
        return false;

    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;

    if (current == tail)
        tail = previous;

    delete current;
    count--;

    return true;
}

Resource* ResourceLinkedList::searchResource(int resourceId)
{
    return resourceIndex.search(resourceId);
}

const Resource* ResourceLinkedList::searchResource(int resourceId) const
{
    return resourceIndex.search(resourceId);
}

bool ResourceLinkedList::updateResource(int resourceId, string name, string category, string description, string unit, int minimumStockLevel)
{
    Resource* resource = searchResource(resourceId);

    if (resource == nullptr || minimumStockLevel < 0)
        return false;

    resource->setResourceName(name);
    resource->setCategory(category);
    resource->setDescription(description);
    resource->setUnit(unit);
    resource->setMinimumStockLevel(minimumStockLevel);

    return true;
}

void ResourceLinkedList::displayAllResources() const
{
    if (isEmpty())
    {
        cout << "\nNo resources available.\n";
        return;
    }

    Node* current = head;

    while (current != nullptr)
    {
        cout << "\n-----------------------------";
        cout << "\nResource ID: " << current->data.getResourceId();
        cout << "\nName: " << current->data.getResourceName();
        cout << "\nCategory: " << current->data.getCategory();
        cout << "\nDescription: " << current->data.getDescription();
        cout << "\nUnit: " << current->data.getUnit();
        cout << "\nMinimum Stock Level: " << current->data.getMinimumStockLevel();
        cout << "\nStatus: " << (current->data.isActive() ? "Active" : "Inactive");
        cout << "\n-----------------------------\n";

        current = current->next;
    }
}

namespace
{
    string toLowerCopy(const string& text)
    {
        string lowered = text;
        for (size_t i = 0; i < lowered.size(); ++i)
        {
            if (lowered[i] >= 'A' && lowered[i] <= 'Z')
                lowered[i] = static_cast<char>(lowered[i] - 'A' + 'a');
        }
        return lowered;
    }
}

int ResourceLinkedList::displayMatchingName(const string& fragment) const
{
    if (fragment.empty())
        return 0;

    string needle = toLowerCopy(fragment);
    int matches = 0;

    for (Node* current = head; current != nullptr; current = current->next)
    {
        if (toLowerCopy(current->data.getResourceName()).find(needle) == string::npos)
            continue;

        cout << "ID: " << current->data.getResourceId()
             << " | " << current->data.getResourceName()
             << " | " << current->data.getCategory()
             << " | Unit: " << current->data.getUnit()
             << " | Min stock: " << current->data.getMinimumStockLevel() << "\n";
        ++matches;
    }

    return matches;
}
