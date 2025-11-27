/**
 * @file network_types.hpp
 * @brief Nova Network™ - Core networking types and structures
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Production-grade networking system supporting 10,000+ concurrent players.
 * 
 * Features:
 * - UDP/TCP socket abstraction
 * - Reliable UDP with packet ordering
 * - Connection management
 * - Bandwidth estimation
 * - Packet fragmentation and reassembly
 * - Delta compression
 * - Encryption support
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec3.hpp"

#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <variant>
#include <optional>

namespace nova::network {

// Import types from nova namespace
using nova::u8;
using nova::u16;
using nova::u32;
using nova::u64;
using nova::i32;
using nova::f32;
using nova::usize;

// ============================================================================
// Forward Declarations
// ============================================================================

class NetworkSocket;
class NetworkConnection;
class NetworkServer;
class NetworkClient;
class NetworkChannel;
class NetworkReplicator;

// ============================================================================
// Network Constants
// ============================================================================

/// Maximum transmission unit (MTU) - safe for most networks
constexpr u16 DEFAULT_MTU = 1200;

/// Maximum packet size including headers
constexpr u16 MAX_PACKET_SIZE = 1400;

/// Maximum payload size per packet
constexpr u16 MAX_PAYLOAD_SIZE = 1200;

/// Maximum number of fragments per message
constexpr u8 MAX_FRAGMENTS = 32;

/// Maximum reliable window size
constexpr u16 MAX_RELIABLE_WINDOW = 512;

/// Default connection timeout in milliseconds
constexpr u32 DEFAULT_TIMEOUT_MS = 10000;

/// Heartbeat interval in milliseconds
constexpr u32 HEARTBEAT_INTERVAL_MS = 1000;

/// Maximum players per server (following 10,000+ target)
constexpr u32 MAX_PLAYERS_PER_SERVER = 10000;

/// Protocol version for compatibility checking
constexpr u32 PROTOCOL_VERSION = 1;

/// Magic number for packet validation
constexpr u32 PACKET_MAGIC = 0x4E4F5641; // "NOVA"

// ============================================================================
// Network Enumerations
// ============================================================================

/**
 * @brief Socket protocol type
 */
enum class SocketProtocol : u8 {
    UDP,        ///< User Datagram Protocol (default for games)
    TCP,        ///< Transmission Control Protocol
    WebSocket   ///< WebSocket for web clients
};

/**
 * @brief Connection state machine states
 */
enum class ConnectionState : u8 {
    Disconnected,       ///< Not connected
    Connecting,         ///< Connection handshake in progress
    Connected,          ///< Fully connected and authenticated
    Disconnecting,      ///< Clean disconnect in progress
    TimedOut,           ///< Connection timed out
    Rejected,           ///< Connection rejected by server
    Failed              ///< Connection failed (error state)
};

/**
 * @brief Packet delivery mode
 */
enum class DeliveryMode : u8 {
    Unreliable,             ///< No guarantee of delivery or order
    UnreliableSequenced,    ///< Dropped if out of order
    Reliable,               ///< Guaranteed delivery, not ordered
    ReliableOrdered,        ///< Guaranteed delivery and order
    ReliableSequenced       ///< Guaranteed delivery, only latest matters
};

/**
 * @brief Network channel types
 */
enum class ChannelType : u8 {
    Default = 0,        ///< Default mixed channel
    Movement,           ///< Position/velocity updates
    Combat,             ///< Combat actions
    Chat,               ///< Chat messages
    VoIP,               ///< Voice over IP
    Replication,        ///< Object replication
    RPC,                ///< Remote procedure calls
    FileTransfer,       ///< Large file transfers
    Custom1,            ///< Custom channel 1
    Custom2,            ///< Custom channel 2
    Custom3,            ///< Custom channel 3
    Custom4,            ///< Custom channel 4
    MaxChannels = 16    ///< Maximum number of channels
};

/**
 * @brief Packet types for the network protocol
 */
enum class PacketType : u8 {
    // Connection management
    ConnectionRequest = 0,
    ConnectionChallenge,
    ConnectionResponse,
    ConnectionAccepted,
    ConnectionRejected,
    Disconnect,
    Heartbeat,
    HeartbeatAck,
    
