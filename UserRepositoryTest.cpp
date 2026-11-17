#include "ResponderModule.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

int main()
{
    cout << "\n========== USER REPOSITORY TEST ==========\n";

    DatabaseConnection database;

    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "FAIL: Database password environment variable not set.\n";
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow", 3306))
    {
        cout << "FAIL: Database connection failed.\n";
        return 1;
    }

    cout << "PASS: Database connected successfully.\n";

    UserRepository repository(database);

    string uniqueId = to_string(static_cast<long long>(time(nullptr)));
    string username = "test_user_" + uniqueId;
    string email = "test_" + uniqueId + "@resqflow.com";

    User user(
        0,
        username,
        "initial_test_hash",
        "Test User",
        email,
        "9876543210",
        UserRole::COORDINATOR
    );

    if (!repository.saveUser(user))
    {
        cout << "FAIL: User could not be saved.\n";
        return 1;
    }

    cout << "PASS: User saved successfully.\n";

    if (!repository.userExists(username))
    {
        cout << "FAIL: User existence check failed.\n";
        return 1;
    }

    cout << "PASS: User existence verified.\n";

    User loadedUser;

    if (!repository.loadUserByUsername(username, loadedUser))
    {
        cout << "FAIL: User could not be loaded.\n";
        return 1;
    }

    if (loadedUser.getUsername() != username ||
        loadedUser.getFullName() != "Test User")
    {
        cout << "FAIL: Loaded user data does not match.\n";
        return 1;
    }

    cout << "PASS: User loaded successfully.\n";

    int userId = loadedUser.getUserId();

    loadedUser.setFullName("Updated Test User");
    loadedUser.setEmail("updated_" + uniqueId + "@resqflow.com");
    loadedUser.setContactNumber("9123456780");
    loadedUser.setRole(UserRole::ADMIN);

    if (!repository.updateUserProfile(loadedUser))
    {
        cout << "FAIL: User profile update failed.\n";
        return 1;
    }

    cout << "PASS: User profile updated successfully.\n";

    User updatedUser;

    if (!repository.loadUserByUsername(username, updatedUser))
    {
        cout << "FAIL: Updated user could not be loaded.\n";
        return 1;
    }

    if (updatedUser.getFullName() != "Updated Test User" ||
        updatedUser.getRole() != UserRole::ADMIN)
    {
        cout << "FAIL: Updated profile verification failed.\n";
        return 1;
    }

    cout << "PASS: Updated profile verified.\n";

    if (!repository.updatePasswordHash(userId, "updated_test_hash"))
    {
        cout << "FAIL: Password hash update failed.\n";
        return 1;
    }

    cout << "PASS: Password hash updated successfully.\n";

    User passwordUpdatedUser;

    if (!repository.loadUserByUsername(username, passwordUpdatedUser))
    {
        cout << "FAIL: User could not be loaded after password update.\n";
        return 1;
    }

    if (passwordUpdatedUser.getPasswordHash() != "updated_test_hash")
    {
        cout << "FAIL: Password hash verification failed.\n";
        return 1;
    }

    cout << "PASS: Password hash verified.\n";

    if (!repository.setUserActive(userId, false))
    {
        cout << "FAIL: User deactivation failed.\n";
        return 1;
    }

    cout << "PASS: User deactivated successfully.\n";

    if (!repository.setUserActive(userId, true))
    {
        cout << "FAIL: User reactivation failed.\n";
        return 1;
    }

    cout << "PASS: User reactivated successfully.\n";

    cout << "\n=========================================\n";
    cout << "ALL USER REPOSITORY TESTS PASSED!\n";
    cout << "=========================================\n";

    return 0;
}
