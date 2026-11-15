#include "../include/Authentication.h"
#include "../include/PasswordHasher.h"
#include <iostream>
#include <string>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

namespace
{
    // Masked password input. Windows: _getch(). POSIX (used for CI/testing): termios.
    string readPasswordImpl()
    {
        string password;

#ifdef _WIN32
        int ch;
        while ((ch = _getch()) != '\r' && ch != '\n')
        {
            if (ch == '\b')
            {
                if (!password.empty())
                {
                    password.pop_back();
                    cout << "\b \b";
                }
            }
            else if (ch >= 32 && ch <= 126)
            {
                password += static_cast<char>(ch);
                cout << '*';
            }
        }
        cout << '\n';
#else
        termios oldSettings{};
        bool isTerminal = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &oldSettings) == 0;

        if (isTerminal)
        {
            termios hidden = oldSettings;
            hidden.c_lflag &= static_cast<tcflag_t>(~ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
        }

        getline(cin, password);

        if (isTerminal)
        {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
            cout << '\n';
        }
#endif
        return password;
    }

    bool createFirstAdmin(UserRepository& repository)
    {
        string username;
        string fullName;
        string password;
        string confirmPassword;

        cout << "\n====================================\n";
        cout << "       RESQFLOW FIRST-TIME SETUP\n";
        cout << "====================================\n";
        cout << "No user accounts found.\n";
        cout << "Create the first ADMIN account.\n\n";

        cout << "Full name: ";
        getline(cin, fullName);

        cout << "Username: ";
        getline(cin, username);

        if (fullName.empty() || username.empty())
        {
            cout << "Name and username cannot be empty.\n";
            return false;
        }

        cout << "Create password (minimum 10 characters): ";
        password = readPasswordImpl();

        cout << "Confirm password: ";
        confirmPassword = readPasswordImpl();

        if (password.length() < 10)
        {
            cout << "Password must contain at least 10 characters.\n";
            return false;
        }

        if (password != confirmPassword)
        {
            cout << "Passwords do not match.\n";
            return false;
        }

        string passwordHash = PasswordHasher::hashPassword(password);

        if (passwordHash.empty())
        {
            cout << "Password hashing failed. Admin account not created.\n";
            return false;
        }

        User admin(
            0,
            username,
            passwordHash,
            fullName,
            "",
            "",
            UserRole::ADMIN
        );

        if (!repository.saveUser(admin))
        {
            cout << "Could not create the administrator account.\n";
            cout << "Check the database and username.\n";
            return false;
        }

        cout << "\nFirst ADMIN account created successfully.\n";
        return true;
    }
}

bool Authentication::authenticate(
    UserRepository& repository,
    User& loggedInUser)
{
    if (!repository.hasAnyUsers())
    {
        if (!createFirstAdmin(repository))
            return false;
    }

    cout << "\n====================================\n";
    cout << "           RESQFLOW LOGIN\n";
    cout << "====================================\n";

    for (int attempt = 1; attempt <= 3; attempt++)
    {
        string username;
        string password;

        cout << "\nUsername: ";
        getline(cin, username);

        cout << "Password: ";
        password = readPasswordImpl();

        User user;

        if (!repository.loadUserByUsername(username, user) ||
            !user.isActive() ||
            !PasswordHasher::verifyPassword(
                password,
                user.getPasswordHash()))
        {
            cout << "\nInvalid username or password.\n";
            cout << "Attempts remaining: " << 3 - attempt << "\n";
            continue;
        }

        loggedInUser = user;

        cout << "\nLogin successful. Welcome, "
             << user.getFullName() << "!\n";

        cout << "Role: " << userRoleToString(user.getRole()) << "\n";
        return true;
    }

    cout << "\nMaximum login attempts exceeded.\n";
    return false;
}

std::string Authentication::promptPassword()
{
    return readPasswordImpl();
}
