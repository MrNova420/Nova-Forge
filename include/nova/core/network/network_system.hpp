/**
 * @file network_system.hpp
 * @brief Nova Network™ - Core networking system interface
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Production-grade networking system supporting 10,000+ concurrent players.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "network_types.hpp"
#include "nova/core/types/result.hpp"

#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace nova::network {

// ============================================================================
// NetworkSocket - Low-level socket abstraction
// ============================================================================

/**
 * @brief Platform-agnostic socket wrapper
 */
class NetworkSocket {
public:
    NetworkSocket() = default;
    ~NetworkSocket();
    
    // Disable copy
    NetworkSocket(const NetworkSocket&) = delete;
    NetworkSocket& operator=(const NetworkSocket&) = delete;
    
    // Enable move
    NetworkSocket(NetworkSocket&& other) noexcept;
    NetworkSocket& operator=(NetworkSocket&& other) noexcept;
    
    /**
     * @brief Create and bind a socket
     * @param protocol Socket protocol
     * @param port Port to bind (0 for any)
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void> bind(SocketProtocol protocol, u16 port);
    
    /**
     * @brief Close the socket
     */
    void close();
    
    /**
     * @brief Check if socket is open
     */
    [[nodiscard]] bool isOpen() const noexcept;
    
    /**
     * @brief Send data to an endpoint
     */
    [[nodiscard]] Result<usize> sendTo(const NetworkEndpoint& endpoint, 
                                        const u8* data, usize size);
    
    /**
     * @brief Receive data from any endpoint
     */
    [[nodiscard]] Result<usize> receiveFrom(NetworkEndpoint& endpoint,
                                             u8* buffer, usize bufferSize);
    
    /**
     * @brief Set socket to non-blocking mode
     */
    [[nodiscard]] Result<void> setNonBlocking(bool nonBlocking);
    
    /**
     * @brief Set socket receive buffer size
     */
    [[nodiscard]] Result<void> setReceiveBufferSize(u32 size);
    
    /**
     * @brief Set socket send buffer size
     */
    [[nodiscard]] Result<void> setSendBufferSize(u32 size);
    
    /**
     * @brief Get local endpoint
     */
    [[nodiscard]] NetworkEndpoint getLocalEndpoint() const;
    
private:
    void* m_handle = nullptr;  // Platform-specific handle
    SocketProtocol m_protocol = SocketProtocol::UDP;
    bool m_bound = false;
};

// ============================================================================
// NetworkConnection - Single connection management
// ============================================================================

/**
 * @brief Represents a single network connection
 */
class NetworkConnection {
public:
    explicit NetworkConnection(u64 id, const NetworkEndpoint& endpoint);
    ~NetworkConnection();
    
    // Disable copy
    NetworkConnection(const NetworkConnection&) = delete;
    NetworkConnection& operator=(const NetworkConnection&) = delete;
    
    /// Get connection ID
    [[nodiscard]] u64 getId() const noexcept { return m_id; }
    
    /// Get remote endpoint
    [[nodiscard]] const NetworkEndpoint& getEndpoint() const noexcept { return m_endpoint; }
    
    /// Get connection state
    [[nodiscard]] ConnectionState getState() const noexcept { return m_state; }
    
    /// Get connection statistics
    [[nodiscard]] const ConnectionStats& getStats() const noexcept { return m_stats; }
    
    /// Get round-trip time
    [[nodiscard]] f32 getRtt() const noexcept { return m_stats.rttMs; }
    
    /// Get packet loss
    [[nodiscard]] f32 getPacketLoss() const noexcept { return m_stats.packetLoss; }
    
    /// Check if connected
    [[nodiscard]] bool isConnected() const noexcept { 
        return m_state == ConnectionState::Connected; 
    }
    
    /**
     * @brief Queue data for sending
     * @param channel Channel to send on
     * @param data Data to send
     * @param size Data size
     * @param mode Delivery mode
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void> send(ChannelType channel, const u8* data, usize size,
                                     DeliveryMode mode = DeliveryMode::ReliableOrdered);
    
    /**
     * @brief Queue a packet for sending
     */
    [[nodiscard]] Result<void> sendPacket(const NetworkPacket& packet);
    
    /**
     * @brief Disconnect this connection
     * @param graceful If true, send disconnect packet first
     */
    void disconnect(bool graceful = true);
    
    /**
     * @brief Process incoming packet
     */
    void processPacket(const NetworkPacket& packet);
    
    /**
     * @brief Update connection (call each tick)
     * @param deltaTime Time since last update
     */
    void update(f32 deltaTime);
    
    /// Set data received callback
    void setDataCallback(DataCallback callback) { m_dataCallback = std::move(callback); }
    
