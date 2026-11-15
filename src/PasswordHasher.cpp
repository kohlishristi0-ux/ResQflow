#include "../include/PasswordHasher.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <sstream>
#include <iomanip>
#include <cstdlib>

namespace
{
    const int ITERATIONS = 210000;
    const int SALT_SIZE = 16;
    const int HASH_SIZE = 32;

    std::string toHex(const unsigned char* data, int length)
    {
        std::ostringstream output;
        output << std::hex << std::setfill('0');

        for (int i = 0; i < length; i++)
            output << std::setw(2) << static_cast<int>(data[i]);

        return output.str();
    }

    bool fromHex(const std::string& hex, unsigned char* output, int length)
    {
        if (hex.length() != static_cast<size_t>(length * 2))
            return false;

        for (int i = 0; i < length; i++)
        {
            char* end = nullptr;
            std::string byte = hex.substr(i * 2, 2);
            long value = std::strtol(byte.c_str(), &end, 16);

            if (end == nullptr || *end != '\0')
                return false;

            output[i] = static_cast<unsigned char>(value);
        }

        return true;
    }
}

std::string PasswordHasher::hashPassword(const std::string& password)
{
    if (password.length() < 10)
        return "";

    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];

    if (RAND_bytes(salt, SALT_SIZE) != 1)
        return "";

    if (PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.length()),
        salt,
        SALT_SIZE,
        ITERATIONS,
        EVP_sha256(),
        HASH_SIZE,
        hash
    ) != 1)
        return "";

    return "pbkdf2_sha256$" +
           std::to_string(ITERATIONS) + "$" +
           toHex(salt, SALT_SIZE) + "$" +
           toHex(hash, HASH_SIZE);
}

bool PasswordHasher::verifyPassword(
    const std::string& password,
    const std::string& storedHash)
{
    std::istringstream input(storedHash);
    std::string algorithm;
    std::string iterationText;
    std::string saltHex;
    std::string hashHex;

    if (!std::getline(input, algorithm, '$') ||
        !std::getline(input, iterationText, '$') ||
        !std::getline(input, saltHex, '$') ||
        !std::getline(input, hashHex, '$'))
        return false;

    if (algorithm != "pbkdf2_sha256")
        return false;

    int iterations = 0;

    try
    {
        size_t position = 0;
        iterations = std::stoi(iterationText, &position);

        if (position != iterationText.length() ||
            iterations < 100000 || iterations > 1000000)
            return false;
    }
    catch (...)
    {
        return false;
    }

    unsigned char salt[SALT_SIZE];
    unsigned char expectedHash[HASH_SIZE];
    unsigned char actualHash[HASH_SIZE];

    if (!fromHex(saltHex, salt, SALT_SIZE) ||
        !fromHex(hashHex, expectedHash, HASH_SIZE))
        return false;

    if (PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.length()),
        salt,
        SALT_SIZE,
        iterations,
        EVP_sha256(),
        HASH_SIZE,
        actualHash
    ) != 1)
        return false;

    return CRYPTO_memcmp(actualHash, expectedHash, HASH_SIZE) == 0;
}