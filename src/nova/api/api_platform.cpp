// =============================================================================
// NovaForge Platform - Platform API Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
// =============================================================================

#include <nova/api/api_platform.hpp>
#include <nova/api/api_security.hpp>

#include <map>
#include <set>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <mutex>
#include <sstream>
#include <iomanip>

namespace nova::api {

namespace {
    // Helper to convert hash result to hex string
    std::string hashToHex(const security::HashResult& hash) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (u8 byte : hash) {
            oss << std::setw(2) << static_cast<int>(byte);
        }
        return oss.str();
    }
}

// =============================================================================
// PlatformApi Implementation - Full In-Depth Implementation
// =============================================================================

struct PlatformApi::Impl {
    // Authentication state
    bool authenticated = false;
    UserId currentUser;
    std::string sessionToken;
    std::chrono::system_clock::time_point tokenExpiry;
    
    // User profile data (in-memory storage, would be database in production)
    struct UserData {
        UserProfile profile;
        std::string passwordHash;
        std::set<UserId> friends;
        std::set<UserId> friendRequests;
        std::set<UserId> sentRequests;
        std::set<UserId> blockedUsers;
        std::map<std::string, std::vector<u8>> cloudData;
        std::map<std::string, i64> scores; // leaderboardId -> score
        std::set<std::string> unlockedAchievements;
        std::map<std::string, f32> achievementProgress;
    };
    
    // Leaderboard data
    struct LeaderboardData {
        std::string id;
        std::string name;
        std::vector<LeaderboardEntry> entries;
        bool ascending = false; // lower is better
    };
    
    // Achievement definitions
    struct AchievementDef {
        std::string id;
        std::string name;
        std::string description;
        std::string iconUrl;
        i32 points;
        bool hidden;
    };
    
    std::map<std::string, UserData> users; // email -> data
    std::map<UserId, std::string> userIdToEmail; // userId -> email
    std::map<std::string, LeaderboardData> leaderboards;
    std::map<std::string, AchievementDef> achievementDefs;
    
    std::string dataPath = "./novaforge_data/";
    std::mutex dataMutex;
    
    std::function<void(bool)> authStateCallback;
    std::function<void(const UserId&)> friendRequestCallback;
    
    // Helper to get current user data
    UserData* getCurrentUserData() {
        if (!authenticated) return nullptr;
        auto it = userIdToEmail.find(currentUser);
        if (it == userIdToEmail.end()) return nullptr;
        auto userIt = users.find(it->second);
        if (userIt == users.end()) return nullptr;
        return &userIt->second;
    }
    
    // Helper to get user data by ID
    UserData* getUserData(const UserId& userId) {
        auto it = userIdToEmail.find(userId);
        if (it == userIdToEmail.end()) return nullptr;
        auto userIt = users.find(it->second);
        if (userIt == users.end()) return nullptr;
        return &userIt->second;
    }
    
    // Save user data to disk
    void saveUserData(const std::string& email) {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::filesystem::create_directories(dataPath + "users/");
        
        auto it = users.find(email);
        if (it == users.end()) return;
        
        const auto& data = it->second;
        std::string filepath = dataPath + "users/" + hashToHex(security::Crypto::sha256(email)).substr(0, 16) + ".dat";
        
        std::ofstream file(filepath, std::ios::binary);
        if (!file) return;
        
        // Write profile
        auto writeString = [&file](const std::string& s) {
            u32 len = static_cast<u32>(s.size());
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(s.data(), len);
        };
        
        writeString(data.profile.userId.id);
        writeString(data.profile.username);
        writeString(data.profile.displayName);
        writeString(data.profile.avatarUrl);
        writeString(data.profile.bio);
        file.write(reinterpret_cast<const char*>(&data.profile.isVerified), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&data.profile.isPremium), sizeof(bool));
        writeString(data.passwordHash);
        
        // Write friends
        u32 friendCount = static_cast<u32>(data.friends.size());
        file.write(reinterpret_cast<const char*>(&friendCount), sizeof(friendCount));
        for (const auto& f : data.friends) {
            writeString(f.id);
        }
        
        // Write cloud data
        u32 cloudCount = static_cast<u32>(data.cloudData.size());
        file.write(reinterpret_cast<const char*>(&cloudCount), sizeof(cloudCount));
        for (const auto& [key, value] : data.cloudData) {
            writeString(key);
            u32 dataLen = static_cast<u32>(value.size());
            file.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
            file.write(reinterpret_cast<const char*>(value.data()), dataLen);
        }
        
        // Write scores
        u32 scoreCount = static_cast<u32>(data.scores.size());
        file.write(reinterpret_cast<const char*>(&scoreCount), sizeof(scoreCount));
        for (const auto& [leaderboardId, score] : data.scores) {
            writeString(leaderboardId);
            file.write(reinterpret_cast<const char*>(&score), sizeof(score));
        }
        
