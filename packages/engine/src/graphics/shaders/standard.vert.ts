/**
 * Standard Vertex Shader
 *
 * Basic vertex shader with model-view-projection transformation
 */

export const standardVertexShader = `#version 300 es
precision highp float;

// Attributes
in vec3 a_position;
in vec3 a_normal;
in vec2 a_texCoord;

// Uniforms
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_normalMatrix;

// Outputs to fragment shader
out vec3 v_worldPosition;
out vec3 v_normal;
out vec2 v_texCoord;

void main() {
  // Transform position to world space
  vec4 worldPosition = u_modelMatrix * vec4(a_position, 1.0);
  v_worldPosition = worldPosition.xyz;
  
  // Transform normal to world space
  v_normal = mat3(u_normalMatrix) * a_normal;
  
  // Pass through texture coordinates
  v_texCoord = a_texCoord;
  
  // Transform to clip space
  gl_Position = u_projectionMatrix * u_viewMatrix * worldPosition;
}
`;
