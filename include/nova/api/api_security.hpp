// =============================================================================
// NovaForge Platform - API Security System
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// Security module for the NovaForge API providing:
// - Authentication and authorization
// - Token management with secure storage
// - Data encryption and hashing
// - Request signing and verification
// - Rate limiting and abuse prevention
// - Secure communication protocols
// =============================================================================

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/types/result.hpp>

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <optional>
#include <array>
#include <map>

namespace nova::api::security {

// =============================================================================
// Security Constants
// =============================================================================

/// AES-256 key size in bytes
inline constexpr usize AES_KEY_SIZE = 32;

/// AES-256 block size in bytes
inline constexpr usize AES_BLOCK_SIZE = 16;

/// AES-256 IV size in bytes
inline constexpr usize AES_IV_SIZE = 16;

/// SHA-256 hash size in bytes
inline constexpr usize SHA256_SIZE = 32;

/// HMAC-SHA256 size in bytes
inline constexpr usize HMAC_SHA256_SIZE = 32;

/// Minimum password length
inline constexpr usize MIN_PASSWORD_LENGTH = 8;

/// Maximum password length
inline constexpr usize MAX_PASSWORD_LENGTH = 128;

/// Token expiration time (default: 1 hour)
inline constexpr u32 DEFAULT_TOKEN_EXPIRY_SECONDS = 3600;

/// Refresh token expiration (default: 30 days)
inline constexpr u32 DEFAULT_REFRESH_TOKEN_EXPIRY_SECONDS = 30 * 24 * 3600;

/// Maximum login attempts before lockout
inline constexpr u32 MAX_LOGIN_ATTEMPTS = 5;

/// Lockout duration in seconds
inline constexpr u32 LOCKOUT_DURATION_SECONDS = 900; // 15 minutes

// =============================================================================
// Security Types
// =============================================================================

/**
 * @brief Encryption algorithm
 */
enum class EncryptionAlgorithm : u8 {
    None = 0,
    AES_128_GCM,        ///< AES-128 in GCM mode (authenticated encryption)
    AES_256_GCM,        ///< AES-256 in GCM mode (authenticated encryption)
    AES_256_CBC,        ///< AES-256 in CBC mode
    ChaCha20_Poly1305   ///< ChaCha20-Poly1305 (mobile-optimized)
};

/**
 * @brief Hash algorithm
 */
enum class HashAlgorithm : u8 {
    SHA256,             ///< SHA-256
    SHA384,             ///< SHA-384
    SHA512,             ///< SHA-512
    Blake2b,            ///< BLAKE2b (faster than SHA)
    Argon2id            ///< Argon2id (password hashing)
};

/**
 * @brief Security level
 */
enum class SecurityLevel : u8 {
    Low = 0,            ///< Basic security (development only)
    Medium = 1,         ///< Standard security
    High = 2,           ///< Enhanced security
    Maximum = 3         ///< Maximum security (may impact performance)
};

/**
 * @brief Permission flags
 */
enum class Permission : u32 {
    None            = 0,
    Read            = 1 << 0,
    Write           = 1 << 1,
    Delete          = 1 << 2,
    Admin           = 1 << 3,
    CreateUsers     = 1 << 4,
    ModifyUsers     = 1 << 5,
    DeleteUsers     = 1 << 6,
    ViewAnalytics   = 1 << 7,
    ManageContent   = 1 << 8,
    ManagePayments  = 1 << 9,
    AccessAPI       = 1 << 10,
    ManageAPI       = 1 << 11,
    
