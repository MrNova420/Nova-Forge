// =============================================================================
// NovaForge Platform - Services API
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// API for accessing NovaForge backend services including analytics,
// remote configuration, in-app purchases, and more.
// =============================================================================

#pragma once

#include "nova/api/api_types.hpp"

#include <map>

namespace nova::api {

// =============================================================================
// Analytics Types
// =============================================================================

/**
 * @brief Analytics event
 */
struct AnalyticsEvent {
    std::string name;
    std::map<std::string, std::string> parameters;
    ApiTimestamp timestamp;
    
    /// Create an event
    [[nodiscard]] static AnalyticsEvent create(std::string_view name) {
        AnalyticsEvent event;
        event.name = std::string(name);
        event.timestamp = std::chrono::system_clock::now();
        return event;
    }
    
    /// Add string parameter
    AnalyticsEvent& withParam(std::string_view key, std::string_view value) {
        parameters[std::string(key)] = std::string(value);
        return *this;
    }
    
    /// Add numeric parameter
    AnalyticsEvent& withParam(std::string_view key, i64 value) {
        parameters[std::string(key)] = std::to_string(value);
        return *this;
    }
    
    /// Add floating point parameter
    AnalyticsEvent& withParam(std::string_view key, f64 value) {
        parameters[std::string(key)] = std::to_string(value);
        return *this;
    }
};

// =============================================================================
// In-App Purchase Types
// =============================================================================

/**
 * @brief Product type
 */
enum class ProductType : u8 {
    Consumable,         ///< Can be purchased multiple times
    NonConsumable,      ///< One-time purchase
    Subscription        ///< Recurring subscription
};

/**
 * @brief Product info
 */
struct Product {
    std::string id;
    std::string name;
    std::string description;
    ProductType type;
    std::string price;              ///< Formatted price string
    f64 priceValue;                 ///< Numeric price value
    std::string currencyCode;       ///< ISO currency code
    bool isAvailable;
};

/**
 * @brief Purchase result
 */
struct PurchaseResult {
    bool success;
    std::string productId;
    std::string transactionId;
    std::string receipt;
    std::string errorMessage;
    ApiTimestamp purchaseTime;
};

/**
 * @brief Subscription status
 */
struct SubscriptionStatus {
    bool isActive;
    std::string productId;
    ApiTimestamp startDate;
    ApiTimestamp expirationDate;
    bool willAutoRenew;
    bool isInTrialPeriod;
    bool isInGracePeriod;
};

// =============================================================================
// Remote Config Types
// =============================================================================

/**
 * @brief Remote config value
 */
struct ConfigValue {
    std::string stringValue;
    i64 intValue;
    f64 doubleValue;
    bool boolValue;
    std::vector<u8> dataValue;
    
    [[nodiscard]] std::string asString() const { return stringValue; }
    [[nodiscard]] i64 asInt() const { return intValue; }
    [[nodiscard]] f64 asDouble() const { return doubleValue; }
    [[nodiscard]] bool asBool() const { return boolValue; }
};

// =============================================================================
// Services API
// =============================================================================

/**
 * @brief Backend services API
 * 
 * Provides access to:
 * - Analytics
 * - Remote configuration
 * - In-app purchases
 * - Push notifications
 * - Crash reporting
 */
class ServicesApi {
public:
    ServicesApi();
    ~ServicesApi();
    
    // Non-copyable, non-movable
    ServicesApi(const ServicesApi&) = delete;
    ServicesApi& operator=(const ServicesApi&) = delete;
    ServicesApi(ServicesApi&&) = delete;
    ServicesApi& operator=(ServicesApi&&) = delete;
    
    // =========================================================================
    // Analytics
    // =========================================================================
    
    /**
     * @brief Log an analytics event
     * @param event The event to log
     */
    void logEvent(const AnalyticsEvent& event);
    
    /**
     * @brief Log a simple event by name
     * @param eventName Event name
     */
    void logEvent(std::string_view eventName);
    
    /**
     * @brief Log screen view
     * @param screenName Screen name
     */
    void logScreenView(std::string_view screenName);
    
    /**
     * @brief Log custom event with parameters
     * @param eventName Event name
     * @param params Parameters
     */
    void logEvent(std::string_view eventName, 
                  const std::map<std::string, std::string>& params);
    
    /**
     * @brief Set user property
     * @param name Property name
     * @param value Property value
     */
    void setUserProperty(std::string_view name, std::string_view value);
    
    /**
     * @brief Set user ID for analytics
     * @param userId User ID
     */
    void setAnalyticsUserId(std::string_view userId);
    
    /**
     * @brief Enable/disable analytics
     * @param enabled true to enable
     */
    void setAnalyticsEnabled(bool enabled);
    
    // =========================================================================
    // Remote Configuration
    // =========================================================================
    
    /**
     * @brief Fetch remote config
     * @param cacheExpiration Cache expiration in seconds (0 = don't cache)
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult fetchRemoteConfig(u32 cacheExpiration = 3600);
    
    /**
     * @brief Activate fetched config
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult activateRemoteConfig();
    
    /**
     * @brief Fetch and activate remote config
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult fetchAndActivateConfig();
    
    /**
     * @brief Get config value
     * @param key Config key
     * @return Config value
     */
    [[nodiscard]] ConfigValue getConfigValue(std::string_view key);
    
