#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include <string>

class Warehouse
{
private:
    int warehouseId;
    std::string warehouseName;
    std::string address;
    std::string contactNumber;
    bool active;

public:
    Warehouse();
    Warehouse(int id, std::string name, std::string address, std::string contactNumber);

    // Backward-compatible constructor for older tests/data loaders.
    // Coordinates are intentionally ignored; warehouse coordinates are not
    // part of the ResQflow warehouse profile.
    Warehouse(int id, std::string name, std::string address,
              double legacyLatitude, double legacyLongitude,
              std::string contactNumber);

    int getWarehouseId() const;
    std::string getWarehouseName() const;
    std::string getAddress() const;
    std::string getContactNumber() const;
    bool isActive() const;

    void setWarehouseName(std::string name);
    void setAddress(std::string address);
    void setContactNumber(std::string contactNumber);
    void setStatus(bool status);

    void displayWarehouse() const;
};

#endif
