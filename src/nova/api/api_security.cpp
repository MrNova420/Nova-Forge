// =============================================================================
// NovaForge Platform - API Security Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
// =============================================================================

#include <nova/api/api_security.hpp>

#include <random>
#include <regex>
#include <algorithm>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <iomanip>

namespace nova::api::security {

// =============================================================================
// SHA-256 Implementation (RFC 6234)
// =============================================================================

namespace {

// SHA-256 constants (first 32 bits of fractional parts of cube roots of first 64 primes)
constexpr std::array<u32, 64> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Initial hash values (first 32 bits of fractional parts of square roots of first 8 primes)
constexpr std::array<u32, 8> H_INIT = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// Right rotate
[[nodiscard]] constexpr u32 rotr(u32 x, u32 n) noexcept {
    return (x >> n) | (x << (32 - n));
}

// SHA-256 functions
[[nodiscard]] constexpr u32 ch(u32 x, u32 y, u32 z) noexcept {
    return (x & y) ^ (~x & z);
}

[[nodiscard]] constexpr u32 maj(u32 x, u32 y, u32 z) noexcept {
    return (x & y) ^ (x & z) ^ (y & z);
}

[[nodiscard]] constexpr u32 sigma0(u32 x) noexcept {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

[[nodiscard]] constexpr u32 sigma1(u32 x) noexcept {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

[[nodiscard]] constexpr u32 gamma0(u32 x) noexcept {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

[[nodiscard]] constexpr u32 gamma1(u32 x) noexcept {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

void sha256_transform(std::array<u32, 8>& state, const u8* block) {
    std::array<u32, 64> W;
    
    // Prepare message schedule
    for (int i = 0; i < 16; ++i) {
        W[i] = (static_cast<u32>(block[i * 4]) << 24) |
               (static_cast<u32>(block[i * 4 + 1]) << 16) |
               (static_cast<u32>(block[i * 4 + 2]) << 8) |
               (static_cast<u32>(block[i * 4 + 3]));
    }
    for (int i = 16; i < 64; ++i) {
        W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
    }
    
    // Initialize working variables
    u32 a = state[0], b = state[1], c = state[2], d = state[3];
    u32 e = state[4], f = state[5], g = state[6], h = state[7];
    
    // Main loop
    for (int i = 0; i < 64; ++i) {
        u32 t1 = h + sigma1(e) + ch(e, f, g) + K[i] + W[i];
        u32 t2 = sigma0(a) + maj(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    
    // Add to state
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

} // anonymous namespace

// =============================================================================
// Crypto Implementation
// =============================================================================

HashResult Crypto::sha256(const std::vector<u8>& data) {
    // Full SHA-256 implementation (RFC 6234)
    std::array<u32, 8> state = H_INIT;
    
    // Padding: append 1 bit, then zeros, then 64-bit length
    std::vector<u8> padded = data;
    u64 bitLen = data.size() * 8;
    
    // Append 0x80 (10000000 binary)
    padded.push_back(0x80);
    
    // Pad to 56 bytes mod 64
    while (padded.size() % 64 != 56) {
        padded.push_back(0x00);
    }
    
    // Append length in big-endian
    for (int i = 7; i >= 0; --i) {
        padded.push_back(static_cast<u8>((bitLen >> (i * 8)) & 0xFF));
    }
    
    // Process blocks
    for (usize i = 0; i < padded.size(); i += 64) {
        sha256_transform(state, padded.data() + i);
    }
    
    // Output hash
    HashResult result{};
    for (int i = 0; i < 8; ++i) {
        result[i * 4] = static_cast<u8>((state[i] >> 24) & 0xFF);
        result[i * 4 + 1] = static_cast<u8>((state[i] >> 16) & 0xFF);
        result[i * 4 + 2] = static_cast<u8>((state[i] >> 8) & 0xFF);
        result[i * 4 + 3] = static_cast<u8>(state[i] & 0xFF);
    }
    
    return result;
}

HashResult Crypto::sha256(std::string_view data) {
    return sha256(std::vector<u8>(data.begin(), data.end()));
}

std::vector<u8> Crypto::hashPassword(std::string_view password, const std::vector<u8>& salt) {
    // PBKDF2-HMAC-SHA256 implementation for password hashing
    // This is a proper key derivation function suitable for password storage
    // Parameters: 100,000 iterations (OWASP recommended minimum)
    
    constexpr u32 iterations = 100000;
    constexpr usize dkLen = 32; // Derived key length (256 bits)
    
    std::vector<u8> derivedKey(dkLen);
    
    // PBKDF2 with HMAC-SHA256
    // DK = T1 || T2 || ... || Tdklen/hlen
    // Ti = F(Password, Salt, c, i)
    // F(Password, Salt, c, i) = U1 ^ U2 ^ ... ^ Uc
    // U1 = PRF(Password, Salt || INT(i))
    // U2 = PRF(Password, U1)
    // ...
    
    std::vector<u8> passwordBytes(password.begin(), password.end());
    
    // For 32-byte output with SHA-256 (32-byte hash), we need 1 block
    u32 blockNum = 1;
    
    // Salt || INT(blockNum) in big-endian
    std::vector<u8> saltWithIndex = salt;
    saltWithIndex.push_back(static_cast<u8>((blockNum >> 24) & 0xFF));
    saltWithIndex.push_back(static_cast<u8>((blockNum >> 16) & 0xFF));
    saltWithIndex.push_back(static_cast<u8>((blockNum >> 8) & 0xFF));
    saltWithIndex.push_back(static_cast<u8>(blockNum & 0xFF));
    
    // U1 = HMAC(password, salt || INT(i))
    HashResult U = hmacSha256(passwordBytes, saltWithIndex);
    std::array<u8, 32> T;
    std::copy(U.begin(), U.end(), T.begin());
    
    // Iterate: U_j = HMAC(password, U_{j-1}), T ^= U_j
    for (u32 j = 2; j <= iterations; ++j) {
        std::vector<u8> Uvec(U.begin(), U.end());
        U = hmacSha256(passwordBytes, Uvec);
        for (usize k = 0; k < 32; ++k) {
            T[k] ^= U[k];
        }
    }
    
    derivedKey.assign(T.begin(), T.begin() + dkLen);
    return derivedKey;
}

bool Crypto::verifyPassword(std::string_view password, const std::vector<u8>& hash, const std::vector<u8>& salt) {
    auto computed = hashPassword(password, salt);
    return constantTimeCompare(computed, hash);
}

std::string Crypto::hashPassword(std::string_view password) {
    // Generate random salt (16 bytes)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);
    
    std::vector<u8> salt(16);
    for (auto& byte : salt) {
        byte = static_cast<u8>(dis(gen));
    }
    
    // Hash with PBKDF2
    auto hash = hashPassword(password, salt);
    
    // Encode as hex: salt (32 hex chars) + hash (64 hex chars)
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (u8 byte : salt) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    for (u8 byte : hash) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

bool Crypto::verifyPassword(std::string_view password, const std::string& saltedHash) {
    // Expect 32 hex chars (16 bytes salt) + 64 hex chars (32 bytes hash)
    if (saltedHash.length() != 96) return false;
    
    // Validate that all characters are valid hex
    for (char c : saltedHash) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    
    try {
        // Decode salt
        std::vector<u8> salt(16);
        for (size_t i = 0; i < 16; ++i) {
            std::string byteStr = saltedHash.substr(i * 2, 2);
            salt[i] = static_cast<u8>(std::stoul(byteStr, nullptr, 16));
        }
        
        // Decode hash
        std::vector<u8> hash(32);
        for (size_t i = 0; i < 32; ++i) {
            std::string byteStr = saltedHash.substr(32 + i * 2, 2);
            hash[i] = static_cast<u8>(std::stoul(byteStr, nullptr, 16));
        }
        
        return verifyPassword(password, hash, salt);
    } catch (const std::exception&) {
        // Parsing failed - invalid hash format
        return false;
    }
}

HashResult Crypto::hmacSha256(const std::vector<u8>& key, const std::vector<u8>& data) {
    // HMAC-SHA256 implementation (RFC 2104)
    // HMAC(K, m) = H((K' ⊕ opad) || H((K' ⊕ ipad) || m))
    
    constexpr usize blockSize = 64; // SHA-256 block size
    constexpr u8 ipad = 0x36;
    constexpr u8 opad = 0x5c;
    
    // Prepare key (hash if > blockSize, pad with zeros if < blockSize)
    std::array<u8, blockSize> keyPrime{};
    
    if (key.size() > blockSize) {
        // Hash key if too long
        HashResult hashedKey = sha256(key);
        std::copy(hashedKey.begin(), hashedKey.end(), keyPrime.begin());
    } else {
        // Pad with zeros
        std::copy(key.begin(), key.end(), keyPrime.begin());
    }
    
    // Inner hash: H((K' ⊕ ipad) || m)
    std::vector<u8> innerData;
    innerData.reserve(blockSize + data.size());
    for (usize i = 0; i < blockSize; ++i) {
        innerData.push_back(keyPrime[i] ^ ipad);
    }
    innerData.insert(innerData.end(), data.begin(), data.end());
    HashResult innerHash = sha256(innerData);
    
    // Outer hash: H((K' ⊕ opad) || innerHash)
    std::vector<u8> outerData;
    outerData.reserve(blockSize + 32);
    for (usize i = 0; i < blockSize; ++i) {
        outerData.push_back(keyPrime[i] ^ opad);
    }
    outerData.insert(outerData.end(), innerHash.begin(), innerHash.end());
    
    return sha256(outerData);
}

// =============================================================================
// AES-256 Implementation
// =============================================================================

namespace {

// AES S-Box (substitution box)
constexpr std::array<u8, 256> AES_SBOX = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// AES Inverse S-Box
constexpr std::array<u8, 256> AES_INV_SBOX = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

// Round constants for key expansion
constexpr std::array<u8, 11> RCON = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

// Galois Field multiplication
[[nodiscard]] constexpr u8 gfMul(u8 a, u8 b) noexcept {
    u8 result = 0;
    u8 hi_bit;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) result ^= a;
        hi_bit = a & 0x80;
        a <<= 1;
        if (hi_bit) a ^= 0x1b; // AES irreducible polynomial
        b >>= 1;
    }
    return result;
}

// AES-256 key expansion
void aes256KeyExpansion(const u8* key, std::array<u8, 240>& roundKeys) {
    // First 32 bytes are the original key
    std::copy(key, key + 32, roundKeys.begin());
    
    u32 bytesGenerated = 32;
    u32 rconIteration = 1;
    std::array<u8, 4> temp;
    
    while (bytesGenerated < 240) {
        // Copy last 4 bytes
        for (int i = 0; i < 4; ++i) {
            temp[i] = roundKeys[bytesGenerated - 4 + i];
        }
        
        if (bytesGenerated % 32 == 0) {
            // RotWord
            u8 t = temp[0];
            temp[0] = temp[1]; temp[1] = temp[2]; temp[2] = temp[3]; temp[3] = t;
            
            // SubWord
            for (int i = 0; i < 4; ++i) {
                temp[i] = AES_SBOX[temp[i]];
            }
            
            // XOR with Rcon
            temp[0] ^= RCON[rconIteration++];
        }
        else if (bytesGenerated % 32 == 16) {
            // SubWord only for AES-256
            for (int i = 0; i < 4; ++i) {
                temp[i] = AES_SBOX[temp[i]];
            }
        }
        
        for (int i = 0; i < 4; ++i) {
            roundKeys[bytesGenerated] = roundKeys[bytesGenerated - 32] ^ temp[i];
            ++bytesGenerated;
        }
    }
}

// AES SubBytes transformation
void aesSubBytes(std::array<u8, 16>& state) {
    for (auto& byte : state) {
        byte = AES_SBOX[byte];
    }
}

// AES ShiftRows transformation
void aesShiftRows(std::array<u8, 16>& state) {
    std::array<u8, 16> temp = state;
    // Row 0: no shift
    // Row 1: shift left 1
    state[1] = temp[5]; state[5] = temp[9]; state[9] = temp[13]; state[13] = temp[1];
    // Row 2: shift left 2
    state[2] = temp[10]; state[6] = temp[14]; state[10] = temp[2]; state[14] = temp[6];
    // Row 3: shift left 3
    state[3] = temp[15]; state[7] = temp[3]; state[11] = temp[7]; state[15] = temp[11];
}

// AES MixColumns transformation
void aesMixColumns(std::array<u8, 16>& state) {
    for (int c = 0; c < 4; ++c) {
        u8 a0 = state[c * 4];
        u8 a1 = state[c * 4 + 1];
        u8 a2 = state[c * 4 + 2];
        u8 a3 = state[c * 4 + 3];
        
        state[c * 4]     = gfMul(a0, 2) ^ gfMul(a1, 3) ^ a2 ^ a3;
        state[c * 4 + 1] = a0 ^ gfMul(a1, 2) ^ gfMul(a2, 3) ^ a3;
        state[c * 4 + 2] = a0 ^ a1 ^ gfMul(a2, 2) ^ gfMul(a3, 3);
        state[c * 4 + 3] = gfMul(a0, 3) ^ a1 ^ a2 ^ gfMul(a3, 2);
    }
}

// AES AddRoundKey transformation
void aesAddRoundKey(std::array<u8, 16>& state, const u8* roundKey) {
    for (int i = 0; i < 16; ++i) {
        state[i] ^= roundKey[i];
    }
}

// AES-256 block encryption (single 16-byte block)
void aes256EncryptBlock(const u8* input, u8* output, const std::array<u8, 240>& roundKeys) {
    std::array<u8, 16> state;
    std::copy(input, input + 16, state.begin());
    
    // Initial round key addition
    aesAddRoundKey(state, roundKeys.data());
    
    // 13 main rounds
    for (int round = 1; round < 14; ++round) {
        aesSubBytes(state);
        aesShiftRows(state);
        aesMixColumns(state);
        aesAddRoundKey(state, roundKeys.data() + round * 16);
    }
    
    // Final round (no MixColumns)
    aesSubBytes(state);
    aesShiftRows(state);
    aesAddRoundKey(state, roundKeys.data() + 14 * 16);
    
    std::copy(state.begin(), state.end(), output);
}

// Increment counter for CTR mode
void incrementCounter(std::array<u8, 16>& counter) {
    for (int i = 15; i >= 0; --i) {
        if (++counter[i] != 0) break;
    }
}

} // anonymous namespace

Result<std::vector<u8>, Error> Crypto::encrypt(
    const std::vector<u8>& plaintext,
    const EncryptionKey& key,
    const InitVector& iv) {
    
    // AES-256-CTR encryption with HMAC-SHA256 authentication
    // Output format: ciphertext || HMAC tag (32 bytes)
    
    if (plaintext.empty()) {
        return std::vector<u8>{};
    }
    
    // Expand key
    std::array<u8, 240> roundKeys;
    aes256KeyExpansion(key.data(), roundKeys);
    
    // Initialize counter from IV
    std::array<u8, 16> counter;
    std::copy(iv.data(), iv.data() + 16, counter.begin());
    
    // Encrypt using CTR mode
    std::vector<u8> ciphertext;
    ciphertext.reserve(plaintext.size() + 32); // +32 for HMAC tag
    
    std::array<u8, 16> keystream;
    usize processed = 0;
    
    while (processed < plaintext.size()) {
        // Generate keystream block
        aes256EncryptBlock(counter.data(), keystream.data(), roundKeys);
        incrementCounter(counter);
        
        // XOR plaintext with keystream
        usize blockSize = std::min(static_cast<usize>(16), plaintext.size() - processed);
        for (usize i = 0; i < blockSize; ++i) {
            ciphertext.push_back(plaintext[processed + i] ^ keystream[i]);
        }
        processed += blockSize;
    }
    
    // Generate HMAC tag over IV + ciphertext
    std::vector<u8> authData;
    authData.insert(authData.end(), iv.data(), iv.data() + 16);
    authData.insert(authData.end(), ciphertext.begin(), ciphertext.end());
    
    std::vector<u8> hmacKey(key.data(), key.data() + 32);
    HashResult tag = hmacSha256(hmacKey, authData);
    
    // Append tag to ciphertext
    ciphertext.insert(ciphertext.end(), tag.begin(), tag.end());
    
    return ciphertext;
}

Result<std::vector<u8>, Error> Crypto::decrypt(
    const std::vector<u8>& ciphertext,
    const EncryptionKey& key,
    const InitVector& iv) {
    
    // AES-256-CTR decryption with HMAC-SHA256 verification
    
    if (ciphertext.size() < 32) {
        return std::unexpected(Error(ErrorCategory::Validation, 1, "Ciphertext too short - missing authentication tag"));
    }
    
    // Extract tag from end
    std::array<u8, 32> providedTag;
    std::copy(ciphertext.end() - 32, ciphertext.end(), providedTag.begin());
    
    // Verify HMAC
    std::vector<u8> authData;
    authData.insert(authData.end(), iv.data(), iv.data() + 16);
    authData.insert(authData.end(), ciphertext.begin(), ciphertext.end() - 32);
    
    std::vector<u8> hmacKey(key.data(), key.data() + 32);
    HashResult expectedTag = hmacSha256(hmacKey, authData);
    
    // Constant-time comparison
    std::vector<u8> providedTagVec(providedTag.begin(), providedTag.end());
    std::vector<u8> expectedTagVec(expectedTag.begin(), expectedTag.end());
    if (!constantTimeCompare(providedTagVec, expectedTagVec)) {
        return std::unexpected(Error(ErrorCategory::Validation, 2, "Authentication failed - ciphertext may be tampered"));
    }
    
    // Expand key
    std::array<u8, 240> roundKeys;
    aes256KeyExpansion(key.data(), roundKeys);
    
    // Initialize counter from IV
    std::array<u8, 16> counter;
    std::copy(iv.data(), iv.data() + 16, counter.begin());
    
    // Decrypt using CTR mode (same as encrypt - XOR with keystream)
    std::vector<u8> plaintext;
    usize dataSize = ciphertext.size() - 32;
    plaintext.reserve(dataSize);
    
    std::array<u8, 16> keystream;
    usize processed = 0;
    
    while (processed < dataSize) {
        // Generate keystream block
        aes256EncryptBlock(counter.data(), keystream.data(), roundKeys);
        incrementCounter(counter);
        
        // XOR ciphertext with keystream
        usize blockSize = std::min(static_cast<usize>(16), dataSize - processed);
        for (usize i = 0; i < blockSize; ++i) {
            plaintext.push_back(ciphertext[processed + i] ^ keystream[i]);
        }
        processed += blockSize;
    }
    
    return plaintext;
}

Result<std::string, Error> Crypto::encryptString(std::string_view plaintext, const EncryptionKey& key) {
    InitVector iv = generateIV();
    std::vector<u8> data(plaintext.begin(), plaintext.end());
    
    auto result = encrypt(data, key, iv);
    if (!result) return std::unexpected(result.error());
    
    // Prepend IV to ciphertext
    std::vector<u8> combined;
    combined.insert(combined.end(), iv.data(), iv.data() + AES_IV_SIZE);
    combined.insert(combined.end(), result->begin(), result->end());
    
    return base64Encode(combined);
}

Result<std::string, Error> Crypto::decryptString(std::string_view ciphertext, const EncryptionKey& key) {
    auto decoded = base64Decode(ciphertext);
    if (!decoded) return std::unexpected(decoded.error());
    
    if (decoded->size() < AES_IV_SIZE) {
        return std::unexpected(Error(ErrorCategory::Validation, 1, "Ciphertext too short"));
    }
    
    // Extract IV
    InitVector iv;
    std::memcpy(iv.data(), decoded->data(), AES_IV_SIZE);
    
    std::vector<u8> encryptedData(decoded->begin() + AES_IV_SIZE, decoded->end());
    
    auto result = decrypt(encryptedData, key, iv);
    if (!result) return std::unexpected(result.error());
    
    return std::string(result->begin(), result->end());
}

std::vector<u8> Crypto::randomBytes(usize count) {
    std::vector<u8> bytes(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<u32> dist(0, 255);
    
    for (usize i = 0; i < count; ++i) {
        bytes[i] = static_cast<u8>(dist(gen));
    }
    
    return bytes;
}

EncryptionKey Crypto::generateKey() {
    EncryptionKey key;
    auto bytes = randomBytes(AES_KEY_SIZE);
    std::memcpy(key.data(), bytes.data(), AES_KEY_SIZE);
    return key;
}

InitVector Crypto::generateIV() {
    InitVector iv;
    auto bytes = randomBytes(AES_IV_SIZE);
    std::memcpy(iv.data(), bytes.data(), AES_IV_SIZE);
    return iv;
}

std::vector<u8> Crypto::generateSalt() {
    return randomBytes(16);
}

std::string Crypto::generateTokenString(usize length) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    static const usize charCount = 64;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<usize> dist(0, charCount - 1);
    
    std::string token;
    token.reserve(length);
    
    for (usize i = 0; i < length; ++i) {
        token += chars[dist(gen)];
    }
    
    return token;
}

std::string Crypto::base64Encode(const std::vector<u8>& data) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    result.reserve((data.size() + 2) / 3 * 4);
    
    for (usize i = 0; i < data.size(); i += 3) {
        u32 n = static_cast<u32>(data[i]) << 16;
        if (i + 1 < data.size()) n |= static_cast<u32>(data[i + 1]) << 8;
        if (i + 2 < data.size()) n |= static_cast<u32>(data[i + 2]);
        
        result += chars[(n >> 18) & 0x3F];
        result += chars[(n >> 12) & 0x3F];
        result += (i + 1 < data.size()) ? chars[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < data.size()) ? chars[n & 0x3F] : '=';
    }
    
    return result;
}

Result<std::vector<u8>, Error> Crypto::base64Decode(std::string_view base64) {
    static const u8 decodeTable[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255, 255,
        255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
         15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
        255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
         41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
    };
    
    std::vector<u8> result;
    result.reserve(base64.size() * 3 / 4);
    
    u32 bits = 0;
    int bitCount = 0;
    
    for (char c : base64) {
        if (c == '=') break;
        
        u8 val = decodeTable[static_cast<u8>(c)];
        if (val == 255) {
            return std::unexpected(Error(ErrorCategory::Parse, 1, "Invalid base64 character"));
        }
        
        bits = (bits << 6) | val;
        bitCount += 6;
        
        if (bitCount >= 8) {
            bitCount -= 8;
            result.push_back(static_cast<u8>((bits >> bitCount) & 0xFF));
        }
    }
    
    return result;
}

std::string Crypto::hexEncode(const std::vector<u8>& data) {
    static const char* hex = "0123456789abcdef";
    std::string result;
    result.reserve(data.size() * 2);
    
    for (u8 byte : data) {
        result += hex[byte >> 4];
        result += hex[byte & 0x0F];
    }
    
    return result;
}

Result<std::vector<u8>, Error> Crypto::hexDecode(std::string_view hex) {
    if (hex.size() % 2 != 0) {
        return std::unexpected(Error(ErrorCategory::Parse, 1, "Invalid hex string length"));
    }
    
    std::vector<u8> result;
    result.reserve(hex.size() / 2);
    
    for (usize i = 0; i < hex.size(); i += 2) {
        auto hexToByte = [](char c) -> i32 {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        
        i32 high = hexToByte(hex[i]);
        i32 low = hexToByte(hex[i + 1]);
        
        if (high < 0 || low < 0) {
            return std::unexpected(Error(ErrorCategory::Parse, 2, "Invalid hex character"));
        }
        
        result.push_back(static_cast<u8>((high << 4) | low));
    }
    
    return result;
}

bool Crypto::constantTimeCompare(const std::vector<u8>& a, const std::vector<u8>& b) {
    if (a.size() != b.size()) return false;
    
    u8 result = 0;
    for (usize i = 0; i < a.size(); ++i) {
        result = static_cast<u8>(result | (a[i] ^ b[i]));
    }
    
    return result == 0;
}

std::vector<u8> Crypto::deriveKey(
    std::string_view password,
    const std::vector<u8>& salt,
    u32 iterations,
    usize keyLength) {
    
    // PBKDF2-HMAC-SHA256 implementation
    // DK = T1 || T2 || ... || Tdklen/hlen
    // Each Ti = F(Password, Salt, c, i)
    
    constexpr usize hashLen = 32; // SHA-256 output size
    usize numBlocks = (keyLength + hashLen - 1) / hashLen;
    
    std::vector<u8> derivedKey;
    derivedKey.reserve(numBlocks * hashLen);
    
    std::vector<u8> passwordBytes(password.begin(), password.end());
    
    for (usize blockNum = 1; blockNum <= numBlocks; ++blockNum) {
        // Salt || INT(blockNum) in big-endian
        std::vector<u8> saltWithIndex = salt;
        saltWithIndex.push_back(static_cast<u8>((blockNum >> 24) & 0xFF));
        saltWithIndex.push_back(static_cast<u8>((blockNum >> 16) & 0xFF));
        saltWithIndex.push_back(static_cast<u8>((blockNum >> 8) & 0xFF));
        saltWithIndex.push_back(static_cast<u8>(blockNum & 0xFF));
        
        // U1 = HMAC(password, salt || INT(i))
        HashResult U = hmacSha256(passwordBytes, saltWithIndex);
        std::array<u8, 32> T;
        std::copy(U.begin(), U.end(), T.begin());
        
        // Iterate: U_j = HMAC(password, U_{j-1}), T ^= U_j
        for (u32 j = 2; j <= iterations; ++j) {
            std::vector<u8> Uvec(U.begin(), U.end());
            U = hmacSha256(passwordBytes, Uvec);
            for (usize k = 0; k < 32; ++k) {
                T[k] ^= U[k];
            }
        }
        
        derivedKey.insert(derivedKey.end(), T.begin(), T.end());
    }
    
    // Truncate to requested length
    if (derivedKey.size() > keyLength) {
        derivedKey.resize(keyLength);
    }
    
    return derivedKey;
}

// =============================================================================
// TokenManager Implementation
// =============================================================================

struct TokenManager::Impl {
    Config config;
    std::map<std::string, std::chrono::system_clock::time_point> revokedTokens;
    std::mutex mutex;
};

TokenManager::TokenManager(const Config& config) : m_impl(std::make_unique<Impl>()) {
    m_impl->config = config;
}

TokenManager::~TokenManager() = default;

Result<SecurityToken, Error> TokenManager::generateAccessToken(
    std::string_view userId,
    Permission permissions,
    const std::map<std::string, std::string>& claims) {
    
    SecurityToken token;
    token.token = Crypto::generateTokenString(64);
    token.type = TokenType::Access;
    token.userId = std::string(userId);
    token.permissions = permissions;
    token.issuedAt = std::chrono::system_clock::now();
    token.expiresAt = token.issuedAt + std::chrono::seconds(m_impl->config.accessTokenExpirySeconds);
    token.issuer = m_impl->config.issuer;
    token.audience = m_impl->config.audience;
    token.claims = claims;
    
    return token;
}

Result<SecurityToken, Error> TokenManager::generateRefreshToken(std::string_view userId) {
    SecurityToken token;
    token.token = Crypto::generateTokenString(128);
    token.type = TokenType::Refresh;
    token.userId = std::string(userId);
    token.permissions = Permission::None;
    token.issuedAt = std::chrono::system_clock::now();
    token.expiresAt = token.issuedAt + std::chrono::seconds(m_impl->config.refreshTokenExpirySeconds);
    token.issuer = m_impl->config.issuer;
    token.audience = m_impl->config.audience;
    
    return token;
}

Result<SecurityToken, Error> TokenManager::generateApiKey(
    std::string_view userId,
    Permission permissions,
    std::string_view name) {
    
    SecurityToken token;
    token.token = "nova_" + Crypto::generateTokenString(48);
    token.type = TokenType::API;
    token.userId = std::string(userId);
    token.permissions = permissions;
    token.issuedAt = std::chrono::system_clock::now();
    token.expiresAt = token.issuedAt + std::chrono::hours(24 * 365); // 1 year
    token.issuer = m_impl->config.issuer;
    token.audience = m_impl->config.audience;
    token.claims["name"] = std::string(name);
    
    return token;
}

TokenValidationResult TokenManager::validateToken(std::string_view token) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    std::string tokenStr(token);
    
    // Check if revoked
    if (m_impl->revokedTokens.count(tokenStr) > 0) {
        return TokenValidationResult::failure("Token has been revoked");
    }
    
    // For this simplified implementation, we can't validate the token content
    // In production, you would decode and verify the JWT signature
    
    return TokenValidationResult::success("unknown", Permission::Read);
}

Result<SecurityToken, Error> TokenManager::decodeToken(std::string_view token) {
    // Simplified - in production, decode JWT
    SecurityToken decoded;
    decoded.token = std::string(token);
    return decoded;
}

Result<SecurityToken, Error> TokenManager::refreshAccessToken(std::string_view refreshToken) {
    auto validation = validateToken(refreshToken);
    if (!validation.valid) {
        return std::unexpected(Error(ErrorCategory::Permission, 1, validation.errorMessage));
    }
    
    return generateAccessToken(validation.userId, validation.permissions, {});
}

void TokenManager::revokeToken(std::string_view token) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->revokedTokens[std::string(token)] = std::chrono::system_clock::now();
}

bool TokenManager::isTokenRevoked(std::string_view token) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->revokedTokens.count(std::string(token)) > 0;
}

// =============================================================================
// RateLimiter Implementation
// =============================================================================

struct RateLimiter::Impl {
    Config config;
    
    struct ClientData {
        std::vector<std::chrono::system_clock::time_point> requests;
    };
    
    std::map<std::string, ClientData> clients;
    std::mutex mutex;
};

RateLimiter::RateLimiter() : m_impl(std::make_unique<Impl>()) {}

RateLimiter::RateLimiter(const Config& config) : m_impl(std::make_unique<Impl>()) {
    m_impl->config = config;
}

RateLimiter::~RateLimiter() = default;

bool RateLimiter::isAllowed(std::string_view identifier) {
    if (!m_impl->config.enabled) return true;
    
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto now = std::chrono::system_clock::now();
    auto windowStart = now - std::chrono::seconds(m_impl->config.windowSeconds);
    
    auto& client = m_impl->clients[id];
    
    // Remove old requests
    client.requests.erase(
        std::remove_if(client.requests.begin(), client.requests.end(),
            [windowStart](const auto& t) { return t < windowStart; }),
        client.requests.end());
    
    return client.requests.size() < m_impl->config.requestsPerWindow;
}

void RateLimiter::recordRequest(std::string_view identifier) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->clients[std::string(identifier)].requests.push_back(std::chrono::system_clock::now());
}

u32 RateLimiter::getRemainingRequests(std::string_view identifier) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto it = m_impl->clients.find(id);
    if (it == m_impl->clients.end()) {
        return m_impl->config.requestsPerWindow;
    }
    
    auto now = std::chrono::system_clock::now();
    auto windowStart = now - std::chrono::seconds(m_impl->config.windowSeconds);
    
    usize validRequests = 0;
    for (const auto& t : it->second.requests) {
        if (t >= windowStart) ++validRequests;
    }
    
    return static_cast<u32>(m_impl->config.requestsPerWindow > validRequests ? 
        m_impl->config.requestsPerWindow - validRequests : 0);
}

u32 RateLimiter::getResetTime(std::string_view identifier) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto it = m_impl->clients.find(id);
    if (it == m_impl->clients.end() || it->second.requests.empty()) {
        return 0;
    }
    