    // Composite permissions
    ReadWrite       = Read | Write,
    FullAccess      = Read | Write | Delete,
    UserManagement  = CreateUsers | ModifyUsers | DeleteUsers,
    AllPermissions  = 0xFFFFFFFF
};

/// Enable bitwise operations for Permission
[[nodiscard]] inline constexpr Permission operator|(Permission a, Permission b) noexcept {
    return static_cast<Permission>(static_cast<u32>(a) | static_cast<u32>(b));
}

[[nodiscard]] inline constexpr Permission operator&(Permission a, Permission b) noexcept {
    return static_cast<Permission>(static_cast<u32>(a) & static_cast<u32>(b));
}

[[nodiscard]] inline constexpr bool hasPermission(Permission permissions, Permission required) noexcept {
    return (static_cast<u32>(permissions) & static_cast<u32>(required)) == static_cast<u32>(required);
}

// =============================================================================
// Secure Data Types
// =============================================================================

/**
 * @brief Secure byte array that is zeroed on destruction
 */
template<usize N>
class SecureArray {
public:
    SecureArray() { m_data.fill(0); }
    
    ~SecureArray() { 
        // Secure erase - prevent compiler optimization from removing
        volatile u8* ptr = m_data.data();
        for (usize i = 0; i < N; ++i) {
            ptr[i] = 0;
        }
    }
    
    // Non-copyable to prevent accidental exposure
    SecureArray(const SecureArray&) = delete;
    SecureArray& operator=(const SecureArray&) = delete;
    
    // Move allowed
    SecureArray(SecureArray&& other) noexcept : m_data(std::move(other.m_data)) {
        other.m_data.fill(0);
    }
    
    SecureArray& operator=(SecureArray&& other) noexcept {
        if (this != &other) {
            // Secure erase current data
            volatile u8* ptr = m_data.data();
            for (usize i = 0; i < N; ++i) {
                ptr[i] = 0;
            }
            m_data = std::move(other.m_data);
            other.m_data.fill(0);
        }
        return *this;
    }
    
    [[nodiscard]] u8* data() noexcept { return m_data.data(); }
    [[nodiscard]] const u8* data() const noexcept { return m_data.data(); }
    [[nodiscard]] constexpr usize size() const noexcept { return N; }
    
    u8& operator[](usize i) { return m_data[i]; }
    const u8& operator[](usize i) const { return m_data[i]; }
    
private:
    std::array<u8, N> m_data;
};

/// Encryption key type
using EncryptionKey = SecureArray<AES_KEY_SIZE>;

/// IV/Nonce type
using InitVector = SecureArray<AES_IV_SIZE>;

/// Hash result type
using HashResult = std::array<u8, SHA256_SIZE>;

/**
 * @brief Secure string that is zeroed on destruction
 */
class SecureString {
public:
    SecureString() = default;
    explicit SecureString(std::string_view str) : m_data(str) {}
    
    ~SecureString() {
        // Secure erase
        if (!m_data.empty()) {
            volatile char* ptr = m_data.data();
            for (usize i = 0; i < m_data.size(); ++i) {
                ptr[i] = 0;
            }
            m_data.clear();
        }
    }
    
    // Non-copyable
    SecureString(const SecureString&) = delete;
    SecureString& operator=(const SecureString&) = delete;
    
    // Move allowed
    SecureString(SecureString&& other) noexcept : m_data(std::move(other.m_data)) {}
    SecureString& operator=(SecureString&& other) noexcept {
        if (this != &other) {
            // Secure erase current
            if (!m_data.empty()) {
                volatile char* ptr = m_data.data();
                for (usize i = 0; i < m_data.size(); ++i) {
                    ptr[i] = 0;
                }
            }
            m_data = std::move(other.m_data);
        }
        return *this;
    }
    
    [[nodiscard]] const std::string& get() const noexcept { return m_data; }
    [[nodiscard]] std::string_view view() const noexcept { return m_data; }
    [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }
    [[nodiscard]] usize size() const noexcept { return m_data.size(); }
    
private:
    std::string m_data;
};

// =============================================================================
// Token Types
// =============================================================================

/**
 * @brief Token type
 */
enum class TokenType : u8 {
    Access,             ///< Short-lived access token
    Refresh,            ///< Long-lived refresh token
    API,                ///< API key token
    Service,            ///< Service-to-service token
    Temporary           ///< One-time use token
};

/**
 * @brief Security token
 */
struct SecurityToken {
    std::string token;
    TokenType type = TokenType::Access;
    std::string userId;
    Permission permissions = Permission::None;
    std::chrono::system_clock::time_point issuedAt;
    std::chrono::system_clock::time_point expiresAt;
    std::string issuer;
    std::string audience;
    std::map<std::string, std::string> claims;
    