    // Data packets
    UnreliableData,
    ReliableData,
    Fragment,
    FragmentAck,
    
    // Flow control
    Ack,
    Nack,
    PingRequest,
    PingResponse,
    
    // Bandwidth
    BandwidthProbe,
    BandwidthResult,
    
    // Replication
    CreateObject,
    DestroyObject,
    UpdateObject,
    FullSnapshot,
    DeltaSnapshot,
    
    // RPC
    RpcCall,
    RpcResponse,
    
    // Custom
    UserDefined = 128
};

/**
 * @brief Compression type for packets
 */
enum class CompressionType : u8 {
    None,
    LZ4,
    Zstd,
    DeltaOnly   ///< Delta compression without additional compression
};

/**
 * @brief Network error codes
 */
enum class NetworkError : u8 {
    None = 0,
    SocketError,
    ConnectionFailed,
    ConnectionTimeout,
    ConnectionRejected,
    InvalidPacket,
    PacketTooLarge,
    ProtocolMismatch,
    AuthenticationFailed,
    BandwidthExceeded,
    ServerFull,
    Kicked,
    Banned,
    InternalError
};

/**
 * @brief Server discovery method
 */
enum class DiscoveryMethod : u8 {
    LAN,            ///< Local network broadcast
    MasterServer,   ///< Central server list
    Direct          ///< Direct IP connection
};

// ============================================================================
// Network Addresses
// ============================================================================

/**
 * @brief IPv4 address representation
 */
struct IPv4Address {
    union {
        u8 octets[4];
        u32 value;
    };
    
    /// Default constructor (0.0.0.0)
    constexpr IPv4Address() noexcept : value(0) {}
    
    /// Construct from octets
    constexpr IPv4Address(u8 a, u8 b, u8 c, u8 d) noexcept
        : octets{a, b, c, d} {}
    
    /// Construct from u32
    explicit constexpr IPv4Address(u32 v) noexcept : value(v) {}
    
    /// Parse from string "a.b.c.d"
    static IPv4Address fromString(const std::string& str);
    
    /// Convert to string
    [[nodiscard]] std::string toString() const;
    
    /// Check if loopback (127.x.x.x)
    [[nodiscard]] constexpr bool isLoopback() const noexcept {
        return octets[0] == 127;
    }
    
    /// Check if local (192.168.x.x, 10.x.x.x, 172.16-31.x.x)
    [[nodiscard]] constexpr bool isLocal() const noexcept {
        return (octets[0] == 192 && octets[1] == 168) ||
               (octets[0] == 10) ||
               (octets[0] == 172 && octets[1] >= 16 && octets[1] <= 31);
    }
    
    /// Check if any address (0.0.0.0)
    [[nodiscard]] constexpr bool isAny() const noexcept {
        return value == 0;
    }
    
    /// Check if broadcast (255.255.255.255)
    [[nodiscard]] constexpr bool isBroadcast() const noexcept {
        return value == 0xFFFFFFFF;
    }
    
    constexpr bool operator==(const IPv4Address& other) const noexcept {
        return value == other.value;
    }
    
    constexpr bool operator!=(const IPv4Address& other) const noexcept {
        return value != other.value;
    }
    
    /// Localhost
    static constexpr IPv4Address localhost() noexcept {
        return IPv4Address(127, 0, 0, 1);
    }
    
    /// Any address
    static constexpr IPv4Address any() noexcept {
        return IPv4Address();
    }
    
    /// Broadcast address
    static constexpr IPv4Address broadcast() noexcept {
        return IPv4Address(255, 255, 255, 255);
    }
};

/**
 * @brief Network endpoint (address + port)
 */
struct NetworkEndpoint {
    IPv4Address address;
    u16 port = 0;
    
    constexpr NetworkEndpoint() noexcept = default;
    
    constexpr NetworkEndpoint(IPv4Address addr, u16 p) noexcept
        : address(addr), port(p) {}
    
    /// Parse from "ip:port" string
    static NetworkEndpoint fromString(const std::string& str);
    
    /// Convert to "ip:port" string
    [[nodiscard]] std::string toString() const;
    
    constexpr bool operator==(const NetworkEndpoint& other) const noexcept {
        return address == other.address && port == other.port;
    }
    
