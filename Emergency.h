#ifndef EMERGENCY_H
#define EMERGENCY_H

#include <string>
using namespace std;

class Emergency
{
private:
    int emergencyId;
    string emergencyType;
    string location;
    string description;
    int severity;
    int registrationOrder;

public:
    Emergency();

    void inputEmergency();
    void displayEmergency() const;

    int getEmergencyId() const;
    string getEmergencyType() const;
    string getLocation() const;
    string getDescription() const;
    int getSeverity() const;
    int getRegistrationOrder() const;

    void setEmergencyId(int id);
    void setRegistrationOrder(int order);
};

#endif