// =============================================================================
// NovaForge Platform - API Security Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
// =============================================================================

#include <nova/api/api_security.hpp>

#include <random>
#include <regex>
#include <algorithm>
#include <cstring>
#include <map>
#include <mutex>

namespace nova::api::security {

// =============================================================================
// Crypto Implementation
// =============================================================================

HashResult Crypto::sha256(const std::vector<u8>& data) {
    // Simplified SHA-256 implementation placeholder
    // In production, use a proper cryptographic library
    HashResult result{};
    
    // Simple hash for demonstration - REPLACE WITH REAL SHA-256
    u64 hash = 0xcbf29ce484222325ULL; // FNV offset basis
    for (u8 byte : data) {
        hash ^= byte;
        hash *= 0x100000001b3ULL; // FNV prime
    }
    
    // Copy hash to result
    for (usize i = 0; i < 8 && i < SHA256_SIZE; ++i) {
        result[i] = static_cast<u8>((hash >> (i * 8)) & 0xFF);
    }
    
    return result;
}

HashResult Crypto::sha256(std::string_view data) {
    return sha256(std::vector<u8>(data.begin(), data.end()));
}

std::vector<u8> Crypto::hashPassword(std::string_view password, const std::vector<u8>& salt) {
    // Simplified password hashing - REPLACE WITH ARGON2ID
    std::vector<u8> combined;
    combined.insert(combined.end(), salt.begin(), salt.end());
    combined.insert(combined.end(), password.begin(), password.end());
    
    // Multiple iterations for key stretching
    HashResult hash = sha256(combined);
    for (int i = 0; i < 10000; ++i) {
        std::vector<u8> temp(hash.begin(), hash.end());
        temp.insert(temp.end(), salt.begin(), salt.end());
        hash = sha256(temp);
    }
    
    return std::vector<u8>(hash.begin(), hash.end());
}

bool Crypto::verifyPassword(std::string_view password, const std::vector<u8>& hash, const std::vector<u8>& salt) {
    auto computed = hashPassword(password, salt);
    return constantTimeCompare(computed, hash);
}

HashResult Crypto::hmacSha256(const std::vector<u8>& key, const std::vector<u8>& data) {
    // Simplified HMAC - REPLACE WITH PROPER HMAC-SHA256
    std::vector<u8> combined;
    combined.insert(combined.end(), key.begin(), key.end());
    combined.insert(combined.end(), data.begin(), data.end());
    return sha256(combined);
}

Result<std::vector<u8>, Error> Crypto::encrypt(
    const std::vector<u8>& plaintext,
    const EncryptionKey& key,
    const InitVector& iv) {
    
    // Placeholder encryption - REPLACE WITH AES-256-GCM
    std::vector<u8> ciphertext(plaintext.size() + 16); // +16 for auth tag
    
    // XOR with key for demonstration
    for (usize i = 0; i < plaintext.size(); ++i) {
        ciphertext[i] = plaintext[i] ^ key.data()[i % AES_KEY_SIZE] ^ iv.data()[i % AES_IV_SIZE];
    }
    
    // Add fake auth tag
    for (usize i = 0; i < 16; ++i) {
        ciphertext[plaintext.size() + i] = static_cast<u8>(i);
    }
    
    return ciphertext;
}

Result<std::vector<u8>, Error> Crypto::decrypt(
    const std::vector<u8>& ciphertext,
    const EncryptionKey& key,
    const InitVector& iv) {
    
    if (ciphertext.size() < 16) {
        return std::unexpected(Error(ErrorCategory::Validation, 1, "Ciphertext too short"));
    }
    
    // Placeholder decryption - REPLACE WITH AES-256-GCM
    std::vector<u8> plaintext(ciphertext.size() - 16);
    
    for (usize i = 0; i < plaintext.size(); ++i) {
        plaintext[i] = ciphertext[i] ^ key.data()[i % AES_KEY_SIZE] ^ iv.data()[i % AES_IV_SIZE];
    }
    
    return plaintext;
}

Result<std::string, Error> Crypto::encryptString(std::string_view plaintext, const EncryptionKey& key) {
    InitVector iv = generateIV();
    std::vector<u8> data(plaintext.begin(), plaintext.end());
    
    auto result = encrypt(data, key, iv);
    if (!result) return std::unexpected(result.error());
    
    // Prepend IV to ciphertext
    std::vector<u8> combined;
    combined.insert(combined.end(), iv.data(), iv.data() + AES_IV_SIZE);
    combined.insert(combined.end(), result->begin(), result->end());
    
    return base64Encode(combined);
}

Result<std::string, Error> Crypto::decryptString(std::string_view ciphertext, const EncryptionKey& key) {
    auto decoded = base64Decode(ciphertext);
    if (!decoded) return std::unexpected(decoded.error());
    
    if (decoded->size() < AES_IV_SIZE) {
        return std::unexpected(Error(ErrorCategory::Validation, 1, "Ciphertext too short"));
    }
    
    // Extract IV
    InitVector iv;
    std::memcpy(iv.data(), decoded->data(), AES_IV_SIZE);
    
    std::vector<u8> encryptedData(decoded->begin() + AES_IV_SIZE, decoded->end());
    
    auto result = decrypt(encryptedData, key, iv);
    if (!result) return std::unexpected(result.error());
    
    return std::string(result->begin(), result->end());
}

std::vector<u8> Crypto::randomBytes(usize count) {
    std::vector<u8> bytes(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<u32> dist(0, 255);
    
    for (usize i = 0; i < count; ++i) {
        bytes[i] = static_cast<u8>(dist(gen));
    }
    
    return bytes;
}

EncryptionKey Crypto::generateKey() {
    EncryptionKey key;
    auto bytes = randomBytes(AES_KEY_SIZE);
    std::memcpy(key.data(), bytes.data(), AES_KEY_SIZE);
    return key;
}

InitVector Crypto::generateIV() {
    InitVector iv;
    auto bytes = randomBytes(AES_IV_SIZE);
    std::memcpy(iv.data(), bytes.data(), AES_IV_SIZE);
    return iv;
}

std::vector<u8> Crypto::generateSalt() {
    return randomBytes(16);
}

std::string Crypto::generateTokenString(usize length) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    static const usize charCount = 64;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<usize> dist(0, charCount - 1);
    
    std::string token;
    token.reserve(length);
    
    for (usize i = 0; i < length; ++i) {
        token += chars[dist(gen)];
    }
    
    return token;
}

std::string Crypto::base64Encode(const std::vector<u8>& data) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    result.reserve((data.size() + 2) / 3 * 4);
    