    /// Check if token is expired
    [[nodiscard]] bool isExpired() const noexcept {
        return std::chrono::system_clock::now() >= expiresAt;
    }
    
    /// Get remaining time until expiration
    [[nodiscard]] std::chrono::seconds remainingTime() const noexcept {
        auto now = std::chrono::system_clock::now();
        if (now >= expiresAt) return std::chrono::seconds(0);
        return std::chrono::duration_cast<std::chrono::seconds>(expiresAt - now);
    }
    
    /// Check if token has permission
    [[nodiscard]] bool hasPermission(Permission perm) const noexcept {
        return security::hasPermission(permissions, perm);
    }
};

/**
 * @brief Token validation result
 */
struct TokenValidationResult {
    bool valid = false;
    std::string userId;
    Permission permissions = Permission::None;
    std::string errorMessage;
    
    [[nodiscard]] static TokenValidationResult success(
        std::string_view userId, 
        Permission perms) {
        TokenValidationResult result;
        result.valid = true;
        result.userId = std::string(userId);
        result.permissions = perms;
        return result;
    }
    
    [[nodiscard]] static TokenValidationResult failure(std::string_view error) {
        TokenValidationResult result;
        result.valid = false;
        result.errorMessage = std::string(error);
        return result;
    }
};

// =============================================================================
// Cryptography Interface
// =============================================================================

/**
 * @brief Cryptographic operations
 */
class Crypto {
public:
    // =========================================================================
    // Hashing
    // =========================================================================
    
    /**
     * @brief Hash data using SHA-256
     * @param data Data to hash
     * @return Hash result
     */
    [[nodiscard]] static HashResult sha256(const std::vector<u8>& data);
    [[nodiscard]] static HashResult sha256(std::string_view data);
    
    /**
     * @brief Hash password using Argon2id
     * @param password Password to hash
     * @param salt Salt (should be random, at least 16 bytes)
     * @return Hashed password
     */
    [[nodiscard]] static std::vector<u8> hashPassword(
        std::string_view password,
        const std::vector<u8>& salt);
    
    /**
     * @brief Hash password with auto-generated salt (salt is prepended to result)
     * @param password Password to hash
     * @return Salt + Hash combined
     */
    [[nodiscard]] static std::string hashPassword(std::string_view password);
    
    /**
     * @brief Verify password against hash
     * @param password Password to verify
     * @param hash Expected hash
     * @param salt Salt used during hashing
     * @return true if password matches
     */
    [[nodiscard]] static bool verifyPassword(
        std::string_view password,
        const std::vector<u8>& hash,
        const std::vector<u8>& salt);
    
    /**
     * @brief Verify password against combined salt+hash string
     * @param password Password to verify
     * @param saltedHash Combined salt and hash from hashPassword(password)
     * @return true if password matches
     */
    [[nodiscard]] static bool verifyPassword(
        std::string_view password,
        const std::string& saltedHash);
    
    /**
     * @brief Compute HMAC-SHA256
     * @param key HMAC key
     * @param data Data to authenticate
     * @return HMAC result
     */
    [[nodiscard]] static HashResult hmacSha256(
        const std::vector<u8>& key,
        const std::vector<u8>& data);
    
    // =========================================================================
    // Encryption
    // =========================================================================
    
    /**
     * @brief Encrypt data using AES-256-GCM
     * @param plaintext Data to encrypt
     * @param key Encryption key
     * @param iv Initialization vector (should be random)
     * @return Ciphertext with authentication tag
     */
    [[nodiscard]] static Result<std::vector<u8>, Error> encrypt(
        const std::vector<u8>& plaintext,
        const EncryptionKey& key,
        const InitVector& iv);
    
