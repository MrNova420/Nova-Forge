// =============================================================================
// NovaForge Platform - API Tests
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// Comprehensive tests for the NovaForge API system including security,
// platform services, and engine integration.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <nova/api/api.hpp>
#include <nova/api/api_security.hpp>

using namespace nova;
using namespace nova::api;
using namespace nova::api::security;

// =============================================================================
// Crypto Tests
// =============================================================================

TEST_CASE("Crypto - Random byte generation", "[api][security][crypto]") {
    SECTION("Generate random bytes with correct size") {
        auto bytes = Crypto::randomBytes(32);
        REQUIRE(bytes.size() == 32);
    }
    
    SECTION("Random bytes are different each time") {
        auto bytes1 = Crypto::randomBytes(32);
        auto bytes2 = Crypto::randomBytes(32);
        
        // Should be extremely unlikely to be equal
        bool allEqual = true;
        for (usize i = 0; i < bytes1.size(); ++i) {
            if (bytes1[i] != bytes2[i]) {
                allEqual = false;
                break;
            }
        }
        REQUIRE_FALSE(allEqual);
    }
    
    SECTION("Generate token string") {
        auto token = Crypto::generateTokenString(32);
        REQUIRE(token.size() == 32);
        
        // All characters should be URL-safe
        for (char c : token) {
            bool valid = (c >= 'A' && c <= 'Z') || 
                        (c >= 'a' && c <= 'z') || 
                        (c >= '0' && c <= '9') || 
                        c == '-' || c == '_';
            REQUIRE(valid);
        }
    }
}

TEST_CASE("Crypto - Base64 encoding/decoding", "[api][security][crypto]") {
    SECTION("Encode and decode round trip") {
        std::vector<u8> original = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
        
        std::string encoded = Crypto::base64Encode(original);
        REQUIRE_FALSE(encoded.empty());
        
        auto decoded = Crypto::base64Decode(encoded);
        REQUIRE(decoded.has_value());
        REQUIRE(decoded->size() == original.size());
        
        for (usize i = 0; i < original.size(); ++i) {
            REQUIRE((*decoded)[i] == original[i]);
        }
    }
    
    SECTION("Known base64 value") {
        std::vector<u8> data = {'M', 'a', 'n'};
        std::string encoded = Crypto::base64Encode(data);
        REQUIRE(encoded == "TWFu");
    }
    
    SECTION("Empty data") {
        std::vector<u8> empty;
        std::string encoded = Crypto::base64Encode(empty);
        REQUIRE(encoded.empty());
    }
}

TEST_CASE("Crypto - Hex encoding/decoding", "[api][security][crypto]") {
    SECTION("Encode and decode round trip") {
        std::vector<u8> original = {0xDE, 0xAD, 0xBE, 0xEF};
        
        std::string encoded = Crypto::hexEncode(original);
        REQUIRE(encoded == "deadbeef");
        
        auto decoded = Crypto::hexDecode(encoded);
        REQUIRE(decoded.has_value());
        REQUIRE(decoded->size() == original.size());
        
        for (usize i = 0; i < original.size(); ++i) {
            REQUIRE((*decoded)[i] == original[i]);
        }
    }
    
    SECTION("Invalid hex string") {
        auto result = Crypto::hexDecode("deadbeefg"); // Odd length
        REQUIRE_FALSE(result.has_value());
        
        auto result2 = Crypto::hexDecode("zzzz"); // Invalid chars
        REQUIRE_FALSE(result2.has_value());
    }
}

TEST_CASE("Crypto - SHA-256 hashing", "[api][security][crypto]") {
    SECTION("Hash produces consistent results") {
        std::string data = "Hello, World!";
        
        auto hash1 = Crypto::sha256(data);
        auto hash2 = Crypto::sha256(data);
        
        for (usize i = 0; i < hash1.size(); ++i) {
            REQUIRE(hash1[i] == hash2[i]);
        }
    }
    
    SECTION("Different inputs produce different hashes") {
        auto hash1 = Crypto::sha256("Hello");
        auto hash2 = Crypto::sha256("World");
        
        bool allEqual = true;
        for (usize i = 0; i < hash1.size(); ++i) {
            if (hash1[i] != hash2[i]) {
                allEqual = false;
                break;
            }
        }
        REQUIRE_FALSE(allEqual);
    }
}