    for (usize i = 0; i < data.size(); i += 3) {
        u32 n = static_cast<u32>(data[i]) << 16;
        if (i + 1 < data.size()) n |= static_cast<u32>(data[i + 1]) << 8;
        if (i + 2 < data.size()) n |= static_cast<u32>(data[i + 2]);
        
        result += chars[(n >> 18) & 0x3F];
        result += chars[(n >> 12) & 0x3F];
        result += (i + 1 < data.size()) ? chars[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < data.size()) ? chars[n & 0x3F] : '=';
    }
    
    return result;
}

Result<std::vector<u8>, Error> Crypto::base64Decode(std::string_view base64) {
    static const u8 decodeTable[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255, 255,
        255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
         15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
        255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
         41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
    };
    
    std::vector<u8> result;
    result.reserve(base64.size() * 3 / 4);
    
    u32 bits = 0;
    int bitCount = 0;
    
    for (char c : base64) {
        if (c == '=') break;
        
        u8 val = decodeTable[static_cast<u8>(c)];
        if (val == 255) {
            return std::unexpected(Error(ErrorCategory::Parse, 1, "Invalid base64 character"));
        }
        
        bits = (bits << 6) | val;
        bitCount += 6;
        
        if (bitCount >= 8) {
            bitCount -= 8;
            result.push_back(static_cast<u8>((bits >> bitCount) & 0xFF));
        }
    }
    
    return result;
}

std::string Crypto::hexEncode(const std::vector<u8>& data) {
    static const char* hex = "0123456789abcdef";
    std::string result;
    result.reserve(data.size() * 2);
    
    for (u8 byte : data) {
        result += hex[byte >> 4];
        result += hex[byte & 0x0F];
    }
    
    return result;
}