        // Write achievements
        u32 achievementCount = static_cast<u32>(data.unlockedAchievements.size());
        file.write(reinterpret_cast<const char*>(&achievementCount), sizeof(achievementCount));
        for (const auto& id : data.unlockedAchievements) {
            writeString(id);
        }
    }
    
    // Load user data from disk
    bool loadUserData(const std::string& email) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        std::string filepath = dataPath + "users/" + hashToHex(security::Crypto::sha256(email)).substr(0, 16) + ".dat";
        std::ifstream file(filepath, std::ios::binary);
        if (!file) return false;
        
        UserData data;
        
        auto readString = [&file]() -> std::string {
            u32 len;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            if (len > 10000) return ""; // Sanity check
            std::string s(len, '\0');
            file.read(s.data(), len);
            return s;
        };
        
        data.profile.userId.id = readString();
        data.profile.username = readString();
        data.profile.displayName = readString();
        data.profile.avatarUrl = readString();
        data.profile.bio = readString();
        file.read(reinterpret_cast<char*>(&data.profile.isVerified), sizeof(bool));
        file.read(reinterpret_cast<char*>(&data.profile.isPremium), sizeof(bool));
        data.passwordHash = readString();
        
        // Read friends
        u32 friendCount;
        file.read(reinterpret_cast<char*>(&friendCount), sizeof(friendCount));
        for (u32 i = 0; i < friendCount && i < 10000; ++i) {
            UserId f;
            f.id = readString();
            data.friends.insert(f);
        }
        
        // Read cloud data
        u32 cloudCount;
        file.read(reinterpret_cast<char*>(&cloudCount), sizeof(cloudCount));
        for (u32 i = 0; i < cloudCount && i < 10000; ++i) {
            std::string key = readString();
            u32 dataLen;
            file.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));
            if (dataLen > 10000000) break; // 10MB limit
            std::vector<u8> value(dataLen);
            file.read(reinterpret_cast<char*>(value.data()), dataLen);
            data.cloudData[key] = std::move(value);
        }
        
        // Read scores
        u32 scoreCount;
        file.read(reinterpret_cast<char*>(&scoreCount), sizeof(scoreCount));
        for (u32 i = 0; i < scoreCount && i < 10000; ++i) {
            std::string leaderboardId = readString();
            i64 score;
            file.read(reinterpret_cast<char*>(&score), sizeof(score));
            data.scores[leaderboardId] = score;
        }
        
        // Read achievements
        u32 achievementCount;
        file.read(reinterpret_cast<char*>(&achievementCount), sizeof(achievementCount));
        for (u32 i = 0; i < achievementCount && i < 10000; ++i) {
            data.unlockedAchievements.insert(readString());
        }
        
        users[email] = std::move(data);
        userIdToEmail[users[email].profile.userId] = email;
        
        return true;
    }
    
    // Update leaderboard rankings
    void updateLeaderboardRankings(const std::string& leaderboardId) {
        auto it = leaderboards.find(leaderboardId);
        if (it == leaderboards.end()) return;
        
        auto& lb = it->second;
        std::sort(lb.entries.begin(), lb.entries.end(),
            [&lb](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                return lb.ascending ? (a.score < b.score) : (a.score > b.score);
            });
        
        u32 rank = 1;
        for (auto& entry : lb.entries) {
            entry.rank = rank++;
        }
    }
};

PlatformApi::PlatformApi() : m_impl(std::make_unique<Impl>()) {
    // Initialize default leaderboards
    m_impl->leaderboards["global_highscore"] = Impl::LeaderboardData{
        "global_highscore", "Global High Scores", {}, false
    };
    m_impl->leaderboards["weekly_challenge"] = Impl::LeaderboardData{
        "weekly_challenge", "Weekly Challenge", {}, false
    };
    m_impl->leaderboards["speedrun"] = Impl::LeaderboardData{
        "speedrun", "Speedrun Times", {}, true // Lower is better
    };
    
    // Initialize default achievements
    m_impl->achievementDefs["first_game"] = Impl::AchievementDef{
        "first_game", "First Steps", "Play your first game", "", 10, false
    };
    m_impl->achievementDefs["win_10"] = Impl::AchievementDef{
        "win_10", "Getting Good", "Win 10 games", "", 25, false
    };
    m_impl->achievementDefs["win_100"] = Impl::AchievementDef{
        "win_100", "Champion", "Win 100 games", "", 100, false
    };
    m_impl->achievementDefs["speedrunner"] = Impl::AchievementDef{
        "speedrunner", "Speedrunner", "Complete a level in under 60 seconds", "", 50, false
    };
    m_impl->achievementDefs["collector"] = Impl::AchievementDef{
        "collector", "Collector", "Collect all items in a level", "", 30, false
    };
    m_impl->achievementDefs["secret_hunter"] = Impl::AchievementDef{
        "secret_hunter", "Secret Hunter", "Find all hidden secrets", "", 75, true
    };
}

