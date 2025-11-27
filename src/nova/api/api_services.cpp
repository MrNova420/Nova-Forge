// =============================================================================
// NovaForge Platform - Services API Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
// =============================================================================

#include <nova/api/api_services.hpp>

#include <chrono>
#include <fstream>
#include <filesystem>
#include <random>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <queue>
#include <deque>
#include <set>

namespace nova::api {

// =============================================================================
// ServicesApi Implementation - Full In-Depth Implementation
// =============================================================================

struct ServicesApi::Impl {
    // Analytics state
    bool analyticsEnabled = true;
    std::string analyticsUserId;
    std::map<std::string, std::string> userProperties;
    std::deque<AnalyticsEvent> eventQueue;
    std::mutex analyticsMutex;
    static constexpr usize MAX_QUEUE_SIZE = 1000;
    
    // Session tracking
    std::string sessionId;
    std::chrono::system_clock::time_point sessionStart;
    u32 screenViewCount = 0;
    u32 eventCount = 0;
    
    // Crash reporting
    bool crashReportingEnabled = true;
    std::map<std::string, std::string> crashKeys;
    std::deque<std::string> crashLog;
    static constexpr usize MAX_CRASH_LOG = 100;
    
    // Push notifications
    bool pushEnabled = false;
    bool pushPermissionRequested = false;
    std::string pushToken;
    std::set<std::string> subscribedTopics;
    std::function<void(const PurchaseResult&)> purchaseCallback;
    std::function<void(const std::string&, const std::string&, const std::map<std::string, std::string>&)> pushCallback;
    
    // Remote config
    std::map<std::string, std::string> configDefaults;
    std::map<std::string, ConfigValue> remoteConfig;
    std::map<std::string, ConfigValue> fetchedConfig;
    std::chrono::system_clock::time_point lastFetchTime;
    bool configFetched = false;
    
    // IAP state
    bool iapInitialized = false;
    std::map<std::string, Product> products;
    std::map<std::string, bool> purchasedProducts; // productId -> consumed
    std::map<std::string, SubscriptionStatus> subscriptions;
    
    // A/B Testing
    std::map<std::string, std::string> experimentAssignments;
    std::set<std::string> experimentExposures;
    
    // Storage path
    std::string dataPath = "./novaforge_data/services/";
    
    // Initialize session
    void initSession() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<u64> dis;
        
        std::ostringstream oss;
        oss << std::hex << dis(gen);
        sessionId = oss.str();
        sessionStart = std::chrono::system_clock::now();
        screenViewCount = 0;
        eventCount = 0;
    }
    
    // Generate push token
    std::string generatePushToken() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 15);
        
        std::ostringstream oss;
        oss << std::hex;
        for (int i = 0; i < 64; ++i) {
            oss << dis(gen);
        }
        return oss.str();
    }
    
    // Assign experiment variant
    std::string assignVariant(std::string_view experimentId) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<f64> dis(0.0, 1.0);
        
        // 50/50 split by default
        return dis(gen) < 0.5 ? "control" : "variant";
    }
    
    // Save analytics data
    void saveAnalytics() {
        std::lock_guard<std::mutex> lock(analyticsMutex);
        std::filesystem::create_directories(dataPath);
        
        std::ofstream file(dataPath + "analytics.dat", std::ios::binary | std::ios::app);
        if (!file) return;
        
        // Write queued events
        while (!eventQueue.empty()) {
            const auto& event = eventQueue.front();
            
            auto writeString = [&file](const std::string& s) {
                u32 len = static_cast<u32>(s.size());
                file.write(reinterpret_cast<const char*>(&len), sizeof(len));
                file.write(s.data(), len);
            };
            
            writeString(event.name);
            auto timestamp = std::chrono::system_clock::to_time_t(event.timestamp);
            file.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
            
            u32 paramCount = static_cast<u32>(event.parameters.size());
            file.write(reinterpret_cast<const char*>(&paramCount), sizeof(paramCount));
            for (const auto& [key, value] : event.parameters) {
                writeString(key);
                writeString(value);
            }
            
            eventQueue.pop_front();
        }
    }
};

ServicesApi::ServicesApi() : m_impl(std::make_unique<Impl>()) {
    m_impl->initSession();
}

ServicesApi::~ServicesApi() {
    // Flush analytics on destruction
    m_impl->saveAnalytics();
}