Result<std::vector<u8>, Error> Crypto::hexDecode(std::string_view hex) {
    if (hex.size() % 2 != 0) {
        return std::unexpected(Error(ErrorCategory::Parse, 1, "Invalid hex string length"));
    }
    
    std::vector<u8> result;
    result.reserve(hex.size() / 2);
    
    for (usize i = 0; i < hex.size(); i += 2) {
        auto hexToByte = [](char c) -> i32 {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        
        i32 high = hexToByte(hex[i]);
        i32 low = hexToByte(hex[i + 1]);
        
        if (high < 0 || low < 0) {
            return std::unexpected(Error(ErrorCategory::Parse, 2, "Invalid hex character"));
        }
        
        result.push_back(static_cast<u8>((high << 4) | low));
    }
    
    return result;
}

bool Crypto::constantTimeCompare(const std::vector<u8>& a, const std::vector<u8>& b) {
    if (a.size() != b.size()) return false;
    
    u8 result = 0;
    for (usize i = 0; i < a.size(); ++i) {
        result = static_cast<u8>(result | (a[i] ^ b[i]));
    }
    
    return result == 0;
}

std::vector<u8> Crypto::deriveKey(
    std::string_view password,
    const std::vector<u8>& salt,
    u32 iterations,
    usize keyLength) {
    
    // Simplified PBKDF2 - REPLACE WITH PROPER IMPLEMENTATION
    std::vector<u8> key = hashPassword(password, salt);
    
    for (u32 i = 0; i < iterations / 10000; ++i) {
        key = hashPassword(std::string_view(reinterpret_cast<const char*>(key.data()), key.size()), salt);
    }
    
    if (key.size() > keyLength) {
        key.resize(keyLength);
    }
    
    return key;
}

// =============================================================================
// TokenManager Implementation
// =============================================================================

struct TokenManager::Impl {
    Config config;
    std::map<std::string, std::chrono::system_clock::time_point> revokedTokens;
    std::mutex mutex;
};

TokenManager::TokenManager(const Config& config) : m_impl(std::make_unique<Impl>()) {
    m_impl->config = config;
}

TokenManager::~TokenManager() = default;

Result<SecurityToken, Error> TokenManager::generateAccessToken(
    std::string_view userId,
    Permission permissions,
    const std::map<std::string, std::string>& claims) {
    
    SecurityToken token;
    token.token = Crypto::generateTokenString(64);
    token.type = TokenType::Access;
    token.userId = std::string(userId);
    token.permissions = permissions;
    token.issuedAt = std::chrono::system_clock::now();
    token.expiresAt = token.issuedAt + std::chrono::seconds(m_impl->config.accessTokenExpirySeconds);
    token.issuer = m_impl->config.issuer;
    token.audience = m_impl->config.audience;
    token.claims = claims;
    
    return token;
}

Result<SecurityToken, Error> TokenManager::generateRefreshToken(std::string_view userId) {
    SecurityToken token;
    token.token = Crypto::generateTokenString(128);
    token.type = TokenType::Refresh;
    token.userId = std::string(userId);
    token.permissions = Permission::None;
    token.issuedAt = std::chrono::system_clock::now();
    token.expiresAt = token.issuedAt + std::chrono::seconds(m_impl->config.refreshTokenExpirySeconds);
    token.issuer = m_impl->config.issuer;
    token.audience = m_impl->config.audience;
    
    return token;
}

Result<SecurityToken, Error> TokenManager::generateApiKey(
    std::string_view userId,
    Permission permissions,
    std::string_view name) {
    
    SecurityToken token;
    token.token = "nova_" + Crypto::generateTokenString(48);
    token.type = TokenType::API;
    token.userId = std::string(userId);
    token.permissions = permissions;
    token.issuedAt = std::chrono::system_clock::now();
    token.expiresAt = token.issuedAt + std::chrono::hours(24 * 365); // 1 year
    token.issuer = m_impl->config.issuer;
    token.audience = m_impl->config.audience;
    token.claims["name"] = std::string(name);
    
    return token;
}

TokenValidationResult TokenManager::validateToken(std::string_view token) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    std::string tokenStr(token);
    
    // Check if revoked
    if (m_impl->revokedTokens.count(tokenStr) > 0) {
        return TokenValidationResult::failure("Token has been revoked");
    }
    
    // For this simplified implementation, we can't validate the token content
    // In production, you would decode and verify the JWT signature
    
    return TokenValidationResult::success("unknown", Permission::Read);
}