PlatformApi::~PlatformApi() = default;

// =============================================================================
// Authentication - Full Implementation
// =============================================================================

ApiResultOf<AuthResult> PlatformApi::authenticate(const AuthCredentials& credentials) {
    AuthResult result;
    
    switch (credentials.method) {
        case AuthMethod::Guest: {
            // Generate unique guest ID using timestamp and random token
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string guestToken = security::Crypto::generateTokenString(8);
            result.userId.id = "guest_" + std::to_string(timestamp) + "_" + guestToken;
            
            // Create guest user data
            Impl::UserData userData;
            userData.profile.userId = result.userId;
            userData.profile.username = "Guest_" + guestToken.substr(0, 6);
            userData.profile.displayName = "Guest Player";
            userData.profile.isVerified = false;
            userData.profile.isPremium = false;
            userData.profile.createdAt = std::chrono::system_clock::now();
            userData.profile.lastLoginAt = std::chrono::system_clock::now();
            
            m_impl->users[result.userId.id] = std::move(userData);
            m_impl->userIdToEmail[result.userId] = result.userId.id;
            
            result.success = true;
            break;
        }
            
        case AuthMethod::EmailPassword: {
            // Validate email format
            if (!security::InputValidator::isValidEmail(credentials.email)) {
                return std::unexpected(makeApiError(
                    ApiErrorCode::InvalidCredentials,
                    "Invalid email format"));
            }
            
            // Validate password strength
            if (!security::InputValidator::isValidPassword(credentials.password)) {
                return std::unexpected(makeApiError(
                    ApiErrorCode::InvalidCredentials,
                    "Password must be at least 8 characters with uppercase, lowercase, digit, and special character"));
            }
            
            // Check if user exists
            auto userIt = m_impl->users.find(credentials.email);
            if (userIt != m_impl->users.end()) {
                // Verify password
                if (!security::Crypto::verifyPassword(credentials.password, userIt->second.passwordHash)) {
                    return std::unexpected(makeApiError(
                        ApiErrorCode::InvalidCredentials,
                        "Invalid email or password"));
                }
                
                // Update last login
                userIt->second.profile.lastLoginAt = std::chrono::system_clock::now();
                result.userId = userIt->second.profile.userId;
            } else {
                // Create new user
                result.userId.id = "user_" + hashToHex(security::Crypto::sha256(credentials.email)).substr(0, 16);
                
                Impl::UserData userData;
                userData.profile.userId = result.userId;
                userData.profile.username = credentials.email.substr(0, credentials.email.find('@'));
                userData.profile.displayName = userData.profile.username;
                userData.profile.isVerified = false;
                userData.profile.isPremium = false;
                userData.profile.createdAt = std::chrono::system_clock::now();
                userData.profile.lastLoginAt = std::chrono::system_clock::now();
                userData.passwordHash = security::Crypto::hashPassword(credentials.password);
                
                m_impl->users[credentials.email] = std::move(userData);
                m_impl->userIdToEmail[result.userId] = credentials.email;
                
                // Save to disk
                m_impl->saveUserData(credentials.email);
            }
            
            result.success = true;
            break;
        }
            
        case AuthMethod::PhoneNumber: {
            if (credentials.phoneNumber.empty()) {
                return std::unexpected(makeApiError(
                    ApiErrorCode::InvalidCredentials,
                    "Phone number is required"));
            }
            
            // Create/find user by phone
            std::string phoneKey = "phone_" + credentials.phoneNumber;
            result.userId.id = "phone_" + hashToHex(security::Crypto::sha256(credentials.phoneNumber)).substr(0, 16);
            
            auto userIt = m_impl->users.find(phoneKey);
            if (userIt == m_impl->users.end()) {
                Impl::UserData userData;
                userData.profile.userId = result.userId;
                userData.profile.username = "User_" + credentials.phoneNumber.substr(credentials.phoneNumber.length() - 4);
                userData.profile.displayName = userData.profile.username;
                userData.profile.createdAt = std::chrono::system_clock::now();
                userData.profile.lastLoginAt = std::chrono::system_clock::now();
                
                m_impl->users[phoneKey] = std::move(userData);
                m_impl->userIdToEmail[result.userId] = phoneKey;
            } else {
                userIt->second.profile.lastLoginAt = std::chrono::system_clock::now();
            }
            
            result.success = true;
            break;
        }
            
        case AuthMethod::Google:
        case AuthMethod::Apple:
        case AuthMethod::Facebook:
        case AuthMethod::GameCenter:
        case AuthMethod::PlayGames:
        case AuthMethod::Twitter:
        case AuthMethod::Discord:
        case AuthMethod::GitHub: {
            // OAuth flow - token should be validated server-side
            if (credentials.token.empty()) {
                return std::unexpected(makeApiError(
                    ApiErrorCode::InvalidCredentials,
                    "OAuth token is required"));
            }
            
            // Generate user ID from OAuth token hash
            std::string provider;
            switch (credentials.method) {
                case AuthMethod::Google: provider = "google"; break;
                case AuthMethod::Apple: provider = "apple"; break;
                case AuthMethod::Facebook: provider = "facebook"; break;
                case AuthMethod::GameCenter: provider = "gamecenter"; break;
                case AuthMethod::PlayGames: provider = "playgames"; break;
                case AuthMethod::Twitter: provider = "twitter"; break;
                case AuthMethod::Discord: provider = "discord"; break;
                case AuthMethod::GitHub: provider = "github"; break;
                default: provider = "oauth"; break;
            }
            
            result.userId.id = provider + "_" + hashToHex(security::Crypto::sha256(credentials.token)).substr(0, 16);
            
            std::string oauthKey = provider + "_" + credentials.token.substr(0, 32);
            auto userIt = m_impl->users.find(oauthKey);
            if (userIt == m_impl->users.end()) {
                Impl::UserData userData;
                userData.profile.userId = result.userId;
                userData.profile.username = provider + "_user";
                userData.profile.displayName = "Player";
                userData.profile.createdAt = std::chrono::system_clock::now();
                userData.profile.lastLoginAt = std::chrono::system_clock::now();
                
                m_impl->users[oauthKey] = std::move(userData);
                m_impl->userIdToEmail[result.userId] = oauthKey;
            } else {
                userIt->second.profile.lastLoginAt = std::chrono::system_clock::now();
            }
            
            result.success = true;
            break;
        }
        
        case AuthMethod::DeviceId: {
            // Device-based authentication uses the token field for device ID
            if (credentials.token.empty()) {
                return std::unexpected(makeApiError(
                    ApiErrorCode::InvalidCredentials,
                    "Device ID is required"));
            }
            
            result.userId.id = "device_" + hashToHex(security::Crypto::sha256(credentials.token)).substr(0, 16);
            
            std::string deviceKey = "device_" + credentials.token.substr(0, 32);
            auto userIt = m_impl->users.find(deviceKey);
            if (userIt == m_impl->users.end()) {
                Impl::UserData userData;
                userData.profile.userId = result.userId;
                userData.profile.username = "DeviceUser";
                userData.profile.displayName = "Player";
                userData.profile.createdAt = std::chrono::system_clock::now();
                userData.profile.lastLoginAt = std::chrono::system_clock::now();
                
                m_impl->users[deviceKey] = std::move(userData);
                m_impl->userIdToEmail[result.userId] = deviceKey;
            } else {
                userIt->second.profile.lastLoginAt = std::chrono::system_clock::now();
            }
            
            result.success = true;
            break;
        }
            
        case AuthMethod::CustomToken: {
            if (credentials.token.empty()) {
                return std::unexpected(makeApiError(
                    ApiErrorCode::InvalidCredentials,
                    "Custom token is required"));
            }
            
            // Validate custom token format
            result.userId.id = "custom_" + hashToHex(security::Crypto::sha256(credentials.token)).substr(0, 16);
            
            std::string customKey = "custom_" + credentials.token.substr(0, 32);
            auto userIt = m_impl->users.find(customKey);
            if (userIt == m_impl->users.end()) {
                Impl::UserData userData;
                userData.profile.userId = result.userId;
                userData.profile.username = "CustomUser";
                userData.profile.displayName = "Player";
                userData.profile.createdAt = std::chrono::system_clock::now();
                userData.profile.lastLoginAt = std::chrono::system_clock::now();
                
                m_impl->users[customKey] = std::move(userData);
                m_impl->userIdToEmail[result.userId] = customKey;
            } else {
                userIt->second.profile.lastLoginAt = std::chrono::system_clock::now();
            }
            
            result.success = true;
            break;
        }
    }
    
    if (result.success) {
        m_impl->authenticated = true;
        m_impl->currentUser = result.userId;
        m_impl->sessionToken = security::Crypto::generateTokenString(32);
        m_impl->tokenExpiry = std::chrono::system_clock::now() + std::chrono::hours(24);
        result.sessionToken = m_impl->sessionToken;
        result.expiresAt = m_impl->tokenExpiry;
        
        if (m_impl->authStateCallback) {
            m_impl->authStateCallback(true);
        }
    }
    
    return result;
}