    /**
     * @brief Decrypt data using AES-256-GCM
     * @param ciphertext Data to decrypt (includes auth tag)
     * @param key Decryption key
     * @param iv Initialization vector used during encryption
     * @return Plaintext or error
     */
    [[nodiscard]] static Result<std::vector<u8>, Error> decrypt(
        const std::vector<u8>& ciphertext,
        const EncryptionKey& key,
        const InitVector& iv);
    
    /**
     * @brief Encrypt string
     * @param plaintext String to encrypt
     * @param key Encryption key
     * @return Base64-encoded ciphertext or error
     */
    [[nodiscard]] static Result<std::string, Error> encryptString(
        std::string_view plaintext,
        const EncryptionKey& key);
    
    /**
     * @brief Decrypt string
     * @param ciphertext Base64-encoded ciphertext
     * @param key Decryption key
     * @return Decrypted string or error
     */
    [[nodiscard]] static Result<std::string, Error> decryptString(
        std::string_view ciphertext,
        const EncryptionKey& key);
    
    // =========================================================================
    // Random Generation
    // =========================================================================
    
    /**
     * @brief Generate cryptographically secure random bytes
     * @param count Number of bytes to generate
     * @return Random bytes
     */
    [[nodiscard]] static std::vector<u8> randomBytes(usize count);
    
    /**
     * @brief Generate random encryption key
     * @return Random key
     */
    [[nodiscard]] static EncryptionKey generateKey();
    
    /**
     * @brief Generate random IV
     * @return Random IV
     */
    [[nodiscard]] static InitVector generateIV();
    
    /**
     * @brief Generate random salt for password hashing
     * @return Random salt (16 bytes)
     */
    [[nodiscard]] static std::vector<u8> generateSalt();
    
    /**
     * @brief Generate secure random token string
     * @param length Token length in characters
     * @return Random token string (URL-safe base64)
     */
    [[nodiscard]] static std::string generateTokenString(usize length = 32);
    
    // =========================================================================
    // Encoding
    // =========================================================================
    
    /**
     * @brief Encode bytes to Base64
     * @param data Data to encode
     * @return Base64 string
     */
    [[nodiscard]] static std::string base64Encode(const std::vector<u8>& data);
    
    /**
     * @brief Decode Base64 to bytes
     * @param base64 Base64 string
     * @return Decoded bytes or error
     */
    [[nodiscard]] static Result<std::vector<u8>, Error> base64Decode(std::string_view base64);
    
    /**
     * @brief Encode bytes to hex string
     * @param data Data to encode
     * @return Hex string
     */
    [[nodiscard]] static std::string hexEncode(const std::vector<u8>& data);
    
    /**
     * @brief Decode hex string to bytes
     * @param hex Hex string
     * @return Decoded bytes or error
     */
    [[nodiscard]] static Result<std::vector<u8>, Error> hexDecode(std::string_view hex);
    
    // =========================================================================
    // Utilities
    // =========================================================================
    
    /**
     * @brief Constant-time comparison to prevent timing attacks
     * @param a First buffer
     * @param b Second buffer
     * @return true if equal
     */
    [[nodiscard]] static bool constantTimeCompare(
        const std::vector<u8>& a, 
        const std::vector<u8>& b);
    
    /**
     * @brief Derive key from password using PBKDF2
     * @param password Password
     * @param salt Salt
     * @param iterations Number of iterations
     * @param keyLength Desired key length
     * @return Derived key
     */
    [[nodiscard]] static std::vector<u8> deriveKey(
        std::string_view password,
        const std::vector<u8>& salt,
        u32 iterations = 100000,
        usize keyLength = AES_KEY_SIZE);
};

// =============================================================================
// Token Manager
// =============================================================================

/**
 * @brief JWT/Token management
 */
class TokenManager {
public:
    /**
     * @brief Token manager configuration
     */
    struct Config {
        std::string issuer = "novaforge";
        std::string audience = "novaforge-api";
        u32 accessTokenExpirySeconds = DEFAULT_TOKEN_EXPIRY_SECONDS;
        u32 refreshTokenExpirySeconds = DEFAULT_REFRESH_TOKEN_EXPIRY_SECONDS;
        std::array<u8, AES_KEY_SIZE> signingKey{};  // Use regular array for config
        bool validateExpiration = true;
        bool validateIssuer = true;
        bool validateAudience = true;
    };
    
