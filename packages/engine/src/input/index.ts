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
 * Input Module
 *
 * Comprehensive input system for keyboard, mouse, touch, and gamepad.
 */

export { InputManager } from './InputManager';
export type { InputManagerOptions } from './InputManager';

export { Keyboard, KeyCode } from './Keyboard';
export { Mouse, MouseButton } from './Mouse';
export { Touch } from './Touch';
export type { TouchPoint } from './Touch';
export { Gamepad, GamepadButton, GamepadAxis } from './Gamepad';
export { InputMapping, InputType } from './InputMapping';
export type { InputBinding } from './InputMapping';
