#ifndef USER_H
#define USER_H

#include <string>

enum class UserRole
{
    ADMIN,
    COORDINATOR,
    RESPONDER
};

class User
{
private:
    int userId;
    std::string username;
    std::string passwordHash;
    std::string fullName;
    std::string email;
    std::string contactNumber;
    UserRole role;
    bool active;

public:
    User();

    User(int id,
         const std::string& username,
         const std::string& passwordHash,
         const std::string& fullName,
         const std::string& email,
         const std::string& contactNumber,
         UserRole role);

    int getUserId() const;
    std::string getUsername() const;
    std::string getPasswordHash() const;
    std::string getFullName() const;
    std::string getEmail() const;
    std::string getContactNumber() const;
    UserRole getRole() const;
    bool isActive() const;

    void setFullName(const std::string& name);
    void setEmail(const std::string& emailAddress);
    void setContactNumber(const std::string& number);
    void setRole(UserRole userRole);
    void setActive(bool status);
};

const char* userRoleToString(UserRole role);
bool stringToUserRole(const std::string& value, UserRole& role);

#endif