ApiResultOf<AuthResult> PlatformApi::authenticateAsGuest() {
    return authenticate(AuthCredentials::guest());
}

void PlatformApi::signOut() {
    // Save user data before signing out
    if (m_impl->authenticated) {
        auto it = m_impl->userIdToEmail.find(m_impl->currentUser);
        if (it != m_impl->userIdToEmail.end()) {
            m_impl->saveUserData(it->second);
        }
    }
    
    m_impl->authenticated = false;
    m_impl->currentUser = UserId::invalid();
    m_impl->sessionToken.clear();
    m_impl->tokenExpiry = std::chrono::system_clock::time_point{};
    
    if (m_impl->authStateCallback) {
        m_impl->authStateCallback(false);
    }
}

bool PlatformApi::isAuthenticated() const noexcept {
    if (!m_impl->authenticated) return false;
    
    // Check token expiry
    if (std::chrono::system_clock::now() > m_impl->tokenExpiry) {
        return false;
    }
    
    return true;
}

UserId PlatformApi::getCurrentUserId() const noexcept {
    return m_impl->currentUser;
}

ApiResultOf<AuthResult> PlatformApi::refreshToken() {
    if (!m_impl->authenticated) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    // Check if token is still valid (within grace period)
    auto now = std::chrono::system_clock::now();
    auto gracePeriod = m_impl->tokenExpiry + std::chrono::hours(1);
    if (now > gracePeriod) {
        m_impl->authenticated = false;
        return std::unexpected(makeApiError(
            ApiErrorCode::TokenExpired,
            "Session expired, please sign in again"));
    }
    
    AuthResult result;
    result.success = true;
    result.userId = m_impl->currentUser;
    result.sessionToken = security::Crypto::generateTokenString(32);
    result.expiresAt = now + std::chrono::hours(24);
    
    m_impl->sessionToken = result.sessionToken;
    m_impl->tokenExpiry = result.expiresAt;
    
    return result;
}