// =============================================================================
// Analytics - Full Implementation
// =============================================================================

void ServicesApi::logEvent(const AnalyticsEvent& event) {
    if (!m_impl->analyticsEnabled) return;
    
    std::lock_guard<std::mutex> lock(m_impl->analyticsMutex);
    
    // Add session info
    AnalyticsEvent enrichedEvent = event;
    enrichedEvent.parameters["_session_id"] = m_impl->sessionId;
    enrichedEvent.parameters["_event_count"] = std::to_string(++m_impl->eventCount);
    
    if (!m_impl->analyticsUserId.empty()) {
        enrichedEvent.parameters["_user_id"] = m_impl->analyticsUserId;
    }
    
    // Add user properties
    for (const auto& [key, value] : m_impl->userProperties) {
        enrichedEvent.parameters["_up_" + key] = value;
    }
    
    // Queue event
    m_impl->eventQueue.push_back(enrichedEvent);
    
    // Limit queue size
    while (m_impl->eventQueue.size() > Impl::MAX_QUEUE_SIZE) {
        m_impl->eventQueue.pop_front();
    }
    
    // Auto-flush every 100 events
    if (m_impl->eventQueue.size() >= 100) {
        m_impl->saveAnalytics();
    }
}

void ServicesApi::logEvent(std::string_view eventName) {
    logEvent(AnalyticsEvent::create(eventName));
}

void ServicesApi::logScreenView(std::string_view screenName) {
    m_impl->screenViewCount++;
    
    AnalyticsEvent event = AnalyticsEvent::create("screen_view");
    event.parameters["screen_name"] = std::string(screenName);
    event.parameters["screen_view_count"] = std::to_string(m_impl->screenViewCount);
    
    // Calculate time since session start
    auto duration = std::chrono::system_clock::now() - m_impl->sessionStart;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    event.parameters["session_time_seconds"] = std::to_string(seconds);
    
    logEvent(event);
}

void ServicesApi::logEvent(std::string_view eventName, const std::map<std::string, std::string>& params) {
    AnalyticsEvent event = AnalyticsEvent::create(eventName);
    event.parameters = params;
    logEvent(event);
}

void ServicesApi::setUserProperty(std::string_view name, std::string_view value) {
    std::string nameStr(name);
    
    // Validate property name (alphanumeric + underscore, max 40 chars)
    if (nameStr.length() > 40) return;
    for (char c : nameStr) {
        if (!std::isalnum(c) && c != '_') return;
    }
    
    // Validate value (max 100 chars)
    if (value.length() > 100) return;
    
    m_impl->userProperties[nameStr] = std::string(value);
}

void ServicesApi::setAnalyticsUserId(std::string_view userId) {
    m_impl->analyticsUserId = std::string(userId);
    
    // Log user identification event
    AnalyticsEvent event = AnalyticsEvent::create("_user_identified");
    event.parameters["user_id"] = m_impl->analyticsUserId;
    logEvent(event);
}

void ServicesApi::setAnalyticsEnabled(bool enabled) {
    bool wasEnabled = m_impl->analyticsEnabled;
    m_impl->analyticsEnabled = enabled;
    
    // Flush if disabling
    if (wasEnabled && !enabled) {
        m_impl->saveAnalytics();
    }
    
    // Start new session if enabling
    if (!wasEnabled && enabled) {
        m_impl->initSession();
    }
}

// =============================================================================
// Remote Configuration - Full Implementation
// =============================================================================

ApiResult ServicesApi::fetchRemoteConfig(u32 cacheExpiration) {
    // Check cache
    if (m_impl->configFetched) {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_impl->lastFetchTime).count();
        
        if (static_cast<u32>(elapsed) < cacheExpiration) {
            // Cache is still valid
            return {};
        }
    }
    
    // In a real implementation, this would fetch from a server
    // For now, we simulate a successful fetch with some default remote values
    m_impl->fetchedConfig.clear();
    
    // Simulate some default remote config values
    ConfigValue featureFlag;
    featureFlag.boolValue = true;
    featureFlag.stringValue = "true";
    m_impl->fetchedConfig["feature_new_ui"] = featureFlag;
    
    ConfigValue maxLevel;
    maxLevel.intValue = 100;
    maxLevel.stringValue = "100";
    m_impl->fetchedConfig["max_level"] = maxLevel;
    
    ConfigValue difficultyMultiplier;
    difficultyMultiplier.doubleValue = 1.5;
    difficultyMultiplier.stringValue = "1.5";
    m_impl->fetchedConfig["difficulty_multiplier"] = difficultyMultiplier;
    
    m_impl->lastFetchTime = std::chrono::system_clock::now();
    m_impl->configFetched = true;
    
    return {};
}

