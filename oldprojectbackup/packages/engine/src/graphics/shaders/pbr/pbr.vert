#version 300 es

// Vertex attributes
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
in vec3 a_tangent;

// Uniforms
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform mat3 u_normalMatrix;

// Outputs to fragment shader
out vec3 v_worldPosition;
out vec3 v_normal;
out vec2 v_uv;
out mat3 v_TBN;

void main() {
  // Transform position to world space
  vec4 worldPosition = u_modelMatrix * vec4(a_position, 1.0);
  v_worldPosition = worldPosition.xyz;
  
  // Transform normal to world space
  v_normal = normalize(u_normalMatrix * a_normal);
  
  // Pass through UV coordinates
  v_uv = a_uv;
  
  // Calculate TBN matrix for normal mapping
  vec3 T = normalize(u_normalMatrix * a_tangent);
  vec3 N = v_normal;
  // Re-orthogonalize T with respect to N
  T = normalize(T - dot(T, N) * N);
  // Calculate bitangent
  vec3 B = cross(N, T);
  v_TBN = mat3(T, B, N);
  
  // Transform to clip space
  gl_Position = u_projectionMatrix * u_viewMatrix * worldPosition;
}