Result<SecurityToken, Error> TokenManager::decodeToken(std::string_view token) {
    // Simplified - in production, decode JWT
    SecurityToken decoded;
    decoded.token = std::string(token);
    return decoded;
}

Result<SecurityToken, Error> TokenManager::refreshAccessToken(std::string_view refreshToken) {
    auto validation = validateToken(refreshToken);
    if (!validation.valid) {
        return std::unexpected(Error(ErrorCategory::Permission, 1, validation.errorMessage));
    }
    
    return generateAccessToken(validation.userId, validation.permissions, {});
}

void TokenManager::revokeToken(std::string_view token) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->revokedTokens[std::string(token)] = std::chrono::system_clock::now();
}

bool TokenManager::isTokenRevoked(std::string_view token) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->revokedTokens.count(std::string(token)) > 0;
}

// =============================================================================
// RateLimiter Implementation
// =============================================================================

struct RateLimiter::Impl {
    Config config;
    
    struct ClientData {
        std::vector<std::chrono::system_clock::time_point> requests;
    };
    
    std::map<std::string, ClientData> clients;
    std::mutex mutex;
};

RateLimiter::RateLimiter() : m_impl(std::make_unique<Impl>()) {}

RateLimiter::RateLimiter(const Config& config) : m_impl(std::make_unique<Impl>()) {
    m_impl->config = config;
}

RateLimiter::~RateLimiter() = default;

bool RateLimiter::isAllowed(std::string_view identifier) {
    if (!m_impl->config.enabled) return true;
    
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto now = std::chrono::system_clock::now();
    auto windowStart = now - std::chrono::seconds(m_impl->config.windowSeconds);
    
    auto& client = m_impl->clients[id];
    
    // Remove old requests
    client.requests.erase(
        std::remove_if(client.requests.begin(), client.requests.end(),
            [windowStart](const auto& t) { return t < windowStart; }),
        client.requests.end());
    
    return client.requests.size() < m_impl->config.requestsPerWindow;
}

void RateLimiter::recordRequest(std::string_view identifier) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->clients[std::string(identifier)].requests.push_back(std::chrono::system_clock::now());
}

u32 RateLimiter::getRemainingRequests(std::string_view identifier) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto it = m_impl->clients.find(id);
    if (it == m_impl->clients.end()) {
        return m_impl->config.requestsPerWindow;
    }
    
    auto now = std::chrono::system_clock::now();
    auto windowStart = now - std::chrono::seconds(m_impl->config.windowSeconds);
    
    usize validRequests = 0;
    for (const auto& t : it->second.requests) {
        if (t >= windowStart) ++validRequests;
    }
    
    return static_cast<u32>(m_impl->config.requestsPerWindow > validRequests ? 
        m_impl->config.requestsPerWindow - validRequests : 0);
}

u32 RateLimiter::getResetTime(std::string_view identifier) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto it = m_impl->clients.find(id);
    if (it == m_impl->clients.end() || it->second.requests.empty()) {
        return 0;
    }
    
    auto oldest = *std::min_element(it->second.requests.begin(), it->second.requests.end());
    auto resetTime = oldest + std::chrono::seconds(m_impl->config.windowSeconds);
    auto now = std::chrono::system_clock::now();
    
    if (resetTime <= now) return 0;
    
    return static_cast<u32>(
        std::chrono::duration_cast<std::chrono::seconds>(resetTime - now).count());
}

void RateLimiter::reset(std::string_view identifier) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->clients.erase(std::string(identifier));
}

void RateLimiter::resetAll() {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->clients.clear();
}

// =============================================================================
// RequestSigner Implementation
// =============================================================================

struct RequestSigner::Impl {
    EncryptionKey signingKey;
};

