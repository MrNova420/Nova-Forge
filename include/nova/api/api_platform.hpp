// =============================================================================
// NovaForge Platform - Platform API
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// API for accessing NovaForge platform functionality including user accounts,
// authentication, cloud services, and social features.
// =============================================================================

#pragma once

#include "nova/api/api_types.hpp"

namespace nova::api {

// =============================================================================
// Authentication Types
// =============================================================================

/**
 * @brief Authentication method
 */
enum class AuthMethod : u8 {
    EmailPassword,      ///< Email and password
    PhoneNumber,        ///< Phone number authentication
    Google,             ///< Google OAuth
    Apple,              ///< Apple Sign In
    Facebook,           ///< Facebook Login
    Twitter,            ///< Twitter OAuth
    Discord,            ///< Discord OAuth
    GitHub,             ///< GitHub OAuth
    GameCenter,         ///< Apple Game Center
    PlayGames,          ///< Google Play Games
    Guest,              ///< Anonymous guest account
    DeviceId,           ///< Device-based authentication
    CustomToken         ///< Custom authentication token
};

/**
 * @brief Authentication credentials
 */
struct AuthCredentials {
    AuthMethod method = AuthMethod::Guest;
    std::string email;
    std::string password;
    std::string token;           // OAuth token or custom token
    std::string phoneNumber;
    std::string providerId;
    
    /// Create guest credentials
    [[nodiscard]] static AuthCredentials guest() {
        AuthCredentials creds;
        creds.method = AuthMethod::Guest;
        return creds;
    }
    
    /// Create email/password credentials
    [[nodiscard]] static AuthCredentials emailPassword(
        std::string_view email, 
        std::string_view password) {
        AuthCredentials creds;
        creds.method = AuthMethod::EmailPassword;
        creds.email = std::string(email);
        creds.password = std::string(password);
        return creds;
    }
    
    /// Create phone number credentials
    [[nodiscard]] static AuthCredentials phone(std::string_view phoneNumber) {
        AuthCredentials creds;
        creds.method = AuthMethod::PhoneNumber;
        creds.phoneNumber = std::string(phoneNumber);
        return creds;
    }
    
    /// Create OAuth token credentials
    [[nodiscard]] static AuthCredentials oauthToken(
        AuthMethod method, 
        std::string_view token) {
        AuthCredentials creds;
        creds.method = method;
        creds.token = std::string(token);
        return creds;
    }
    
    /// Create custom token credentials
    [[nodiscard]] static AuthCredentials customToken(std::string_view token) {
        AuthCredentials creds;
        creds.method = AuthMethod::CustomToken;
        creds.token = std::string(token);
        return creds;
    }
};

/**
 * @brief Authentication result
 */
struct AuthResult {
    bool success;
    UserId userId;
    std::string sessionToken;
    std::string refreshToken;
    ApiTimestamp expiresAt;
    std::string errorMessage;
};

// =============================================================================
// Platform API
// =============================================================================

/**
 * @brief Platform services API
 * 
 * Provides access to:
 * - User authentication
 * - User profiles
 * - Friends and social features
 * - Leaderboards
 * - Achievements
 * - Cloud save
 */
class PlatformApi {
public:
    PlatformApi();
    ~PlatformApi();
    
    // Non-copyable, non-movable
    PlatformApi(const PlatformApi&) = delete;
    PlatformApi& operator=(const PlatformApi&) = delete;
    PlatformApi(PlatformApi&&) = delete;
    PlatformApi& operator=(PlatformApi&&) = delete;
    
    // =========================================================================
    // Authentication
    // =========================================================================
    
    /**
     * @brief Authenticate with credentials
     * @param credentials Authentication credentials
     * @return Authentication result
     */
    [[nodiscard]] ApiResultOf<AuthResult> authenticate(const AuthCredentials& credentials);
    
    /**
     * @brief Authenticate as guest
     * @return Authentication result
     */
    [[nodiscard]] ApiResultOf<AuthResult> authenticateAsGuest();
    
    /**
     * @brief Sign out
     */
    void signOut();
    
    /**
     * @brief Check if user is authenticated
     * @return true if authenticated
     */
    [[nodiscard]] bool isAuthenticated() const noexcept;
    
    /**
     * @brief Get current user ID
     * @return User ID or invalid if not authenticated
     */
    [[nodiscard]] UserId getCurrentUserId() const noexcept;
    
    /**
     * @brief Refresh authentication token
     * @return New authentication result
     */
    [[nodiscard]] ApiResultOf<AuthResult> refreshToken();
    
    // =========================================================================
    // User Profile
    // =========================================================================
    
    /**
     * @brief Get current user's profile
     * @return User profile or error
     */
    [[nodiscard]] ApiResultOf<UserProfile> getCurrentProfile();
    
    /**
     * @brief Get a user's profile by ID
     * @param userId User ID
     * @return User profile or error
     */
    [[nodiscard]] ApiResultOf<UserProfile> getProfile(const UserId& userId);
    
