#include <iostream>
#include "../include/WarehouseLinkedList.h"

using namespace std;

WarehouseLinkedList::WarehouseLinkedList()
    : head(nullptr), tail(nullptr), count(0)
{
}

WarehouseLinkedList::~WarehouseLinkedList()
{
    clear();
}

bool WarehouseLinkedList::isEmpty() const { return head == nullptr; }
bool WarehouseLinkedList::isDuplicate(int id) const { return warehouseIndex.contains(id); }
int WarehouseLinkedList::getCount() const { return count; }

bool WarehouseLinkedList::addWarehouse(const Warehouse& warehouse)
{
    int id = warehouse.getWarehouseId();
    if (id <= 0 || isDuplicate(id) || !warehouse.isActive()) return false;

    Node* node = new Node(warehouse);
    if (!warehouseIndex.insert(id, &node->data)) { delete node; return false; }

    if (!head) head = tail = node;
    else { tail->next = node; tail = node; }
    ++count;
    return true;
}

bool WarehouseLinkedList::deleteWarehouse(int id)
{
    if (id <= 0) return false;
    Node* current = head;
    Node* previous = nullptr;
    while (current)
    {
        if (current->data.getWarehouseId() == id)
        {
            if (!warehouseIndex.remove(id)) return false;
            if (!previous) head = current->next;
            else previous->next = current->next;
            if (current == tail) tail = previous;
            delete current;
            --count;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

Warehouse* WarehouseLinkedList::searchWarehouse(int id) { return warehouseIndex.search(id); }
const Warehouse* WarehouseLinkedList::searchWarehouse(int id) const { return warehouseIndex.search(id); }

bool WarehouseLinkedList::updateWarehouse(int id, string name, string address, string contact)
{
    Warehouse* warehouse = searchWarehouse(id);
    if (!warehouse || name.empty() || address.empty()) return false;
    warehouse->setWarehouseName(name);
    warehouse->setAddress(address);
    warehouse->setContactNumber(contact);
    return true;
}

bool WarehouseLinkedList::updateWarehouse(int id, string name, string address,
                                          double /*legacyLatitude*/, double /*legacyLongitude*/,
                                          string contact)
{
    return updateWarehouse(id, name, address, contact);
}

void WarehouseLinkedList::displayAllWarehouses() const
{
    cout << "\n========== WAREHOUSE LIST ==========";
    if (isEmpty()) { cout << "\nNo warehouses available.\n"; return; }
    Node* current = head;
    while (current) { current->data.displayWarehouse(); current = current->next; }
    cout << "\nTotal Warehouses: " << count << "\n====================================\n";
}

void WarehouseLinkedList::clear()
{
    while (head) deleteWarehouse(head->data.getWarehouseId());
}
