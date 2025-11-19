/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * Unlit Vertex Shader
 *
 * Simple vertex shader without lighting calculations
 */

export const unlitVertexShader = `#version 300 es
precision highp float;

// Attributes
in vec3 a_position;
in vec2 a_texCoord;

// Uniforms
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;

// Outputs to fragment shader
out vec2 v_texCoord;

void main() {
  // Pass through texture coordinates
  v_texCoord = a_texCoord;
  
  // Transform to clip space
  vec4 worldPosition = u_modelMatrix * vec4(a_position, 1.0);
  gl_Position = u_projectionMatrix * u_viewMatrix * worldPosition;
}
`;
