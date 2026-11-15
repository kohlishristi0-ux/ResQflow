#ifndef EMERGENCY_REPOSITORY_H
#define EMERGENCY_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../Emergency.h"
#include "../EmergencyManager.h"

class EmergencyRepository
{
private:
    DatabaseConnection& database;

public:
    explicit EmergencyRepository(DatabaseConnection& db);

    bool saveEmergency(const Emergency& emergency);
    bool loadEmergencies(EmergencyManager& emergencyManager);

    bool updateEmergencyDetails(const Emergency& emergency);
    bool updateEmergencySeverity(const Emergency& emergency);
    bool updateEmergencyAffectedPeople(const Emergency& emergency);

    bool updateEmergencyStatus(
        int emergencyId,
        EmergencyStatus newStatus,
        const char* notes = nullptr,
        int changedBy = 0
    );

    bool emergencyExists(int emergencyId);

    // Prints the full persisted status lifecycle of one emergency (emergency_status_history).
    bool displayStatusHistory(int emergencyId);

    // Highest emergency_id stored in MySQL (0 when the table is empty). Used to avoid ID reuse.
    int getMaxEmergencyId();
};

#endif