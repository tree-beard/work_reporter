#include "AuthService.h"

#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <chrono>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

constexpr int kSaltBytes = 16;
constexpr int kHashBytes = 32;
// OWASP-recommended minimum iteration count for PBKDF2-HMAC-SHA256 (2023 guidance).
constexpr int kIterations = 600000;

std::string base64Encode(const std::vector<unsigned char>& data) {
    // Base64 works by taking input in groups of 3 bytes and turning each group into 4 output characters
    std::vector<unsigned char> out(((data.size() + 2) / 3) * 4 + 1);
    int len = EVP_EncodeBlock(out.data(), data.data(), static_cast<int>(data.size()));
    return std::string(reinterpret_cast<char*>(out.data()), len);
}

// EVP_DecodeBlock does not strip padding automatically: it decodes '=' as a
// zero value rather than omitting it, so the caller has to trim as many
// trailing bytes as there were '=' characters in the input.
std::vector<unsigned char> base64Decode(const std::string& encoded) {
    if (encoded.empty() || encoded.size() % 4 != 0) return {};

    std::vector<unsigned char> out((encoded.size() / 4) * 3);
    int len = EVP_DecodeBlock(out.data(), reinterpret_cast<const unsigned char*>(encoded.data()),
                               static_cast<int>(encoded.size()));
    if (len < 0) return {};

    int padding = 0;
    if (encoded[encoded.size() - 1] == '=') padding++;
    if (encoded[encoded.size() - 2] == '=') padding++;
    out.resize(len - padding);
    return out;
}

// Derives a password hash and packs it with everything needed to verify it
// later into one self-describing string:
//
//   pbkdf2-sha256$<iterations>$<salt-base64>$<hash-base64>
//
// Storing the scheme name and iteration count alongside the hash (rather
// than hardcoding kIterations at verify time) means kIterations can be
// raised in the future without invalidating hashes already in the database.
std::string hashPassword(const std::string& password) {
    // A random salt is generated per password and stored (not secret) next
    // to the hash. Without it, two users with the same password would get
    // identical hashes, letting an attacker precompute a hash once (a
    // "rainbow table") and check it against every row instantly. With a
    // unique salt per row, the attacker has to redo the expensive PBKDF2
    // work separately for every single password.
    unsigned char salt[kSaltBytes];
    if (RAND_bytes(salt, kSaltBytes) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }

    // PBKDF2 repeatedly re-hashes password+salt kIterations times. That
    // deliberate slowness (as opposed to a single fast SHA-256 call) is
    // what makes brute-forcing/guessing passwords against a stolen database
    // computationally expensive.
    unsigned char hash[kHashBytes];
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()), salt, kSaltBytes,
                           kIterations, EVP_sha256(), kHashBytes, hash) != 1) {
        throw std::runtime_error("Failed to hash password");
    }

    std::ostringstream out;
    out << "pbkdf2-sha256$" << kIterations << "$"
        << base64Encode({salt, salt + kSaltBytes}) << "$"
        << base64Encode({hash, hash + kHashBytes});
    return out.str();
}

// Re-derives a hash from the candidate password using the same salt and
// iteration count that were used originally (read back out of `stored`),
// then compares it to the stored hash. There is no "decrypting" the stored
// hash back into a password -- PBKDF2 is one-way, so verification always
// works by redoing the hash and comparing outputs.
bool verifyPassword(const std::string& password, const std::string& stored) {
    std::istringstream in(stored);
    std::string scheme, iterationsStr, saltB64, hashB64;
    if (!std::getline(in, scheme, '$') || scheme != "pbkdf2-sha256" ||
        !std::getline(in, iterationsStr, '$') || !std::getline(in, saltB64, '$') ||
        !std::getline(in, hashB64, '$')) {
        return false;
    }

    int iterations = 0;
    try {
        iterations = std::stoi(iterationsStr);
    } catch (const std::exception&) {
        return false;
    }

    auto salt = base64Decode(saltB64);
    auto expected = base64Decode(hashB64);
    if (salt.empty() || expected.empty()) return false;

    std::vector<unsigned char> actual(expected.size());
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()), salt.data(),
                           static_cast<int>(salt.size()), iterations, EVP_sha256(),
                           static_cast<int>(actual.size()), actual.data()) != 1) {
        return false;
    }

    // A plain `==` on the byte buffers would return as soon as it finds the
    // first mismatched byte, so how long the comparison takes would leak
    // how many leading bytes of the guess were correct. CRYPTO_memcmp
    // always inspects every byte, so the timing is the same win or lose.
    return CRYPTO_memcmp(actual.data(), expected.data(), actual.size()) == 0;
}

}  // namespace

AuthResult AuthService::registerUser(const std::string& email, const std::string& password) {
    if (users_.findByEmail(email)) {
        return {false, "", "Email already registered"};
    }

    std::string hash;
    try {
        hash = hashPassword(password);
    } catch (const std::exception&) {
        return {false, "", "Failed to hash password"};
    }

    int64_t userId = users_.create(email, hash);
    return {true, issueToken(userId), ""};
}

AuthResult AuthService::login(const std::string& email, const std::string& password) {
    auto user = users_.findByEmail(email);
    if (!user) {
        return {false, "", "Invalid email or password"};
    }

    if (!verifyPassword(password, user->password_hash)) {
        return {false, "", "Invalid email or password"};
    }

    return {true, issueToken(user->id), ""};
}

std::string AuthService::issueToken(int64_t userId) const {
    auto now = std::chrono::system_clock::now();
    return jwt::create()
        .set_issuer("work-reporter")
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::hours(24 * 7))
        .set_payload_claim("sub", jwt::claim(std::to_string(userId)))
        .sign(jwt::algorithm::hs256{jwtSecret_});
}