    /// Set state change callback
    void setStateCallback(ConnectionCallback callback) { m_stateCallback = std::move(callback); }
    
private:
    u64 m_id;
    NetworkEndpoint m_endpoint;
    ConnectionState m_state = ConnectionState::Disconnected;
    ConnectionStats m_stats;
    
    // Reliable packet handling
    u16 m_localSequence = 0;
    u16 m_remoteSequence = 0;
    u32 m_ackBitfield = 0;
    
    // Pending packets
    std::vector<NetworkPacket> m_sendQueue;
    std::vector<NetworkPacket> m_pendingAcks;
    
    // Callbacks
    DataCallback m_dataCallback;
    ConnectionCallback m_stateCallback;
    
    // Timing
    f32 m_timeSinceLastReceive = 0.0f;
    f32 m_timeSinceLastSend = 0.0f;
    f32 m_heartbeatTimer = 0.0f;
    
    void setState(ConnectionState newState);
    void updateRtt(f32 rttSample);
    bool processReliablePacket(const NetworkPacket& packet);
};

// ============================================================================
// NetworkServer - Server-side networking
// ============================================================================

/**
 * @brief Network server for hosting multiplayer games
 */
class NetworkServer {
public:
    NetworkServer();
    ~NetworkServer();
    
    // Disable copy
    NetworkServer(const NetworkServer&) = delete;
    NetworkServer& operator=(const NetworkServer&) = delete;
    
    /**
     * @brief Start the server
     * @param config Server configuration
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void> start(const ServerConfig& config);
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     */
    [[nodiscard]] bool isRunning() const noexcept { return m_running; }
    
    /**
     * @brief Update server (call each tick)
     * @param deltaTime Time since last update
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Send data to a specific connection
     */
    [[nodiscard]] Result<void> send(u64 connectionId, ChannelType channel,
                                     const u8* data, usize size,
                                     DeliveryMode mode = DeliveryMode::ReliableOrdered);
    
    /**
     * @brief Send data to all connections
     */
    void broadcast(ChannelType channel, const u8* data, usize size,
                   DeliveryMode mode = DeliveryMode::ReliableOrdered);
    
    /**
     * @brief Send data to all connections except one
     */
    void broadcastExcept(u64 excludeId, ChannelType channel,
                         const u8* data, usize size,
                         DeliveryMode mode = DeliveryMode::ReliableOrdered);
    
    /**
     * @brief Disconnect a client
     */
    void disconnect(u64 connectionId, const std::string& reason = "");
    
    /**
     * @brief Get all connected client IDs
     */
    [[nodiscard]] std::vector<u64> getConnectedClients() const;
    
    /**
     * @brief Get connection by ID
     */
    [[nodiscard]] NetworkConnection* getConnection(u64 connectionId);
    [[nodiscard]] const NetworkConnection* getConnection(u64 connectionId) const;
    
    /**
     * @brief Get connection count
     */
    [[nodiscard]] u32 getConnectionCount() const noexcept;
    
    /**
     * @brief Get server statistics
     */
    [[nodiscard]] const ServerStats& getStats() const noexcept { return m_stats; }
    
    /**
     * @brief Get server configuration
     */
    [[nodiscard]] const ServerConfig& getConfig() const noexcept { return m_config; }
    
    /// Set connection callback
    void setConnectionCallback(ConnectionCallback callback) { 
        m_connectionCallback = std::move(callback); 
    }
    
    /// Set data callback
    void setDataCallback(DataCallback callback) { 
        m_dataCallback = std::move(callback); 
    }
    
private:
    ServerConfig m_config;
    std::unique_ptr<NetworkSocket> m_socket;
    std::unordered_map<u64, std::unique_ptr<NetworkConnection>> m_connections;
    std::unordered_map<NetworkEndpoint, u64> m_endpointToConnection;
    
    ServerStats m_stats;
    
    std::atomic<bool> m_running{false};
    std::atomic<u64> m_nextConnectionId{1};
    
    mutable std::mutex m_mutex;
    
    // Callbacks
    ConnectionCallback m_connectionCallback;
    DataCallback m_dataCallback;
    
    // Receive buffer
    std::vector<u8> m_receiveBuffer;
    
    void processIncomingPackets();
    void handleConnectionRequest(const NetworkEndpoint& endpoint, const NetworkPacket& packet);
    void removeConnection(u64 connectionId);
};

// ============================================================================
// NetworkClient - Client-side networking
// ============================================================================

/**
 * @brief Network client for connecting to multiplayer servers
 */