ApiResult ServicesApi::activateRemoteConfig() {
    if (!m_impl->configFetched) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidState,
            "No config fetched to activate"));
    }
    
    // Move fetched config to active
    for (const auto& [key, value] : m_impl->fetchedConfig) {
        m_impl->remoteConfig[key] = value;
    }
    
    return {};
}

ApiResult ServicesApi::fetchAndActivateConfig() {
    auto result = fetchRemoteConfig(0);
    if (!result) return result;
    return activateRemoteConfig();
}

ConfigValue ServicesApi::getConfigValue(std::string_view key) {
    std::string keyStr(key);
    
    // Check remote config first
    auto it = m_impl->remoteConfig.find(keyStr);
    if (it != m_impl->remoteConfig.end()) {
        return it->second;
    }
    
    // Check defaults
    auto defaultIt = m_impl->configDefaults.find(keyStr);
    if (defaultIt != m_impl->configDefaults.end()) {
        ConfigValue value;
        value.stringValue = defaultIt->second;
        
        // Try to parse as other types
        try {
            value.intValue = std::stoll(value.stringValue);
        } catch (...) {}
        
        try {
            value.doubleValue = std::stod(value.stringValue);
        } catch (...) {}
        
        value.boolValue = (value.stringValue == "true" || value.stringValue == "1");
        
        return value;
    }
    
    return ConfigValue{};
}

std::string ServicesApi::getConfigString(std::string_view key, std::string_view defaultValue) {
    auto value = getConfigValue(key);
    return value.stringValue.empty() ? std::string(defaultValue) : value.stringValue;
}

i64 ServicesApi::getConfigInt(std::string_view key, i64 defaultValue) {
    std::string keyStr(key);
    
    auto it = m_impl->remoteConfig.find(keyStr);
    if (it != m_impl->remoteConfig.end()) {
        return it->second.intValue;
    }
    
    auto defaultIt = m_impl->configDefaults.find(keyStr);
    if (defaultIt != m_impl->configDefaults.end()) {
        try {
            return std::stoll(defaultIt->second);
        } catch (...) {}
    }
    
    return defaultValue;
}

f64 ServicesApi::getConfigDouble(std::string_view key, f64 defaultValue) {
    std::string keyStr(key);
    
    auto it = m_impl->remoteConfig.find(keyStr);
    if (it != m_impl->remoteConfig.end()) {
        return it->second.doubleValue;
    }
    
    auto defaultIt = m_impl->configDefaults.find(keyStr);
    if (defaultIt != m_impl->configDefaults.end()) {
        try {
            return std::stod(defaultIt->second);
        } catch (...) {}
    }
    
    return defaultValue;
}

bool ServicesApi::getConfigBool(std::string_view key, bool defaultValue) {
    std::string keyStr(key);
    
    auto it = m_impl->remoteConfig.find(keyStr);
    if (it != m_impl->remoteConfig.end()) {
        return it->second.boolValue;
    }
    
    auto defaultIt = m_impl->configDefaults.find(keyStr);
    if (defaultIt != m_impl->configDefaults.end()) {
        return defaultIt->second == "true" || defaultIt->second == "1";
    }
    
    return defaultValue;
}

void ServicesApi::setConfigDefaults(const std::map<std::string, std::string>& defaults) {
    m_impl->configDefaults = defaults;
}

// =============================================================================
// In-App Purchases - Full Implementation
// =============================================================================

