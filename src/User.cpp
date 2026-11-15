#include "../include/User.h"

User::User()
    : userId(0),
      username(""),
      passwordHash(""),
      fullName(""),
      email(""),
      contactNumber(""),
      role(UserRole::COORDINATOR),
      active(true)
{
}

User::User(int id,
           const std::string& user,
           const std::string& hash,
           const std::string& name,
           const std::string& emailAddress,
           const std::string& number,
           UserRole userRole)
    : userId(id),
      username(user),
      passwordHash(hash),
      fullName(name),
      email(emailAddress),
      contactNumber(number),
      role(userRole),
      active(true)
{
}

int User::getUserId() const { return userId; }
std::string User::getUsername() const { return username; }
std::string User::getPasswordHash() const { return passwordHash; }
std::string User::getFullName() const { return fullName; }
std::string User::getEmail() const { return email; }
std::string User::getContactNumber() const { return contactNumber; }
UserRole User::getRole() const { return role; }
bool User::isActive() const { return active; }

void User::setFullName(const std::string& name)
{
    if (!name.empty())
        fullName = name;
}

void User::setEmail(const std::string& emailAddress)
{
    email = emailAddress;
}

void User::setContactNumber(const std::string& number)
{
    contactNumber = number;
}

void User::setRole(UserRole userRole)
{
    role = userRole;
}

void User::setActive(bool status)
{
    active = status;
}

const char* userRoleToString(UserRole role)
{
    switch (role)
    {
        case UserRole::ADMIN:
            return "ADMIN";

        case UserRole::COORDINATOR:
            return "COORDINATOR";

        case UserRole::RESPONDER:
            return "RESPONDER";

        default:
            return "COORDINATOR";
    }
}

bool stringToUserRole(const std::string& value, UserRole& role)
{
    if (value == "ADMIN")
        role = UserRole::ADMIN;
    else if (value == "COORDINATOR")
        role = UserRole::COORDINATOR;
    else if (value == "RESPONDER")
        role = UserRole::RESPONDER;
    else
        return false;

    return true;
}