#ifndef EMERGENCY_H
#define EMERGENCY_H

#include <string>
#include <ctime>
using namespace std;

enum class EmergencyStatus
{
    REPORTED,
    VERIFIED,
    IN_PROGRESS,
    RESOLVED,
    CANCELLED
};

class Emergency
{
private:
    int emergencyId;
    string emergencyType;
    string location;
    string description;
    int severity;
    int affectedPeople;
    double latitude;
    double longitude;
    EmergencyStatus status;
    long long registrationOrder;
    time_t registrationTime;

public:
    Emergency();

    Emergency(int id, string type, string location, string description,
              int severity, double latitude, double longitude,
              long long registrationOrder, int affectedPeople = 1,
              time_t registrationTime = 0);

    int getEmergencyId() const;
    string getEmergencyType() const;
    string getLocation() const;
    string getDescription() const;
    int getSeverity() const;
    int getAffectedPeople() const;
    double getLatitude() const;
    double getLongitude() const;
    EmergencyStatus getStatus() const;
    long long getRegistrationOrder() const;
    time_t getRegistrationTime() const;

    void setEmergencyType(string type);
    void setLocation(string location);
    void setDescription(string description);
    bool setSeverity(int severity);
    bool setAffectedPeople(int affectedPeople);
    bool setCoordinates(double latitude, double longitude);
    void setStatus(EmergencyStatus status);

    double calculatePopulationScore() const;
    double calculateWaitingTimeBonus() const;
    double calculatePriorityScore() const;

    string getStatusText() const;
    void displayEmergency() const;
};

#endif