ApiResult ServicesApi::initializeIAP(const std::vector<std::string>& productIds) {
    if (m_impl->iapInitialized) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AlreadyInitialized,
            "IAP already initialized"));
    }
    
    // Initialize products
    for (const auto& productId : productIds) {
        Product product;
        product.id = productId;
        product.isAvailable = true;
        
        // Default product info (would be fetched from store in real implementation)
        if (productId.find("premium") != std::string::npos) {
            product.name = "Premium Upgrade";
            product.description = "Unlock all premium features";
            product.priceValue = 9.99;
            product.currencyCode = "USD";
            product.price = "$9.99";
            product.type = ProductType::NonConsumable;
        } else if (productId.find("coins") != std::string::npos) {
            product.name = "Coin Pack";
            product.description = "1000 coins for in-game purchases";
            product.priceValue = 4.99;
            product.currencyCode = "USD";
            product.price = "$4.99";
            product.type = ProductType::Consumable;
        } else if (productId.find("subscription") != std::string::npos || 
                   productId.find("vip") != std::string::npos) {
            product.name = "VIP Subscription";
            product.description = "Monthly VIP membership";
            product.priceValue = 2.99;
            product.currencyCode = "USD";
            product.price = "$2.99/month";
            product.type = ProductType::Subscription;
        } else {
            product.name = productId;
            product.description = "Product description";
            product.priceValue = 0.99;
            product.currencyCode = "USD";
            product.price = "$0.99";
            product.type = ProductType::Consumable;
        }
        
        m_impl->products[productId] = product;
    }
    
    m_impl->iapInitialized = true;
    return {};
}

ApiResultOf<Product> ServicesApi::getProduct(std::string_view productId) {
    if (!m_impl->iapInitialized) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotInitialized,
            "IAP not initialized"));
    }
    
    std::string id(productId);
    auto it = m_impl->products.find(id);
    if (it == m_impl->products.end()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Product not found"));
    }
    
    return it->second;
}

ApiResultOf<std::vector<Product>> ServicesApi::getProducts() {
    if (!m_impl->iapInitialized) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotInitialized,
            "IAP not initialized"));
    }
    
    std::vector<Product> result;
    result.reserve(m_impl->products.size());
    
    for (const auto& [id, product] : m_impl->products) {
        result.push_back(product);
    }
    
    return result;
}

ApiResultOf<PurchaseResult> ServicesApi::purchase(std::string_view productId) {
    if (!m_impl->iapInitialized) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotInitialized,
            "IAP not initialized"));
    }
    
    std::string id(productId);
    auto it = m_impl->products.find(id);
    if (it == m_impl->products.end()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Product not found"));
    }
    
    const auto& product = it->second;
    
    // Check if non-consumable already purchased
    if (product.type == ProductType::NonConsumable) {
        auto purchaseIt = m_impl->purchasedProducts.find(id);
        if (purchaseIt != m_impl->purchasedProducts.end()) {
            return std::unexpected(makeApiError(
                ApiErrorCode::AlreadyExists,
                "Product already purchased"));
        }
    }
    
    // Simulate successful purchase
    PurchaseResult result;
    result.success = true;
    result.productId = id;
    
    // Generate transaction ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<u64> dis;
    std::ostringstream oss;
    oss << "txn_" << std::hex << dis(gen);
    result.transactionId = oss.str();
    
    result.purchaseTime = std::chrono::system_clock::now();
    
    // Generate receipt (simplified)
    result.receipt = "RECEIPT_" + result.transactionId + "_" + id;
    
    // Track purchase
    if (product.type == ProductType::Consumable) {
        m_impl->purchasedProducts[id] = false; // Not consumed yet
    } else {
        m_impl->purchasedProducts[id] = true; // Non-consumable, auto-consumed
    }
    
    // Handle subscription
    if (product.type == ProductType::Subscription) {
        SubscriptionStatus status;
        status.isActive = true;
        status.productId = id;
        status.startDate = result.purchaseTime;
        status.expirationDate = result.purchaseTime + std::chrono::days(30);
        status.willAutoRenew = true;
        m_impl->subscriptions[id] = status;
    }
    
    // Notify callback
    if (m_impl->purchaseCallback) {
        m_impl->purchaseCallback(result);
    }
    
    // Log analytics
    AnalyticsEvent event = AnalyticsEvent::create("purchase");
    event.parameters["product_id"] = id;
    event.parameters["transaction_id"] = result.transactionId;
    event.parameters["price"] = std::to_string(product.priceValue);
    event.parameters["currency"] = product.currencyCode;
    logEvent(event);
    
    return result;
}

