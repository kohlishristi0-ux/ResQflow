#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H
#include "Database/UserRepository.h"
#include "User.h"
#include <string>
class Authentication
{
public:
    static bool authenticate(UserRepository& repository, User& loggedInUser);

    // Masked password prompt (no echo). Use for every password the application asks for.
    static std::string promptPassword();
};
#endif