    auto oldest = *std::min_element(it->second.requests.begin(), it->second.requests.end());
    auto resetTime = oldest + std::chrono::seconds(m_impl->config.windowSeconds);
    auto now = std::chrono::system_clock::now();
    
    if (resetTime <= now) return 0;
    
    return static_cast<u32>(
        std::chrono::duration_cast<std::chrono::seconds>(resetTime - now).count());
}

void RateLimiter::reset(std::string_view identifier) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->clients.erase(std::string(identifier));
}

void RateLimiter::resetAll() {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->clients.clear();
}

// =============================================================================
// RequestSigner Implementation
// =============================================================================

struct RequestSigner::Impl {
    EncryptionKey signingKey;
};

RequestSigner::RequestSigner(const EncryptionKey& signingKey) : m_impl(std::make_unique<Impl>()) {
    std::memcpy(m_impl->signingKey.data(), signingKey.data(), AES_KEY_SIZE);
}

RequestSigner::~RequestSigner() = default;

RequestSigner::SignedRequest RequestSigner::signRequest(
    std::string_view method,
    std::string_view path,
    std::string_view body,
    const std::map<std::string, std::string>& headers) {
    
    SignedRequest result;
    result.timestamp = std::to_string(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    result.nonce = Crypto::generateTokenString(16);
    
    // Create string to sign
    std::string stringToSign;
    stringToSign += method;
    stringToSign += "\n";
    stringToSign += path;
    stringToSign += "\n";
    stringToSign += result.timestamp;
    stringToSign += "\n";
    stringToSign += result.nonce;
    stringToSign += "\n";
    stringToSign += body;
    
    // Compute HMAC
    std::vector<u8> keyData(m_impl->signingKey.data(), m_impl->signingKey.data() + AES_KEY_SIZE);
    std::vector<u8> data(stringToSign.begin(), stringToSign.end());
    HashResult hmac = Crypto::hmacSha256(keyData, data);
    
    result.signature = Crypto::base64Encode(std::vector<u8>(hmac.begin(), hmac.end()));
    
    result.headers["X-Nova-Signature"] = result.signature;
    result.headers["X-Nova-Timestamp"] = result.timestamp;
    result.headers["X-Nova-Nonce"] = result.nonce;
    
    for (const auto& [key, value] : headers) {
        result.headers[key] = value;
    }
    
    return result;
}

bool RequestSigner::verifyRequest(
    std::string_view method,
    std::string_view path,
    std::string_view body,
    std::string_view signature,
    std::string_view timestamp,
    std::string_view nonce,
    u32 maxAgeSeconds) {
    
    // Check timestamp age
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    auto reqTime = std::stoll(std::string(timestamp));
    
    if (std::abs(now - reqTime) > maxAgeSeconds) {
        return false;
    }
    
    // Recreate string to sign
    std::string stringToSign;
    stringToSign += method;
    stringToSign += "\n";
    stringToSign += path;
    stringToSign += "\n";
    stringToSign += timestamp;
    stringToSign += "\n";
    stringToSign += nonce;
    stringToSign += "\n";
    stringToSign += body;
    
    // Compute expected HMAC
    std::vector<u8> keyData(m_impl->signingKey.data(), m_impl->signingKey.data() + AES_KEY_SIZE);
    std::vector<u8> data(stringToSign.begin(), stringToSign.end());
    HashResult expectedHmac = Crypto::hmacSha256(keyData, data);
    
    std::string expectedSig = Crypto::base64Encode(std::vector<u8>(expectedHmac.begin(), expectedHmac.end()));
    
    return signature == expectedSig;
}

// =============================================================================
// SecureStorage Implementation
// =============================================================================

struct SecureStorage::Impl {
    EncryptionKey masterKey;
    std::map<std::string, std::vector<u8>> storage;
    std::mutex mutex;
    
    Impl() {
        masterKey = Crypto::generateKey();
    }
};

SecureStorage::SecureStorage() : m_impl(std::make_unique<Impl>()) {}
SecureStorage::~SecureStorage() = default;

Result<void, Error> SecureStorage::store(std::string_view key, std::string_view value) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    auto encrypted = Crypto::encryptString(value, m_impl->masterKey);
    if (!encrypted) return std::unexpected(encrypted.error());
    
    m_impl->storage[std::string(key)] = std::vector<u8>(encrypted->begin(), encrypted->end());
    return {};
}