TEST_CASE("Crypto - Password hashing", "[api][security][crypto]") {
    SECTION("Hash and verify password") {
        std::string password = "SecureP@ssw0rd!";
        auto salt = Crypto::generateSalt();
        
        auto hash = Crypto::hashPassword(password, salt);
        REQUIRE_FALSE(hash.empty());
        
        // Verify correct password
        bool valid = Crypto::verifyPassword(password, hash, salt);
        REQUIRE(valid);
        
        // Verify incorrect password
        bool invalid = Crypto::verifyPassword("WrongPassword", hash, salt);
        REQUIRE_FALSE(invalid);
    }
    
    SECTION("Different salts produce different hashes") {
        std::string password = "TestPassword123";
        auto salt1 = Crypto::generateSalt();
        auto salt2 = Crypto::generateSalt();
        
        auto hash1 = Crypto::hashPassword(password, salt1);
        auto hash2 = Crypto::hashPassword(password, salt2);
        
        REQUIRE_FALSE(Crypto::constantTimeCompare(hash1, hash2));
    }
}

TEST_CASE("Crypto - Encryption/Decryption", "[api][security][crypto]") {
    SECTION("Encrypt and decrypt round trip") {
        std::vector<u8> plaintext = {'S', 'e', 'c', 'r', 'e', 't', '!'};
        EncryptionKey key = Crypto::generateKey();
        InitVector iv = Crypto::generateIV();
        
        auto encrypted = Crypto::encrypt(plaintext, key, iv);
        REQUIRE(encrypted.has_value());
        REQUIRE(encrypted->size() > plaintext.size()); // Includes auth tag
        
        auto decrypted = Crypto::decrypt(*encrypted, key, iv);
        REQUIRE(decrypted.has_value());
        REQUIRE(decrypted->size() == plaintext.size());
        
        for (usize i = 0; i < plaintext.size(); ++i) {
            REQUIRE((*decrypted)[i] == plaintext[i]);
        }
    }
    
    SECTION("String encryption round trip") {
        std::string plaintext = "This is a secret message!";
        EncryptionKey key = Crypto::generateKey();
        
        auto encrypted = Crypto::encryptString(plaintext, key);
        REQUIRE(encrypted.has_value());
        REQUIRE_FALSE(encrypted->empty());
        
        auto decrypted = Crypto::decryptString(*encrypted, key);
        REQUIRE(decrypted.has_value());
        REQUIRE(*decrypted == plaintext);
    }
}

TEST_CASE("Crypto - Constant time compare", "[api][security][crypto]") {
    SECTION("Equal vectors") {
        std::vector<u8> a = {1, 2, 3, 4, 5};
        std::vector<u8> b = {1, 2, 3, 4, 5};
        REQUIRE(Crypto::constantTimeCompare(a, b));
    }
    
    SECTION("Unequal vectors same size") {
        std::vector<u8> a = {1, 2, 3, 4, 5};
        std::vector<u8> b = {1, 2, 3, 4, 6};
        REQUIRE_FALSE(Crypto::constantTimeCompare(a, b));
    }
    
    SECTION("Different sizes") {
        std::vector<u8> a = {1, 2, 3};
        std::vector<u8> b = {1, 2, 3, 4};
        REQUIRE_FALSE(Crypto::constantTimeCompare(a, b));
    }
}

// =============================================================================
// Input Validator Tests
// =============================================================================

TEST_CASE("InputValidator - Email validation", "[api][security][validation]") {
    SECTION("Valid emails") {
        REQUIRE(InputValidator::isValidEmail("user@example.com"));
        REQUIRE(InputValidator::isValidEmail("user.name@domain.co.uk"));
        REQUIRE(InputValidator::isValidEmail("user+tag@example.org"));
    }
    
    SECTION("Invalid emails") {
        REQUIRE_FALSE(InputValidator::isValidEmail(""));
        REQUIRE_FALSE(InputValidator::isValidEmail("userexample.com"));
        REQUIRE_FALSE(InputValidator::isValidEmail("@example.com"));
        REQUIRE_FALSE(InputValidator::isValidEmail("user@"));
        REQUIRE_FALSE(InputValidator::isValidEmail("user@domain"));
    }
}

