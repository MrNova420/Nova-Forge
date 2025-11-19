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