    explicit TokenManager(const Config& config);
    ~TokenManager();
    
    /**
     * @brief Generate access token
     * @param userId User ID
     * @param permissions User permissions
     * @param claims Additional claims
     * @return Generated token
     */
    [[nodiscard]] Result<SecurityToken, Error> generateAccessToken(
        std::string_view userId,
        Permission permissions,
        const std::map<std::string, std::string>& claims = {});
    
    /**
     * @brief Generate refresh token
     * @param userId User ID
     * @return Generated token
     */
    [[nodiscard]] Result<SecurityToken, Error> generateRefreshToken(std::string_view userId);
    
    /**
     * @brief Generate API key
     * @param userId User ID
     * @param permissions Permissions
     * @param name Key name/description
     * @return Generated API key
     */
    [[nodiscard]] Result<SecurityToken, Error> generateApiKey(
        std::string_view userId,
        Permission permissions,
        std::string_view name);
    
    /**
     * @brief Validate token
     * @param token Token string
     * @return Validation result
     */
    [[nodiscard]] TokenValidationResult validateToken(std::string_view token);
    
    /**
     * @brief Decode token without validation (for inspection)
     * @param token Token string
     * @return Decoded token or error
     */
    [[nodiscard]] Result<SecurityToken, Error> decodeToken(std::string_view token);
    
    /**
     * @brief Refresh access token
     * @param refreshToken Refresh token
     * @return New access token
     */
    [[nodiscard]] Result<SecurityToken, Error> refreshAccessToken(std::string_view refreshToken);
    
    /**
     * @brief Revoke token
     * @param token Token to revoke
     */
    void revokeToken(std::string_view token);
    
    /**
     * @brief Check if token is revoked
     * @param token Token to check
     * @return true if revoked
     */
    [[nodiscard]] bool isTokenRevoked(std::string_view token) const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Rate Limiter
// =============================================================================

/**
 * @brief Rate limiting for API endpoints
 */
class RateLimiter {
public:
    /**
     * @brief Rate limit configuration
     */
    struct Config {
        u32 requestsPerWindow{100};    ///< Max requests per time window
        u32 windowSeconds{60};         ///< Time window in seconds
        u32 burstLimit{10};            ///< Max burst requests
        bool enabled{true};
        
        Config() = default;
    };
    
    explicit RateLimiter(const Config& config);
    RateLimiter(); // Default constructor
    ~RateLimiter();
    
    /**
     * @brief Check if request is allowed
     * @param identifier Client identifier (IP, user ID, etc.)
     * @return true if request is allowed
     */
    [[nodiscard]] bool isAllowed(std::string_view identifier);
    
    /**
     * @brief Record a request
     * @param identifier Client identifier
     */
    void recordRequest(std::string_view identifier);
    
    /**
     * @brief Get remaining requests for identifier
     * @param identifier Client identifier
     * @return Number of remaining requests in current window
     */
    [[nodiscard]] u32 getRemainingRequests(std::string_view identifier) const;
    
    /**
     * @brief Get time until rate limit resets
     * @param identifier Client identifier
     * @return Seconds until reset
     */
    [[nodiscard]] u32 getResetTime(std::string_view identifier) const;
    
    /**
     * @brief Reset rate limit for identifier
     * @param identifier Client identifier
     */
    void reset(std::string_view identifier);
    