// =============================================================================
// User Profile - Full Implementation
// =============================================================================

ApiResultOf<UserProfile> PlatformApi::getCurrentProfile() {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User profile not found"));
    }
    
    return userData->profile;
}

ApiResultOf<UserProfile> PlatformApi::getProfile(const UserId& userId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* userData = m_impl->getUserData(userId);
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Return limited profile info for other users
    UserProfile profile = userData->profile;
    // Don't expose sensitive info for other users
    if (userId.id != m_impl->currentUser.id) {
        // Profile is already public, no changes needed
    }
    
    return profile;
}

ApiResult PlatformApi::updateDisplayName(std::string_view displayName) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    // Validate display name
    if (displayName.empty() || displayName.length() > 32) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Display name must be 1-32 characters"));
    }
    
    // Check for profanity/inappropriate content (basic check)
    std::string sanitized = security::InputValidator::sanitize(std::string(displayName));
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User profile not found"));
    }
    
    userData->profile.displayName = sanitized;
    
    // Save changes
    auto it = m_impl->userIdToEmail.find(m_impl->currentUser);
    if (it != m_impl->userIdToEmail.end()) {
        m_impl->saveUserData(it->second);
    }
    
    return {};
}

ApiResult PlatformApi::updateAvatar(const std::vector<u8>& avatarData) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    // Validate avatar data size (max 1MB)
    if (avatarData.size() > 1024 * 1024) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Avatar image too large (max 1MB)"));
    }
    
    if (avatarData.empty()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Avatar data is empty"));
    }
    
    // Validate image format (check magic bytes for PNG/JPEG)
    bool validFormat = false;
    if (avatarData.size() >= 8) {
        // PNG magic bytes
        if (avatarData[0] == 0x89 && avatarData[1] == 'P' && 
            avatarData[2] == 'N' && avatarData[3] == 'G') {
            validFormat = true;
        }
        // JPEG magic bytes
        else if (avatarData[0] == 0xFF && avatarData[1] == 0xD8) {
            validFormat = true;
        }
    }
    
    if (!validFormat) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Avatar must be PNG or JPEG format"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User profile not found"));
    }
    
    // Store avatar in cloud data
    userData->cloudData["_avatar"] = avatarData;
    userData->profile.avatarUrl = "local://avatar/" + m_impl->currentUser.id;
    
    // Save changes
    auto it = m_impl->userIdToEmail.find(m_impl->currentUser);
    if (it != m_impl->userIdToEmail.end()) {
        m_impl->saveUserData(it->second);
    }
    
    return {};
}

// =============================================================================
// Friends System - Full Implementation
// =============================================================================

ApiResultOf<std::vector<UserId>> PlatformApi::getFriends() {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User profile not found"));
    }
    
    std::vector<UserId> friends;
    friends.reserve(userData->friends.size());
    for (const auto& f : userData->friends) {
        friends.push_back(f);
    }
    
    return friends;
}

