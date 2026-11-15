#include "../include/Responder.h"
#include <iostream>
#include <cmath>
using namespace std;
Responder::Responder() : responderId(0), responderName(""), role(""), contactNumber(""), specialization(""), latitude(0.0), longitude(0.0), status(ResponderStatus::AVAILABLE), assignedEmergencyId(-1) {}
Responder::Responder(int id, string name, string responderRole, string contact, string responderSpecialization, double lat, double lon)
    : responderId(id), responderName(name), role(responderRole), contactNumber(contact), specialization(responderSpecialization), latitude(0.0), longitude(0.0), status(ResponderStatus::AVAILABLE), assignedEmergencyId(-1)
{
    if (!setCoordinates(lat, lon)) { latitude = lat; longitude = lon; }
}
int Responder::getResponderId() const { return responderId; }
string Responder::getResponderName() const { return responderName; }
string Responder::getRole() const { return role; }
string Responder::getContactNumber() const { return contactNumber; }
string Responder::getSpecialization() const { return specialization; }
double Responder::getLatitude() const { return latitude; }
double Responder::getLongitude() const { return longitude; }
ResponderStatus Responder::getStatus() const { return status; }
int Responder::getAssignedEmergencyId() const { return assignedEmergencyId; }
void Responder::setResponderName(string name) { if (!name.empty()) responderName = name; }
void Responder::setRole(string value) { if (!value.empty()) role = value; }
void Responder::setContactNumber(string value) { if (!value.empty()) contactNumber = value; }
void Responder::setSpecialization(string value) { if (!value.empty()) specialization = value; }
bool Responder::setCoordinates(double lat, double lon)
{
    if (!isfinite(lat) || !isfinite(lon) || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) return false;
    latitude = lat; longitude = lon; return true;
}
bool Responder::assignEmergency(int id)
{
    if (id <= 0 || status != ResponderStatus::AVAILABLE) return false;
    assignedEmergencyId = id; status = ResponderStatus::ASSIGNED; return true;
}
bool Responder::releaseResponder()
{
    if (status != ResponderStatus::ASSIGNED) return false;
    assignedEmergencyId = -1; status = ResponderStatus::AVAILABLE; return true;
}
bool Responder::setOffDuty()
{
    if (status == ResponderStatus::ASSIGNED) return false;
    status = ResponderStatus::OFF_DUTY; return true;
}
bool Responder::setAvailable()
{
    if (status == ResponderStatus::ASSIGNED) return false;
    status = ResponderStatus::AVAILABLE; return true;
}
string Responder::getStatusText() const
{
    switch (status) { case ResponderStatus::AVAILABLE: return "Available"; case ResponderStatus::ASSIGNED: return "Assigned"; case ResponderStatus::OFF_DUTY: return "Off Duty"; default: return "Unknown"; }
}
void Responder::displayResponder() const
{
    cout << "\nResponder ID: " << responderId << "\nName: " << responderName << "\nRole: " << role << "\nContact: " << contactNumber << "\nSpecialization: " << specialization << "\nLatitude: " << latitude << "\nLongitude: " << longitude << "\nStatus: " << getStatusText();
    if (status == ResponderStatus::ASSIGNED) cout << "\nAssigned Emergency ID: " << assignedEmergencyId;
    cout << "\n";
}

bool Responder::restoreState(ResponderStatus restoredStatus, int emergencyId)
{
    if (restoredStatus == ResponderStatus::ASSIGNED)
    {
        if (emergencyId <= 0)
            return false;
    }
    else
    {
        if (emergencyId != -1)
            return false;
    }

    if (restoredStatus != ResponderStatus::AVAILABLE &&
        restoredStatus != ResponderStatus::ASSIGNED &&
        restoredStatus != ResponderStatus::OFF_DUTY)
        return false;

    status = restoredStatus;
    assignedEmergencyId = emergencyId;
    return true;
}