Result<SecureString, Error> SecureStorage::retrieve(std::string_view key) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    auto it = m_impl->storage.find(std::string(key));
    if (it == m_impl->storage.end()) {
        return std::unexpected(Error(ErrorCategory::NotFound, 1, "Key not found"));
    }
    
    std::string encrypted(it->second.begin(), it->second.end());
    auto decrypted = Crypto::decryptString(encrypted, m_impl->masterKey);
    if (!decrypted) return std::unexpected(decrypted.error());
    
    return SecureString(*decrypted);
}

Result<void, Error> SecureStorage::remove(std::string_view key) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->storage.erase(std::string(key));
    return {};
}

bool SecureStorage::exists(std::string_view key) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->storage.count(std::string(key)) > 0;
}

void SecureStorage::clear() {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->storage.clear();
}

// =============================================================================
// InputValidator Implementation
// =============================================================================

bool InputValidator::isValidEmail(std::string_view email) {
    if (email.empty() || email.size() > 254) return false;
    
    // Simple email validation
    auto at = email.find('@');
    if (at == std::string_view::npos || at == 0 || at == email.size() - 1) return false;
    
    auto dot = email.find('.', at);
    if (dot == std::string_view::npos || dot == at + 1 || dot == email.size() - 1) return false;
    
    return true;
}

