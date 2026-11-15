#include "../include/ResponderCircularList.h"
#include <new>

ResponderCircularList::ResponderCircularList() : tail(nullptr), cursor(nullptr), count(0) {}
ResponderCircularList::~ResponderCircularList()
{
    if (!tail) return;
    Node* current = tail->next;
    tail->next = nullptr;
    while (current)
    {
        Node* next = current->next;
        delete current;
        current = next;
    }
}
bool ResponderCircularList::contains(int id) const
{
    if (!tail) return false;
    Node* current = tail->next;
    do
    {
        if (current->responderId == id) return true;
        current = current->next;
    } while (current != tail->next);
    return false;
}
bool ResponderCircularList::addResponderId(int id)
{
    if (id <= 0 || contains(id)) return false;
    Node* node = new (std::nothrow) Node(id);
    if (!node) return false;
    if (!tail)
    {
        node->next = node;
        tail = node;
        cursor = node;
    }
    else
    {
        node->next = tail->next;
        tail->next = node;
        tail = node;
    }
    ++count;
    return true;
}
bool ResponderCircularList::removeResponderId(int id)
{
    if (!tail) return false;
    Node* previous = tail;
    Node* current = tail->next;
    do
    {
        if (current->responderId == id)
        {
            if (current == previous) tail = nullptr;
            else
            {
                previous->next = current->next;
                if (current == tail) tail = previous;
            }
            if (cursor == current) cursor = tail ? current->next : nullptr;
            delete current;
            --count;
            if (tail && !cursor) cursor = tail->next;
            return true;
        }
        previous = current;
        current = current->next;
    } while (current != tail->next);
    return false;
}
bool ResponderCircularList::getNextResponderId(int& id)
{
    if (!cursor) return false;
    id = cursor->responderId;
    cursor = cursor->next;
    return true;
}
int ResponderCircularList::getCount() const { return count; }
bool ResponderCircularList::isEmpty() const { return tail == nullptr; }
