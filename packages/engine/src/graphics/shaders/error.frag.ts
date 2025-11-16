/**
 * Error Fragment Shader
 *
 * Magenta shader used to indicate missing or error shaders
 */

export const errorFragmentShader = `#version 300 es
precision highp float;

// Output
out vec4 fragColor;

void main() {
  // Bright magenta to indicate error
  fragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
`;
