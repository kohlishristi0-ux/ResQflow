#include "../../include/Database/UserRepository.h"
#include <mysql.h>
#include <cstring>
#include <iostream>
#include <cstdlib>

UserRepository::UserRepository(DatabaseConnection& db)
    : database(db)
{
}

bool UserRepository::saveUser(const User& user)
{
    MYSQL* connection = database.getConnection();
    if (!connection)
        return false;

    std::string username = user.getUsername();
    std::string passwordHash = user.getPasswordHash();
    std::string fullName = user.getFullName();
    std::string email = user.getEmail();
    std::string contact = user.getContactNumber();
    std::string role = userRoleToString(user.getRole());

    if (username.empty() || passwordHash.empty() ||
        fullName.empty() || role.empty())
        return false;

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query =
        "INSERT INTO users "
        "(username, password_hash, full_name, email, contact_number, role, is_active) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)";

    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    MYSQL_BIND parameters[7] = {};
    unsigned long lengths[6] = {
        static_cast<unsigned long>(username.length()),
        static_cast<unsigned long>(passwordHash.length()),
        static_cast<unsigned long>(fullName.length()),
        static_cast<unsigned long>(email.length()),
        static_cast<unsigned long>(contact.length()),
        static_cast<unsigned long>(role.length())
    };

    DbBool emailNull = email.empty();
    DbBool contactNull = contact.empty();
    unsigned char active = user.isActive() ? 1 : 0;

    parameters[0].buffer_type = MYSQL_TYPE_STRING;
    parameters[0].buffer = &username[0];
    parameters[0].buffer_length = lengths[0];
    parameters[0].length = &lengths[0];

    parameters[1].buffer_type = MYSQL_TYPE_STRING;
    parameters[1].buffer = &passwordHash[0];
    parameters[1].buffer_length = lengths[1];
    parameters[1].length = &lengths[1];

    parameters[2].buffer_type = MYSQL_TYPE_STRING;
    parameters[2].buffer = &fullName[0];
    parameters[2].buffer_length = lengths[2];
    parameters[2].length = &lengths[2];

    parameters[3].buffer_type = MYSQL_TYPE_STRING;
    parameters[3].buffer = email.empty() ? nullptr : &email[0];
    parameters[3].buffer_length = lengths[3];
    parameters[3].length = &lengths[3];
    parameters[3].is_null = &emailNull;

    parameters[4].buffer_type = MYSQL_TYPE_STRING;
    parameters[4].buffer = contact.empty() ? nullptr : &contact[0];
    parameters[4].buffer_length = lengths[4];
    parameters[4].length = &lengths[4];
    parameters[4].is_null = &contactNull;

    parameters[5].buffer_type = MYSQL_TYPE_STRING;
    parameters[5].buffer = &role[0];
    parameters[5].buffer_length = lengths[5];
    parameters[5].length = &lengths[5];

    parameters[6].buffer_type = MYSQL_TYPE_TINY;
    parameters[6].buffer = &active;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);
    return success;
}