TEST_CASE("InputValidator - Password validation", "[api][security][validation]") {
    SECTION("Strong password") {
        auto result = InputValidator::validatePassword("MyStr0ng!Pass");
        REQUIRE(result.valid);
        REQUIRE(result.strength >= 60);
    }
    
    SECTION("Weak password - too short") {
        auto result = InputValidator::validatePassword("short");
        REQUIRE_FALSE(result.valid);
    }
    
    SECTION("Password strength scoring") {
        // Simple password
        auto weak = InputValidator::validatePassword("password");
        
        // Complex password
        auto strong = InputValidator::validatePassword("C0mpl3x!P@ssw0rd");
        
        REQUIRE(strong.strength > weak.strength);
    }
}

TEST_CASE("InputValidator - Username validation", "[api][security][validation]") {
    SECTION("Valid usernames") {
        REQUIRE(InputValidator::isValidUsername("user123"));
        REQUIRE(InputValidator::isValidUsername("John_Doe"));
        REQUIRE(InputValidator::isValidUsername("player-one"));
    }
    
    SECTION("Invalid usernames") {
        REQUIRE_FALSE(InputValidator::isValidUsername("")); // Empty
        REQUIRE_FALSE(InputValidator::isValidUsername("ab")); // Too short
        REQUIRE_FALSE(InputValidator::isValidUsername("123user")); // Starts with number
        REQUIRE_FALSE(InputValidator::isValidUsername("user@name")); // Invalid char
    }
}

TEST_CASE("InputValidator - HTML sanitization tags", "[api][security][validation]") {
    std::string input = "<script>alert('XSS')</script>";
    std::string sanitized = InputValidator::sanitizeHtml(input);
    
    REQUIRE(sanitized.find('<') == std::string::npos);
    REQUIRE(sanitized.find('>') == std::string::npos);
    REQUIRE(sanitized.find("&lt;") != std::string::npos);
    REQUIRE(sanitized.find("&gt;") != std::string::npos);
}

TEST_CASE("InputValidator - HTML sanitization quotes", "[api][security][validation]") {
    std::string input = "onclick=\"alert('test')\"";
    std::string sanitized = InputValidator::sanitizeHtml(input);
    
    // Check that double quotes are escaped
    REQUIRE(sanitized.find("&quot;") != std::string::npos);
}

TEST_CASE("InputValidator - UUID validation", "[api][security][validation]") {
    SECTION("Valid UUIDs") {
        REQUIRE(InputValidator::isValidUuid("550e8400-e29b-41d4-a716-446655440000"));
        REQUIRE(InputValidator::isValidUuid("123e4567-e89b-12d3-a456-426614174000"));
    }
    
    SECTION("Invalid UUIDs") {
        REQUIRE_FALSE(InputValidator::isValidUuid("")); // Empty
        REQUIRE_FALSE(InputValidator::isValidUuid("not-a-uuid")); // Wrong format
        REQUIRE_FALSE(InputValidator::isValidUuid("550e8400-e29b-41d4-a716-44665544000")); // Too short
        REQUIRE_FALSE(InputValidator::isValidUuid("550e8400-e29b-41d4-a716-4466554400000")); // Too long
        REQUIRE_FALSE(InputValidator::isValidUuid("550e8400xe29b-41d4-a716-446655440000")); // Wrong separator
    }
}

// =============================================================================
// Token Manager Tests
// =============================================================================

