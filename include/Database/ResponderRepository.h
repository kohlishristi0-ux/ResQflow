#ifndef RESPONDER_REPOSITORY_H
#define RESPONDER_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../Responder.h"
#include "../ResponderManager.h"

class ResponderRepository
{
private:
    DatabaseConnection& database;

public:
    explicit ResponderRepository(DatabaseConnection& db);

    bool saveResponder(const Responder& responder);

    bool loadActiveResponders(ResponderManager& manager);

    bool updateResponder(const Responder& responder);

    bool deactivateResponder(int responderId);

    bool responderExists(int responderId);

    bool linkUserToResponder(int userId, int responderId);

    bool submitResponderRegistration(
    const std::string& username,
    const std::string& passwordHash,
    const std::string& fullName,
    const std::string& email,
    const std::string& contact,
    const std::string& responderRole,
    const std::string& specialization,
    const std::string& city,
    const std::string& area,
    const std::string& baseAddress
);
    void displayPendingResponderRegistrations();
    bool approveResponderRegistration(int registrationId);
    bool rejectResponderRegistration(int registrationId);
    int findResponderIdForUser(int userId);
    bool activateResponderForUser(int userId);
    bool updateResponderStatus(int responderId, ResponderStatus status);

    bool assignResponder(
        int responderId,
        int emergencyId,
        const char* notes = nullptr
    );

    bool releaseResponder(int responderId);
};

#endif