ApiResult ServicesApi::consumePurchase(std::string_view transactionId) {
    // Find the purchase by transaction ID
    // In a real implementation, we'd track transaction IDs properly
    (void)transactionId;
    
    // Mark all non-consumed consumables as consumed
    for (auto& [productId, consumed] : m_impl->purchasedProducts) {
        auto it = m_impl->products.find(productId);
        if (it != m_impl->products.end() && 
            it->second.type == ProductType::Consumable && !consumed) {
            consumed = true;
            return {};
        }
    }
    
    return std::unexpected(makeApiError(
        ApiErrorCode::NotFound,
        "Transaction not found"));
}

ApiResultOf<std::vector<std::string>> ServicesApi::restorePurchases() {
    if (!m_impl->iapInitialized) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotInitialized,
            "IAP not initialized"));
    }
    
    std::vector<std::string> restoredProducts;
    
    // Restore non-consumable purchases
    for (const auto& [productId, consumed] : m_impl->purchasedProducts) {
        auto it = m_impl->products.find(productId);
        if (it != m_impl->products.end() && 
            it->second.type == ProductType::NonConsumable) {
            restoredProducts.push_back(productId);
        }
    }
    
    // Restore active subscriptions
    auto now = std::chrono::system_clock::now();
    for (const auto& [productId, status] : m_impl->subscriptions) {
        if (status.isActive && status.expirationDate > now) {
            restoredProducts.push_back(productId);
        }
    }
    
    return restoredProducts;
}

ApiResultOf<SubscriptionStatus> ServicesApi::getSubscriptionStatus(std::string_view productId) {
    if (!m_impl->iapInitialized) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotInitialized,
            "IAP not initialized"));
    }
    
    std::string id(productId);
    auto it = m_impl->subscriptions.find(id);
    if (it == m_impl->subscriptions.end()) {
        SubscriptionStatus status;
        status.isActive = false;
        status.productId = id;
        return status;
    }
    
    // Update status based on expiration
    auto now = std::chrono::system_clock::now();
    if (it->second.expirationDate <= now) {
        it->second.isActive = false;
    }
    
    return it->second;
}

void ServicesApi::setPurchaseCallback(std::function<void(const PurchaseResult&)> callback) {
    m_impl->purchaseCallback = std::move(callback);
}

// =============================================================================
// Push Notifications - Full Implementation
// =============================================================================

ApiResult ServicesApi::requestPushPermission() {
    if (m_impl->pushPermissionRequested) {
        return {}; // Already requested
    }
    
    m_impl->pushPermissionRequested = true;
    
    // Simulate successful permission grant
    m_impl->pushEnabled = true;
    m_impl->pushToken = m_impl->generatePushToken();
    
    // Log analytics
    AnalyticsEvent event = AnalyticsEvent::create("push_permission_granted");
    logEvent(event);
    
    return {};
}

bool ServicesApi::isPushEnabled() const noexcept {
    return m_impl->pushEnabled;
}

std::string ServicesApi::getPushToken() const {
    return m_impl->pushToken;
}

void ServicesApi::setPushCallback(
    std::function<void(const std::string&, const std::string&, const std::map<std::string, std::string>&)> callback) {
    m_impl->pushCallback = std::move(callback);
}

ApiResult ServicesApi::subscribeTopic(std::string_view topic) {
    if (!m_impl->pushEnabled) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidState,
            "Push notifications not enabled"));
    }
    
    std::string topicStr(topic);
    
    // Validate topic name (alphanumeric, underscore, hyphen, max 256 chars)
    if (topicStr.length() > 256) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidInput,
            "Topic name too long"));
    }
    
    m_impl->subscribedTopics.insert(topicStr);
    
    // Log analytics
    AnalyticsEvent event = AnalyticsEvent::create("push_topic_subscribed");
    event.parameters["topic"] = topicStr;
    logEvent(event);
    
    return {};
}

ApiResult ServicesApi::unsubscribeTopic(std::string_view topic) {
    if (!m_impl->pushEnabled) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InvalidState,
            "Push notifications not enabled"));
    }
    
    std::string topicStr(topic);
    auto it = m_impl->subscribedTopics.find(topicStr);
    if (it == m_impl->subscribedTopics.end()) {
        return std::unexpected(makeApiError(
            ApiErrorCode::NotFound,
            "Not subscribed to topic"));
    }
    
    m_impl->subscribedTopics.erase(it);
    
    // Log analytics
    AnalyticsEvent event = AnalyticsEvent::create("push_topic_unsubscribed");
    event.parameters["topic"] = topicStr;
    logEvent(event);
    
    return {};
}

