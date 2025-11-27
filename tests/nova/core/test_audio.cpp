/**
 * @file test_audio.cpp
 * @brief Comprehensive tests for Nova Audio System
 * @copyright (c) 2025 WeNova Interactive. All rights reserved.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/audio/audio_types.hpp>

using namespace nova;
using namespace nova::audio;
using Catch::Approx;

// =============================================================================
// Audio Configuration Tests
// =============================================================================

TEST_CASE("Audio: Config constants", "[audio][config]") {
    SECTION("Sample rate") {
        REQUIRE(AudioConfig::DEFAULT_SAMPLE_RATE == 48000);
    }
    
    SECTION("Buffer size") {
        REQUIRE(AudioConfig::DEFAULT_BUFFER_SIZE == 1024);
    }
    
    SECTION("Channel limits") {
        REQUIRE(AudioConfig::MAX_CHANNELS == 32);
        REQUIRE(AudioConfig::MAX_BUSES == 16);
        REQUIRE(AudioConfig::MAX_EFFECTS_PER_BUS == 8);
        REQUIRE(AudioConfig::MAX_LISTENERS == 4);
    }
    
    SECTION("Volume limits") {
        REQUIRE(AudioConfig::MIN_VOLUME == Approx(0.0f));
        REQUIRE(AudioConfig::MAX_VOLUME == Approx(2.0f));
    }
    
    SECTION("Pitch limits") {
        REQUIRE(AudioConfig::MIN_PITCH == Approx(0.25f));
        REQUIRE(AudioConfig::MAX_PITCH == Approx(4.0f));
    }
    
    SECTION("Spatial audio constants") {
        REQUIRE(AudioConfig::DEFAULT_DOPPLER_FACTOR == Approx(1.0f));
        REQUIRE(AudioConfig::SPEED_OF_SOUND == Approx(343.0f));
    }
}

// =============================================================================
// Sample Format Tests
// =============================================================================

TEST_CASE("Audio: SampleFormat", "[audio][format]") {
    SECTION("All formats defined") {
        REQUIRE(SampleFormat::Int8 != SampleFormat::Int16);
        REQUIRE(SampleFormat::Int16 != SampleFormat::Int24);
        REQUIRE(SampleFormat::Int24 != SampleFormat::Int32);
        REQUIRE(SampleFormat::Int32 != SampleFormat::Float32);
        REQUIRE(SampleFormat::Float32 != SampleFormat::Float64);
    }
}

// =============================================================================
// Channel Layout Tests
// =============================================================================

TEST_CASE("Audio: ChannelLayout", "[audio][format]") {
    SECTION("Channel counts") {
        REQUIRE(static_cast<u8>(ChannelLayout::Mono) == 1);
        REQUIRE(static_cast<u8>(ChannelLayout::Stereo) == 2);
        REQUIRE(static_cast<u8>(ChannelLayout::Quad) == 4);
        REQUIRE(static_cast<u8>(ChannelLayout::Surround51) == 6);
        REQUIRE(static_cast<u8>(ChannelLayout::Surround71) == 8);
    }
}

// =============================================================================
// Audio Codec Tests
// =============================================================================

TEST_CASE("Audio: AudioCodec", "[audio][format]") {
    SECTION("All codecs defined") {
        REQUIRE(AudioCodec::PCM != AudioCodec::WAV);
        REQUIRE(AudioCodec::OGG != AudioCodec::MP3);
        REQUIRE(AudioCodec::FLAC != AudioCodec::AAC);
        REQUIRE(AudioCodec::OPUS != AudioCodec::ADPCM);
    }
}

// =============================================================================
// Audio Format Tests
// =============================================================================

TEST_CASE("Audio: AudioFormat presets", "[audio][format]") {
    SECTION("Default values") {
        AudioFormat fmt{};
        REQUIRE(fmt.sampleFormat == SampleFormat::Float32);
        REQUIRE(fmt.channelLayout == ChannelLayout::Stereo);
        REQUIRE(fmt.sampleRate == AudioConfig::DEFAULT_SAMPLE_RATE);
        REQUIRE(fmt.channels == 2);
    }
    
    SECTION("Stereo 44100") {
        AudioFormat fmt = AudioFormat::stereo44100();
        REQUIRE(fmt.sampleRate == 44100);
        REQUIRE(fmt.channels == 2);
    }
    
    SECTION("Stereo 48000") {
        AudioFormat fmt = AudioFormat::stereo48000();
        REQUIRE(fmt.sampleRate == 48000);
        REQUIRE(fmt.channels == 2);
    }
    
    SECTION("Mono 44100") {
        AudioFormat fmt = AudioFormat::mono44100();
        REQUIRE(fmt.sampleRate == 44100);
        REQUIRE(fmt.channels == 1);
    }
    
    SECTION("Surround 5.1") {
        AudioFormat fmt = AudioFormat::surround51();
        REQUIRE(fmt.channels == 6);
    }
}

// =============================================================================
// Playback Mode Tests
// =============================================================================

TEST_CASE("Audio: PlaybackMode", "[audio][playback]") {
    SECTION("All modes defined") {
        REQUIRE(PlaybackMode::Once != PlaybackMode::Loop);
        REQUIRE(PlaybackMode::Loop != PlaybackMode::LoopCount);
        REQUIRE(PlaybackMode::LoopCount != PlaybackMode::PingPong);
    }
}

// =============================================================================
// Load Mode Tests
// =============================================================================

TEST_CASE("Audio: LoadMode", "[audio][playback]") {
    SECTION("All modes defined") {
        REQUIRE(LoadMode::Streaming != LoadMode::Decompressed);
        REQUIRE(LoadMode::Decompressed != LoadMode::Compressed);
    }
}

// =============================================================================
// Sound Priority Tests
// =============================================================================

TEST_CASE("Audio: SoundPriority", "[audio][playback]") {
    SECTION("Priority ordering") {
        REQUIRE(static_cast<u8>(SoundPriority::Lowest) < static_cast<u8>(SoundPriority::Low));
        REQUIRE(static_cast<u8>(SoundPriority::Low) < static_cast<u8>(SoundPriority::Normal));
        REQUIRE(static_cast<u8>(SoundPriority::Normal) < static_cast<u8>(SoundPriority::High));
        REQUIRE(static_cast<u8>(SoundPriority::High) < static_cast<u8>(SoundPriority::Highest));
    }
    
    SECTION("Priority values") {
        REQUIRE(static_cast<u8>(SoundPriority::Lowest) == 0);
        REQUIRE(static_cast<u8>(SoundPriority::Normal) == 128);
        REQUIRE(static_cast<u8>(SoundPriority::Highest) == 255);
    }
}