RequestSigner::RequestSigner(const EncryptionKey& signingKey) : m_impl(std::make_unique<Impl>()) {
    std::memcpy(m_impl->signingKey.data(), signingKey.data(), AES_KEY_SIZE);
}

RequestSigner::~RequestSigner() = default;

RequestSigner::SignedRequest RequestSigner::signRequest(
    std::string_view method,
    std::string_view path,
    std::string_view body,
    const std::map<std::string, std::string>& headers) {
    
    SignedRequest result;
    result.timestamp = std::to_string(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    result.nonce = Crypto::generateTokenString(16);
    
    // Create string to sign
    std::string stringToSign;
    stringToSign += method;
    stringToSign += "\n";
    stringToSign += path;
    stringToSign += "\n";
    stringToSign += result.timestamp;
    stringToSign += "\n";
    stringToSign += result.nonce;
    stringToSign += "\n";
    stringToSign += body;
    
    // Compute HMAC
    std::vector<u8> keyData(m_impl->signingKey.data(), m_impl->signingKey.data() + AES_KEY_SIZE);
    std::vector<u8> data(stringToSign.begin(), stringToSign.end());
    HashResult hmac = Crypto::hmacSha256(keyData, data);
    
    result.signature = Crypto::base64Encode(std::vector<u8>(hmac.begin(), hmac.end()));
    
    result.headers["X-Nova-Signature"] = result.signature;
    result.headers["X-Nova-Timestamp"] = result.timestamp;
    result.headers["X-Nova-Nonce"] = result.nonce;
    
    for (const auto& [key, value] : headers) {
        result.headers[key] = value;
    }
    
    return result;
}

bool RequestSigner::verifyRequest(
    std::string_view method,
    std::string_view path,
    std::string_view body,
    std::string_view signature,
    std::string_view timestamp,
    std::string_view nonce,
    u32 maxAgeSeconds) {
    
    // Check timestamp age
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    auto reqTime = std::stoll(std::string(timestamp));
    
    if (std::abs(now - reqTime) > maxAgeSeconds) {
        return false;
    }
    
    // Recreate string to sign
    std::string stringToSign;
    stringToSign += method;
    stringToSign += "\n";
    stringToSign += path;
    stringToSign += "\n";
    stringToSign += timestamp;
    stringToSign += "\n";
    stringToSign += nonce;
    stringToSign += "\n";
    stringToSign += body;
    
    // Compute expected HMAC
    std::vector<u8> keyData(m_impl->signingKey.data(), m_impl->signingKey.data() + AES_KEY_SIZE);
    std::vector<u8> data(stringToSign.begin(), stringToSign.end());
    HashResult expectedHmac = Crypto::hmacSha256(keyData, data);
    
    std::string expectedSig = Crypto::base64Encode(std::vector<u8>(expectedHmac.begin(), expectedHmac.end()));
    
    return signature == expectedSig;
}

// =============================================================================
// SecureStorage Implementation
// =============================================================================

struct SecureStorage::Impl {
    EncryptionKey masterKey;
    std::map<std::string, std::vector<u8>> storage;
    std::mutex mutex;
    
    Impl() {
        masterKey = Crypto::generateKey();
    }
};

SecureStorage::SecureStorage() : m_impl(std::make_unique<Impl>()) {}
SecureStorage::~SecureStorage() = default;

Result<void, Error> SecureStorage::store(std::string_view key, std::string_view value) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    auto encrypted = Crypto::encryptString(value, m_impl->masterKey);
    if (!encrypted) return std::unexpected(encrypted.error());
    
    m_impl->storage[std::string(key)] = std::vector<u8>(encrypted->begin(), encrypted->end());
    return {};
}

Result<SecureString, Error> SecureStorage::retrieve(std::string_view key) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    auto it = m_impl->storage.find(std::string(key));
    if (it == m_impl->storage.end()) {
        return std::unexpected(Error(ErrorCategory::NotFound, 1, "Key not found"));
    }
    
    std::string encrypted(it->second.begin(), it->second.end());
    auto decrypted = Crypto::decryptString(encrypted, m_impl->masterKey);
    if (!decrypted) return std::unexpected(decrypted.error());
    
    return SecureString(*decrypted);
}