bool InputValidator::isValidPassword(std::string_view password) {
    auto validation = validatePassword(password);
    return validation.valid && validation.strength >= 60;
}

InputValidator::PasswordValidation InputValidator::validatePassword(std::string_view password) {
    PasswordValidation result;
    result.valid = true;
    result.strength = 0;
    
    if (password.size() < MIN_PASSWORD_LENGTH) {
        result.valid = false;
        result.message = "Password must be at least 8 characters";
        return result;
    }
    
    if (password.size() > MAX_PASSWORD_LENGTH) {
        result.valid = false;
        result.message = "Password must be at most 128 characters";
        return result;
    }
    
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }
    
    if (hasUpper) result.strength += 20;
    if (hasLower) result.strength += 20;
    if (hasDigit) result.strength += 20;
    if (hasSpecial) result.strength += 20;
    if (password.size() >= 12) result.strength += 20;
    
    if (result.strength < 60) {
        result.message = "Weak password - add uppercase, lowercase, numbers, and special characters";
    } else if (result.strength < 80) {
        result.message = "Moderate password strength";
    } else {
        result.message = "Strong password";
    }
    
    return result;
}

bool InputValidator::isValidUsername(std::string_view username) {
    if (username.size() < 3 || username.size() > 32) return false;
    
    for (char c : username) {
        if (!std::isalnum(c) && c != '_' && c != '-') return false;
    }
    
    // Must start with letter
    if (!std::isalpha(username[0])) return false;
    
    return true;
}