TEST_CASE("TokenManager - Token generation", "[api][security][token]") {
    TokenManager::Config config;
    config.issuer = "test-issuer";
    config.accessTokenExpirySeconds = 3600;
    TokenManager manager(config);
    
    SECTION("Generate access token") {
        auto result = manager.generateAccessToken("user123", Permission::ReadWrite);
        REQUIRE(result.has_value());
        
        auto& token = *result;
        REQUIRE_FALSE(token.token.empty());
        REQUIRE(token.type == TokenType::Access);
        REQUIRE(token.userId == "user123");
        REQUIRE(token.hasPermission(Permission::Read));
        REQUIRE(token.hasPermission(Permission::Write));
        REQUIRE_FALSE(token.isExpired());
    }
    
    SECTION("Generate refresh token") {
        auto result = manager.generateRefreshToken("user123");
        REQUIRE(result.has_value());
        
        auto& token = *result;
        REQUIRE_FALSE(token.token.empty());
        REQUIRE(token.type == TokenType::Refresh);
        REQUIRE(token.userId == "user123");
    }
    
    SECTION("Generate API key") {
        auto result = manager.generateApiKey("user123", Permission::Read, "Test API Key");
        REQUIRE(result.has_value());
        
        auto& token = *result;
        REQUIRE(token.token.substr(0, 5) == "nova_");
        REQUIRE(token.type == TokenType::API);
    }
    
    SECTION("Token revocation") {
        auto result = manager.generateAccessToken("user123", Permission::Read);
        REQUIRE(result.has_value());
        
        std::string tokenStr = result->token;
        
        REQUIRE_FALSE(manager.isTokenRevoked(tokenStr));
        
        manager.revokeToken(tokenStr);
        
        REQUIRE(manager.isTokenRevoked(tokenStr));
    }
}

// =============================================================================
// Rate Limiter Tests
// =============================================================================

TEST_CASE("RateLimiter - Basic rate limiting", "[api][security][ratelimit]") {
    RateLimiter::Config config;
    config.requestsPerWindow = 5;
    config.windowSeconds = 60;
    config.enabled = true;
    RateLimiter limiter(config);
    
    SECTION("Allow requests under limit") {
        std::string clientId = "client1";
        
        for (int i = 0; i < 5; ++i) {
            REQUIRE(limiter.isAllowed(clientId));
            limiter.recordRequest(clientId);
        }
    }
    
    SECTION("Block requests over limit") {
        std::string clientId = "client2";
        
        // Use up all requests
        for (int i = 0; i < 5; ++i) {
            limiter.recordRequest(clientId);
        }
        
        // Should be blocked
        REQUIRE_FALSE(limiter.isAllowed(clientId));
    }
    
    SECTION("Reset clears limit") {
        std::string clientId = "client3";
        
        // Use up all requests
        for (int i = 0; i < 5; ++i) {
            limiter.recordRequest(clientId);
        }
        
        REQUIRE_FALSE(limiter.isAllowed(clientId));
        
        limiter.reset(clientId);
        
        REQUIRE(limiter.isAllowed(clientId));
    }
    
    SECTION("Different clients are independent") {
        std::string client1 = "clientA";
        std::string client2 = "clientB";
        
        // Use up client1's requests
        for (int i = 0; i < 5; ++i) {
            limiter.recordRequest(client1);
        }
        
        // Client2 should still be allowed
        REQUIRE(limiter.isAllowed(client2));
    }
    
    SECTION("Remaining requests count") {
        std::string clientId = "client4";
        
        REQUIRE(limiter.getRemainingRequests(clientId) == 5);
        
        limiter.recordRequest(clientId);
        limiter.recordRequest(clientId);
        
        REQUIRE(limiter.getRemainingRequests(clientId) == 3);
    }
}

// =============================================================================
// Secure Storage Tests
// =============================================================================

TEST_CASE("SecureStorage - Store and retrieve", "[api][security][storage]") {
    SecureStorage storage;
    
    SECTION("Store and retrieve value") {
        auto storeResult = storage.store("test_key", "secret_value");
        REQUIRE(storeResult.has_value());
        
        REQUIRE(storage.exists("test_key"));
        
        auto retrieveResult = storage.retrieve("test_key");
        REQUIRE(retrieveResult.has_value());
        REQUIRE(retrieveResult->view() == "secret_value");
    }
    
    SECTION("Non-existent key") {
        REQUIRE_FALSE(storage.exists("nonexistent"));
        
        auto result = storage.retrieve("nonexistent");
        REQUIRE_FALSE(result.has_value());
    }
    
    SECTION("Remove key") {
        (void)storage.store("to_remove", "value");
        REQUIRE(storage.exists("to_remove"));
        
        auto removeResult = storage.remove("to_remove");
        REQUIRE(removeResult.has_value());
        
        REQUIRE_FALSE(storage.exists("to_remove"));
    }
    
    SECTION("Clear all") {
        (void)storage.store("key1", "value1");
        (void)storage.store("key2", "value2");
        
        storage.clear();
        
        REQUIRE_FALSE(storage.exists("key1"));
        REQUIRE_FALSE(storage.exists("key2"));
    }
}

