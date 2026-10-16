#include <iostream>
#include "ResourceLinkedList.h"

using namespace std;

ResourceLinkedList::ResourceLinkedList()
{
    head = nullptr;
}

ResourceLinkedList::~ResourceLinkedList()
{
    ResourceNode* current = head;

    while (current != nullptr)
    {
        ResourceNode* temp = current;
        current = current->next;
        delete temp;
    }
}

bool ResourceLinkedList::insertResource()
{
    Resource resource;
    resource.inputResource();

    if (resource.getResourceId() <= 0 ||
        resource.getQuantity() < 0)
    {
        cout << "Invalid resource ID or quantity.\n";
        return false;
    }

    if (searchResource(resource.getResourceId()) != nullptr)
    {
        cout << "Resource ID already exists.\n";
        return false;
    }

    ResourceNode* temp = new ResourceNode;
    temp->data = resource;
    temp->next = nullptr;

    if (head == nullptr)
    {
        head = temp;
    }
    else
    {
        ResourceNode* ptr = head;

        while (ptr->next != nullptr)
        {
            ptr = ptr->next;
        }

        ptr->next = temp;
    }

    cout << "Resource added successfully!\n";
    return true;
}

void ResourceLinkedList::displayResources() const
{
    if (head == nullptr)
    {
        cout << "No resources available.\n";
        return;
    }

    ResourceNode* ptr = head;

    while (ptr != nullptr)
    {
        ptr->data.displayResource();
        cout << "----------------------\n";

        ptr = ptr->next;
    }
}

ResourceNode* ResourceLinkedList::searchResource(int id) const
{
    ResourceNode* ptr = head;

    while (ptr != nullptr)
    {
        if (ptr->data.getResourceId() == id)
        {
            return ptr;
        }

        ptr = ptr->next;
    }

    return nullptr;
}

bool ResourceLinkedList::updateQuantity(int id, int newQuantity)
{
    if (newQuantity < 0)
    {
        cout << "Quantity cannot be negative.\n";
        return false;
    }

    ResourceNode* ptr = searchResource(id);

    if (ptr == nullptr)
    {
        cout << "Resource not found.\n";
        return false;
    }

    ptr->data.setQuantity(newQuantity);

    cout << "Resource quantity updated successfully!\n";
    return true;
}


bool ResourceLinkedList::getResourceDetails(
    int id,
    string& resourceName,
    int& availableQuantity
)
{
    ResourceNode* ptr = head;

    while (ptr != nullptr)
    {
        if (ptr->data.getResourceId() == id)
        {
            resourceName = ptr->data.getResourceName();
            availableQuantity = ptr->data.getQuantity();

            return true;
        }

        ptr = ptr->next;
    }

    return false;
}


bool ResourceLinkedList::reduceResourceQuantity(
    int id,
    int quantity
)
{
    if (quantity <= 0)
        return false;

    ResourceNode* ptr = head;

    while (ptr != nullptr)
    {
        if (ptr->data.getResourceId() == id)
        {
            int available = ptr->data.getQuantity();

            if (quantity > available)
                return false;

            ptr->data.setQuantity(available - quantity);

            return true;
        }

        ptr = ptr->next;
    }

    return false;
}