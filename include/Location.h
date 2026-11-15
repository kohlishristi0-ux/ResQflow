#ifndef LOCATION_H
#define LOCATION_H
#include <string>
using namespace std;
class Location
{
private:
    int locationId;
    string locationName;
    double latitude;
    double longitude;
    bool active;
public:
    Location();
    Location(int id, string name, double latitude, double longitude);
    int getLocationId() const;
    string getLocationName() const;
    double getLatitude() const;
    double getLongitude() const;
    bool isActive() const;
    void setLocationName(string name);
    bool setCoordinates(double latitude, double longitude);
    void setActive(bool status);
    void displayLocation() const;

    // Great-circle distance in km to another stored location (haversine formula).
    double distanceKmTo(const Location& other) const;
};
#endif