bool UserRepository::loadUserByUsername(
    const std::string& username,
    User& user)
{
    MYSQL* connection = database.getConnection();
    if (!connection || username.empty())
        return false;

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query =
        "SELECT user_id, username, password_hash, full_name, "
        "email, contact_number, role, is_active "
        "FROM users WHERE username = ?";

    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    MYSQL_BIND input[1] = {};
    unsigned long usernameLength =
        static_cast<unsigned long>(username.length());

    input[0].buffer_type = MYSQL_TYPE_STRING;
    input[0].buffer = const_cast<char*>(username.c_str());
    input[0].buffer_length = usernameLength;
    input[0].length = &usernameLength;

    if (mysql_stmt_bind_param(statement, input) != 0 ||
        mysql_stmt_execute(statement) != 0 ||
        mysql_stmt_store_result(statement) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    int id = 0;
    char usernameBuffer[61] = {};
    char hashBuffer[256] = {};
    char nameBuffer[121] = {};
    char emailBuffer[151] = {};
    char contactBuffer[26] = {};
    char roleBuffer[20] = {};
    unsigned char active = 0;

    unsigned long lengths[7] = {};
    DbBool emailNull = 0;
    DbBool contactNull = 0;

    MYSQL_BIND results[8] = {};

    results[0].buffer_type = MYSQL_TYPE_LONG;
    results[0].buffer = &id;

    results[1].buffer_type = MYSQL_TYPE_STRING;
    results[1].buffer = usernameBuffer;
    results[1].buffer_length = sizeof(usernameBuffer);
    results[1].length = &lengths[0];

    results[2].buffer_type = MYSQL_TYPE_STRING;
    results[2].buffer = hashBuffer;
    results[2].buffer_length = sizeof(hashBuffer);
    results[2].length = &lengths[1];

    results[3].buffer_type = MYSQL_TYPE_STRING;
    results[3].buffer = nameBuffer;
    results[3].buffer_length = sizeof(nameBuffer);
    results[3].length = &lengths[2];

    results[4].buffer_type = MYSQL_TYPE_STRING;
    results[4].buffer = emailBuffer;
    results[4].buffer_length = sizeof(emailBuffer);
    results[4].length = &lengths[3];
    results[4].is_null = &emailNull;

    results[5].buffer_type = MYSQL_TYPE_STRING;
    results[5].buffer = contactBuffer;
    results[5].buffer_length = sizeof(contactBuffer);
    results[5].length = &lengths[4];
    results[5].is_null = &contactNull;

    results[6].buffer_type = MYSQL_TYPE_STRING;
    results[6].buffer = roleBuffer;
    results[6].buffer_length = sizeof(roleBuffer);
    results[6].length = &lengths[5];

    results[7].buffer_type = MYSQL_TYPE_TINY;
    results[7].buffer = &active;

    if (mysql_stmt_bind_result(statement, results) != 0 ||
        mysql_stmt_fetch(statement) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    UserRole role;
    std::string roleValue(roleBuffer, lengths[5]);

    if (!stringToUserRole(roleValue, role))
    {
        mysql_stmt_close(statement);
        return false;
    }

    User loadedUser(
        id,
        std::string(usernameBuffer, lengths[0]),
        std::string(hashBuffer, lengths[1]),
        std::string(nameBuffer, lengths[2]),
        emailNull ? "" : std::string(emailBuffer, lengths[3]),
        contactNull ? "" : std::string(contactBuffer, lengths[4]),
        role
    );

    loadedUser.setActive(active != 0);
    user = loadedUser;

    mysql_stmt_close(statement);
    return true;
}

bool UserRepository::updateUserProfile(const User& user)
{
    MYSQL* connection = database.getConnection();
    if (!connection || user.getUserId() <= 0 ||
        user.getFullName().empty())
        return false;

    std::string name = user.getFullName();
    std::string email = user.getEmail();
    std::string contact = user.getContactNumber();
    std::string role = userRoleToString(user.getRole());

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query =
        "UPDATE users SET full_name = ?, email = ?, "
        "contact_number = ?, role = ? WHERE user_id = ?";

    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    MYSQL_BIND parameters[5] = {};
    unsigned long lengths[4] = {
        static_cast<unsigned long>(name.length()),
        static_cast<unsigned long>(email.length()),
        static_cast<unsigned long>(contact.length()),
        static_cast<unsigned long>(role.length())
    };

    DbBool emailNull = email.empty();
    DbBool contactNull = contact.empty();
    int id = user.getUserId();

    parameters[0].buffer_type = MYSQL_TYPE_STRING;
    parameters[0].buffer = &name[0];
    parameters[0].buffer_length = lengths[0];
    parameters[0].length = &lengths[0];

    parameters[1].buffer_type = MYSQL_TYPE_STRING;
    parameters[1].buffer = email.empty() ? nullptr : &email[0];
    parameters[1].buffer_length = lengths[1];
    parameters[1].length = &lengths[1];
    parameters[1].is_null = &emailNull;

    parameters[2].buffer_type = MYSQL_TYPE_STRING;
    parameters[2].buffer = contact.empty() ? nullptr : &contact[0];
    parameters[2].buffer_length = lengths[2];
    parameters[2].length = &lengths[2];
    parameters[2].is_null = &contactNull;

    parameters[3].buffer_type = MYSQL_TYPE_STRING;
    parameters[3].buffer = &role[0];
    parameters[3].buffer_length = lengths[3];
    parameters[3].length = &lengths[3];

    parameters[4].buffer_type = MYSQL_TYPE_LONG;
    parameters[4].buffer = &id;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);
    return success;
}

bool UserRepository::updatePasswordHash(
    int userId,
    const std::string& passwordHash)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0 || passwordHash.empty())
        return false;

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query =
        "UPDATE users SET password_hash = ? WHERE user_id = ?";

    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    int id = userId;
    unsigned long hashLength =
        static_cast<unsigned long>(passwordHash.length());

    MYSQL_BIND parameters[2] = {};

    parameters[0].buffer_type = MYSQL_TYPE_STRING;
    parameters[0].buffer = const_cast<char*>(passwordHash.c_str());
    parameters[0].buffer_length = hashLength;
    parameters[0].length = &hashLength;

    parameters[1].buffer_type = MYSQL_TYPE_LONG;
    parameters[1].buffer = &id;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);
    return success;
}

