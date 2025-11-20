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