// =============================================================================
// Security Context Tests
// =============================================================================

TEST_CASE("SecurityContext - Permission checks", "[api][security][context]") {
    SECTION("Authenticated with permissions") {
        SecurityContext ctx;
        ctx.authenticated = true;
        ctx.userId = "user123";
        ctx.permissions = Permission::ReadWrite;
        
        REQUIRE(ctx.hasPermission(Permission::Read));
        REQUIRE(ctx.hasPermission(Permission::Write));
        REQUIRE_FALSE(ctx.hasPermission(Permission::Admin));
    }
    
    SECTION("Unauthenticated has no permissions") {
        SecurityContext ctx;
        ctx.authenticated = false;
        ctx.permissions = Permission::ReadWrite;
        
        // Even with permissions set, unauthenticated should fail
        REQUIRE_FALSE(ctx.hasPermission(Permission::Read));
    }
    
    SECTION("Require permission - success") {
        SecurityContext ctx;
        ctx.authenticated = true;
        ctx.permissions = Permission::Read;
        
        auto result = ctx.requirePermission(Permission::Read);
        REQUIRE(result.has_value());
    }
    
    SECTION("Require permission - failure") {
        SecurityContext ctx;
        ctx.authenticated = true;
        ctx.permissions = Permission::Read;
        
        auto result = ctx.requirePermission(Permission::Admin);
        REQUIRE_FALSE(result.has_value());
    }
}

// =============================================================================
// API Types Tests
// =============================================================================

TEST_CASE("API Types - ResourceId", "[api][types]") {
    SECTION("Valid resource ID") {
        ResourceId id;
        id.id = 12345;
        id.path = "/assets/texture.png";
        
        REQUIRE(id.isValid());
    }
    
    SECTION("Invalid resource ID") {
        ResourceId id = ResourceId::invalid();
        REQUIRE_FALSE(id.isValid());
    }
    
    SECTION("Equality comparison") {
        ResourceId id1{123, "/path"};
        ResourceId id2{123, "/path"};
        ResourceId id3{456, "/other"};
        
        REQUIRE(id1 == id2);
        REQUIRE(id1 != id3);
    }
}

TEST_CASE("API Types - UserId", "[api][types]") {
    SECTION("Valid user ID") {
        UserId id;
        id.id = "user_123";
        
        REQUIRE(id.isValid());
    }
    
    SECTION("Invalid user ID") {
        UserId id = UserId::invalid();
        REQUIRE_FALSE(id.isValid());
    }
}

TEST_CASE("API Types - ApiVersion", "[api][types]") {
    ApiVersion version{1, 2, 3};
    
    SECTION("To string") {
        REQUIRE(version.toString() == "1.2.3");
    }
    
    SECTION("Packed version") {
        u32 packed = version.packed();
        REQUIRE(packed == ((1 << 16) | (2 << 8) | 3));
    }
}

TEST_CASE("API Types - ApiEvent", "[api][types]") {
    auto event = ApiEvent::create(ApiEventType::Initialized, "Test message");
    
    REQUIRE(event.type == ApiEventType::Initialized);
    REQUIRE(event.message == "Test message");
    // Timestamp should be close to now
}

// =============================================================================
// API Config Tests
// =============================================================================

TEST_CASE("ApiConfig - Presets", "[api][config]") {
    SECTION("Default config") {
        auto config = ApiConfig::defaults();
        REQUIRE(config.debugMode == false);
        REQUIRE(config.enableAnalytics == true);
    }
    
    SECTION("Debug config") {
        auto config = ApiConfig::debug();
        REQUIRE(config.debugMode == true);
        REQUIRE(config.logLevel == ApiLogLevel::Debug);
    }
    
    SECTION("Release config") {
        auto config = ApiConfig::release();
        REQUIRE(config.debugMode == false);
        REQUIRE(config.logLevel == ApiLogLevel::Warning);
    }
}

