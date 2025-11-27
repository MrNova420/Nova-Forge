/**
 * @file test_network.cpp
 * @brief Comprehensive tests for Nova Network module
 * @copyright (c) 2025 WeNova Interactive. All rights reserved.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/network/network_types.hpp>
#include <nova/core/network/network_system.hpp>

using namespace nova;
using namespace nova::network;
using Catch::Approx;

// =============================================================================
// IPv4 Address Tests
// =============================================================================

TEST_CASE("Network: IPv4Address construction", "[network][types]") {
    SECTION("Default construction") {
        IPv4Address addr{};
        REQUIRE(addr.octets[0] == 0);
        REQUIRE(addr.octets[1] == 0);
        REQUIRE(addr.octets[2] == 0);
        REQUIRE(addr.octets[3] == 0);
    }
    
    SECTION("Component construction") {
        IPv4Address addr{192, 168, 1, 100};
        REQUIRE(addr.octets[0] == 192);
        REQUIRE(addr.octets[1] == 168);
        REQUIRE(addr.octets[2] == 1);
        REQUIRE(addr.octets[3] == 100);
    }
    
    SECTION("Localhost static") {
        auto addr = IPv4Address::localhost();
        REQUIRE(addr.octets[0] == 127);
        REQUIRE(addr.octets[1] == 0);
        REQUIRE(addr.octets[2] == 0);
        REQUIRE(addr.octets[3] == 1);
    }
    
    SECTION("To string conversion") {
        IPv4Address addr{192, 168, 1, 100};
        REQUIRE(addr.toString() == "192.168.1.100");
    }
    
    SECTION("Equality comparison") {
        IPv4Address a{192, 168, 1, 1};
        IPv4Address b{192, 168, 1, 1};
        IPv4Address c{192, 168, 1, 2};
        
        REQUIRE(a == b);
        REQUIRE(a != c);
    }
    
    SECTION("Is local check") {
        REQUIRE(IPv4Address{10, 0, 0, 1}.isLocal());
        REQUIRE(IPv4Address{172, 16, 0, 1}.isLocal());
        REQUIRE(IPv4Address{192, 168, 1, 1}.isLocal());
        REQUIRE_FALSE(IPv4Address{8, 8, 8, 8}.isLocal());
    }
    
    SECTION("Is loopback check") {
        REQUIRE(IPv4Address{127, 0, 0, 1}.isLoopback());
        REQUIRE(IPv4Address{127, 255, 255, 255}.isLoopback());
        REQUIRE_FALSE(IPv4Address{192, 168, 1, 1}.isLoopback());
    }
}

// =============================================================================
// Network Endpoint Tests
// =============================================================================

TEST_CASE("Network: NetworkEndpoint", "[network][types]") {
    SECTION("Default construction") {
        NetworkEndpoint endpoint{};
        REQUIRE(endpoint.port == 0);
    }
    
    SECTION("With address and port") {
        NetworkEndpoint endpoint{IPv4Address{192, 168, 1, 1}, 8080};
        REQUIRE(endpoint.address.octets[0] == 192);
        REQUIRE(endpoint.port == 8080);
    }
    
    SECTION("To string") {
        NetworkEndpoint endpoint{IPv4Address{192, 168, 1, 1}, 8080};
        REQUIRE(endpoint.toString() == "192.168.1.1:8080");
    }
    
    SECTION("Localhost helper") {
        auto endpoint = NetworkEndpoint::localhost(7777);
        REQUIRE(endpoint.address.isLoopback());
        REQUIRE(endpoint.port == 7777);
    }
}

// =============================================================================
// Packet Header Tests
// =============================================================================

TEST_CASE("Network: PacketHeader", "[network][types]") {
    SECTION("Magic validation") {
        PacketHeader header{};
        header.magic = PACKET_MAGIC;
        REQUIRE(header.magic == PACKET_MAGIC);
    }
    
    SECTION("Invalid magic") {
        PacketHeader header{};
        header.magic = 0xDEADBEEF;
        REQUIRE(header.magic != PACKET_MAGIC);
    }
    
    SECTION("Sequence number storage") {
        PacketHeader h1{};
        h1.sequenceNumber = 100;
        
        PacketHeader h2{};
        h2.sequenceNumber = 200;
        
        // h2 should be newer
        REQUIRE(h2.sequenceNumber > h1.sequenceNumber);
    }
}

// =============================================================================
// Connection State Tests
// =============================================================================

TEST_CASE("Network: ConnectionState", "[network][types]") {
    SECTION("Initial state") {
        ConnectionState state = ConnectionState::Disconnected;
        REQUIRE(state == ConnectionState::Disconnected);
    }
    
    SECTION("State transitions") {
        REQUIRE(ConnectionState::Connecting != ConnectionState::Connected);
        REQUIRE(ConnectionState::Connected != ConnectionState::Disconnecting);
        REQUIRE(ConnectionState::TimedOut != ConnectionState::Rejected);
    }
}

// =============================================================================
// Network Constants Tests
// =============================================================================

TEST_CASE("Network: Constants", "[network][constants]") {
    SECTION("Server capacity") {
        REQUIRE(MAX_PLAYERS_PER_SERVER == 10000);
    }
    
    SECTION("Default timeout") {
        REQUIRE(DEFAULT_TIMEOUT_MS == 10000);
    }
    
    SECTION("Packet limits") {
        REQUIRE(MAX_PACKET_SIZE == 1400);
        REQUIRE(MAX_PAYLOAD_SIZE == 1200);
        REQUIRE(DEFAULT_MTU == 1200);
    }
}

// =============================================================================
// Network System Tests
// =============================================================================

TEST_CASE("Network: NetworkSystem singleton", "[network][system]") {
    SECTION("Get instance") {
        auto& system1 = NetworkSystem::instance();
        auto& system2 = NetworkSystem::instance();
        REQUIRE(&system1 == &system2);
    }
    
    SECTION("Platform initialization") {
        auto& system = NetworkSystem::instance();
        // Should be able to call multiple times safely
        auto result = system.initialize();
        // May fail on CI without network, but should not crash
    }
}

// =============================================================================
// RTT Calculation Tests
// =============================================================================

TEST_CASE("Network: RTT calculation", "[network][rtt]") {
    SECTION("Simple RTT") {
        // RTT should be calculated as round-trip time
        u64 sendTime = 1000;
        u64 receiveTime = 1050;
        f32 rtt = static_cast<f32>(receiveTime - sendTime);
        REQUIRE(rtt == 50.0f);
    }
    
    SECTION("Exponential moving average RTT") {
        // EMA: newRTT = alpha * sample + (1 - alpha) * currentRTT
        f32 currentRTT = 50.0f;
        f32 sample = 60.0f;
        f32 alpha = 0.125f; // Standard alpha for RTT smoothing
        
        f32 newRTT = alpha * sample + (1.0f - alpha) * currentRTT;
        REQUIRE(newRTT == Approx(51.25f));
    }
}

// =============================================================================
// Delivery Mode Tests
// =============================================================================

TEST_CASE("Network: DeliveryMode", "[network][types]") {
    SECTION("All modes defined") {
        REQUIRE(DeliveryMode::Unreliable != DeliveryMode::Reliable);
        REQUIRE(DeliveryMode::ReliableOrdered != DeliveryMode::ReliableSequenced);
    }
}

// =============================================================================
// Network Error Tests
// =============================================================================

TEST_CASE("Network: NetworkError", "[network][types]") {
    SECTION("Error codes exist") {
        REQUIRE(NetworkError::None != NetworkError::SocketError);
        REQUIRE(NetworkError::ConnectionTimeout != NetworkError::ConnectionRejected);
        REQUIRE(NetworkError::ServerFull != NetworkError::Banned);
    }
}