Result<void, Error> SecureStorage::remove(std::string_view key) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->storage.erase(std::string(key));
    return {};
}

bool SecureStorage::exists(std::string_view key) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->storage.count(std::string(key)) > 0;
}

void SecureStorage::clear() {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->storage.clear();
}

// =============================================================================
// InputValidator Implementation
// =============================================================================

bool InputValidator::isValidEmail(std::string_view email) {
    if (email.empty() || email.size() > 254) return false;
    
    // Simple email validation
    auto at = email.find('@');
    if (at == std::string_view::npos || at == 0 || at == email.size() - 1) return false;
    
    auto dot = email.find('.', at);
    if (dot == std::string_view::npos || dot == at + 1 || dot == email.size() - 1) return false;
    
    return true;
}

InputValidator::PasswordValidation InputValidator::validatePassword(std::string_view password) {
    PasswordValidation result;
    result.valid = true;
    result.strength = 0;
    
    if (password.size() < MIN_PASSWORD_LENGTH) {
        result.valid = false;
        result.message = "Password must be at least 8 characters";
        return result;
    }
    
    if (password.size() > MAX_PASSWORD_LENGTH) {
        result.valid = false;
        result.message = "Password must be at most 128 characters";
        return result;
    }
    
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }
    
    if (hasUpper) result.strength += 20;
    if (hasLower) result.strength += 20;
    if (hasDigit) result.strength += 20;
    if (hasSpecial) result.strength += 20;
    if (password.size() >= 12) result.strength += 20;
    
    if (result.strength < 60) {
        result.message = "Weak password - add uppercase, lowercase, numbers, and special characters";
    } else if (result.strength < 80) {
        result.message = "Moderate password strength";
    } else {
        result.message = "Strong password";
    }
    
    return result;
}

bool InputValidator::isValidUsername(std::string_view username) {
    if (username.size() < 3 || username.size() > 32) return false;
    
    for (char c : username) {
        if (!std::isalnum(c) && c != '_' && c != '-') return false;
    }
    
    // Must start with letter
    if (!std::isalpha(username[0])) return false;
    
    return true;
}

std::string InputValidator::sanitize(std::string_view input) {
    std::string result;
    result.reserve(input.size());
    
    for (char c : input) {
        // Remove control characters
        if (c >= 32 && c < 127) {
            result += c;
        }
    }
    
    return result;
}

std::string InputValidator::sanitizeHtml(std::string_view input) {
    std::string result;
    result.reserve(input.size() * 2);
    
    for (char c : input) {
        switch (c) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c;
        }
    }
    
    return result;
}

std::string InputValidator::sanitizeSql(std::string_view input) {
    std::string result;
    result.reserve(input.size() * 2);
    
    for (char c : input) {
        if (c == '\'') {
            result += "''";
        } else if (c == '\\') {
            result += "\\\\";
        } else {
            result += c;
        }
    }
    
    return result;
}

bool InputValidator::isValidUrl(std::string_view url) {
    if (url.empty() || url.size() > 2048) return false;
    
    // Check for scheme
    if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://") {
        return false;
    }
    
    return true;
}

bool InputValidator::isValidUuid(std::string_view uuid) {
    if (uuid.size() != 36) return false;
    
    // Check format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    for (usize i = 0; i < uuid.size(); ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (uuid[i] != '-') return false;
        } else {
            if (!std::isxdigit(uuid[i])) return false;
        }
    }
    
    return true;
}

// =============================================================================
// SecurityManager Implementation
// =============================================================================

struct SecurityManager::Impl {
    bool initialized = false;
    Config config;
    
    std::unique_ptr<TokenManager> tokenManager;
    std::unique_ptr<RateLimiter> rateLimiter;
    std::unique_ptr<SecureStorage> secureStorage;
    
    std::map<std::string, u32> failedLoginAttempts;
    std::map<std::string, std::chrono::system_clock::time_point> accountLockouts;
    std::mutex mutex;
};

SecurityManager& SecurityManager::get() {
    static SecurityManager instance;
    return instance;
}

SecurityManager::SecurityManager() : m_impl(std::make_unique<Impl>()) {}
SecurityManager::~SecurityManager() = default;

