#include <iostream>
#include "../include/Warehouse.h"

using namespace std;

Warehouse::Warehouse()
    : warehouseId(0), warehouseName(""), address(""),
      contactNumber(""), active(true)
{
}

Warehouse::Warehouse(int id, string name, string warehouseAddress, string contact)
    : warehouseId(id), warehouseName(name), address(warehouseAddress),
      contactNumber(contact), active(true)
{
}

Warehouse::Warehouse(int id, string name, string warehouseAddress,
                     double /*legacyLatitude*/, double /*legacyLongitude*/,
                     string contact)
    : Warehouse(id, name, warehouseAddress, contact)
{
}

int Warehouse::getWarehouseId() const { return warehouseId; }
string Warehouse::getWarehouseName() const { return warehouseName; }
string Warehouse::getAddress() const { return address; }
string Warehouse::getContactNumber() const { return contactNumber; }
bool Warehouse::isActive() const { return active; }

void Warehouse::setWarehouseName(string name) { warehouseName = name; }
void Warehouse::setAddress(string warehouseAddress) { address = warehouseAddress; }
void Warehouse::setContactNumber(string contact) { contactNumber = contact; }
void Warehouse::setStatus(bool status) { active = status; }

void Warehouse::displayWarehouse() const
{
    cout << "\n-----------------------------";
    cout << "\nWarehouse ID: " << warehouseId;
    cout << "\nWarehouse Name: " << warehouseName;
    cout << "\nFull Address: " << address;
    cout << "\nContact Number: " << contactNumber;
    cout << "\nStatus: " << (active ? "Active" : "Inactive");
    cout << "\n-----------------------------\n";
}