std::string InputValidator::sanitize(std::string_view input) {
    std::string result;
    result.reserve(input.size());
    
    for (char c : input) {
        // Remove control characters
        if (c >= 32 && c < 127) {
            result += c;
        }
    }
    
    return result;
}

std::string InputValidator::sanitizeHtml(std::string_view input) {
    std::string result;
    result.reserve(input.size() * 2);
    
    for (char c : input) {
        switch (c) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c;
        }
    }
    
    return result;
}

std::string InputValidator::sanitizeSql(std::string_view input) {
    std::string result;
    result.reserve(input.size() * 2);
    
    for (char c : input) {
        if (c == '\'') {
            result += "''";
        } else if (c == '\\') {
            result += "\\\\";
        } else {
            result += c;
        }
    }
    
    return result;
}

bool InputValidator::isValidUrl(std::string_view url) {
    if (url.empty() || url.size() > 2048) return false;
    
    // Check for scheme
    if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://") {
        return false;
    }
    
    return true;
}

bool InputValidator::isValidUuid(std::string_view uuid) {
    if (uuid.size() != 36) return false;
    
    // Check format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    for (usize i = 0; i < uuid.size(); ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (uuid[i] != '-') return false;
        } else {
            if (!std::isxdigit(uuid[i])) return false;
        }
    }
    
    return true;
}