bool UserRepository::setUserActive(int userId, bool active)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0)
        return false;

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query =
        "UPDATE users SET is_active = ? WHERE user_id = ?";

    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    unsigned char activeValue = active ? 1 : 0;
    int id = userId;

    MYSQL_BIND parameters[2] = {};

    parameters[0].buffer_type = MYSQL_TYPE_TINY;
    parameters[0].buffer = &activeValue;

    parameters[1].buffer_type = MYSQL_TYPE_LONG;
    parameters[1].buffer = &id;

    bool success =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    if (!success)
        return false;

    MYSQL_STMT* check = mysql_stmt_init(connection);
    if (!check)
        return false;

    const char* checkQuery =
        "SELECT user_id FROM users WHERE user_id = ?";

    if (mysql_stmt_prepare(check, checkQuery, static_cast<unsigned long>(strlen(checkQuery))) != 0)
    {
        mysql_stmt_close(check);
        return false;
    }

    MYSQL_BIND checkInput[1] = {};
    checkInput[0].buffer_type = MYSQL_TYPE_LONG;
    checkInput[0].buffer = &id;

    bool exists =
        mysql_stmt_bind_param(check, checkInput) == 0 &&
        mysql_stmt_execute(check) == 0 &&
        mysql_stmt_store_result(check) == 0 &&
        mysql_stmt_num_rows(check) > 0;

    mysql_stmt_close(check);
    return exists;
}

bool UserRepository::userExists(const std::string& username)
{
    MYSQL* connection = database.getConnection();
    if (!connection || username.empty())
        return false;

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query =
        "SELECT user_id FROM users WHERE username = ?";

    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    unsigned long usernameLength =
        static_cast<unsigned long>(username.length());

    MYSQL_BIND parameters[1] = {};
    parameters[0].buffer_type = MYSQL_TYPE_STRING;
    parameters[0].buffer = const_cast<char*>(username.c_str());
    parameters[0].buffer_length = usernameLength;
    parameters[0].length = &usernameLength;

    bool exists =
        mysql_stmt_bind_param(statement, parameters) == 0 &&
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_store_result(statement) == 0 &&
        mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);
    return exists;
}

bool UserRepository::hasAnyUsers()
{
    MYSQL* connection = database.getConnection();
    if (!connection)
        return false;

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query = "SELECT user_id FROM users LIMIT 1";

    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    bool success =
        mysql_stmt_execute(statement) == 0 &&
        mysql_stmt_store_result(statement) == 0;

    bool found = success && mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);
    return found;
}