    /**
     * @brief Get string config value
     * @param key Config key
     * @param defaultValue Default value if not found
     * @return Config value
     */
    [[nodiscard]] std::string getConfigString(std::string_view key, 
                                               std::string_view defaultValue = "");
    
    /**
     * @brief Get integer config value
     * @param key Config key
     * @param defaultValue Default value if not found
     * @return Config value
     */
    [[nodiscard]] i64 getConfigInt(std::string_view key, i64 defaultValue = 0);
    
    /**
     * @brief Get double config value
     * @param key Config key
     * @param defaultValue Default value if not found
     * @return Config value
     */
    [[nodiscard]] f64 getConfigDouble(std::string_view key, f64 defaultValue = 0.0);
    
    /**
     * @brief Get boolean config value
     * @param key Config key
     * @param defaultValue Default value if not found
     * @return Config value
     */
    [[nodiscard]] bool getConfigBool(std::string_view key, bool defaultValue = false);
    
    /**
     * @brief Set default config values
     * @param defaults Map of key-value defaults
     */
    void setConfigDefaults(const std::map<std::string, std::string>& defaults);
    
    // =========================================================================
    // In-App Purchases
    // =========================================================================
    
    /**
     * @brief Initialize IAP with product IDs
     * @param productIds List of product IDs to load
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult initializeIAP(const std::vector<std::string>& productIds);
    
    /**
     * @brief Get product info
     * @param productId Product ID
     * @return Product info or error
     */
    [[nodiscard]] ApiResultOf<Product> getProduct(std::string_view productId);
    
    /**
     * @brief Get all available products
     * @return List of products
     */
    [[nodiscard]] ApiResultOf<std::vector<Product>> getProducts();
    
    /**
     * @brief Purchase a product
     * @param productId Product ID
     * @return Purchase result
     */
    [[nodiscard]] ApiResultOf<PurchaseResult> purchase(std::string_view productId);
    
    /**
     * @brief Consume a consumable purchase
     * @param transactionId Transaction ID
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult consumePurchase(std::string_view transactionId);
    
    /**
     * @brief Restore purchases
     * @return List of restored transaction IDs
     */
    [[nodiscard]] ApiResultOf<std::vector<std::string>> restorePurchases();
    
    /**
     * @brief Get subscription status
     * @param productId Subscription product ID
     * @return Subscription status
     */
    [[nodiscard]] ApiResultOf<SubscriptionStatus> getSubscriptionStatus(std::string_view productId);
    
    /**
     * @brief Set purchase callback
     * @param callback Called when purchase completes
     */
    void setPurchaseCallback(std::function<void(const PurchaseResult&)> callback);
    
    // =========================================================================
    // Push Notifications
    // =========================================================================
    
    /**
     * @brief Request push notification permission
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult requestPushPermission();
    
    /**
     * @brief Check if push notifications are enabled
     * @return true if enabled
     */
    [[nodiscard]] bool isPushEnabled() const noexcept;
    
    /**
     * @brief Get push notification token
     * @return Token or empty string if not available
     */
    [[nodiscard]] std::string getPushToken() const;
    
    /**
     * @brief Set push notification callback
     * @param callback Called when notification is received
     */
    void setPushCallback(std::function<void(const std::string& title, 
                                            const std::string& body,
                                            const std::map<std::string, std::string>& data)> callback);
    
    /**
     * @brief Subscribe to topic
     * @param topic Topic name
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult subscribeTopic(std::string_view topic);
    
    /**
     * @brief Unsubscribe from topic
     * @param topic Topic name
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult unsubscribeTopic(std::string_view topic);
    
    // =========================================================================
    // Crash Reporting
    // =========================================================================
    
    /**
     * @brief Enable/disable crash reporting
     * @param enabled true to enable
     */
    void setCrashReportingEnabled(bool enabled);
    
    /**
     * @brief Log a custom message for crash reports
     * @param message Message to log
     */
    void logCrashMessage(std::string_view message);
    
    /**
     * @brief Set custom key for crash reports
     * @param key Key name
     * @param value Value
     */
    void setCrashKey(std::string_view key, std::string_view value);
    
    /**
     * @brief Record a non-fatal error
     * @param error Error message
     */
    void recordError(std::string_view error);
    
    /**
     * @brief Force a crash (for testing)
     */
    void forceCrash();
    
    // =========================================================================
    // A/B Testing
    // =========================================================================
    
    /**
     * @brief Get experiment variant
     * @param experimentId Experiment ID
     * @return Variant name or default
     */
    [[nodiscard]] std::string getExperimentVariant(std::string_view experimentId);
    
    /**
     * @brief Log experiment exposure
     * @param experimentId Experiment ID
     * @param variant Variant name
     */
    void logExperimentExposure(std::string_view experimentId, std::string_view variant);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nova::api