    /**
     * @brief Reset all rate limits
     */
    void resetAll();
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Request Signing
// =============================================================================

/**
 * @brief Request signing for API calls
 */
class RequestSigner {
public:
    /**
     * @brief Signed request info
     */
    struct SignedRequest {
        std::string signature;
        std::string timestamp;
        std::string nonce;
        std::map<std::string, std::string> headers;
    };
    
    explicit RequestSigner(const EncryptionKey& signingKey);
    ~RequestSigner();
    
    /**
     * @brief Sign an API request
     * @param method HTTP method (GET, POST, etc.)
     * @param path Request path
     * @param body Request body (empty for GET)
     * @param headers Additional headers to sign
     * @return Signed request info
     */
    [[nodiscard]] SignedRequest signRequest(
        std::string_view method,
        std::string_view path,
        std::string_view body = "",
        const std::map<std::string, std::string>& headers = {});
    
    /**
     * @brief Verify request signature
     * @param method HTTP method
     * @param path Request path
     * @param body Request body
     * @param signature Signature to verify
     * @param timestamp Request timestamp
     * @param nonce Request nonce
     * @param maxAgeSeconds Maximum age of request
     * @return true if signature is valid
     */
    [[nodiscard]] bool verifyRequest(
        std::string_view method,
        std::string_view path,
        std::string_view body,
        std::string_view signature,
        std::string_view timestamp,
        std::string_view nonce,
        u32 maxAgeSeconds = 300);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Secure Storage
// =============================================================================

/**
 * @brief Secure storage for sensitive data
 */
class SecureStorage {
public:
    SecureStorage();
    ~SecureStorage();
    
    /**
     * @brief Store encrypted value
     * @param key Storage key
     * @param value Value to store
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void, Error> store(std::string_view key, std::string_view value);
    
    /**
     * @brief Retrieve encrypted value
     * @param key Storage key
     * @return Decrypted value or error
     */
    [[nodiscard]] Result<SecureString, Error> retrieve(std::string_view key);
    
    /**
     * @brief Delete stored value
     * @param key Storage key
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void, Error> remove(std::string_view key);
    
    /**
     * @brief Check if key exists
     * @param key Storage key
     * @return true if key exists
     */
    [[nodiscard]] bool exists(std::string_view key) const;
    
    /**
     * @brief Clear all stored values
     */
    void clear();
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Input Validation
// =============================================================================

/**
 * @brief Input validation utilities
 */
class InputValidator {
public:
    /**
     * @brief Validate email format
     * @param email Email to validate
     * @return true if valid
     */
    [[nodiscard]] static bool isValidEmail(std::string_view email);
    
    /**
     * @brief Check if password meets minimum requirements
     * @param password Password to check
     * @return true if password is valid
     */
    [[nodiscard]] static bool isValidPassword(std::string_view password);
    
    /**
     * @brief Validate password strength
     * @param password Password to validate
     * @return Validation result with message
     */
    struct PasswordValidation {
        bool valid;
        std::string message;
        u8 strength; // 0-100
    };
    [[nodiscard]] static PasswordValidation validatePassword(std::string_view password);
    
    /**
     * @brief Validate username
     * @param username Username to validate
     * @return true if valid
     */
    [[nodiscard]] static bool isValidUsername(std::string_view username);
    
    /**
     * @brief Sanitize string input (prevent injection)
     * @param input Input string
     * @return Sanitized string
     */
    [[nodiscard]] static std::string sanitize(std::string_view input);
    
    /**
     * @brief Sanitize for HTML output
     * @param input Input string
     * @return HTML-safe string
     */
    [[nodiscard]] static std::string sanitizeHtml(std::string_view input);
    
    /**
     * @brief Sanitize for SQL (if not using parameterized queries)
     * @param input Input string
     * @return SQL-safe string
     */
    [[nodiscard]] static std::string sanitizeSql(std::string_view input);
    
    /**
     * @brief Validate URL format
     * @param url URL to validate
     * @return true if valid
     */
    [[nodiscard]] static bool isValidUrl(std::string_view url);
    
