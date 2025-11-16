/**
 * Unlit Fragment Shader
 *
 * Simple fragment shader that outputs a solid color without lighting
 */

export const unlitFragmentShader = `#version 300 es
precision highp float;

// Inputs from vertex shader
in vec2 v_texCoord;

// Uniforms
uniform vec4 u_baseColor;

// Output
out vec4 fragColor;

void main() {
  fragColor = u_baseColor;
}
`;
