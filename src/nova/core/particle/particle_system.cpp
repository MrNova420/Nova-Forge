/**
 * @file particle_system.cpp
 * @brief NovaCore Particle System™ - Particle System Implementation
 * 
 * Full implementation of the particle system including:
 * - Particle emission with various shapes
 * - Lifetime modules (size, color, velocity, rotation)
 * - Force fields and physics
 * - Collision detection
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/core/particle/particle_system.hpp>
#include <nova/core/logging/logging.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <chrono>

namespace nova::particle {

using namespace nova::logging;
using namespace nova::math;

// ============================================================================
// Helper Functions
// ============================================================================

namespace {

/// Linear interpolation
f32 lerp(f32 a, f32 b, f32 t) {
    return a + (b - a) * t;
}

/// Hash function for random
u32 hashU32(u32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

/// Random float from seed (0-1)
f32 randomFromSeed(u32 seed) {
    return static_cast<f32>(hashU32(seed) & 0xFFFFFF) / static_cast<f32>(0xFFFFFF);
}

/// Simplex noise approximation (simple version)
f32 noise3D(f32 x, f32 y, f32 z) {
    // Simple noise using sin combinations
    return (std::sin(x * 1.0f + y * 2.1f + z * 3.2f) +
            std::sin(x * 5.4f + y * 3.3f + z * 2.2f) * 0.5f +
            std::sin(x * 12.1f + y * 8.7f + z * 7.3f) * 0.25f) / 1.75f;
}

} // anonymous namespace

// ============================================================================
// ParticleCurve Implementation
// ============================================================================

f32 ParticleCurve::evaluate(f32 t) const {
    switch (type) {
        case CurveType::Constant:
            return constantValue;
            
        case CurveType::Linear:
            if (keys.size() < 2) return constantValue;
            return lerp(keys.front().value, keys.back().value, t);
            
        case CurveType::Curve: {
            if (keys.empty()) return constantValue;
            if (keys.size() == 1) return keys[0].value;
            
            // Find surrounding keys
            usize nextIdx = 0;
            for (usize i = 0; i < keys.size(); ++i) {
                if (keys[i].time > t) {
                    nextIdx = i;
                    break;
                }
                nextIdx = i;
            }
            
            if (nextIdx == 0) return keys[0].value;
            
            const auto& prev = keys[nextIdx - 1];
            const auto& next = keys[nextIdx];
            
            f32 dt = next.time - prev.time;
            f32 localT = (dt > 0.0f) ? (t - prev.time) / dt : 0.0f;
            
            // Cubic hermite interpolation
            f32 t2 = localT * localT;
            f32 t3 = t2 * localT;
            
            f32 h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
            f32 h10 = t3 - 2.0f * t2 + localT;
            f32 h01 = -2.0f * t3 + 3.0f * t2;
            f32 h11 = t3 - t2;
            
            return h00 * prev.value + 
                   h10 * prev.outTangent * dt + 
                   h01 * next.value + 
                   h11 * next.inTangent * dt;
        }
            
        case CurveType::RandomBetweenTwo:
            return lerp(minValue, maxValue, 0.5f); // Default to middle
            
        case CurveType::RandomBetweenCurves:
            return evaluate(t); // Use base curve
            
        default:
            return constantValue;
    }
}

f32 ParticleCurve::evaluateRandom(f32 t, u32 seed) const {
    switch (type) {
        case CurveType::RandomBetweenTwo: {
            f32 r = randomFromSeed(seed);
            return lerp(minValue, maxValue, r);
        }
            
        case CurveType::RandomBetweenCurves: {
            if (keysMax.empty()) return evaluate(t);
            
            // Evaluate both curves and interpolate
            f32 r = randomFromSeed(seed);
            f32 minVal = evaluate(t);
            
            // Evaluate max curve manually
            f32 maxVal = minVal;
            if (!keysMax.empty()) {
                // Similar evaluation for max curve
                usize nextIdx = 0;
                for (usize i = 0; i < keysMax.size(); ++i) {
                    if (keysMax[i].time > t) {
                        nextIdx = i;
                        break;
                    }
                    nextIdx = i;
                }
                
                if (nextIdx == 0 || keysMax.size() == 1) {
                    maxVal = keysMax[0].value;
                } else {
                    const auto& prev = keysMax[nextIdx - 1];
                    const auto& next = keysMax[nextIdx];
                    f32 dt = next.time - prev.time;
                    f32 localT = (dt > 0.0f) ? (t - prev.time) / dt : 0.0f;
                    maxVal = lerp(prev.value, next.value, localT);
                }
            }
            
            return lerp(minVal, maxVal, r);
        }
            
        default:
            return evaluate(t);
    }
}

// ============================================================================
// ColorGradient Implementation
// ============================================================================

Color ColorGradient::evaluate(f32 t) const {
    Color result{1.0f, 1.0f, 1.0f, 1.0f};
    
    // Evaluate color
    if (!colorKeys.empty()) {
        if (colorKeys.size() == 1 || t <= colorKeys.front().time) {
            result = colorKeys.front().color;
        } else if (t >= colorKeys.back().time) {
            result = colorKeys.back().color;
        } else {
            // Find surrounding keys
            for (usize i = 1; i < colorKeys.size(); ++i) {
                if (colorKeys[i].time >= t) {
                    const auto& prev = colorKeys[i - 1];
                    const auto& next = colorKeys[i];
                    f32 dt = next.time - prev.time;
                    f32 localT = (dt > 0.0f) ? (t - prev.time) / dt : 0.0f;
                    
                    result.x = lerp(prev.color.x, next.color.x, localT);
                    result.y = lerp(prev.color.y, next.color.y, localT);
                    result.z = lerp(prev.color.z, next.color.z, localT);
                    break;
                }
            }
        }
    }
    
    // Evaluate alpha
    if (!alphaKeys.empty()) {
        if (alphaKeys.size() == 1 || t <= alphaKeys.front().time) {
            result.w = alphaKeys.front().alpha;
        } else if (t >= alphaKeys.back().time) {
            result.w = alphaKeys.back().alpha;
        } else {
            for (usize i = 1; i < alphaKeys.size(); ++i) {
                if (alphaKeys[i].time >= t) {
                    const auto& prev = alphaKeys[i - 1];
                    const auto& next = alphaKeys[i];
                    f32 dt = next.time - prev.time;
                    f32 localT = (dt > 0.0f) ? (t - prev.time) / dt : 0.0f;
                    
                    result.w = lerp(prev.alpha, next.alpha, localT);
                    break;
                }
            }
        }
    }
    
    return result;
}

// ============================================================================
// MinMaxValue Implementation
// ============================================================================

f32 MinMaxValue::getValue(u32 seed) const {
    switch (type) {
        case CurveType::Constant:
            return constantValue;
            
        case CurveType::RandomBetweenTwo: {
            f32 r = randomFromSeed(seed);
            return lerp(minValue, maxValue, r);
        }
            
        default:
            return constantValue;
    }
}

Vec3 MinMaxVec3::getValue(u32 seed) const {
    if (separateAxes) {
        return Vec3(
            lerp(min.x, max.x, randomFromSeed(seed)),
            lerp(min.y, max.y, randomFromSeed(seed + 1)),
            lerp(min.z, max.z, randomFromSeed(seed + 2))
        );
    } else {
        f32 r = randomFromSeed(seed);
        return Vec3(
            lerp(min.x, max.x, r),
            lerp(min.y, max.y, r),
            lerp(min.z, max.z, r)
        );
    }
}

// ============================================================================
// ForceField Implementation
// ============================================================================

Vec3 ForceField::calculateForce(const Vec3& particlePos, const Vec3& particleVel) const {
    Vec3 force{};
    
    Vec3 toParticle = particlePos - position;
    f32 distance = toParticle.length();
    
    // Apply range falloff
    f32 attenuation = 1.0f;
    if (range > 0.0f && distance > 0.0f) {
        if (distance > range) return Vec3{};
        attenuation = std::pow(1.0f - (distance / range), falloff);
    }
    
    switch (type) {
        case ForceType::Gravity:
            force = direction * strength;
            break;
            
        case ForceType::Wind:
            force = direction * strength;
            // Add noise
            if (noiseStrength > 0.0f) {
                f32 n = noise3D(particlePos.x * frequency, 
                                particlePos.y * frequency, 
                                particlePos.z * frequency);
                force = force + Vec3(n, n * 0.5f, n * 0.3f) * noiseStrength;
            }
            break;
            
        case ForceType::Turbulence: {
            f32 nx = noise3D(particlePos.x * frequency, 
                             particlePos.y * frequency, 
                             particlePos.z * frequency);
            f32 ny = noise3D(particlePos.x * frequency + 100.0f, 
                             particlePos.y * frequency + 100.0f, 
                             particlePos.z * frequency);
            f32 nz = noise3D(particlePos.x * frequency + 200.0f, 
                             particlePos.y * frequency, 
                             particlePos.z * frequency + 200.0f);
            force = Vec3(nx, ny, nz) * strength;
            break;
        }
            
        case ForceType::Vortex: {
            if (distance > 0.0f) {
                Vec3 dir = toParticle.normalized();
                Vec3 tangent = axis.cross(dir).normalized();
                force = tangent * strength;
            }
            break;
        }
            
        case ForceType::Attractor:
            if (distance > 0.001f) {
                force = -toParticle.normalized() * strength / (distance * distance + 0.1f);
            }
            break;
            
        case ForceType::Repulsor:
            if (distance > 0.001f) {
                force = toParticle.normalized() * strength / (distance * distance + 0.1f);
            }
            break;
            
        case ForceType::Drag:
            force = -particleVel * strength;
            break;
            
        default:
            break;
    }
    
    return force * attenuation;
}

// ============================================================================
// ParticleEmitter Implementation
// ============================================================================

void ParticleEmitter::initialize(const ParticleSystemData& data) {
    m_data = data;
    m_particles.reserve(data.main.maxParticles);
    
    // Initialize burst tracking
    m_burstCyclesRemaining.resize(data.emission.bursts.size());
    m_burstNextTime.resize(data.emission.bursts.size());
    
    for (usize i = 0; i < data.emission.bursts.size(); ++i) {
        m_burstCyclesRemaining[i] = data.emission.bursts[i].cycles;
        m_burstNextTime[i] = data.emission.bursts[i].time;
    }
    
    // Auto-play if configured
    if (data.main.playOnAwake) {
        play();
    }
}

void ParticleEmitter::reset() {
    m_particles.clear();
    m_time = 0.0f;
    m_emissionAccumulator = 0.0f;
    m_playing = false;
    m_paused = false;
    
    // Reset bursts
    for (usize i = 0; i < m_data.emission.bursts.size(); ++i) {
        m_burstCyclesRemaining[i] = m_data.emission.bursts[i].cycles;
        m_burstNextTime[i] = m_data.emission.bursts[i].time;
    }
}

void ParticleEmitter::update(f32 deltaTime, const std::vector<ForceField>& globalForces) {
    if (!m_playing || m_paused) {
        return;
    }
    
    f32 dt = deltaTime * m_playbackSpeed * m_data.main.simulationSpeed;
    m_time += dt;
    
    // Handle delay
    if (m_time < m_data.main.startDelay) {
        return;
    }
    
    // Handle looping/duration
    if (!m_data.main.loop && m_time >= m_data.main.duration + m_data.main.startDelay) {
        m_playing = false;
    }
    
    // Emission
    if (m_data.emission.enabled) {
        // Rate over time
        f32 rate = m_data.emission.rateOverTime.getValue(m_randomSeed);
        m_emissionAccumulator += rate * dt;
        
        while (m_emissionAccumulator >= 1.0f) {
            if (m_particles.size() < m_data.main.maxParticles) {
                emitParticle();
            }
            m_emissionAccumulator -= 1.0f;
        }
        
        // Bursts
        for (usize i = 0; i < m_data.emission.bursts.size(); ++i) {
            const auto& burst = m_data.emission.bursts[i];
            
            if (m_burstCyclesRemaining[i] != 0 && m_time >= m_burstNextTime[i]) {
                // Check probability
                if (randomFloat() <= burst.probability) {
                    u32 count = static_cast<u32>(burst.count.getValue(nextRandom()));
                    emit(count);
                }
                
                // Update burst state
                if (m_burstCyclesRemaining[i] > 0) {
                    m_burstCyclesRemaining[i]--;
                }
                m_burstNextTime[i] += burst.interval;
            }
        }
    }
    
    // Combine global forces with emitter forces
    std::vector<ForceField> allForces = globalForces;
    allForces.insert(allForces.end(), m_data.forces.begin(), m_data.forces.end());
    
    // Update particles
    for (auto& p : m_particles) {
        if (p.alive) {
            updateParticle(p, dt, allForces);
        }
    }
    
    // Remove dead particles
    removeDeadParticles();
}

void ParticleEmitter::play() {
    m_playing = true;
    m_paused = false;
}

void ParticleEmitter::pause() {
    m_paused = true;
}

void ParticleEmitter::stop() {
    m_playing = false;
    m_paused = false;
    m_particles.clear();
}

void ParticleEmitter::restart() {
    reset();
    play();
}

void ParticleEmitter::emit(u32 count) {
    for (u32 i = 0; i < count && m_particles.size() < m_data.main.maxParticles; ++i) {
        emitParticle();
    }
}

void ParticleEmitter::triggerBurst(u32 burstIndex) {
    if (burstIndex < m_data.emission.bursts.size()) {
        const auto& burst = m_data.emission.bursts[burstIndex];
        u32 count = static_cast<u32>(burst.count.getValue(nextRandom()));
        emit(count);
    }
}

void ParticleEmitter::setSpawnCallback(ParticleSpawnCallback callback) {
    m_spawnCallback = std::move(callback);
}

void ParticleEmitter::setDeathCallback(ParticleDeathCallback callback) {
    m_deathCallback = std::move(callback);
}

void ParticleEmitter::setCollisionCallback(ParticleCollisionCallback callback) {
    m_collisionCallback = std::move(callback);
}

void ParticleEmitter::emitParticle() {
    Particle p;
    p.randomSeed = nextRandom();
    
    // Position and direction from shape
    p.position = getEmissionPosition();
    Vec3 direction = getEmissionDirection();
    
    // Apply main module settings
    p.maxLifetime = m_data.main.startLifetime.getValue(p.randomSeed);
    p.lifetime = 0.0f;
    
    f32 speed = m_data.main.startSpeed.getValue(p.randomSeed + 1);
    p.velocity = direction * speed;
    
    f32 size = m_data.main.startSize.getValue(p.randomSeed + 2);
    p.size = Vec3(size, size, size);
    
    p.rotation = m_data.main.startRotation.getValue(p.randomSeed + 3);
    
    p.color = m_data.main.startColor.evaluate(0.0f);
    
    p.alive = true;
    
    // Transform to world space if needed
    if (m_data.main.simulationSpace == SimulationSpace::World) {
        // Apply emitter transform
        Mat4 transform = Mat4::translate(m_position) * m_rotation.toMat4() * Mat4::scale(m_scale);
        Vec4 worldPos = transform * Vec4(p.position, 1.0f);
        p.position = Vec3(worldPos.x, worldPos.y, worldPos.z);
        
        // Transform velocity
        Mat4 rotMat = m_rotation.toMat4();
        Vec4 worldVel = rotMat * Vec4(p.velocity, 0.0f);
        p.velocity = Vec3(worldVel.x, worldVel.y, worldVel.z);
    }
    
    // Callback
    if (m_spawnCallback) {
        m_spawnCallback(p);
    }
    
    m_particles.push_back(p);
}

void ParticleEmitter::updateParticle(Particle& p, f32 dt, const std::vector<ForceField>& forces) {
    // Update lifetime
    p.lifetime += dt;
    
    if (p.lifetime >= p.maxLifetime) {
        p.alive = false;
        if (m_deathCallback) {
            m_deathCallback(p);
        }
        return;
    }
    
    // Apply forces
    applyForces(p, dt, forces);
    
    // Apply modules
    applyModules(p, dt);
    
    // Update position
    p.position = p.position + p.velocity * dt;
    
    // Update rotation
    p.rotation = p.rotation + p.angularVelocity * dt;
    
    // Collision
    if (m_data.collision.enabled) {
        checkCollisions(p, dt);
    }
}

void ParticleEmitter::applyModules(Particle& p, [[maybe_unused]] f32 dt) {
    f32 t = p.getNormalizedLifetime();
    
    // Size over lifetime
    if (m_data.sizeOverLifetime.enabled) {
        f32 sizeMult = m_data.sizeOverLifetime.size.evaluateRandom(t, p.randomSeed + 100);
        f32 baseSize = m_data.main.startSize.getValue(p.randomSeed + 2);
        p.size = Vec3(baseSize * sizeMult, baseSize * sizeMult, baseSize * sizeMult);
    }
    
    // Color over lifetime
    if (m_data.colorOverLifetime.enabled) {
        Color startColor = m_data.main.startColor.evaluate(0.0f);
        Color lifetimeColor = m_data.colorOverLifetime.color.evaluate(t);
        p.color.x = startColor.x * lifetimeColor.x;
        p.color.y = startColor.y * lifetimeColor.y;
        p.color.z = startColor.z * lifetimeColor.z;
        p.color.w = startColor.w * lifetimeColor.w;
    }
    
    // Velocity over lifetime
    if (m_data.velocityOverLifetime.enabled) {
        f32 speedMod = m_data.velocityOverLifetime.speedModifier.evaluateRandom(t, p.randomSeed + 200);
        p.velocity = p.velocity * speedMod;
        
        Vec3 linearVel = m_data.velocityOverLifetime.linear.getValue(p.randomSeed + 201);
        p.velocity = p.velocity + linearVel;
    }
    
    // Rotation over lifetime
    if (m_data.rotationOverLifetime.enabled) {
        f32 angVel = m_data.rotationOverLifetime.angularVelocity.evaluateRandom(t, p.randomSeed + 300);
        p.angularVelocity = Vec3(0.0f, 0.0f, angVel);
    }
    
    // Noise
    if (m_data.noise.enabled) {
        f32 noiseScale = m_data.noise.frequency;
        f32 noiseStr = m_data.noise.strength;
        
        if (m_data.noise.damping) {
            noiseStr *= (1.0f - t);
        }
        
        f32 nx = noise3D(p.position.x * noiseScale, 
                         p.position.y * noiseScale, 
                         p.position.z * noiseScale);
        f32 ny = noise3D(p.position.x * noiseScale + 100.0f, 
                         p.position.y * noiseScale + 100.0f, 
                         p.position.z * noiseScale);
        f32 nz = noise3D(p.position.x * noiseScale + 200.0f, 
                         p.position.y * noiseScale, 
                         p.position.z * noiseScale + 200.0f);
        
        p.velocity = p.velocity + Vec3(nx, ny, nz) * noiseStr;
    }
}

void ParticleEmitter::applyForces(Particle& p, f32 dt, const std::vector<ForceField>& forces) {
    // Gravity
    p.velocity.y -= m_data.main.gravityModifier * 9.81f * dt;
    
    // Force over lifetime
    if (m_data.forceOverLifetime.enabled) {
        Vec3 force = m_data.forceOverLifetime.force.getValue(p.randomSeed + 400);
        p.velocity = p.velocity + force * dt;
    }
    
    // External forces
    for (const auto& field : forces) {
        Vec3 force = field.calculateForce(p.position, p.velocity);
        p.velocity = p.velocity + force * dt;
    }
}

void ParticleEmitter::checkCollisions(Particle& p, [[maybe_unused]] f32 dt) {
    if (m_data.collision.mode == CollisionMode::None) {
        return;
    }
    
    // Check against planes
    for (const auto& plane : m_data.collision.planes) {
        Vec3 normal(plane.x, plane.y, plane.z);
        f32 distance = plane.w;
        
        // Distance from particle to plane
        f32 d = normal.dot(p.position) - distance;
        
        if (d < 0.0f) {
            // Collision!
            Vec3 hitPoint = p.position - normal * d;
            
            switch (m_data.collision.response) {
                case CollisionResponse::Kill:
                    p.alive = false;
                    if (m_deathCallback) {
                        m_deathCallback(p);
                    }
                    break;
                    
                case CollisionResponse::Bounce:
                    // Reflect velocity
                    p.velocity = p.velocity - normal * 2.0f * p.velocity.dot(normal);
                    p.velocity = p.velocity * m_data.collision.bounce;
                    p.position = hitPoint + normal * 0.01f;
                    
                    // Lifetime loss
                    p.lifetime += p.maxLifetime * m_data.collision.lifetimeLoss;
                    break;
                    
                case CollisionResponse::Stick:
                    p.velocity = Vec3{};
                    p.position = hitPoint + normal * 0.01f;
                    break;
                    
                case CollisionResponse::Callback:
                    if (m_collisionCallback) {
                        m_collisionCallback(p, hitPoint, normal);
                    }
                    break;
            }
            
            break; // Only handle one collision per frame
        }
    }
}

void ParticleEmitter::removeDeadParticles() {
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
                       [](const Particle& p) { return !p.alive; }),
        m_particles.end()
    );
}

Vec3 ParticleEmitter::getEmissionPosition() {
    const auto& shape = m_data.shape;
    
    if (!shape.enabled) {
        return shape.position;
    }
    
    Vec3 pos = shape.position;
    
    switch (shape.shape) {
        case EmissionShape::Point:
            // Just the position
            break;
            
        case EmissionShape::Sphere: {
            // Random point in/on sphere
            f32 theta = randomFloat(0.0f, 2.0f * math::PI_F32);
            f32 phi = std::acos(randomFloat(-1.0f, 1.0f));
            f32 r = shape.radius;
            
            if (shape.radiusThickness < 1.0f) {
                r *= std::pow(randomFloat(shape.radiusThickness, 1.0f), 1.0f / 3.0f);
            }
            
            pos.x += r * std::sin(phi) * std::cos(theta);
            pos.y += r * std::sin(phi) * std::sin(theta);
            pos.z += r * std::cos(phi);
            break;
        }
            
        case EmissionShape::Cone: {
            f32 angle = math::radians(shape.angle);
            f32 theta = randomFloat(0.0f, 2.0f * math::PI_F32);
            f32 r = shape.radius * randomFloat(0.0f, 1.0f);
            
            pos.x += r * std::cos(theta);
            pos.z += r * std::sin(theta);
            
            // Height based on angle
            f32 height = randomFloat(0.0f, shape.length);
            pos.y += height;
            
            // Expand cone at height
            f32 expandRadius = height * std::tan(angle);
            pos.x += expandRadius * std::cos(theta);
            pos.z += expandRadius * std::sin(theta);
            break;
        }
            
        case EmissionShape::Box: {
            pos.x += randomFloat(-0.5f, 0.5f) * shape.scale.x;
            pos.y += randomFloat(-0.5f, 0.5f) * shape.scale.y;
            pos.z += randomFloat(-0.5f, 0.5f) * shape.scale.z;
            break;
        }
            
        case EmissionShape::Circle: {
            f32 theta = randomFloat(0.0f, math::radians(shape.arc));
            f32 r = shape.radius;
            
            if (shape.radiusThickness < 1.0f) {
                r *= std::sqrt(randomFloat(shape.radiusThickness, 1.0f));
            }
            
            pos.x += r * std::cos(theta);
            pos.z += r * std::sin(theta);
            break;
        }
            
        case EmissionShape::Hemisphere: {
            f32 theta = randomFloat(0.0f, 2.0f * math::PI_F32);
            f32 phi = std::acos(randomFloat(0.0f, 1.0f));
            f32 r = shape.radius;
            
            if (shape.radiusThickness < 1.0f) {
                r *= std::pow(randomFloat(shape.radiusThickness, 1.0f), 1.0f / 3.0f);
            }
            
            pos.x += r * std::sin(phi) * std::cos(theta);
            pos.y += r * std::cos(phi); // Only positive Y
            pos.z += r * std::sin(phi) * std::sin(theta);
            break;
        }
            
        case EmissionShape::Edge: {
            f32 t = randomFloat(0.0f, 1.0f);
            pos.x += (t - 0.5f) * shape.length;
            break;
        }
            
        case EmissionShape::Rectangle: {
            pos.x += randomFloat(-0.5f, 0.5f) * shape.scale.x;
            pos.z += randomFloat(-0.5f, 0.5f) * shape.scale.z;
            break;
        }
            
        default:
            break;
    }
    
    return pos * shape.scale;
}

Vec3 ParticleEmitter::getEmissionDirection() {
    const auto& shape = m_data.shape;
    
    Vec3 dir(0.0f, 1.0f, 0.0f); // Default up
    
    if (!shape.enabled) {
        return dir;
    }
    
    switch (shape.shape) {
        case EmissionShape::Cone: {
            f32 angle = math::radians(shape.angle);
            f32 theta = randomFloat(0.0f, 2.0f * math::PI_F32);
            f32 phi = randomFloat(0.0f, angle);
            
            dir.x = std::sin(phi) * std::cos(theta);
            dir.y = std::cos(phi);
            dir.z = std::sin(phi) * std::sin(theta);
            break;
        }
            
        case EmissionShape::Sphere:
        case EmissionShape::Hemisphere: {
            f32 theta = randomFloat(0.0f, 2.0f * math::PI_F32);
            f32 phi = std::acos(randomFloat(-1.0f, 1.0f));
            
            dir.x = std::sin(phi) * std::cos(theta);
            dir.y = std::sin(phi) * std::sin(theta);
            dir.z = std::cos(phi);
            
            if (shape.shape == EmissionShape::Hemisphere && dir.y < 0.0f) {
                dir.y = -dir.y;
            }
            break;
        }
            
        default:
            // Use shape normal
            break;
    }
    
    // Add randomization
    if (shape.randomDirectionAmount > 0.0f) {
        f32 rx = randomFloat(-1.0f, 1.0f);
        f32 ry = randomFloat(-1.0f, 1.0f);
        f32 rz = randomFloat(-1.0f, 1.0f);
        
        dir = dir.lerp(Vec3(rx, ry, rz).normalized(), shape.randomDirectionAmount);
    }
    
    return dir.normalized();
}

u32 ParticleEmitter::nextRandom() {
    m_randomSeed = m_randomSeed * 1103515245 + 12345;
    return m_randomSeed;
}

f32 ParticleEmitter::randomFloat() {
    return static_cast<f32>(nextRandom() & 0xFFFFFF) / static_cast<f32>(0xFFFFFF);
}

f32 ParticleEmitter::randomFloat(f32 minVal, f32 maxVal) {
    return minVal + randomFloat() * (maxVal - minVal);
}

// ============================================================================
// ParticleManager Implementation
// ============================================================================

ParticleManager& ParticleManager::get() {
    static ParticleManager instance;
    return instance;
}

bool ParticleManager::initialize(const ParticleSystemConfig& config) {
    if (m_initialized) {
        return true;
    }
    
    NOVA_LOG_INFO(LogCategory::Core, "Initializing Nova Particle System...");
    
    m_config = config;
    m_initialized = true;
    
    NOVA_LOG_INFO(LogCategory::Core, "Nova Particle System initialized");
    return true;
}

void ParticleManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    NOVA_LOG_INFO(LogCategory::Core, "Shutting down Nova Particle System...");
    
    m_emitters.clear();
    m_systems.clear();
    m_globalForces.clear();
    
    m_initialized = false;
    
    NOVA_LOG_INFO(LogCategory::Core, "Nova Particle System shutdown complete");
}

void ParticleManager::update(f32 deltaTime) {
    if (!m_initialized) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    m_stats.activeEmitters = 0;
    m_stats.totalParticles = 0;
    
    for (auto& [id, emitter] : m_emitters) {
        if (emitter && emitter->isAlive()) {
            emitter->update(deltaTime, m_globalForces);
            m_stats.activeEmitters++;
            m_stats.totalParticles += emitter->getParticleCount();
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.simulationTimeMs = std::chrono::duration<f64, std::milli>(
        endTime - startTime).count();
}

ParticleEmitterHandle ParticleManager::createEmitter(const ParticleSystemData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    u64 id = m_nextEmitterId.fetch_add(1, std::memory_order_relaxed);
    
    auto emitter = std::make_unique<ParticleEmitter>();
    emitter->initialize(data);
    
    m_emitters[id] = std::move(emitter);
    
    return ParticleEmitterHandle{static_cast<u32>(id)};
}

void ParticleManager::destroyEmitter(ParticleEmitterHandle handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_emitters.erase(handle.value);
}

ParticleEmitter* ParticleManager::getEmitter(ParticleEmitterHandle handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_emitters.find(handle.value);
    return (it != m_emitters.end()) ? it->second.get() : nullptr;
}

ParticleSystemHandle ParticleManager::loadSystem(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    ParticleSystemData data;
    if (!loadSystemFromFile(path, data)) {
        return ParticleSystemHandle{};
    }
    
    u64 id = m_nextSystemId.fetch_add(1, std::memory_order_relaxed);
    m_systems[id] = std::move(data);
    
    return ParticleSystemHandle{static_cast<u32>(id)};
}

ParticleSystemHandle ParticleManager::createSystem(const ParticleSystemData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    u64 id = m_nextSystemId.fetch_add(1, std::memory_order_relaxed);
    m_systems[id] = data;
    
    return ParticleSystemHandle{static_cast<u32>(id)};
}

void ParticleManager::unloadSystem(ParticleSystemHandle handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_systems.erase(handle.value);
}

const ParticleSystemData* ParticleManager::getSystem(ParticleSystemHandle handle) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_systems.find(handle.value);
    return (it != m_systems.end()) ? &it->second : nullptr;
}

void ParticleManager::addGlobalForceField(const ForceField& field) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_globalForces.push_back(field);
}

void ParticleManager::removeGlobalForceField(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_globalForces.erase(
        std::remove_if(m_globalForces.begin(), m_globalForces.end(),
                       [&name](const ForceField& f) { return f.name == name; }),
        m_globalForces.end()
    );
}

void ParticleManager::clearGlobalForceFields() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_globalForces.clear();
}

void ParticleManager::resetStats() {
    m_stats = ParticleStats{};
}

bool ParticleManager::loadSystemFromFile(const std::string& path, 
                                          ParticleSystemData& outData) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        NOVA_LOG_WARN(LogCategory::Core, "Failed to open particle system file: {}", path);
        return false;
    }
    
    // Extract name from path
    std::string filename = path.substr(path.find_last_of("/\\") + 1);
    std::string name = filename.substr(0, filename.find_last_of('.'));
    outData.name = name;
    
    // Create default particle system
    outData.main.duration = 5.0f;
    outData.main.startLifetime = MinMaxValue::constant(5.0f);
    outData.main.startSpeed = MinMaxValue::constant(5.0f);
    outData.main.startSize = MinMaxValue::constant(1.0f);
    
    outData.emission.rateOverTime = MinMaxValue::constant(10.0f);
    
    outData.shape.shape = EmissionShape::Cone;
    outData.shape.radius = 1.0f;
    outData.shape.angle = 25.0f;
    
    return true;
}

} // namespace nova::particle