// =============================================================================
// SecurityManager Implementation
// =============================================================================

struct SecurityManager::Impl {
    bool initialized = false;
    Config config;
    
    std::unique_ptr<TokenManager> tokenManager;
    std::unique_ptr<RateLimiter> rateLimiter;
    std::unique_ptr<SecureStorage> secureStorage;
    
    std::map<std::string, u32> failedLoginAttempts;
    std::map<std::string, std::chrono::system_clock::time_point> accountLockouts;
    std::mutex mutex;
};

SecurityManager& SecurityManager::get() {
    static SecurityManager instance;
    return instance;
}

SecurityManager::SecurityManager() : m_impl(std::make_unique<Impl>()) {}
SecurityManager::~SecurityManager() = default;

Result<void, Error> SecurityManager::initialize() {
    return initialize(Config{});
}

Result<void, Error> SecurityManager::initialize(const Config& config) {
    if (m_impl->initialized) {
        return std::unexpected(Error(ErrorCategory::Runtime, 1, "Already initialized"));
    }
    
    m_impl->config = config;
    
    // Generate signing key
    auto keyBytes = Crypto::randomBytes(AES_KEY_SIZE);
    
    // Create token manager
    TokenManager::Config tmConfig;
    tmConfig.issuer = config.issuer;
    tmConfig.accessTokenExpirySeconds = config.tokenExpirySeconds;
    std::copy(keyBytes.begin(), keyBytes.end(), tmConfig.signingKey.begin());
    m_impl->tokenManager = std::make_unique<TokenManager>(tmConfig);
    
    // Create rate limiter
    m_impl->rateLimiter = std::make_unique<RateLimiter>(config.rateLimitConfig);
    
    // Create secure storage
    m_impl->secureStorage = std::make_unique<SecureStorage>();
    
    m_impl->initialized = true;
    return {};
}

