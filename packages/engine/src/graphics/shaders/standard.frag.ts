/**
 * Standard Fragment Shader
 *
 * Basic fragment shader with simple diffuse lighting
 */

export const standardFragmentShader = `#version 300 es
precision highp float;

// Inputs from vertex shader
in vec3 v_worldPosition;
in vec3 v_normal;
in vec2 v_texCoord;

// Uniforms
uniform vec3 u_cameraPosition;
uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;
uniform vec4 u_baseColor;

// Output
out vec4 fragColor;

void main() {
  // Normalize the normal
  vec3 normal = normalize(v_normal);
  
  // Simple diffuse lighting
  float diffuse = max(dot(normal, -u_lightDirection), 0.0);
  
  // Combine lighting
  vec3 ambient = u_ambientColor;
  vec3 lighting = ambient + u_lightColor * diffuse;
  
  // Apply to base color
  vec3 color = u_baseColor.rgb * lighting;
  
  // Output final color
  fragColor = vec4(color, u_baseColor.a);
}
`;