    /**
     * @brief Update current user's profile
     * @param displayName New display name
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult updateDisplayName(std::string_view displayName);
    
    /**
     * @brief Update current user's avatar
     * @param avatarData Avatar image data
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult updateAvatar(const std::vector<u8>& avatarData);
    
    // =========================================================================
    // Friends
    // =========================================================================
    
    /**
     * @brief Get friends list
     * @return List of friend user IDs
     */
    [[nodiscard]] ApiResultOf<std::vector<UserId>> getFriends();
    
    /**
     * @brief Send friend request
     * @param userId User to send request to
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult sendFriendRequest(const UserId& userId);
    
    /**
     * @brief Accept friend request
     * @param userId User who sent the request
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult acceptFriendRequest(const UserId& userId);
    
    /**
     * @brief Decline friend request
     * @param userId User who sent the request
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult declineFriendRequest(const UserId& userId);
    
    /**
     * @brief Remove friend
     * @param userId Friend to remove
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult removeFriend(const UserId& userId);
    
    /**
     * @brief Block user
     * @param userId User to block
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult blockUser(const UserId& userId);
    
    /**
     * @brief Unblock user
     * @param userId User to unblock
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult unblockUser(const UserId& userId);
    
    // =========================================================================
    // Leaderboards
    // =========================================================================
    
    /**
     * @brief Leaderboard entry
     */
    struct LeaderboardEntry {
        u32 rank;
        UserId userId;
        std::string username;
        std::string displayName;
        i64 score;
        ApiTimestamp timestamp;
    };
    
    /**
     * @brief Submit score to leaderboard
     * @param leaderboardId Leaderboard ID
     * @param score Score to submit
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult submitScore(std::string_view leaderboardId, i64 score);
    
    /**
     * @brief Get leaderboard entries
     * @param leaderboardId Leaderboard ID
     * @param count Number of entries to get
     * @param offset Starting offset
     * @return List of entries or error
     */
    [[nodiscard]] ApiResultOf<std::vector<LeaderboardEntry>> getLeaderboard(
        std::string_view leaderboardId,
        u32 count = 10,
        u32 offset = 0);
    
    /**
     * @brief Get user's rank on leaderboard
     * @param leaderboardId Leaderboard ID
     * @param userId User ID
     * @return Leaderboard entry or error
     */
    [[nodiscard]] ApiResultOf<LeaderboardEntry> getUserRank(
        std::string_view leaderboardId,
        const UserId& userId);
    
    // =========================================================================
    // Achievements
    // =========================================================================
    
    /**
     * @brief Achievement info
     */
    struct Achievement {
        std::string id;
        std::string name;
        std::string description;
        std::string iconUrl;
        i32 points;
        bool isUnlocked = false;
        bool isHidden = false;
        f32 progress = 0.0f;
        ApiTimestamp unlockedAt;
    };
    
    /**
     * @brief Get all achievements
     * @return List of achievements
     */
    [[nodiscard]] ApiResultOf<std::vector<Achievement>> getAchievements();
    
    /**
     * @brief Unlock achievement
     * @param achievementId Achievement ID
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult unlockAchievement(std::string_view achievementId);
    
    /**
     * @brief Update achievement progress
     * @param achievementId Achievement ID
     * @param progress Progress (0-100)
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult updateAchievementProgress(std::string_view achievementId, f32 progress);
    
    // =========================================================================
    // Cloud Save
    // =========================================================================
    
    /**
     * @brief Save data to cloud
     * @param key Save key
     * @param data Save data
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult cloudSave(std::string_view key, const std::vector<u8>& data);
    
    /**
     * @brief Save JSON data to cloud
     * @param key Save key
     * @param json JSON string
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult cloudSaveJson(std::string_view key, std::string_view json);
    
    /**
     * @brief Load data from cloud
     * @param key Save key
     * @return Save data or error
     */
    [[nodiscard]] ApiResultOf<std::vector<u8>> cloudLoad(std::string_view key);
    
    /**
     * @brief Load JSON data from cloud
     * @param key Save key
     * @return JSON string or error
     */
    [[nodiscard]] ApiResultOf<std::string> cloudLoadJson(std::string_view key);
    
    /**
     * @brief Delete cloud save
     * @param key Save key
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult cloudDelete(std::string_view key);
    
    /**
     * @brief List cloud saves
     * @return List of save keys
     */
    [[nodiscard]] ApiResultOf<std::vector<std::string>> cloudList();
    
    // =========================================================================
    // Callbacks
    // =========================================================================
    
    /**
     * @brief Set authentication state change callback
     * @param callback Called when auth state changes
     */
    void setAuthStateCallback(std::function<void(bool isAuthenticated)> callback);
    
    /**
     * @brief Set friend request callback
     * @param callback Called when friend request is received
     */
    void setFriendRequestCallback(std::function<void(const UserId& fromUser)> callback);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nova::api