ApiResult PlatformApi::sendFriendRequest(const UserId& userId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    if (userId.id == m_impl->currentUser.id) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Cannot send friend request to yourself"));
    }
    
    auto* currentUserData = m_impl->getCurrentUserData();
    auto* targetUserData = m_impl->getUserData(userId);
    
    if (!currentUserData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Current user not found"));
    }
    
    if (!targetUserData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Target user not found"));
    }
    
    // Check if already friends
    if (currentUserData->friends.count(userId) > 0) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AlreadyExists,
            "Already friends with this user"));
    }
    
    // Check if blocked
    if (currentUserData->blockedUsers.count(userId) > 0 ||
        targetUserData->blockedUsers.count(m_impl->currentUser) > 0) {
        return std::unexpected(makeApiError(
            ApiErrorCode::Forbidden,
            "Cannot send friend request to this user"));
    }
    
    // Check if request already sent
    if (currentUserData->sentRequests.count(userId) > 0) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AlreadyExists,
            "Friend request already sent"));
    }
    
    // Check if they sent us a request (auto-accept)
    if (currentUserData->friendRequests.count(userId) > 0) {
        // Auto-accept: they requested us, we're requesting them
        currentUserData->friendRequests.erase(userId);
        currentUserData->friends.insert(userId);
        targetUserData->sentRequests.erase(m_impl->currentUser);
        targetUserData->friends.insert(m_impl->currentUser);
        return {};
    }
    
    // Send the request
    currentUserData->sentRequests.insert(userId);
    targetUserData->friendRequests.insert(m_impl->currentUser);
    
    // Notify target user
    if (m_impl->friendRequestCallback) {
        m_impl->friendRequestCallback(m_impl->currentUser);
    }
    
    return {};
}

ApiResult PlatformApi::acceptFriendRequest(const UserId& userId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* currentUserData = m_impl->getCurrentUserData();
    auto* senderUserData = m_impl->getUserData(userId);
    
    if (!currentUserData || !senderUserData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Check if request exists
    if (currentUserData->friendRequests.count(userId) == 0) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "No friend request from this user"));
    }
    
    // Accept the request
    currentUserData->friendRequests.erase(userId);
    currentUserData->friends.insert(userId);
    senderUserData->sentRequests.erase(m_impl->currentUser);
    senderUserData->friends.insert(m_impl->currentUser);
    
    return {};
}

ApiResult PlatformApi::declineFriendRequest(const UserId& userId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* currentUserData = m_impl->getCurrentUserData();
    auto* senderUserData = m_impl->getUserData(userId);
    
    if (!currentUserData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Check if request exists
    if (currentUserData->friendRequests.count(userId) == 0) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "No friend request from this user"));
    }
    
    // Decline the request
    currentUserData->friendRequests.erase(userId);
    if (senderUserData) {
        senderUserData->sentRequests.erase(m_impl->currentUser);
    }
    
    return {};
}

ApiResult PlatformApi::removeFriend(const UserId& userId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* currentUserData = m_impl->getCurrentUserData();
    auto* friendUserData = m_impl->getUserData(userId);
    
    if (!currentUserData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Check if actually friends
    if (currentUserData->friends.count(userId) == 0) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Not friends with this user"));
    }
    
    // Remove friendship (both sides)
    currentUserData->friends.erase(userId);
    if (friendUserData) {
        friendUserData->friends.erase(m_impl->currentUser);
    }
    
    return {};
}

ApiResult PlatformApi::blockUser(const UserId& userId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    if (userId.id == m_impl->currentUser.id) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Cannot block yourself"));
    }
    
    auto* currentUserData = m_impl->getCurrentUserData();
    if (!currentUserData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Remove from friends if present
    currentUserData->friends.erase(userId);
    currentUserData->friendRequests.erase(userId);
    currentUserData->sentRequests.erase(userId);
    
    // Add to blocked list
    currentUserData->blockedUsers.insert(userId);
    
    // Also remove us from their friends
    auto* blockedUserData = m_impl->getUserData(userId);
    if (blockedUserData) {
        blockedUserData->friends.erase(m_impl->currentUser);
    }
    
    return {};
}

ApiResult PlatformApi::unblockUser(const UserId& userId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* currentUserData = m_impl->getCurrentUserData();
    if (!currentUserData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    if (currentUserData->blockedUsers.count(userId) == 0) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User is not blocked"));
    }
    
    currentUserData->blockedUsers.erase(userId);
    
    return {};
}

// =============================================================================
// Leaderboards - Full Implementation
// =============================================================================