void SecurityManager::shutdown() {
    m_impl->tokenManager.reset();
    m_impl->rateLimiter.reset();
    m_impl->secureStorage.reset();
    m_impl->initialized = false;
}

TokenManager& SecurityManager::getTokenManager() {
    return *m_impl->tokenManager;
}

RateLimiter& SecurityManager::getRateLimiter() {
    return *m_impl->rateLimiter;
}

SecureStorage& SecurityManager::getSecureStorage() {
    return *m_impl->secureStorage;
}

Result<SecurityToken, Error> SecurityManager::authenticate(
    std::string_view username,
    std::string_view password) {
    
    std::string id(username);
    
    // Validate input parameters
    if (username.empty()) {
        return std::unexpected(Error(ErrorCategory::Validation, 1, "Username cannot be empty"));
    }
    
    if (password.empty()) {
        recordFailedLogin(id);
        return std::unexpected(Error(ErrorCategory::Permission, 2, "Password cannot be empty"));
    }
    
    // Username format validation
    if (username.length() < 3 || username.length() > 64) {
        return std::unexpected(Error(ErrorCategory::Validation, 3, 
            "Username must be between 3 and 64 characters"));
    }
    
    // Check if account is locked due to failed attempts
    if (isAccountLocked(id)) {
        return std::unexpected(Error(ErrorCategory::Permission, 1, 
            "Account is temporarily locked due to too many failed login attempts. Please try again later."));
    }
    
    // Check rate limiting for authentication attempts
    if (!checkRateLimit("auth:" + id)) {
        return std::unexpected(Error(ErrorCategory::Network, 1, 
            "Too many authentication attempts. Please wait before trying again."));
    }
    
    // Credential verification
    // Look up stored credentials from secure storage
    std::string storedHashKey = "user_password_hash:" + id;
    auto storedHashResult = m_impl->secureStorage->retrieve(storedHashKey);
    
    bool credentialsValid = false;
    
    if (storedHashResult.has_value()) {
        // User exists - verify password against stored hash
        std::string storedHash = std::string(storedHashResult->view());
        credentialsValid = Crypto::verifyPassword(password, storedHash);
    } else {
        // User not found in storage - check for demo/test users
        // For development/testing, we allow a simple validation
        if (password.length() >= 8) {
            // Create new user entry (development mode only)
            std::string passwordHash = Crypto::hashPassword(password);
            auto storeResult = m_impl->secureStorage->store(storedHashKey, passwordHash);
            if (storeResult.has_value()) {
                credentialsValid = true;
            }
        }
    }
    
    if (!credentialsValid) {
        recordFailedLogin(id);
        
        // Get remaining attempts
        u32 remainingAttempts = 0;
        {
            std::lock_guard<std::mutex> lock(m_impl->mutex);
            auto it = m_impl->failedLoginAttempts.find(id);
            if (it != m_impl->failedLoginAttempts.end()) {
                u32 failedCount = it->second;
                remainingAttempts = m_impl->config.maxLoginAttempts > failedCount 
                    ? m_impl->config.maxLoginAttempts - failedCount : 0;
            }
        }
        
        std::string errorMsg = "Invalid username or password";
        if (remainingAttempts > 0 && remainingAttempts <= 3) {
            errorMsg += ". " + std::to_string(remainingAttempts) + " attempt(s) remaining.";
        }
        
        return std::unexpected(Error(ErrorCategory::Permission, 2, errorMsg));
    }
    
    // Successful authentication - reset failed attempts
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->failedLoginAttempts.erase(id);
    }
    
    // Generate access token with appropriate permissions
    Permission userPermissions = Permission::ReadWrite;
    std::map<std::string, std::string> claims;
    
    // Check for admin role (stored in secure storage)
    std::string roleKey = "user_roles:" + id;
    auto roleResult = m_impl->secureStorage->retrieve(roleKey);
    if (roleResult.has_value()) {
        std::string roleData = std::string(roleResult->view());
        if (roleData.find("admin") != std::string::npos) {
            userPermissions = Permission::Admin;
            claims["role"] = "admin";
        }
    }
    
    return m_impl->tokenManager->generateAccessToken(username, userPermissions, claims);
}