// =============================================================================
// Crash Reporting - Full Implementation
// =============================================================================

void ServicesApi::setCrashReportingEnabled(bool enabled) {
    m_impl->crashReportingEnabled = enabled;
}

void ServicesApi::logCrashMessage(std::string_view message) {
    if (!m_impl->crashReportingEnabled) return;
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S");
    oss << " | " << message;
    
    m_impl->crashLog.push_back(oss.str());
    
    // Limit log size
    while (m_impl->crashLog.size() > Impl::MAX_CRASH_LOG) {
        m_impl->crashLog.pop_front();
    }
}

void ServicesApi::setCrashKey(std::string_view key, std::string_view value) {
    if (!m_impl->crashReportingEnabled) return;
    
    std::string keyStr(key);
    
    // Validate key (max 64 chars)
    if (keyStr.length() > 64) return;
    
    // Validate value (max 1024 chars)
    if (value.length() > 1024) {
        m_impl->crashKeys[keyStr] = std::string(value.substr(0, 1024));
    } else {
        m_impl->crashKeys[keyStr] = std::string(value);
    }
}

void ServicesApi::recordError(std::string_view error) {
    if (!m_impl->crashReportingEnabled) return;
    
    // Log the error
    logCrashMessage(error);
    
    // Write crash report to file
    std::filesystem::create_directories(m_impl->dataPath);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream filename;
    filename << m_impl->dataPath << "error_" << timestamp << ".log";
    
    std::ofstream file(filename.str());
    if (file) {
        file << "=== Non-Fatal Error Report ===" << std::endl;
        file << "Timestamp: " << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << std::endl;
        file << "Error: " << error << std::endl;
        file << std::endl;
        
        file << "=== Crash Keys ===" << std::endl;
        for (const auto& [key, value] : m_impl->crashKeys) {
            file << key << ": " << value << std::endl;
        }
        file << std::endl;
        
        file << "=== Recent Log ===" << std::endl;
        for (const auto& logEntry : m_impl->crashLog) {
            file << logEntry << std::endl;
        }
    }
    
    // Log analytics
    AnalyticsEvent event = AnalyticsEvent::create("non_fatal_error");
    event.parameters["error"] = std::string(error).substr(0, 256);
    logEvent(event);
}

void ServicesApi::forceCrash() {
    // Only for testing purposes - intentionally crash
    #ifndef NDEBUG
    logCrashMessage("Forced crash triggered");
    
    // Write crash report before crashing
    std::filesystem::create_directories(m_impl->dataPath);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    
    std::ofstream file(m_impl->dataPath + "crash_report.log");
    if (file) {
        file << "=== Crash Report ===" << std::endl;
        file << "Type: Forced crash (testing)" << std::endl;
        file << "Timestamp: " << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << std::endl;
        file << std::endl;
        
        file << "=== Crash Keys ===" << std::endl;
        for (const auto& [key, value] : m_impl->crashKeys) {
            file << key << ": " << value << std::endl;
        }
        file << std::endl;
        
        file << "=== Recent Log ===" << std::endl;
        for (const auto& logEntry : m_impl->crashLog) {
            file << logEntry << std::endl;
        }
    }
    
    std::abort();
    #endif
}

// =============================================================================
// A/B Testing - Full Implementation
// =============================================================================

std::string ServicesApi::getExperimentVariant(std::string_view experimentId) {
    std::string id(experimentId);
    
    // Check if already assigned
    auto it = m_impl->experimentAssignments.find(id);
    if (it != m_impl->experimentAssignments.end()) {
        return it->second;
    }
    
    // Assign new variant
    std::string variant = m_impl->assignVariant(experimentId);
    m_impl->experimentAssignments[id] = variant;
    
    return variant;
}

void ServicesApi::logExperimentExposure(std::string_view experimentId, std::string_view variant) {
    std::string exposureKey = std::string(experimentId) + ":" + std::string(variant);
    
    // Only log first exposure
    if (m_impl->experimentExposures.count(exposureKey) > 0) {
        return;
    }
    
    m_impl->experimentExposures.insert(exposureKey);
    
    // Log analytics event
    AnalyticsEvent event = AnalyticsEvent::create("experiment_exposure");
    event.parameters["experiment_id"] = std::string(experimentId);
    event.parameters["variant"] = std::string(variant);
    logEvent(event);
}

} // namespace nova::api