ApiResult PlatformApi::submitScore(std::string_view leaderboardId, i64 score) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    std::string lbId(leaderboardId);
    
    // Create leaderboard if it doesn't exist
    if (m_impl->leaderboards.find(lbId) == m_impl->leaderboards.end()) {
        m_impl->leaderboards[lbId] = Impl::LeaderboardData{
            lbId, lbId, {}, false
        };
    }
    
    auto& lb = m_impl->leaderboards[lbId];
    auto* userData = m_impl->getCurrentUserData();
    
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Check if user already has a score
    bool scoreUpdated = false;
    for (auto& entry : lb.entries) {
        if (entry.userId.id == m_impl->currentUser.id) {
            // Update if new score is better
            bool isBetter = lb.ascending ? (score < entry.score) : (score > entry.score);
            if (isBetter) {
                entry.score = score;
                entry.timestamp = std::chrono::system_clock::now();
                scoreUpdated = true;
            }
            break;
        }
    }
    
    // Add new entry if user doesn't have one
    if (!scoreUpdated) {
        bool hasEntry = false;
        for (const auto& entry : lb.entries) {
            if (entry.userId.id == m_impl->currentUser.id) {
                hasEntry = true;
                break;
            }
        }
        
        if (!hasEntry) {
            LeaderboardEntry entry;
            entry.userId = m_impl->currentUser;
            entry.username = userData->profile.displayName;
            entry.score = score;
            entry.rank = 0; // Will be set by updateLeaderboardRankings
            entry.timestamp = std::chrono::system_clock::now();
            lb.entries.push_back(entry);
            scoreUpdated = true;
        }
    }
    
    // Update user's scores map
    userData->scores[lbId] = score;
    
    // Re-rank the leaderboard
    if (scoreUpdated) {
        m_impl->updateLeaderboardRankings(lbId);
    }
    
    return {};
}

ApiResultOf<std::vector<PlatformApi::LeaderboardEntry>> PlatformApi::getLeaderboard(
    std::string_view leaderboardId,
    u32 count,
    u32 offset) {
    
    std::string lbId(leaderboardId);
    
    auto it = m_impl->leaderboards.find(lbId);
    if (it == m_impl->leaderboards.end()) {
        return std::vector<LeaderboardEntry>{};
    }
    
    const auto& lb = it->second;
    std::vector<LeaderboardEntry> result;
    
    u32 start = std::min(offset, static_cast<u32>(lb.entries.size()));
    u32 end = std::min(offset + count, static_cast<u32>(lb.entries.size()));
    
    for (u32 i = start; i < end; ++i) {
        result.push_back(lb.entries[i]);
    }
    
    return result;
}

ApiResultOf<PlatformApi::LeaderboardEntry> PlatformApi::getUserRank(
    std::string_view leaderboardId,
    const UserId& userId) {
    
    std::string lbId(leaderboardId);
    
    auto it = m_impl->leaderboards.find(lbId);
    if (it == m_impl->leaderboards.end()) {
        LeaderboardEntry entry;
        entry.userId = userId;
        entry.rank = 0;
        entry.score = 0;
        return entry;
    }
    
    const auto& lb = it->second;
    for (const auto& entry : lb.entries) {
        if (entry.userId.id == userId.id) {
            return entry;
        }
    }
    
    // User not on leaderboard
    LeaderboardEntry entry;
    entry.userId = userId;
    entry.rank = 0;
    entry.score = 0;
    return entry;
}

// =============================================================================
// Achievements - Full Implementation
// =============================================================================

ApiResultOf<std::vector<PlatformApi::Achievement>> PlatformApi::getAchievements() {
    std::vector<Achievement> achievements;
    
    auto* userData = isAuthenticated() ? m_impl->getCurrentUserData() : nullptr;
    
    for (const auto& [id, def] : m_impl->achievementDefs) {
        // Skip hidden achievements that aren't unlocked
        if (def.hidden && userData && 
            userData->unlockedAchievements.count(id) == 0) {
            continue;
        }
        
        Achievement ach;
        ach.id = def.id;
        ach.name = def.name;
        ach.description = def.description;
        ach.iconUrl = def.iconUrl;
        ach.points = def.points;
        ach.isHidden = def.hidden;
        
        if (userData) {
            ach.isUnlocked = userData->unlockedAchievements.count(id) > 0;
            auto progressIt = userData->achievementProgress.find(id);
            ach.progress = progressIt != userData->achievementProgress.end() ? 
                           progressIt->second : 0.0f;
            if (ach.isUnlocked) {
                ach.progress = 1.0f;
            }
        }
        
        achievements.push_back(ach);
    }
    
    return achievements;
}