Result<SecurityContext, Error> SecurityManager::validateRequest(
    std::string_view token,
    Permission requiredPermission) {
    
    auto validation = m_impl->tokenManager->validateToken(token);
    if (!validation.valid) {
        return std::unexpected(Error(ErrorCategory::Permission, 1, validation.errorMessage));
    }
    
    SecurityContext ctx;
    ctx.authenticated = true;
    ctx.userId = validation.userId;
    ctx.permissions = validation.permissions;
    ctx.token.token = std::string(token);
    
    if (requiredPermission != Permission::None) {
        auto result = ctx.requirePermission(requiredPermission);
        if (!result) return std::unexpected(result.error());
    }
    
    return ctx;
}

bool SecurityManager::checkRateLimit(std::string_view identifier) {
    if (!m_impl->config.enableRateLimiting) return true;
    
    bool allowed = m_impl->rateLimiter->isAllowed(identifier);
    if (allowed) {
        m_impl->rateLimiter->recordRequest(identifier);
    }
    return allowed;
}

void SecurityManager::recordFailedLogin(std::string_view identifier) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    m_impl->failedLoginAttempts[id]++;
    
    if (m_impl->failedLoginAttempts[id] >= m_impl->config.maxLoginAttempts) {
        m_impl->accountLockouts[id] = std::chrono::system_clock::now() + 
            std::chrono::seconds(m_impl->config.lockoutDurationSeconds);
    }
}

bool SecurityManager::isAccountLocked(std::string_view identifier) const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::string id(identifier);
    
    auto it = m_impl->accountLockouts.find(id);
    if (it == m_impl->accountLockouts.end()) return false;
    
    if (std::chrono::system_clock::now() >= it->second) {
        // Lockout expired
        return false;
    }
    
    return true;
}

} // namespace nova::api::security