class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();
    
    // Disable copy
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;
    
    /**
     * @brief Connect to a server
     * @param config Client configuration
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void> connect(const ClientConfig& config);
    
    /**
     * @brief Disconnect from server
     */
    void disconnect();
    
    /**
     * @brief Check if connected
     */
    [[nodiscard]] bool isConnected() const noexcept;
    
    /**
     * @brief Get connection state
     */
    [[nodiscard]] ConnectionState getState() const noexcept;
    
    /**
     * @brief Update client (call each tick)
     * @param deltaTime Time since last update
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Send data to server
     */
    [[nodiscard]] Result<void> send(ChannelType channel, const u8* data, usize size,
                                     DeliveryMode mode = DeliveryMode::ReliableOrdered);
    
    /**
     * @brief Get connection statistics
     */
    [[nodiscard]] const ConnectionStats& getStats() const;
    
    /**
     * @brief Get round-trip time in milliseconds
     */
    [[nodiscard]] f32 getRtt() const;
    
    /**
     * @brief Get packet loss percentage
     */
    [[nodiscard]] f32 getPacketLoss() const;
    
    /**
     * @brief Get assigned connection ID
     */
    [[nodiscard]] u64 getConnectionId() const noexcept { return m_connectionId; }
    
    /// Set connection callback
    void setConnectionCallback(ConnectionCallback callback) { 
        m_connectionCallback = std::move(callback); 
    }
    
    /// Set data callback
    void setDataCallback(DataCallback callback) { 
        m_dataCallback = std::move(callback); 
    }
    
    /**
     * @brief Discover servers on LAN
     * @param callback Called for each discovered server
     * @param timeoutMs How long to search (milliseconds)
     */
    void discoverLAN(DiscoveryCallback callback, u32 timeoutMs = 3000);
    
private:
    ClientConfig m_config;
    std::unique_ptr<NetworkSocket> m_socket;
    std::unique_ptr<NetworkConnection> m_connection;
    
    u64 m_connectionId = 0;
    ConnectionState m_state = ConnectionState::Disconnected;
    
    // Callbacks
    ConnectionCallback m_connectionCallback;
    DataCallback m_dataCallback;
    
    // Receive buffer
    std::vector<u8> m_receiveBuffer;
    
    // Handshake state
    u64 m_challengeToken = 0;
    std::array<u8, 32> m_clientRandom{};
    
    void processIncomingPackets();
    void handleConnectionChallenge(const NetworkPacket& packet);
    void handleConnectionAccepted(const NetworkPacket& packet);
    void handleConnectionRejected(const NetworkPacket& packet);
    void setState(ConnectionState newState);
};

// ============================================================================
// NetworkSystem - Global network system manager
// ============================================================================

/**
 * @brief Global network system singleton
 */
class NetworkSystem {
public:
    /// Get singleton instance
    [[nodiscard]] static NetworkSystem& instance();
    
    /**
     * @brief Initialize the network system
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void> initialize();
    
    /**
     * @brief Shutdown the network system
     */
    void shutdown();
    
    /**
     * @brief Check if network is available
     */
    [[nodiscard]] bool isAvailable() const noexcept { return m_initialized; }
    
    /**
     * @brief Create a server instance
     * @return Unique pointer to new server
     */
    [[nodiscard]] std::unique_ptr<NetworkServer> createServer();
    
    /**
     * @brief Create a client instance
     * @return Unique pointer to new client
     */
    [[nodiscard]] std::unique_ptr<NetworkClient> createClient();
    
    /**
     * @brief Get local IP addresses
     * @return Vector of local IP addresses
     */
    [[nodiscard]] std::vector<IPv4Address> getLocalAddresses() const;
    
    /**
     * @brief Resolve hostname to IP address
     * @param hostname Hostname to resolve
     * @return Resolved IP address or error
     */
    [[nodiscard]] Result<IPv4Address> resolveHostname(const std::string& hostname) const;
    
private:
    NetworkSystem() = default;
    ~NetworkSystem();
    
    NetworkSystem(const NetworkSystem&) = delete;
    NetworkSystem& operator=(const NetworkSystem&) = delete;
    
    bool m_initialized = false;
};

// ============================================================================
// Convenience Functions
// ============================================================================

/// Initialize networking
inline Result<void> initializeNetwork() {
    return NetworkSystem::instance().initialize();
}

/// Shutdown networking
inline void shutdownNetwork() {
    NetworkSystem::instance().shutdown();
}

/// Check if networking is available
inline bool isNetworkAvailable() {
    return NetworkSystem::instance().isAvailable();
}

/// Create a network server
inline std::unique_ptr<NetworkServer> createServer() {
    return NetworkSystem::instance().createServer();
}

/// Create a network client
inline std::unique_ptr<NetworkClient> createClient() {
    return NetworkSystem::instance().createClient();
}

} // namespace nova::network