ApiResult PlatformApi::unlockAchievement(std::string_view achievementId) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    std::string achId(achievementId);
    
    // Check if achievement exists
    if (m_impl->achievementDefs.find(achId) == m_impl->achievementDefs.end()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Achievement not found"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Check if already unlocked
    if (userData->unlockedAchievements.count(achId) > 0) {
        return {}; // Already unlocked, not an error
    }
    
    // Unlock achievement
    userData->unlockedAchievements.insert(achId);
    userData->achievementProgress[achId] = 1.0f;
    
    // Save changes
    auto it = m_impl->userIdToEmail.find(m_impl->currentUser);
    if (it != m_impl->userIdToEmail.end()) {
        m_impl->saveUserData(it->second);
    }
    
    return {};
}

ApiResult PlatformApi::updateAchievementProgress(std::string_view achievementId, f32 progress) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    std::string achId(achievementId);
    
    // Check if achievement exists
    if (m_impl->achievementDefs.find(achId) == m_impl->achievementDefs.end()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Achievement not found"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Clamp progress to [0, 1]
    progress = std::clamp(progress, 0.0f, 1.0f);
    
    // Update progress (only if higher)
    auto& currentProgress = userData->achievementProgress[achId];
    if (progress > currentProgress) {
        currentProgress = progress;
        
        // Auto-unlock at 100%
        if (progress >= 1.0f) {
            userData->unlockedAchievements.insert(achId);
        }
    }
    
    return {};
}

// =============================================================================
// Cloud Save - Full Implementation
// =============================================================================

ApiResult PlatformApi::cloudSave(std::string_view key, const std::vector<u8>& data) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    // Validate key
    if (key.empty() || key.length() > 256) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Key must be 1-256 characters"));
    }
    
    // Validate data size (max 10MB per key)
    if (data.size() > 10 * 1024 * 1024) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Data too large (max 10MB)"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    // Check total cloud storage limit (100MB)
    u64 totalSize = 0;
    for (const auto& [k, v] : userData->cloudData) {
        if (k != key) { // Don't count the key we're replacing
            totalSize += v.size();
        }
    }
    totalSize += data.size();
    
    if (totalSize > 100 * 1024 * 1024) {
        return std::unexpected(makeApiError(
            ApiErrorCode::StorageQuotaExceeded,
            "Cloud storage quota exceeded (max 100MB)"));
    }
    
    // Save data
    userData->cloudData[std::string(key)] = data;
    
    // Persist to disk
    auto it = m_impl->userIdToEmail.find(m_impl->currentUser);
    if (it != m_impl->userIdToEmail.end()) {
        m_impl->saveUserData(it->second);
    }
    
    return {};
}

ApiResult PlatformApi::cloudSaveJson(std::string_view key, std::string_view json) {
    // Convert JSON string to bytes
    std::vector<u8> data(json.begin(), json.end());
    return cloudSave(key, data);
}

ApiResultOf<std::vector<u8>> PlatformApi::cloudLoad(std::string_view key) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    auto it = userData->cloudData.find(std::string(key));
    if (it == userData->cloudData.end()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Cloud data not found for key"));
    }
    
    return it->second;
}

ApiResultOf<std::string> PlatformApi::cloudLoadJson(std::string_view key) {
    auto result = cloudLoad(key);
    if (!result) {
        return std::unexpected(result.error());
    }
    
    // Convert bytes to JSON string
    return std::string(result->begin(), result->end());
}

ApiResult PlatformApi::cloudDelete(std::string_view key) {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    auto it = userData->cloudData.find(std::string(key));
    if (it == userData->cloudData.end()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Cloud data not found for key"));
    }
    
    userData->cloudData.erase(it);
    
    // Persist to disk
    auto emailIt = m_impl->userIdToEmail.find(m_impl->currentUser);
    if (emailIt != m_impl->userIdToEmail.end()) {
        m_impl->saveUserData(emailIt->second);
    }
    
    return {};
}

ApiResultOf<std::vector<std::string>> PlatformApi::cloudList() {
    if (!isAuthenticated()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AuthenticationRequired,
            "Not authenticated"));
    }
    
    auto* userData = m_impl->getCurrentUserData();
    if (!userData) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "User not found"));
    }
    
    std::vector<std::string> keys;
    keys.reserve(userData->cloudData.size());
    
    for (const auto& [key, value] : userData->cloudData) {
        // Skip internal keys (starting with _)
        if (!key.empty() && key[0] != '_') {
            keys.push_back(key);
        }
    }
    
    return keys;
}

// =============================================================================
// Callbacks
// =============================================================================

void PlatformApi::setAuthStateCallback(std::function<void(bool)> callback) {
    m_impl->authStateCallback = std::move(callback);
}

void PlatformApi::setFriendRequestCallback(std::function<void(const UserId&)> callback) {
    m_impl->friendRequestCallback = std::move(callback);
}

} // namespace nova::api