    /**
     * @brief Validate UUID format
     * @param uuid UUID to validate
     * @return true if valid
     */
    [[nodiscard]] static bool isValidUuid(std::string_view uuid);
};

// =============================================================================
// Security Context
// =============================================================================

/**
 * @brief Security context for a request/session
 */
struct SecurityContext {
    bool authenticated = false;
    std::string userId;
    Permission permissions = Permission::None;
    std::string clientIp;
    std::string userAgent;
    std::string sessionId;
    SecurityToken token;
    
    /// Check if has permission
    [[nodiscard]] bool hasPermission(Permission perm) const noexcept {
        return authenticated && security::hasPermission(permissions, perm);
    }
    
    /// Require permission (throws if not present)
    [[nodiscard]] Result<void, Error> requirePermission(Permission perm) const {
        if (!authenticated) {
            return std::unexpected(Error(
                ErrorCategory::Permission, 
                1, 
                "Authentication required"));
        }
        if (!security::hasPermission(permissions, perm)) {
            return std::unexpected(Error(
                ErrorCategory::Permission, 
                2, 
                "Insufficient permissions"));
        }
        return {};
    }
};

// =============================================================================
// Security Manager (Main Interface)
// =============================================================================

/**
 * @brief Main security manager for the API
 */
class SecurityManager {
public:
    /**
     * @brief Security configuration
     */
    struct Config {
        SecurityLevel level{SecurityLevel::High};
        std::string issuer{"novaforge"};
        u32 tokenExpirySeconds{DEFAULT_TOKEN_EXPIRY_SECONDS};
        u32 maxLoginAttempts{MAX_LOGIN_ATTEMPTS};
        u32 lockoutDurationSeconds{LOCKOUT_DURATION_SECONDS};
        bool enableRateLimiting{true};
        bool enableRequestSigning{true};
        RateLimiter::Config rateLimitConfig{};
        
        Config() = default;
    };
    
    /// Get singleton instance
    [[nodiscard]] static SecurityManager& get();
    
    /**
     * @brief Initialize security manager
     * @param config Security configuration
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void, Error> initialize(const Config& config);
    [[nodiscard]] Result<void, Error> initialize(); // Default config
    
    /**
     * @brief Shutdown security manager
     */
    void shutdown();
    
    /**
     * @brief Get token manager
     * @return Reference to token manager
     */
    [[nodiscard]] TokenManager& getTokenManager();
    
    /**
     * @brief Get rate limiter
     * @return Reference to rate limiter
     */
    [[nodiscard]] RateLimiter& getRateLimiter();
    
    /**
     * @brief Get secure storage
     * @return Reference to secure storage
     */
    [[nodiscard]] SecureStorage& getSecureStorage();
    
    /**
     * @brief Authenticate user
     * @param username Username or email
     * @param password Password
     * @return Security token or error
     */
    [[nodiscard]] Result<SecurityToken, Error> authenticate(
        std::string_view username,
        std::string_view password);
    
    /**
     * @brief Validate request
     * @param token Authorization token
     * @param requiredPermission Required permission
     * @return Security context or error
     */
    [[nodiscard]] Result<SecurityContext, Error> validateRequest(
        std::string_view token,
        Permission requiredPermission = Permission::None);
    
    /**
     * @brief Check rate limit
     * @param identifier Client identifier
     * @return true if request is allowed
     */
    [[nodiscard]] bool checkRateLimit(std::string_view identifier);
    
    /**
     * @brief Record failed login attempt
     * @param identifier User identifier
     */
    void recordFailedLogin(std::string_view identifier);
    
    /**
     * @brief Check if account is locked
     * @param identifier User identifier
     * @return true if account is locked
     */
    [[nodiscard]] bool isAccountLocked(std::string_view identifier) const;
    
private:
    SecurityManager();
    ~SecurityManager();
    
    SecurityManager(const SecurityManager&) = delete;
    SecurityManager& operator=(const SecurityManager&) = delete;
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nova::api::security