// =============================================================================
// API Platform Info Tests
// =============================================================================

TEST_CASE("API Types - ApiPlatformInfo", "[api][types]") {
    SECTION("Default values") {
        ApiPlatformInfo info;
        REQUIRE(info.osName.empty());
        REQUIRE(info.osVersion.empty());
        REQUIRE(info.architecture.empty());
        REQUIRE(info.is64Bit == true);
        REQUIRE(info.cpuCores == 1);
        REQUIRE(info.cpuThreads == 1);
        REQUIRE(info.totalMemoryMB == 0);
        REQUIRE(info.hasGpu == false);
        REQUIRE(info.supportsMultithreading == true);
    }
    
    SECTION("Populated platform info") {
        ApiPlatformInfo info;
        info.osName = "Linux";
        info.osVersion = "6.5.0";
        info.architecture = "x86_64";
        info.cpuName = "AMD Ryzen 9 5900X";
        info.cpuVendor = "AMD";
        info.cpuCores = 12;
        info.cpuThreads = 24;
        info.cpuHasAVX2 = true;
        info.totalMemoryMB = 32768;
        info.hasGpu = true;
        info.gpuName = "NVIDIA RTX 4090";
        info.gpuSupportsVulkan = true;
        
        REQUIRE(info.osName == "Linux");
        REQUIRE(info.cpuCores == 12);
        REQUIRE(info.cpuThreads == 24);
        REQUIRE(info.cpuHasAVX2 == true);
        REQUIRE(info.hasGpu == true);
        REQUIRE(info.gpuSupportsVulkan == true);
    }
}

// =============================================================================
// API Progress Tests
// =============================================================================

TEST_CASE("API Types - ApiProgress", "[api][types]") {
    SECTION("Progress not complete") {
        ApiProgress progress;
        progress.percentage = 50.0f;
        progress.bytesCompleted = 500;
        progress.bytesTotal = 1000;
        progress.status = "Downloading...";
        
        REQUIRE_FALSE(progress.isComplete());
        REQUIRE(progress.percentage == 50.0f);
        REQUIRE(progress.bytesCompleted == 500);
        REQUIRE(progress.bytesTotal == 1000);
    }
    
    SECTION("Progress complete") {
        ApiProgress progress;
        progress.percentage = 100.0f;
        progress.bytesCompleted = 1000;
        progress.bytesTotal = 1000;
        progress.status = "Complete";
        
        REQUIRE(progress.isComplete());
    }
    
    SECTION("Progress over 100% still complete") {
        ApiProgress progress;
        progress.percentage = 105.0f;
        
        REQUIRE(progress.isComplete());
    }
}

// =============================================================================
// API Error Code Tests
// =============================================================================

