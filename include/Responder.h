#ifndef RESPONDER_H
#define RESPONDER_H
#include <string>
using namespace std;
enum class ResponderStatus { AVAILABLE, ASSIGNED, OFF_DUTY };
class Responder
{
private:
    int responderId;
    string responderName;
    string role;
    string contactNumber;
    string specialization;
    double latitude;
    double longitude;
    ResponderStatus status;
    int assignedEmergencyId;
public:
    Responder();
    Responder(int id, string name, string role, string contact, string specialization, double latitude, double longitude);
    int getResponderId() const;
    string getResponderName() const;
    string getRole() const;
    string getContactNumber() const;
    string getSpecialization() const;
    double getLatitude() const;
    double getLongitude() const;
    ResponderStatus getStatus() const;
    int getAssignedEmergencyId() const;
    void setResponderName(string name);
    void setRole(string role);
    void setContactNumber(string contact);
    void setSpecialization(string specialization);
    bool setCoordinates(double latitude, double longitude);
    bool assignEmergency(int emergencyId);
    bool releaseResponder();
    bool setOffDuty();
    bool setAvailable();
    bool restoreState(ResponderStatus restoredStatus, int emergencyId);
    string getStatusText() const;
    void displayResponder() const;
};
#endif