Result<void, Error> SecurityManager::initialize() {
    return initialize(Config{});
}

Result<void, Error> SecurityManager::initialize(const Config& config) {
    if (m_impl->initialized) {
        return std::unexpected(Error(ErrorCategory::Runtime, 1, "Already initialized"));
    }
    
    m_impl->config = config;
    
    // Generate signing key
    auto keyBytes = Crypto::randomBytes(AES_KEY_SIZE);
    
    // Create token manager
    TokenManager::Config tmConfig;
    tmConfig.issuer = config.issuer;
    tmConfig.accessTokenExpirySeconds = config.tokenExpirySeconds;
    std::copy(keyBytes.begin(), keyBytes.end(), tmConfig.signingKey.begin());
    m_impl->tokenManager = std::make_unique<TokenManager>(tmConfig);
    
    // Create rate limiter
    m_impl->rateLimiter = std::make_unique<RateLimiter>(config.rateLimitConfig);
    
    // Create secure storage
    m_impl->secureStorage = std::make_unique<SecureStorage>();
    
    m_impl->initialized = true;
    return {};
}

void SecurityManager::shutdown() {
    m_impl->tokenManager.reset();
    m_impl->rateLimiter.reset();
    m_impl->secureStorage.reset();
    m_impl->initialized = false;
}

TokenManager& SecurityManager::getTokenManager() {
    return *m_impl->tokenManager;
}

RateLimiter& SecurityManager::getRateLimiter() {
    return *m_impl->rateLimiter;
}

SecureStorage& SecurityManager::getSecureStorage() {
    return *m_impl->secureStorage;
}

Result<SecurityToken, Error> SecurityManager::authenticate(
    std::string_view username,
    std::string_view password) {
    
    std::string id(username);
    
    // Check if account is locked
    if (isAccountLocked(id)) {
        return std::unexpected(Error(ErrorCategory::Permission, 1, "Account is temporarily locked"));
    }
    
    // TODO: Verify credentials against actual user database
    // This is a placeholder
    
    if (password.empty()) {
        recordFailedLogin(id);
        return std::unexpected(Error(ErrorCategory::Permission, 2, "Invalid credentials"));
    }
    
    // Reset failed attempts on success
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->failedLoginAttempts.erase(id);
    }
    
    return m_impl->tokenManager->generateAccessToken(username, Permission::ReadWrite, {});
}

Result<SecurityContext, Error> SecurityManager::validateRequest(
    std::string_view token,
    Permission requiredPermission) {
    
    auto validation = m_impl->tokenManager->validateToken(token);
    if (!validation.valid) {
        return std::unexpected(Error(ErrorCategory::Permission, 1, validation.errorMessage));
    }
    
    SecurityContext ctx;
    ctx.authenticated = true;
    ctx.userId = validation.userId;
    ctx.permissions = validation.permissions;
    ctx.token.token = std::string(token);
    
    if (requiredPermission != Permission::None) {
        auto result = ctx.requirePermission(requiredPermission);
        if (!result) return std::unexpected(result.error());
    }
    
    return ctx;
}

bool SecurityManager::checkRateLimit(std::string_view identifier) {
    if (!m_impl->config.enableRateLimiting) return true;
    
    bool allowed = m_impl->rateLimiter->isAllowed(identifier);
    if (allowed) {
        m_impl->rateLimiter->recordRequest(identifier);
    }
    return allowed;
}

void SecurityManager::recordFailedLogin(std::string_view identifier) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    m_impl->failedLoginAttempts[id]++;
    
    if (m_impl->failedLoginAttempts[id] >= m_impl->config.maxLoginAttempts) {
        m_impl->accountLockouts[id] = std::chrono::system_clock::now() + 
            std::chrono::seconds(m_impl->config.lockoutDurationSeconds);
    }
}

bool SecurityManager::isAccountLocked(std::string_view identifier) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto it = m_impl->accountLockouts.find(id);
    if (it == m_impl->accountLockouts.end()) return false;
    
    if (std::chrono::system_clock::now() >= it->second) {
        // Lockout expired
        return false;
    }
    
    return true;
}

} // namespace nova::api::security