    constexpr bool operator!=(const NetworkEndpoint& other) const noexcept {
        return !(*this == other);
    }
    
    /// Create localhost endpoint
    static constexpr NetworkEndpoint localhost(u16 port) noexcept {
        return NetworkEndpoint(IPv4Address::localhost(), port);
    }
};

// ============================================================================
// Packet Structures
// ============================================================================

/**
 * @brief Packet header (12 bytes)
 */
struct PacketHeader {
    u32 magic = PACKET_MAGIC;       ///< Magic number for validation
    u16 protocolVersion;            ///< Protocol version
    u16 sequenceNumber;             ///< Packet sequence number
    PacketType type;                ///< Packet type
    ChannelType channel;            ///< Channel
    u8 flags;                       ///< Packet flags
    u8 fragmentInfo;                ///< Fragment number / total
    
    /// Flag bits
    static constexpr u8 FLAG_RELIABLE = 0x01;
    static constexpr u8 FLAG_COMPRESSED = 0x02;
    static constexpr u8 FLAG_ENCRYPTED = 0x04;
    static constexpr u8 FLAG_FRAGMENTED = 0x08;
    static constexpr u8 FLAG_ACK_REQUESTED = 0x10;
    
    [[nodiscard]] bool isReliable() const noexcept { return flags & FLAG_RELIABLE; }
    [[nodiscard]] bool isCompressed() const noexcept { return flags & FLAG_COMPRESSED; }
    [[nodiscard]] bool isEncrypted() const noexcept { return flags & FLAG_ENCRYPTED; }
    [[nodiscard]] bool isFragmented() const noexcept { return flags & FLAG_FRAGMENTED; }
    [[nodiscard]] bool ackRequested() const noexcept { return flags & FLAG_ACK_REQUESTED; }
    
    [[nodiscard]] u8 fragmentNumber() const noexcept { return fragmentInfo >> 5; }
    [[nodiscard]] u8 fragmentTotal() const noexcept { return fragmentInfo & 0x1F; }
};

/**
 * @brief Network packet container
 */
struct NetworkPacket {
    PacketHeader header;
    std::vector<u8> payload;
    
    /// Source endpoint (set on receive)
    NetworkEndpoint source;
    
    /// Timestamp when packet was created/received
    std::chrono::steady_clock::time_point timestamp;
    
    /// Get total size
    [[nodiscard]] usize totalSize() const noexcept {
        return sizeof(PacketHeader) + payload.size();
    }
    
    /// Create an empty packet
    static NetworkPacket create(PacketType type, ChannelType channel = ChannelType::Default);
    
    /// Serialize to bytes
    [[nodiscard]] std::vector<u8> serialize() const;
    
    /// Deserialize from bytes
    static std::optional<NetworkPacket> deserialize(const std::vector<u8>& data);
    static std::optional<NetworkPacket> deserialize(const u8* data, usize size);
};

// ============================================================================
// Connection Statistics
// ============================================================================

/**
 * @brief Per-connection statistics
 */
struct ConnectionStats {
    /// Packets sent/received
    u64 packetsSent = 0;
    u64 packetsReceived = 0;
    u64 packetsDropped = 0;
    u64 packetsOutOfOrder = 0;
    u64 packetsDuplicate = 0;
    
    /// Bytes sent/received
    u64 bytesSent = 0;
    u64 bytesReceived = 0;
    
    /// Reliability
    u64 reliableSent = 0;
    u64 reliableAcked = 0;
    u64 reliableResent = 0;
    
    /// Timing
    f32 rttMs = 0.0f;              ///< Round-trip time in ms
    f32 rttVariance = 0.0f;        ///< RTT variance
    f32 packetLoss = 0.0f;         ///< Packet loss percentage (0-100)
    f32 jitter = 0.0f;             ///< Jitter in ms
    
    /// Bandwidth
    f32 sendBandwidth = 0.0f;      ///< Send bandwidth (bytes/sec)
    f32 recvBandwidth = 0.0f;      ///< Receive bandwidth (bytes/sec)
    f32 availableBandwidth = 0.0f; ///< Estimated available bandwidth
    
    /// Quality metrics
    f32 connectionQuality = 1.0f;  ///< Overall quality (0-1)
    