TEST_CASE("API Types - ApiErrorCode", "[api][types]") {
    SECTION("Success code") {
        REQUIRE(static_cast<u32>(ApiErrorCode::Success) == 0);
    }
    
    SECTION("Initialization errors range") {
        REQUIRE(static_cast<u32>(ApiErrorCode::NotInitialized) == 1);
        REQUIRE(static_cast<u32>(ApiErrorCode::AlreadyInitialized) == 2);
        REQUIRE(static_cast<u32>(ApiErrorCode::InitializationFailed) == 3);
    }
    
    SECTION("Authentication errors range") {
        REQUIRE(static_cast<u32>(ApiErrorCode::AuthenticationRequired) == 100);
        REQUIRE(static_cast<u32>(ApiErrorCode::AuthenticationFailed) == 101);
        REQUIRE(static_cast<u32>(ApiErrorCode::SessionExpired) == 102);
        REQUIRE(static_cast<u32>(ApiErrorCode::InvalidCredentials) == 103);
        REQUIRE(static_cast<u32>(ApiErrorCode::AccountLocked) == 104);
        REQUIRE(static_cast<u32>(ApiErrorCode::AccountSuspended) == 105);
        REQUIRE(static_cast<u32>(ApiErrorCode::TokenExpired) == 106);
    }
    
    SECTION("Network errors range") {
        REQUIRE(static_cast<u32>(ApiErrorCode::NetworkUnavailable) == 200);
        REQUIRE(static_cast<u32>(ApiErrorCode::ConnectionFailed) == 201);
        REQUIRE(static_cast<u32>(ApiErrorCode::ConnectionTimeout) == 202);
        REQUIRE(static_cast<u32>(ApiErrorCode::ServerUnavailable) == 203);
    }
    
    SECTION("Resource errors range") {
        REQUIRE(static_cast<u32>(ApiErrorCode::ResourceNotFound) == 300);
        REQUIRE(static_cast<u32>(ApiErrorCode::ResourceLoadFailed) == 301);
        REQUIRE(static_cast<u32>(ApiErrorCode::ResourceInvalid) == 302);
        REQUIRE(static_cast<u32>(ApiErrorCode::ResourceAccessDenied) == 303);
    }
    
    SECTION("Operation errors range") {
        REQUIRE(static_cast<u32>(ApiErrorCode::OperationFailed) == 400);
        REQUIRE(static_cast<u32>(ApiErrorCode::OperationCancelled) == 401);
        REQUIRE(static_cast<u32>(ApiErrorCode::OperationTimeout) == 402);
        REQUIRE(static_cast<u32>(ApiErrorCode::InvalidParameter) == 403);
    }
    
    SECTION("Platform errors range") {
        REQUIRE(static_cast<u32>(ApiErrorCode::PlatformError) == 500);
        REQUIRE(static_cast<u32>(ApiErrorCode::FeatureNotSupported) == 501);
        REQUIRE(static_cast<u32>(ApiErrorCode::PermissionDenied) == 502);
        REQUIRE(static_cast<u32>(ApiErrorCode::StorageQuotaExceeded) == 503);
    }
    
    SECTION("Internal errors range") {
        REQUIRE(static_cast<u32>(ApiErrorCode::InternalError) == 900);
        REQUIRE(static_cast<u32>(ApiErrorCode::OutOfMemory) == 901);
        REQUIRE(static_cast<u32>(ApiErrorCode::UnknownError) == 999);
    }
}

// =============================================================================
// API Resource State Tests
// =============================================================================

TEST_CASE("API Types - ResourceState", "[api][types]") {
    SECTION("All states are defined") {
        REQUIRE(static_cast<u8>(ResourceState::Unloaded) == 0);
        REQUIRE(static_cast<u8>(ResourceState::Loading) == 1);
        REQUIRE(static_cast<u8>(ResourceState::Loaded) == 2);
        REQUIRE(static_cast<u8>(ResourceState::Failed) == 3);
    }
}

// =============================================================================
// API Log Level Tests
// =============================================================================

TEST_CASE("API Types - ApiLogLevel", "[api][types]") {
    SECTION("All log levels are defined") {
        REQUIRE(static_cast<u8>(ApiLogLevel::Trace) == 0);
        REQUIRE(static_cast<u8>(ApiLogLevel::Debug) == 1);
        REQUIRE(static_cast<u8>(ApiLogLevel::Info) == 2);
        REQUIRE(static_cast<u8>(ApiLogLevel::Warning) == 3);
        REQUIRE(static_cast<u8>(ApiLogLevel::Error) == 4);
        REQUIRE(static_cast<u8>(ApiLogLevel::Fatal) == 5);
        REQUIRE(static_cast<u8>(ApiLogLevel::Off) == 6);
    }
}

// =============================================================================
// API Event Type Tests
// =============================================================================

TEST_CASE("API Types - ApiEventType", "[api][types]") {
    SECTION("Lifecycle events") {
        REQUIRE(static_cast<u32>(ApiEventType::Initialized) == 0);
        REQUIRE(static_cast<u32>(ApiEventType::ShuttingDown) == 1);
    }
    
    SECTION("Engine events") {
        REQUIRE(static_cast<u32>(ApiEventType::EngineStarted) == 100);
        REQUIRE(static_cast<u32>(ApiEventType::EngineStopped) == 101);
        REQUIRE(static_cast<u32>(ApiEventType::FrameBegin) == 102);
        REQUIRE(static_cast<u32>(ApiEventType::FrameEnd) == 103);
    }
    
    SECTION("Platform events") {
        REQUIRE(static_cast<u32>(ApiEventType::UserLoggedIn) == 200);
        REQUIRE(static_cast<u32>(ApiEventType::UserLoggedOut) == 201);
        REQUIRE(static_cast<u32>(ApiEventType::ConnectionChanged) == 202);
    }
    
    SECTION("Service events") {
        REQUIRE(static_cast<u32>(ApiEventType::CloudSyncStarted) == 300);
        REQUIRE(static_cast<u32>(ApiEventType::CloudSyncCompleted) == 301);
        REQUIRE(static_cast<u32>(ApiEventType::CloudSyncFailed) == 302);
    }
    
    SECTION("Error events") {
        REQUIRE(static_cast<u32>(ApiEventType::ErrorOccurred) == 900);
        REQUIRE(static_cast<u32>(ApiEventType::WarningOccurred) == 901);
    }
    
    SECTION("Custom events start value") {
        REQUIRE(static_cast<u32>(ApiEventType::Custom) == 10000);
    }
}

