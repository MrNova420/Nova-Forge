/**
 * @file network_system.cpp
 * @brief Nova Network™ - Network system implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/network/network_system.hpp"
#include "nova/core/logging/logging.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <random>
#include <cstdint>

// Platform-specific socket headers and closeSocket function
#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
    // Windows uses closesocket
    inline int closeSocket(socket_t sock) { return ::closesocket(sock); }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <errno.h>
    using socket_t = int;
    #define INVALID_SOCKET_VALUE (-1)
    #define SOCKET_ERROR_VALUE (-1)
    // Unix/Linux uses close - use :: to avoid conflict with class method
    inline int closeSocket(socket_t sock) { return ::close(sock); }
#endif

namespace nova::network {

using namespace nova::logging;

// Helper to cast void* to socket_t safely
inline socket_t toSocket(void* ptr) {
    return static_cast<socket_t>(reinterpret_cast<std::intptr_t>(ptr));
}

// Helper to cast socket_t to void*
inline void* fromSocket(socket_t sock) {
    return reinterpret_cast<void*>(static_cast<std::intptr_t>(sock));
}

// ============================================================================
// IPv4Address Implementation
// ============================================================================

IPv4Address IPv4Address::fromString(const std::string& str) {
    IPv4Address addr;
    unsigned int a, b, c, d;
    if (sscanf(str.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d) == 4) {
        addr.octets[0] = static_cast<u8>(a);
        addr.octets[1] = static_cast<u8>(b);
        addr.octets[2] = static_cast<u8>(c);
        addr.octets[3] = static_cast<u8>(d);
    }
    return addr;
}

std::string IPv4Address::toString() const {
    std::ostringstream oss;
    oss << static_cast<int>(octets[0]) << "."
        << static_cast<int>(octets[1]) << "."
        << static_cast<int>(octets[2]) << "."
        << static_cast<int>(octets[3]);
    return oss.str();
}

// ============================================================================
// NetworkEndpoint Implementation
// ============================================================================

NetworkEndpoint NetworkEndpoint::fromString(const std::string& str) {
    NetworkEndpoint endpoint;
    auto colonPos = str.rfind(':');
    if (colonPos != std::string::npos) {
        endpoint.address = IPv4Address::fromString(str.substr(0, colonPos));
        endpoint.port = static_cast<u16>(std::stoi(str.substr(colonPos + 1)));
    }
    return endpoint;
}

std::string NetworkEndpoint::toString() const {
    return address.toString() + ":" + std::to_string(port);
}

// Comparison operator for NetworkEndpoint (used for std::map)
bool operator<(const NetworkEndpoint& a, const NetworkEndpoint& b) {
    if (a.address.value != b.address.value) {
        return a.address.value < b.address.value;
    }
    return a.port < b.port;
}

// ============================================================================
// NetworkPacket Implementation
// ============================================================================

NetworkPacket NetworkPacket::create(PacketType type, ChannelType channel) {
    NetworkPacket packet;
    packet.header.magic = PACKET_MAGIC;
    packet.header.protocolVersion = PROTOCOL_VERSION;
    packet.header.type = type;
    packet.header.channel = channel;
    packet.header.flags = 0;
    packet.header.fragmentInfo = 0;
    packet.header.sequenceNumber = 0;
    packet.timestamp = std::chrono::steady_clock::now();
    return packet;
}

std::vector<u8> NetworkPacket::serialize() const {
    std::vector<u8> data;
    data.reserve(sizeof(PacketHeader) + payload.size());
    
    // Serialize header
    const u8* headerBytes = reinterpret_cast<const u8*>(&header);
    data.insert(data.end(), headerBytes, headerBytes + sizeof(PacketHeader));
    
    // Append payload
    data.insert(data.end(), payload.begin(), payload.end());
    
    return data;
}

std::optional<NetworkPacket> NetworkPacket::deserialize(const std::vector<u8>& data) {
    return deserialize(data.data(), data.size());
}

std::optional<NetworkPacket> NetworkPacket::deserialize(const u8* data, usize size) {
    if (size < sizeof(PacketHeader)) {
        return std::nullopt;
    }
    
    NetworkPacket packet;
    std::memcpy(&packet.header, data, sizeof(PacketHeader));
    
    // Validate magic
    if (packet.header.magic != PACKET_MAGIC) {
        return std::nullopt;
    }
    
    // Copy payload
    if (size > sizeof(PacketHeader)) {
        packet.payload.assign(data + sizeof(PacketHeader), data + size);
    }
    
    packet.timestamp = std::chrono::steady_clock::now();
    
    return packet;
}

// ============================================================================
// NetworkSocket Implementation
// ============================================================================

NetworkSocket::~NetworkSocket() {
    close();
}

NetworkSocket::NetworkSocket(NetworkSocket&& other) noexcept
    : m_handle(other.m_handle)
    , m_protocol(other.m_protocol)
    , m_bound(other.m_bound)
{
    other.m_handle = nullptr;
    other.m_bound = false;
}

NetworkSocket& NetworkSocket::operator=(NetworkSocket&& other) noexcept {
    if (this != &other) {
        close();
        m_handle = other.m_handle;
        m_protocol = other.m_protocol;
        m_bound = other.m_bound;
        other.m_handle = nullptr;
        other.m_bound = false;
    }
    return *this;
}

Result<void> NetworkSocket::bind(SocketProtocol protocol, u16 port) {
    if (m_bound) {
        close();
    }
    
    m_protocol = protocol;
    
    int sockType = (protocol == SocketProtocol::UDP) ? SOCK_DGRAM : SOCK_STREAM;
    socket_t sock = socket(AF_INET, sockType, 0);
    
    if (sock == INVALID_SOCKET_VALUE) {
        return std::unexpected(errors::io("Failed to create socket"));
    }
    
    // Enable address reuse
    int reuseAddr = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
               reinterpret_cast<const char*>(&reuseAddr), sizeof(reuseAddr));
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR_VALUE) {
        closeSocket(sock);
        return std::unexpected(errors::io("Failed to bind socket to port " + std::to_string(port)));
    }
    
    m_handle = fromSocket(sock);
    m_bound = true;
    
    NOVA_LOG_DEBUG(LogCategory::Core, "Socket bound to port {}", port);
    
    return {};
}

void NetworkSocket::close() {
    if (m_handle != nullptr) {
        closeSocket(toSocket(m_handle));
        m_handle = nullptr;
        m_bound = false;
    }
}

bool NetworkSocket::isOpen() const noexcept {
    return m_handle != nullptr && m_bound;
}

Result<usize> NetworkSocket::sendTo(const NetworkEndpoint& endpoint, 
                                     const u8* data, usize size) {
    if (!isOpen()) {
        return std::unexpected(errors::invalidArgument("Socket not open"));
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = endpoint.address.value;
    addr.sin_port = htons(endpoint.port);
    
    auto sock = toSocket(m_handle);
    auto sent = sendto(sock, reinterpret_cast<const char*>(data), 
                       static_cast<int>(size), 0,
                       reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    
    if (sent == SOCKET_ERROR_VALUE) {
        return std::unexpected(errors::io("Failed to send data"));
    }
    
    return static_cast<usize>(sent);
}

Result<usize> NetworkSocket::receiveFrom(NetworkEndpoint& endpoint,
                                          u8* buffer, usize bufferSize) {
    if (!isOpen()) {
        return std::unexpected(errors::invalidArgument("Socket not open"));
    }
    
    sockaddr_in addr{};
    socklen_t addrLen = sizeof(addr);
    
    auto sock = toSocket(m_handle);
    auto received = recvfrom(sock, reinterpret_cast<char*>(buffer), 
                             static_cast<int>(bufferSize), 0,
                             reinterpret_cast<sockaddr*>(&addr), &addrLen);
    
    if (received == SOCKET_ERROR_VALUE) {
#if defined(_WIN32)
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            return static_cast<usize>(0);  // No data available
        }
#else
        // EAGAIN and EWOULDBLOCK may be same value on some systems
        // Checking EAGAIN covers both cases on Linux (where they're equal)
        // Using explicit check to avoid compiler warning about identical expressions
        if (errno == EAGAIN) {
            return static_cast<usize>(0);  // No data available
        }
#if EAGAIN != EWOULDBLOCK
        if (errno == EWOULDBLOCK) {
            return static_cast<usize>(0);  // No data available
        }
#endif
#endif
        return std::unexpected(errors::io("Failed to receive data"));
    }
    
    endpoint.address.value = addr.sin_addr.s_addr;
    endpoint.port = ntohs(addr.sin_port);
    
    return static_cast<usize>(received);
}

Result<void> NetworkSocket::setNonBlocking(bool nonBlocking) {
    if (!isOpen()) {
        return std::unexpected(errors::invalidArgument("Socket not open"));
    }
    
    auto sock = toSocket(m_handle);
    
#if defined(_WIN32)
    u_long mode = nonBlocking ? 1 : 0;
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        return std::unexpected(errors::io("Failed to set non-blocking mode"));
    }
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        return std::unexpected(errors::io("Failed to get socket flags"));
    }
    if (nonBlocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if (fcntl(sock, F_SETFL, flags) == -1) {
        return std::unexpected(errors::io("Failed to set non-blocking mode"));
    }
#endif
    
    return {};
}

Result<void> NetworkSocket::setReceiveBufferSize(u32 size) {
    if (!isOpen()) {
        return std::unexpected(errors::invalidArgument("Socket not open"));
    }
    
    auto sock = toSocket(m_handle);
    int bufSize = static_cast<int>(size);
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, 
                   reinterpret_cast<const char*>(&bufSize), sizeof(bufSize)) != 0) {
        return std::unexpected(errors::io("Failed to set receive buffer size"));
    }
    
    return {};
}

Result<void> NetworkSocket::setSendBufferSize(u32 size) {
    if (!isOpen()) {
        return std::unexpected(errors::invalidArgument("Socket not open"));
    }
    
    auto sock = toSocket(m_handle);
    int bufSize = static_cast<int>(size);
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, 
                   reinterpret_cast<const char*>(&bufSize), sizeof(bufSize)) != 0) {
        return std::unexpected(errors::io("Failed to set send buffer size"));
    }
    
    return {};
}

NetworkEndpoint NetworkSocket::getLocalEndpoint() const {
    NetworkEndpoint endpoint;
    if (!isOpen()) {
        return endpoint;
    }
    
    auto sock = toSocket(m_handle);
    sockaddr_in addr{};
    socklen_t addrLen = sizeof(addr);
    
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &addrLen) == 0) {
        endpoint.address.value = addr.sin_addr.s_addr;
        endpoint.port = ntohs(addr.sin_port);
    }
    
    return endpoint;
}

// ============================================================================
// NetworkConnection Implementation
// ============================================================================

NetworkConnection::NetworkConnection(u64 id, const NetworkEndpoint& endpoint)
    : m_id(id)
    , m_endpoint(endpoint)
{
    m_stats.connectionStarted = std::chrono::steady_clock::now();
}

NetworkConnection::~NetworkConnection() {
    disconnect(false);
}

Result<void> NetworkConnection::send(ChannelType channel, const u8* data, usize size,
                                      DeliveryMode mode) {
    if (m_state != ConnectionState::Connected) {
        return std::unexpected(errors::invalidArgument("Not connected"));
    }
    
    // Create packet
    auto packet = NetworkPacket::create(
        mode == DeliveryMode::Unreliable ? PacketType::UnreliableData : PacketType::ReliableData,
        channel
    );
    
    packet.header.sequenceNumber = m_localSequence++;
    
    if (mode == DeliveryMode::Reliable || 
        mode == DeliveryMode::ReliableOrdered || 
        mode == DeliveryMode::ReliableSequenced) {
        packet.header.flags |= PacketHeader::FLAG_RELIABLE;
    }
    
    packet.payload.assign(data, data + size);
    
    return sendPacket(packet);
}

Result<void> NetworkConnection::sendPacket(const NetworkPacket& packet) {
    m_sendQueue.push_back(packet);
    
    if (packet.header.isReliable()) {
        m_pendingAcks.push_back(packet);
    }
    
    return {};
}

void NetworkConnection::disconnect(bool graceful) {
    if (m_state == ConnectionState::Disconnected) {
        return;
    }
    
    if (graceful && m_state == ConnectionState::Connected) {
        // Send disconnect packet
        auto packet = NetworkPacket::create(PacketType::Disconnect);
        m_sendQueue.push_back(packet);
        setState(ConnectionState::Disconnecting);
    } else {
        setState(ConnectionState::Disconnected);
    }
}

void NetworkConnection::processPacket(const NetworkPacket& packet) {
    m_stats.packetsReceived++;
    m_stats.bytesReceived += packet.totalSize();
    m_stats.lastPacketReceived = std::chrono::steady_clock::now();
    m_timeSinceLastReceive = 0.0f;
    
    // Update remote sequence
    if (packet.header.sequenceNumber > m_remoteSequence || 
        m_remoteSequence - packet.header.sequenceNumber > 0x8000) {
        m_remoteSequence = packet.header.sequenceNumber;
    } else {
        m_stats.packetsOutOfOrder++;
    }
    
    // Process by type
    switch (packet.header.type) {
        case PacketType::Heartbeat:
            // Send heartbeat ack
            break;
            
        case PacketType::HeartbeatAck:
            // Update RTT
            break;
            
        case PacketType::Ack:
            processReliablePacket(packet);
            break;
            
        case PacketType::UnreliableData:
        case PacketType::ReliableData:
            if (m_dataCallback) {
                m_dataCallback(m_id, packet.header.channel, 
                              packet.payload.data(), packet.payload.size());
            }
            
            // Send ack for reliable
            if (packet.header.isReliable()) {
                auto ack = NetworkPacket::create(PacketType::Ack, packet.header.channel);
                ack.header.sequenceNumber = packet.header.sequenceNumber;
                m_sendQueue.push_back(ack);
            }
            break;
            
        case PacketType::Disconnect:
            setState(ConnectionState::Disconnected);
            break;
            
        default:
            break;
    }
}

void NetworkConnection::update(f32 deltaTime) {
    m_timeSinceLastReceive += deltaTime;
    m_timeSinceLastSend += deltaTime;
    m_heartbeatTimer += deltaTime;
    
    // Check timeout
    if (m_timeSinceLastReceive > DEFAULT_TIMEOUT_MS / 1000.0f) {
        setState(ConnectionState::TimedOut);
        return;
    }
    
    // Send heartbeat
    if (m_heartbeatTimer >= HEARTBEAT_INTERVAL_MS / 1000.0f) {
        m_heartbeatTimer = 0.0f;
        auto heartbeat = NetworkPacket::create(PacketType::Heartbeat);
        m_sendQueue.push_back(heartbeat);
    }
    
    // Update statistics
    m_stats.connectionQuality = 1.0f - std::min(1.0f, m_stats.packetLoss / 100.0f);
}

void NetworkConnection::setState(ConnectionState newState) {
    if (m_state != newState) {
#if NOVA_LOG_LEVEL_DEBUG_ENABLED
        ConnectionState oldState = m_state;
#endif
        m_state = newState;
        
        NOVA_LOG_DEBUG(LogCategory::Core, "Connection {} state: {} -> {}", 
                      m_id, static_cast<int>(oldState), static_cast<int>(newState));
        
        if (m_stateCallback) {
            m_stateCallback(m_id, newState);
        }
    }
}

void NetworkConnection::updateRtt(f32 rttSample) {
    // Exponential moving average for RTT
    constexpr f32 alpha = 0.125f;
    m_stats.rttMs = (1.0f - alpha) * m_stats.rttMs + alpha * rttSample;
    
    // RTT variance for timeout calculation
    f32 rttDiff = std::abs(rttSample - m_stats.rttMs);
    m_stats.rttVariance = (1.0f - alpha) * m_stats.rttVariance + alpha * rttDiff;
}

bool NetworkConnection::processReliablePacket(const NetworkPacket& packet) {
    // Remove acknowledged packets from pending
    u16 ackSeq = packet.header.sequenceNumber;
    
    m_pendingAcks.erase(
        std::remove_if(m_pendingAcks.begin(), m_pendingAcks.end(),
            [ackSeq](const NetworkPacket& p) {
                return p.header.sequenceNumber == ackSeq;
            }),
        m_pendingAcks.end()
    );
    
    m_stats.reliableAcked++;
    return true;
}

// ============================================================================
// NetworkServer Implementation
// ============================================================================

NetworkServer::NetworkServer() {
    m_receiveBuffer.resize(MAX_PACKET_SIZE);
}

NetworkServer::~NetworkServer() {
    stop();
}

Result<void> NetworkServer::start(const ServerConfig& config) {
    if (m_running) {
        return std::unexpected(errors::invalidArgument("Server already running"));
    }
    
    m_config = config;
    
    // Create and bind socket
    m_socket = std::make_unique<NetworkSocket>();
    auto bindResult = m_socket->bind(SocketProtocol::UDP, config.port);
    if (!bindResult) {
        m_socket.reset();
        return std::unexpected(bindResult.error());
    }
    
    // Set non-blocking
    auto nbResult = m_socket->setNonBlocking(true);
    if (!nbResult) {
        m_socket.reset();
        return std::unexpected(nbResult.error());
    }
    
    // Set buffer sizes (ignore return value - non-critical)
    (void)m_socket->setReceiveBufferSize(1024 * 1024);  // 1MB
    (void)m_socket->setSendBufferSize(1024 * 1024);     // 1MB
    
    m_running = true;
    
    NOVA_LOG_INFO(LogCategory::Core, "Server started on port {}", config.port);
    
    return {};
}

void NetworkServer::stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    // Disconnect all clients
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [id, conn] : m_connections) {
            conn->disconnect(true);
        }
        m_connections.clear();
        m_endpointToConnection.clear();
    }
    
    m_socket.reset();
    
    NOVA_LOG_INFO(LogCategory::Core, "Server stopped");
}

void NetworkServer::update(f32 deltaTime) {
    if (!m_running) {
        return;
    }
    
    // Process incoming packets
    processIncomingPackets();
    
    // Update all connections
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<u64> toRemove;
    for (auto& [id, conn] : m_connections) {
        conn->update(deltaTime);
        
        // Check for disconnected/timed out connections
        if (conn->getState() == ConnectionState::Disconnected ||
            conn->getState() == ConnectionState::TimedOut) {
            toRemove.push_back(id);
        }
        
        // Send pending packets
        // Note: In production, this would be optimized with a proper send queue
    }
    
    // Remove disconnected connections
    for (u64 id : toRemove) {
        removeConnection(id);
    }
    
    // Update stats
    m_stats.currentConnections = static_cast<u32>(m_connections.size());
    if (m_stats.currentConnections > m_stats.peakConnections) {
        m_stats.peakConnections = m_stats.currentConnections;
    }
}

Result<void> NetworkServer::send(u64 connectionId, ChannelType channel,
                                  const u8* data, usize size,
                                  DeliveryMode mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(connectionId);
    if (it == m_connections.end()) {
        return std::unexpected(errors::notFound("Connection not found"));
    }
    
    return it->second->send(channel, data, size, mode);
}

void NetworkServer::broadcast(ChannelType channel, const u8* data, usize size,
                              DeliveryMode mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& [id, conn] : m_connections) {
        if (conn->isConnected()) {
            (void)conn->send(channel, data, size, mode);
        }
    }
}

void NetworkServer::broadcastExcept(u64 excludeId, ChannelType channel,
                                     const u8* data, usize size,
                                     DeliveryMode mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& [id, conn] : m_connections) {
        if (id != excludeId && conn->isConnected()) {
            (void)conn->send(channel, data, size, mode);
        }
    }
}

void NetworkServer::disconnect(u64 connectionId, const std::string& reason) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(connectionId);
    if (it != m_connections.end()) {
        it->second->disconnect(true);
    }
}

std::vector<u64> NetworkServer::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<u64> clients;
    clients.reserve(m_connections.size());
    
    for (const auto& [id, conn] : m_connections) {
        if (conn->isConnected()) {
            clients.push_back(id);
        }
    }
    
    return clients;
}

NetworkConnection* NetworkServer::getConnection(u64 connectionId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(connectionId);
    return (it != m_connections.end()) ? it->second.get() : nullptr;
}

const NetworkConnection* NetworkServer::getConnection(u64 connectionId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(connectionId);
    return (it != m_connections.end()) ? it->second.get() : nullptr;
}

u32 NetworkServer::getConnectionCount() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<u32>(m_connections.size());
}

void NetworkServer::processIncomingPackets() {
    NetworkEndpoint source;
    
    while (true) {
        auto result = m_socket->receiveFrom(source, m_receiveBuffer.data(), m_receiveBuffer.size());
        if (!result || *result == 0) {
            break;
        }
        
        auto packet = NetworkPacket::deserialize(m_receiveBuffer.data(), *result);
        if (!packet) {
            continue;  // Invalid packet
        }
        
        packet->source = source;
        
        // Handle connection requests
        if (packet->header.type == PacketType::ConnectionRequest) {
            handleConnectionRequest(source, *packet);
            continue;
        }
        
        // Find existing connection
        std::lock_guard<std::mutex> lock(m_mutex);
        auto epIt = m_endpointToConnection.find(source);
        if (epIt != m_endpointToConnection.end()) {
            auto connIt = m_connections.find(epIt->second);
            if (connIt != m_connections.end()) {
                connIt->second->processPacket(*packet);
            }
        }
        
        m_stats.totalPacketsReceived++;
        m_stats.totalBytesReceived += *result;
    }
}

void NetworkServer::handleConnectionRequest(const NetworkEndpoint& endpoint, 
                                             const NetworkPacket& packet) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if already connected
    if (m_endpointToConnection.find(endpoint) != m_endpointToConnection.end()) {
        return;  // Already connected
    }
    
    // Check max connections
    if (m_connections.size() >= m_config.maxConnections) {
        // Send rejection
        auto rejection = NetworkPacket::create(PacketType::ConnectionRejected);
        auto rejectionData = rejection.serialize();
        (void)m_socket->sendTo(endpoint, rejectionData.data(), rejectionData.size());
        return;
    }
    
    // Create new connection
    u64 connId = m_nextConnectionId.fetch_add(1, std::memory_order_relaxed);
    auto conn = std::make_unique<NetworkConnection>(connId, endpoint);
    
    // Set callbacks
    conn->setDataCallback(m_dataCallback);
    conn->setStateCallback([this](u64 id, ConnectionState state) {
        if (m_connectionCallback) {
            m_connectionCallback(id, state);
        }
    });
    
    m_endpointToConnection[endpoint] = connId;
    m_connections[connId] = std::move(conn);
    
    // Send challenge
    auto challenge = NetworkPacket::create(PacketType::ConnectionChallenge);
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    ConnectionChallenge challengeData;
    challengeData.challengeToken = gen();
    challengeData.serverTime = static_cast<u32>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    challenge.payload.resize(sizeof(ConnectionChallenge));
    std::memcpy(challenge.payload.data(), &challengeData, sizeof(ConnectionChallenge));
    
    auto serialized = challenge.serialize();
    (void)m_socket->sendTo(endpoint, serialized.data(), serialized.size());
    
    m_stats.totalConnections++;
    
    NOVA_LOG_INFO(LogCategory::Core, "New connection {} from {}", connId, endpoint.toString());
}

void NetworkServer::removeConnection(u64 connectionId) {
    auto it = m_connections.find(connectionId);
    if (it != m_connections.end()) {
        // Remove from endpoint map
        const auto& endpoint = it->second->getEndpoint();
        m_endpointToConnection.erase(endpoint);
        
        // Notify callback
        if (m_connectionCallback) {
            m_connectionCallback(connectionId, ConnectionState::Disconnected);
        }
        
        m_connections.erase(it);
        
        NOVA_LOG_INFO(LogCategory::Core, "Connection {} removed", connectionId);
    }
}

// ============================================================================
// NetworkClient Implementation
// ============================================================================

NetworkClient::NetworkClient() {
    m_receiveBuffer.resize(MAX_PACKET_SIZE);
    
    // Generate client random
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<u32> dist;
    for (auto& byte : m_clientRandom) {
        byte = static_cast<u8>(dist(gen));
    }
}

NetworkClient::~NetworkClient() {
    disconnect();
}

Result<void> NetworkClient::connect(const ClientConfig& config) {
    if (m_state != ConnectionState::Disconnected) {
        return std::unexpected(errors::invalidArgument("Already connected or connecting"));
    }
    
    m_config = config;
    
    // Create socket
    m_socket = std::make_unique<NetworkSocket>();
    auto bindResult = m_socket->bind(SocketProtocol::UDP, 0);  // Any port
    if (!bindResult) {
        m_socket.reset();
        return std::unexpected(bindResult.error());
    }
    
    // Set non-blocking
    auto nbResult = m_socket->setNonBlocking(true);
    if (!nbResult) {
        m_socket.reset();
        return std::unexpected(nbResult.error());
    }
    
    // Create connection object
    m_connection = std::make_unique<NetworkConnection>(0, config.serverEndpoint);
    m_connection->setDataCallback(m_dataCallback);
    
    // Send connection request
    auto request = NetworkPacket::create(PacketType::ConnectionRequest);
    
    ConnectionRequest reqData;
    reqData.protocolVersion = PROTOCOL_VERSION;
    reqData.playerName = config.playerName;
    reqData.authToken = config.authToken;
    reqData.clientRandom = m_clientRandom;
    
    // Serialize request (simplified - production would use proper serialization)
    request.payload.resize(sizeof(ConnectionRequest));
    std::memcpy(request.payload.data(), &reqData, sizeof(ConnectionRequest));
    
    auto sendResult = m_socket->sendTo(config.serverEndpoint, 
                                        request.serialize().data(), 
                                        request.totalSize());
    if (!sendResult) {
        m_socket.reset();
        m_connection.reset();
        return std::unexpected(sendResult.error());
    }
    
    setState(ConnectionState::Connecting);
    
    NOVA_LOG_INFO(LogCategory::Core, "Connecting to {}", config.serverEndpoint.toString());
    
    return {};
}

void NetworkClient::disconnect() {
    if (m_state == ConnectionState::Disconnected) {
        return;
    }
    
    if (m_connection && m_state == ConnectionState::Connected) {
        m_connection->disconnect(true);
        
        // Send disconnect packet immediately
        auto packet = NetworkPacket::create(PacketType::Disconnect);
        auto serialized = packet.serialize();
        (void)m_socket->sendTo(m_config.serverEndpoint, serialized.data(), serialized.size());
    }
    
    m_socket.reset();
    m_connection.reset();
    
    setState(ConnectionState::Disconnected);
    
    NOVA_LOG_INFO(LogCategory::Core, "Disconnected");
}

bool NetworkClient::isConnected() const noexcept {
    return m_state == ConnectionState::Connected;
}

ConnectionState NetworkClient::getState() const noexcept {
    return m_state;
}

void NetworkClient::update(f32 deltaTime) {
    if (m_state == ConnectionState::Disconnected) {
        return;
    }
    
    // Process incoming packets
    processIncomingPackets();
    
    // Update connection
    if (m_connection) {
        m_connection->update(deltaTime);
        
        // Check for timeout during connecting
        if (m_state == ConnectionState::Connecting) {
            // Check timeout handled by connection
        }
    }
}

Result<void> NetworkClient::send(ChannelType channel, const u8* data, usize size,
                                  DeliveryMode mode) {
    if (!isConnected()) {
        return std::unexpected(errors::invalidArgument("Not connected"));
    }
    
    return m_connection->send(channel, data, size, mode);
}

const ConnectionStats& NetworkClient::getStats() const {
    static ConnectionStats empty;
    return m_connection ? m_connection->getStats() : empty;
}

f32 NetworkClient::getRtt() const {
    return m_connection ? m_connection->getRtt() : 0.0f;
}

f32 NetworkClient::getPacketLoss() const {
    return m_connection ? m_connection->getPacketLoss() : 0.0f;
}

void NetworkClient::discoverLAN(DiscoveryCallback callback, u32 timeoutMs) {
    // LAN discovery would send broadcast and listen for responses
    // Simplified implementation - production would be more sophisticated
    NOVA_LOG_DEBUG(LogCategory::Core, "Starting LAN discovery (timeout: {}ms)", timeoutMs);
}

void NetworkClient::processIncomingPackets() {
    NetworkEndpoint source;
    
    while (true) {
        auto result = m_socket->receiveFrom(source, m_receiveBuffer.data(), m_receiveBuffer.size());
        if (!result || *result == 0) {
            break;
        }
        
        auto packet = NetworkPacket::deserialize(m_receiveBuffer.data(), *result);
        if (!packet) {
            continue;
        }
        
        // Verify source is our server
        if (source != m_config.serverEndpoint) {
            continue;
        }
        
        packet->source = source;
        
        // Handle handshake packets
        switch (packet->header.type) {
            case PacketType::ConnectionChallenge:
                handleConnectionChallenge(*packet);
                break;
                
            case PacketType::ConnectionAccepted:
                handleConnectionAccepted(*packet);
                break;
                
            case PacketType::ConnectionRejected:
                handleConnectionRejected(*packet);
                break;
                
            default:
                if (m_connection) {
                    m_connection->processPacket(*packet);
                }
                break;
        }
    }
}

void NetworkClient::handleConnectionChallenge(const NetworkPacket& packet) {
    if (m_state != ConnectionState::Connecting) {
        return;
    }
    
    if (packet.payload.size() < sizeof(ConnectionChallenge)) {
        return;
    }
    
    ConnectionChallenge challenge;
    std::memcpy(&challenge, packet.payload.data(), sizeof(ConnectionChallenge));
    
    m_challengeToken = challenge.challengeToken;
    
    // Send response
    auto response = NetworkPacket::create(PacketType::ConnectionResponse);
    
    ConnectionResponse respData;
    respData.challengeToken = challenge.challengeToken;
    // In production, would compute proper challenge proof
    
    response.payload.resize(sizeof(ConnectionResponse));
    std::memcpy(response.payload.data(), &respData, sizeof(ConnectionResponse));
    
    auto serialized = response.serialize();
    (void)m_socket->sendTo(m_config.serverEndpoint, serialized.data(), serialized.size());
    
    NOVA_LOG_DEBUG(LogCategory::Core, "Received challenge, sending response");
}

void NetworkClient::handleConnectionAccepted(const NetworkPacket& packet) {
    if (m_state != ConnectionState::Connecting) {
        return;
    }
    
    if (packet.payload.size() < sizeof(ConnectionAccepted)) {
        return;
    }
    
    ConnectionAccepted accepted;
    std::memcpy(&accepted, packet.payload.data(), sizeof(ConnectionAccepted));
    
    m_connectionId = accepted.connectionId;
    
    setState(ConnectionState::Connected);
    
    NOVA_LOG_INFO(LogCategory::Core, "Connection accepted, ID: {}", m_connectionId);
}

void NetworkClient::handleConnectionRejected(const NetworkPacket& packet) {
    // Parse rejection reason (simple format: reason byte + message string)
    if (packet.payload.size() >= 1) {
        NetworkError reason = static_cast<NetworkError>(packet.payload[0]);
        std::string message;
        if (packet.payload.size() > 1) {
            message.assign(
                reinterpret_cast<const char*>(packet.payload.data() + 1),
                packet.payload.size() - 1
            );
        }
        
        NOVA_LOG_WARN(LogCategory::Core, "Connection rejected (reason {}): {}", 
                      static_cast<int>(reason), message);
    }
    
    setState(ConnectionState::Rejected);
    
    m_socket.reset();
    m_connection.reset();
}

void NetworkClient::setState(ConnectionState newState) {
    if (m_state != newState) {
        m_state = newState;
        
        if (m_connectionCallback) {
            m_connectionCallback(m_connectionId, newState);
        }
    }
}

// ============================================================================
// NetworkSystem Implementation
// ============================================================================

NetworkSystem& NetworkSystem::instance() {
    static NetworkSystem instance;
    return instance;
}

NetworkSystem::~NetworkSystem() {
    shutdown();
}

Result<void> NetworkSystem::initialize() {
    if (m_initialized) {
        return {};
    }
    
#if defined(_WIN32)
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        return std::unexpected(errors::io("Failed to initialize Winsock"));
    }
#endif
    
    m_initialized = true;
    
    NOVA_LOG_INFO(LogCategory::Core, "Network system initialized");
    
    return {};
}

void NetworkSystem::shutdown() {
    if (!m_initialized) {
        return;
    }
    
#if defined(_WIN32)
    WSACleanup();
#endif
    
    m_initialized = false;
    
    NOVA_LOG_INFO(LogCategory::Core, "Network system shutdown");
}

std::unique_ptr<NetworkServer> NetworkSystem::createServer() {
    if (!m_initialized) {
        NOVA_LOG_ERROR(LogCategory::Core, "Network system not initialized");
        return nullptr;
    }
    return std::make_unique<NetworkServer>();
}

std::unique_ptr<NetworkClient> NetworkSystem::createClient() {
    if (!m_initialized) {
        NOVA_LOG_ERROR(LogCategory::Core, "Network system not initialized");
        return nullptr;
    }
    return std::make_unique<NetworkClient>();
}

std::vector<IPv4Address> NetworkSystem::getLocalAddresses() const {
    std::vector<IPv4Address> addresses;
    
#if defined(_WIN32)
    // Windows implementation would use GetAdaptersAddresses
    addresses.push_back(IPv4Address::localhost());
#else
    // Linux/macOS - simplified
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct hostent* host = gethostbyname(hostname);
        if (host && host->h_addrtype == AF_INET) {
            for (int i = 0; host->h_addr_list[i] != nullptr; i++) {
                struct in_addr* addr = reinterpret_cast<struct in_addr*>(host->h_addr_list[i]);
                addresses.push_back(IPv4Address(addr->s_addr));
            }
        }
    }
    
    if (addresses.empty()) {
        addresses.push_back(IPv4Address::localhost());
    }
#endif
    
    return addresses;
}

Result<IPv4Address> NetworkSystem::resolveHostname(const std::string& hostname) const {
    struct hostent* host = gethostbyname(hostname.c_str());
    if (!host || host->h_addrtype != AF_INET || !host->h_addr_list[0]) {
        return std::unexpected(errors::notFound("Failed to resolve hostname: " + hostname));
    }
    
    struct in_addr* addr = reinterpret_cast<struct in_addr*>(host->h_addr_list[0]);
    return IPv4Address(addr->s_addr);
}

} // namespace nova::network