    /// Timestamps
    std::chrono::steady_clock::time_point lastPacketReceived;
    std::chrono::steady_clock::time_point lastPacketSent;
    std::chrono::steady_clock::time_point connectionStarted;
    
    /// Calculate average RTT
    [[nodiscard]] f32 averageRtt() const noexcept { return rttMs; }
    
    /// Calculate packet loss rate
    [[nodiscard]] f32 lossRate() const noexcept {
        return packetsSent > 0 
            ? static_cast<f32>(packetsDropped) / static_cast<f32>(packetsSent) * 100.0f 
            : 0.0f;
    }
    
    /// Get connection duration
    [[nodiscard]] f32 connectionDurationSeconds() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<f32>(now - connectionStarted).count();
    }
};

/**
 * @brief Server statistics
 */
struct ServerStats {
    u32 currentConnections = 0;
    u32 peakConnections = 0;
    u64 totalConnections = 0;
    u64 totalBytesReceived = 0;
    u64 totalBytesSent = 0;
    u64 totalPacketsReceived = 0;
    u64 totalPacketsSent = 0;
    f32 averageRtt = 0.0f;
    f32 averagePacketLoss = 0.0f;
    f32 cpuUsage = 0.0f;
    f32 bandwidth = 0.0f;
};

// ============================================================================
// Network Messages
// ============================================================================

/**
 * @brief Connection request message
 */
struct ConnectionRequest {
    u32 protocolVersion = PROTOCOL_VERSION;
    u64 clientId = 0;                       ///< Unique client identifier
    std::string playerName;                 ///< Player display name
    std::string authToken;                  ///< Authentication token
    std::array<u8, 32> clientRandom{};      ///< Client random for key derivation
};

/**
 * @brief Connection challenge message (server response)
 */
struct ConnectionChallenge {
    u64 challengeToken;                     ///< Challenge to verify
    std::array<u8, 32> serverRandom{};      ///< Server random for key derivation
    u32 serverTime;                         ///< Server time for synchronization
};

/**
 * @brief Connection response message (client answer)
 */
struct ConnectionResponse {
    u64 challengeToken;                     ///< Echoed challenge token
    std::array<u8, 32> challengeProof{};    ///< Proof of challenge
};

/**
 * @brief Connection accepted message
 */
struct ConnectionAccepted {
    u64 connectionId;                       ///< Unique connection ID
    u32 playerId;                           ///< Assigned player ID
    u32 serverTime;                         ///< Server time
    f32 tickRate;                           ///< Server tick rate
    u16 assignedPort;                       ///< Port for data transfer
};

/**
 * @brief Connection rejected message
 */
struct ConnectionRejected {
    NetworkError reason;
    std::string message;
};

// ============================================================================
// Replication Types
// ============================================================================

/**
 * @brief Network object identifier
 */
struct NetworkId {
    u64 value = 0;
    
    constexpr NetworkId() noexcept = default;
    explicit constexpr NetworkId(u64 v) noexcept : value(v) {}
    
    constexpr bool isValid() const noexcept { return value != 0; }
    constexpr operator bool() const noexcept { return isValid(); }
    
    constexpr bool operator==(NetworkId other) const noexcept { return value == other.value; }
    constexpr bool operator!=(NetworkId other) const noexcept { return value != other.value; }
    constexpr bool operator<(NetworkId other) const noexcept { return value < other.value; }
};

/**
 * @brief Replication priority
 */
enum class ReplicationPriority : u8 {
    Critical,   ///< Always replicate immediately
    High,       ///< High priority (weapons, effects)
    Normal,     ///< Normal priority (most objects)
    Low,        ///< Low priority (decoration, ambient)
    Background  ///< Background replication (distant objects)
};

/**
 * @brief Replicated property change
 */
struct PropertyChange {
    u32 propertyId;
    std::vector<u8> newValue;
    u32 timestamp;
};

/**
 * @brief Object snapshot for replication
 */
struct ObjectSnapshot {
    NetworkId objectId;
    u32 classId;
    u32 timestamp;
    std::vector<PropertyChange> properties;
    
    /// Delta against previous snapshot
    std::optional<u32> baseTimestamp;
};

// ============================================================================
// RPC Types
// ============================================================================

/**
 * @brief RPC target specifier
 */
