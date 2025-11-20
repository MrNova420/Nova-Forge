#version 300 es
precision highp float;

// Math constants
const float PI = 3.14159265359;
const float EPSILON = 1e-6;

// Inputs from vertex shader
in vec3 v_worldPosition;
in vec3 v_normal;
in vec2 v_uv;
in mat3 v_TBN;

// Material properties
uniform vec3 u_albedo;
uniform float u_metallic;
uniform float u_roughness;
uniform vec3 u_emissive;
uniform float u_emissiveIntensity;
uniform float u_opacity;
uniform int u_alphaMode;
uniform float u_alphaCutoff;
uniform float u_occlusionStrength;
uniform float u_normalScale;
uniform float u_environmentIntensity;

// Texture maps
uniform bool u_hasAlbedoMap;
uniform sampler2D u_albedoMap;
uniform bool u_hasMetallicRoughnessMap;
uniform sampler2D u_metallicRoughnessMap;
uniform bool u_hasNormalMap;
uniform sampler2D u_normalMap;
uniform bool u_hasOcclusionMap;
uniform sampler2D u_occlusionMap;
uniform bool u_hasEmissiveMap;
uniform sampler2D u_emissiveMap;

// Lighting
uniform vec3 u_cameraPosition;
uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform float u_lightIntensity;

// Output
out vec4 fragColor;

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;
  
  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;
  
  return nom / max(denom, EPSILON);
}

// Geometry Function (Smith's Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;
  
  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;
  
  return nom / max(denom, EPSILON);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);
  
  return ggx1 * ggx2;
}

// Fresnel Equation (Schlick's approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// Get normal from normal map
vec3 getNormalFromMap() {
  if (!u_hasNormalMap) {
    return normalize(v_normal);
  }
  
  vec3 tangentNormal = texture(u_normalMap, v_uv).xyz * 2.0 - 1.0;
  tangentNormal.xy *= u_normalScale;
  return normalize(v_TBN * tangentNormal);
}

void main() {
  // Sample textures
  vec3 albedo = u_hasAlbedoMap ? texture(u_albedoMap, v_uv).rgb : u_albedo;
  float metallic = u_metallic;
  float roughness = u_roughness;
  
  if (u_hasMetallicRoughnessMap) {
    vec3 mr = texture(u_metallicRoughnessMap, v_uv).rgb;
    metallic = mr.b;
    roughness = mr.g;
  }
  
  vec3 normal = getNormalFromMap();
  float ao = u_hasOcclusionMap ? texture(u_occlusionMap, v_uv).r : 1.0;
  ao = mix(1.0, ao, u_occlusionStrength);
  
  vec3 emissive = u_hasEmissiveMap ? texture(u_emissiveMap, v_uv).rgb : u_emissive;
  emissive *= u_emissiveIntensity;
  
  // Calculate reflectance at normal incidence
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);
  
  // View and light directions
  vec3 V = normalize(u_cameraPosition - v_worldPosition);
  vec3 L = normalize(-u_lightDirection);
  vec3 H = normalize(V + L);
  
  // Calculate radiance
  vec3 radiance = u_lightColor * u_lightIntensity;
  
  // Cook-Torrance BRDF
  float NDF = DistributionGGX(normal, H, roughness);
  float G = GeometrySmith(normal, V, L, roughness);
  vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
  
  vec3 numerator = NDF * G * F;
  float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0);
  vec3 specular = numerator / max(denominator, EPSILON);
  
  // Energy conservation
  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metallic;
  
  // Final lighting calculation
  float NdotL = max(dot(normal, L), 0.0);
  vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
  
  // Ambient lighting (simple approximation)
  vec3 ambient = vec3(0.03) * albedo * ao * u_environmentIntensity;
  
  vec3 color = ambient + Lo + emissive;
  
  // Tone mapping (simple Reinhard)
  color = color / (color + vec3(1.0));
  
  // Gamma correction
  color = pow(color, vec3(1.0 / 2.2));
  
  // Alpha handling
  float alpha = u_opacity;
  if (u_hasAlbedoMap) {
    alpha *= texture(u_albedoMap, v_uv).a;
  }
  
  // Alpha mode handling
  if (u_alphaMode == 1) { // MASK
    if (alpha < u_alphaCutoff) {
      discard;
    }
    alpha = 1.0;
  }
  
  fragColor = vec4(color, alpha);
}