void UserRepository::displayAllUsers()
{
    MYSQL* connection = database.getConnection();
    if (!connection)
    {
        std::cout << "Database connection unavailable.\n";
        return;
    }

    const char* query =
        "SELECT user_id, username, full_name, role, is_active "
        "FROM users ORDER BY user_id";

    if (mysql_query(connection, query) != 0)
    {
        std::cout << "Could not retrieve users: " << mysql_error(connection) << "\n";
        return;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    if (!result)
    {
        std::cout << "Could not read user records.\n";
        return;
    }

    std::cout << "\n---------------- REGISTERED USERS ----------------\n";
    std::cout << "ID\tUSERNAME\t\tFULL NAME\t\tROLE\tSTATUS\n";
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        std::cout << (row[0] ? row[0] : "-") << "\t"
                  << (row[1] ? row[1] : "-") << "\t\t"
                  << (row[2] ? row[2] : "-") << "\t\t"
                  << (row[3] ? row[3] : "-") << "\t"
                  << ((row[4] && std::string(row[4]) == "1") ? "ACTIVE" : "INACTIVE")
                  << "\n";
    }
    std::cout << "--------------------------------------------------\n";
    mysql_free_result(result);
}

bool UserRepository::deactivateUserSafely(int userId, int currentUserId)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0 || userId == currentUserId)
        return false;

    MYSQL_STMT* lookup = mysql_stmt_init(connection);
    if (!lookup)
        return false;
    const char* lookupQuery = "SELECT role, is_active FROM users WHERE user_id = ?";
    if (mysql_stmt_prepare(lookup, lookupQuery, static_cast<unsigned long>(strlen(lookupQuery))) != 0)
    {
        mysql_stmt_close(lookup);
        return false;
    }
    MYSQL_BIND in[1] = {};
    in[0].buffer_type = MYSQL_TYPE_LONG;
    in[0].buffer = &userId;
    if (mysql_stmt_bind_param(lookup, in) != 0 || mysql_stmt_execute(lookup) != 0 || mysql_stmt_store_result(lookup) != 0)
    {
        mysql_stmt_close(lookup);
        return false;
    }
    char role[32] = {};
    unsigned char active = 0;
    unsigned long roleLength = 0;
    MYSQL_BIND out[2] = {};
    out[0].buffer_type = MYSQL_TYPE_STRING;
    out[0].buffer = role;
    out[0].buffer_length = sizeof(role) - 1;
    out[0].length = &roleLength;
    out[1].buffer_type = MYSQL_TYPE_TINY;
    out[1].buffer = &active;
    bool fetched = mysql_stmt_bind_result(lookup, out) == 0 && mysql_stmt_fetch(lookup) == 0;
    mysql_stmt_close(lookup);
    if (!fetched || !active)
        return false;

    if (std::string(role) == "ADMIN")
    {
        if (mysql_query(connection, "SELECT COUNT(*) FROM users WHERE role='ADMIN' AND is_active=1") != 0)
            return false;
        MYSQL_RES* countResult = mysql_store_result(connection);
        if (!countResult)
            return false;
        MYSQL_ROW countRow = mysql_fetch_row(countResult);
        unsigned long activeAdmins = countRow && countRow[0] ? std::strtoul(countRow[0], nullptr, 10) : 0;
        mysql_free_result(countResult);
        if (activeAdmins <= 1)
            return false;
    }
    return setUserActive(userId, false);
}


bool UserRepository::reactivateUserSafely(int userId)
{
    MYSQL* connection = database.getConnection();
    if (!connection || userId <= 0)
        return false;

    MYSQL_STMT* statement = mysql_stmt_init(connection);
    if (!statement)
        return false;

    const char* query = "SELECT is_active FROM users WHERE user_id = ?";
    if (mysql_stmt_prepare(statement, query, static_cast<unsigned long>(strlen(query))) != 0)
    {
        mysql_stmt_close(statement);
        return false;
    }

    MYSQL_BIND input[1] = {};
    input[0].buffer_type = MYSQL_TYPE_LONG;
    input[0].buffer = &userId;

    unsigned char active = 0;
    MYSQL_BIND output[1] = {};
    output[0].buffer_type = MYSQL_TYPE_TINY;
    output[0].buffer = &active;

    bool fetched = mysql_stmt_bind_param(statement, input) == 0 &&
                   mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_bind_result(statement, output) == 0 &&
                   mysql_stmt_fetch(statement) == 0;
    mysql_stmt_close(statement);

    if (!fetched || active != 0)
        return false;

    return setUserActive(userId, true);
}