enum class RpcTarget : u8 {
    Server,         ///< Client-to-server RPC
    Client,         ///< Server-to-specific-client RPC
    AllClients,     ///< Server-to-all-clients RPC
    OtherClients,   ///< Server-to-all-except-sender RPC
    Owner           ///< Server-to-owner RPC
};

/**
 * @brief RPC call information
 */
struct RpcCall {
    u32 rpcId;                      ///< Unique RPC identifier
    u32 objectId;                   ///< Target object (0 for static)
    u16 functionId;                 ///< Function to call
    RpcTarget target;               ///< Target for the RPC
    std::vector<u8> parameters;     ///< Serialized parameters
    bool reliable = true;           ///< Whether to send reliably
};

// ============================================================================
// Network Configuration
// ============================================================================

/**
 * @brief Client configuration
 */
struct ClientConfig {
    NetworkEndpoint serverEndpoint;
    std::string playerName;
    std::string authToken;
    
    /// Timeouts (milliseconds)
    u32 connectionTimeoutMs = DEFAULT_TIMEOUT_MS;
    u32 heartbeatIntervalMs = HEARTBEAT_INTERVAL_MS;
    u32 reliableTimeoutMs = 1000;
    
    /// Rates
    u32 sendRateHz = 60;            ///< Packets per second to send
    u32 maxBandwidth = 0;           ///< Max bandwidth (0 = unlimited)
    
    /// Features
    bool enableCompression = true;
    bool enableEncryption = false;
    CompressionType compression = CompressionType::LZ4;
};

/**
 * @brief Server configuration
 */
struct ServerConfig {
    u16 port = 7777;                        ///< Listen port
    u32 maxConnections = MAX_PLAYERS_PER_SERVER;
    std::string serverName = "NovaCore Server";
    
    /// Timeouts (milliseconds)
    u32 connectionTimeoutMs = DEFAULT_TIMEOUT_MS;
    u32 heartbeatIntervalMs = HEARTBEAT_INTERVAL_MS;
    
    /// Rates
    u32 tickRateHz = 60;            ///< Server tick rate
    u32 snapshotRateHz = 20;        ///< Snapshot send rate
    
    /// Security
    bool requireAuthentication = false;
    u32 maxConnectionsPerIp = 4;
    
    /// Features
    bool enableCompression = true;
    bool enableEncryption = false;
    CompressionType compression = CompressionType::LZ4;
    
    /// Bandwidth limits
    u32 maxBandwidthPerClient = 0;  ///< Max bandwidth per client (0 = unlimited)
    u32 maxTotalBandwidth = 0;      ///< Max total bandwidth (0 = unlimited)
};

// ============================================================================
// Callbacks
// ============================================================================

/// Connection state change callback
using ConnectionCallback = std::function<void(u64 connectionId, ConnectionState state)>;

/// Data received callback
using DataCallback = std::function<void(u64 connectionId, ChannelType channel, 
                                         const u8* data, usize size)>;

/// RPC callback
using RpcCallback = std::function<void(const RpcCall& call)>;

/// Discovery callback (found server)
using DiscoveryCallback = std::function<void(const NetworkEndpoint& endpoint, 
                                              const std::string& serverName,
                                              u32 playerCount, u32 maxPlayers)>;

// ============================================================================
// Network Handle Types
// ============================================================================

/// Handle for network connections
struct NetworkConnectionHandle {
    u64 value = 0;
    [[nodiscard]] constexpr bool isValid() const noexcept { return value != 0; }
    constexpr explicit operator bool() const noexcept { return isValid(); }
};

/// Handle for network channels
struct NetworkChannelHandle {
    u32 value = 0;
    [[nodiscard]] constexpr bool isValid() const noexcept { return value != 0; }
    constexpr explicit operator bool() const noexcept { return isValid(); }
};

} // namespace nova::network

// Hash specialization for NetworkEndpoint (needed for unordered_map)
namespace std {
    template<>
    struct hash<nova::network::NetworkEndpoint> {
        size_t operator()(const nova::network::NetworkEndpoint& ep) const noexcept {
            return std::hash<uint64_t>()(
                (static_cast<uint64_t>(ep.address.value) << 16) | ep.port
            );
        }
    };
}