// =============================================================================
// API User Profile Tests
// =============================================================================

TEST_CASE("API Types - UserProfile", "[api][types]") {
    SECTION("Default values") {
        UserProfile profile;
        REQUIRE_FALSE(profile.userId.isValid());
        REQUIRE(profile.username.empty());
        REQUIRE(profile.displayName.empty());
        REQUIRE(profile.isVerified == false);
        REQUIRE(profile.isPremium == false);
    }
    
    SECTION("Populated profile") {
        UserProfile profile;
        profile.userId.id = "user_12345";
        profile.username = "johndoe";
        profile.displayName = "John Doe";
        profile.email = "john@example.com";
        profile.avatarUrl = "https://example.com/avatar.png";
        profile.bio = "Game developer";
        profile.isVerified = true;
        profile.isPremium = true;
        
        REQUIRE(profile.userId.isValid());
        REQUIRE(profile.username == "johndoe");
        REQUIRE(profile.displayName == "John Doe");
        REQUIRE(profile.isVerified == true);
        REQUIRE(profile.isPremium == true);
    }
}

// =============================================================================
// API Config Comprehensive Tests
// =============================================================================

TEST_CASE("API Types - ApiConfig Comprehensive", "[api][types]") {
    SECTION("Default configuration values") {
        ApiConfig config;
        REQUIRE(config.appName == "NovaForge Application");
        REQUIRE(config.appVersion == "1.0.0");
        REQUIRE(config.debugMode == false);
        REQUIRE(config.logLevel == ApiLogLevel::Info);
        REQUIRE(config.enableAnalytics == true);
        REQUIRE(config.enableCrashReporting == true);
        REQUIRE(config.dataDirectory.empty());
        REQUIRE(config.cacheDirectory.empty());
        REQUIRE(config.maxMemoryMB == 0);
        REQUIRE(config.enableThreading == true);
        REQUIRE(config.workerThreads == 0);
    }
    
    SECTION("Custom configuration") {
        ApiConfig config;
        config.appName = "My Game";
        config.appVersion = "2.5.0";
        config.debugMode = true;
        config.logLevel = ApiLogLevel::Trace;
        config.maxMemoryMB = 4096;
        config.workerThreads = 8;
        
        REQUIRE(config.appName == "My Game");
        REQUIRE(config.appVersion == "2.5.0");
        REQUIRE(config.debugMode == true);
        REQUIRE(config.logLevel == ApiLogLevel::Trace);
        REQUIRE(config.maxMemoryMB == 4096);
        REQUIRE(config.workerThreads == 8);
    }
}

// =============================================================================
// Make API Error Tests
// =============================================================================

TEST_CASE("API Types - makeApiError", "[api][types]") {
    SECTION("Create API error with code and message") {
        auto error = makeApiError(ApiErrorCode::NetworkUnavailable, "No internet connection");
        REQUIRE(error.code() == static_cast<i32>(ApiErrorCode::NetworkUnavailable));
        REQUIRE(error.message() == "No internet connection");
    }
    
    SECTION("Create API error for authentication failure") {
        auto error = makeApiError(ApiErrorCode::AuthenticationFailed, "Invalid credentials");
        REQUIRE(error.code() == static_cast<i32>(ApiErrorCode::AuthenticationFailed));
        REQUIRE(error.message() == "Invalid credentials");
    }
}
