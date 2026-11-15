#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../User.h"
#include <string>

class UserRepository
{
private:
    DatabaseConnection& database;

public:
    explicit UserRepository(DatabaseConnection& db);

    bool saveUser(const User& user);

    bool loadUserByUsername(
        const std::string& username,
        User& user);

    bool updateUserProfile(const User& user);

    bool updatePasswordHash(
        int userId,
        const std::string& passwordHash);

    bool setUserActive(int userId, bool active);

    bool userExists(const std::string& username);

    bool hasAnyUsers();
    void displayAllUsers();
    bool deactivateUserSafely(int userId, int currentUserId);
    bool reactivateUserSafely(int userId);
};

#endif