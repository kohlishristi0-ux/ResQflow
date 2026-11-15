#include <cmath>
#include "../include/Location.h"
#include <iostream>
#include <cmath>
using namespace std;
Location::Location() : locationId(0), locationName(""), latitude(0.0), longitude(0.0), active(true) {}
Location::Location(int id, string name, double lat, double lon)
    : locationId(id), locationName(name), latitude(0.0), longitude(0.0), active(true)
{
    setCoordinates(lat, lon);
}
int Location::getLocationId() const { return locationId; }
string Location::getLocationName() const { return locationName; }
double Location::getLatitude() const { return latitude; }
double Location::getLongitude() const { return longitude; }
bool Location::isActive() const { return active; }
void Location::setLocationName(string name) { if (!name.empty()) locationName = name; }
bool Location::setCoordinates(double lat, double lon)
{
    if (!isfinite(lat) || !isfinite(lon) || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) return false;
    latitude = lat; longitude = lon; return true;
}
void Location::setActive(bool status) { active = status; }
void Location::displayLocation() const
{
    cout << "\nLocation ID: " << locationId << "\nName: " << locationName
         << "\nLatitude: " << latitude << "\nLongitude: " << longitude
         << "\nStatus: " << (active ? "Active" : "Inactive") << "\n";
}

double Location::distanceKmTo(const Location& other) const
{
    const double earthRadiusKm = 6371.0;
    const double toRadians = 3.14159265358979323846 / 180.0;

    double dLat = (other.latitude - latitude) * toRadians;
    double dLon = (other.longitude - longitude) * toRadians;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(latitude * toRadians) * std::cos(other.latitude * toRadians) *
               std::sin(dLon / 2) * std::sin(dLon / 2);

    return earthRadiusKm * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}
