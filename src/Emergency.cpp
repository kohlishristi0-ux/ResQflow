#include <iostream>
#include <cmath>
#include <ctime>
#include "../include/Emergency.h"

using namespace std;

Emergency::Emergency()
{
    emergencyId = 0;
    emergencyType = "";
    location = "";
    description = "";
    severity = 1;
    affectedPeople = 1;
    latitude = 0.0;
    longitude = 0.0;
    status = EmergencyStatus::REPORTED;
    registrationOrder = 0;
    registrationTime = time(nullptr);
}

Emergency::Emergency(int id, string type, string location, string description,
                     int severity, double latitude, double longitude,
                     long long registrationOrder, int affectedPeople,
                     time_t registrationTime)
{
    emergencyId = id;
    emergencyType = type;
    this->location = location;
    this->description = description;
    this->severity = (severity >= 1 && severity <= 5) ? severity : 1;
    this->affectedPeople = (affectedPeople > 0) ? affectedPeople : 1;

    if (isfinite(latitude) && isfinite(longitude) &&
        latitude >= -90 && latitude <= 90 &&
        longitude >= -180 && longitude <= 180)
    {
        this->latitude = latitude;
        this->longitude = longitude;
    }
    else
    {
        this->latitude = 0.0;
        this->longitude = 0.0;
    }

    status = EmergencyStatus::REPORTED;
    this->registrationOrder = registrationOrder;
    this->registrationTime = (registrationTime > 0) ? registrationTime : time(nullptr);
}

int Emergency::getEmergencyId() const
{
    return emergencyId;
}

string Emergency::getEmergencyType() const
{
    return emergencyType;
}

string Emergency::getLocation() const
{
    return location;
}

string Emergency::getDescription() const
{
    return description;
}

int Emergency::getSeverity() const
{
    return severity;
}

int Emergency::getAffectedPeople() const
{
    return affectedPeople;
}

double Emergency::getLatitude() const
{
    return latitude;
}

double Emergency::getLongitude() const
{
    return longitude;
}

EmergencyStatus Emergency::getStatus() const
{
    return status;
}

long long Emergency::getRegistrationOrder() const
{
    return registrationOrder;
}

time_t Emergency::getRegistrationTime() const
{
    return registrationTime;
}

void Emergency::setEmergencyType(string type)
{
    emergencyType = type;
}

void Emergency::setLocation(string location)
{
    this->location = location;
}

void Emergency::setDescription(string description)
{
    this->description = description;
}

bool Emergency::setSeverity(int severity)
{
    if (severity < 1 || severity > 5)
        return false;

    this->severity = severity;
    return true;
}

bool Emergency::setAffectedPeople(int affectedPeople)
{
    if (affectedPeople <= 0)
        return false;

    this->affectedPeople = affectedPeople;
    return true;
}

bool Emergency::setCoordinates(double latitude, double longitude)
{
    if (!isfinite(latitude) || !isfinite(longitude) ||
        latitude < -90 || latitude > 90 ||
        longitude < -180 || longitude > 180)
        return false;

    this->latitude = latitude;
    this->longitude = longitude;

    return true;
}

void Emergency::setStatus(EmergencyStatus status)
{
    this->status = status;
}

double Emergency::calculatePopulationScore() const
{
    double score = affectedPeople / 100.0;

    if (score > 10.0)
        score = 10.0;

    return score;
}

double Emergency::calculateWaitingTimeBonus() const
{
    time_t currentTime = time(nullptr);

    if (registrationTime <= 0 || currentTime <= registrationTime)
        return 0.0;

    double waitingSeconds = difftime(currentTime, registrationTime);
    double bonus = waitingSeconds / 1800.0;

    if (bonus > 10.0)
        bonus = 10.0;

    return bonus;
}

double Emergency::calculatePriorityScore() const
{
    double severityScore = severity * 30.0;
    double populationScore = calculatePopulationScore();
    double waitingBonus = calculateWaitingTimeBonus();

    return severityScore + populationScore + waitingBonus;
}

string Emergency::getStatusText() const
{
    switch (status)
    {
        case EmergencyStatus::REPORTED:
            return "Reported";
        case EmergencyStatus::VERIFIED:
            return "Verified";
        case EmergencyStatus::IN_PROGRESS:
            return "In Progress";
        case EmergencyStatus::RESOLVED:
            return "Resolved";
        case EmergencyStatus::CANCELLED:
            return "Cancelled";
        default:
            return "Unknown";
    }
}

void Emergency::displayEmergency() const
{
    cout << "\n--------------------------------";
    cout << "\nEmergency ID: " << emergencyId;
    cout << "\nType: " << emergencyType;
    cout << "\nLocation: " << location;
    cout << "\nDescription: " << description;
    cout << "\nSeverity: " << severity << "/5";
    cout << "\nAffected People: " << affectedPeople;
    cout << "\nLatitude: " << latitude;
    cout << "\nLongitude: " << longitude;
    cout << "\nStatus: " << getStatusText();
    cout << "\nRegistration Order: " << registrationOrder;
    cout << "\nRegistration Time: ";

    if (registrationTime > 0)
        cout << ctime(&registrationTime);
    else
        cout << "Unavailable\n";

    cout << "Population Score: " << calculatePopulationScore();
    cout << "\nWaiting Time Bonus: " << calculateWaitingTimeBonus();
    cout << "\nTotal Priority Score: " << calculatePriorityScore();
    cout << "\n--------------------------------\n";
}