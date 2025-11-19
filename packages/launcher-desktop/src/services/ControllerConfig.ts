/**
 * ControllerConfig - Game controller configuration and management
 *
 * Features:
 * - Detect and configure multiple controllers
 * - Button/axis mapping customization
 * - Controller profiles per game
 * - Vibration/rumble support
 * - Dead zone configuration
 * - Input visualization
 */

export interface ControllerInfo {
  id: string;
  index: number;
  name: string;
  connected: boolean;
  mapping: 'standard' | 'custom';
  buttons: number;
  axes: number;
  vibrationSupport: boolean;
}

export interface ControllerMapping {
  gameId: string;
  controllerId: string;
  buttonMappings: Map<number, string>; // button index -> action
  axisMappings: Map<number, string>; // axis index -> action
  deadZone: number; // 0-1
  sensitivity: number; // 0-2
  invertY: boolean;
  vibrationEnabled: boolean;
}

export interface ControllerState {
  controllerId: string;
  buttons: boolean[]; // pressed state
  axes: number[]; // -1 to 1
  timestamp: number;
}

const DEFAULT_BUTTON_MAPPINGS: Record<number, string> = {
  0: 'A / X',
  1: 'B / Circle',
  2: 'X / Square',
  3: 'Y / Triangle',
  4: 'L1 / LB',
  5: 'R1 / RB',
  6: 'L2 / LT',
  7: 'R2 / RT',
  8: 'Select / Share',
  9: 'Start / Options',
  10: 'L3',
  11: 'R3',
  12: 'D-Pad Up',
  13: 'D-Pad Down',
  14: 'D-Pad Left',
  15: 'D-Pad Right',
  16: 'Home / PS / Xbox',
};

const DEFAULT_AXIS_MAPPINGS: Record<number, string> = {
  0: 'Left Stick X',
  1: 'Left Stick Y',
  2: 'Right Stick X',
  3: 'Right Stick Y',
};

class ControllerConfigService {
  private controllers: Map<string, ControllerInfo> = new Map();
  private mappings: Map<string, ControllerMapping> = new Map(); // gameId-controllerId -> mapping
  private states: Map<string, ControllerState> = new Map();
  private polling: boolean = false;
  private pollInterval: number | null = null;

  /**
   * Initialize controller service
   */
  async initialize(): Promise<void> {
    // Start polling for controllers
    this.startPolling();

    // Listen for controller connection events
    window.addEventListener('gamepadconnected', (e: GamepadEvent) => {
      this.handleControllerConnected(e.gamepad);
    });

    window.addEventListener('gamepaddisconnected', (e: GamepadEvent) => {
      this.handleControllerDisconnected(e.gamepad);
    });

    // eslint-disable-next-line no-console
    console.log('Controller configuration service initialized');
  }

  /**
   * Start polling for controller state
   */
  startPolling(): void {
    if (this.polling) return;

    this.polling = true;
    this.pollInterval = window.setInterval(() => {
      this.pollControllers();
    }, 16); // ~60Hz

    // eslint-disable-next-line no-console
    console.log('Controller polling started');
  }

  /**
   * Stop polling for controller state
   */
  stopPolling(): void {
    if (!this.polling) return;

    this.polling = false;
    if (this.pollInterval) {
      clearInterval(this.pollInterval);
      this.pollInterval = null;
    }

    // eslint-disable-next-line no-console
    console.log('Controller polling stopped');
  }

  /**
   * Poll all connected controllers
   */
  private pollControllers(): void {
    const gamepads = navigator.getGamepads();

    for (let i = 0; i < gamepads.length; i++) {
      const gamepad = gamepads[i];
      if (gamepad) {
        this.updateControllerState(gamepad);
      }
    }
  }

  /**
   * Update controller state
   */
  private updateControllerState(gamepad: Gamepad): void {
    const id = `controller-${gamepad.index}`;

    const state: ControllerState = {
      controllerId: id,
      buttons: gamepad.buttons.map((b) => b.pressed),
      axes: [...gamepad.axes],
      timestamp: gamepad.timestamp,
    };

    this.states.set(id, state);
  }

  /**
   * Handle controller connected
   */
  private handleControllerConnected(gamepad: Gamepad): void {
    const id = `controller-${gamepad.index}`;

    const info: ControllerInfo = {
      id,
      index: gamepad.index,
      name: gamepad.id,
      connected: true,
      mapping: gamepad.mapping as 'standard' | 'custom',
      buttons: gamepad.buttons.length,
      axes: gamepad.axes.length,
      vibrationSupport: 'vibrationActuator' in gamepad,
    };

    this.controllers.set(id, info);

    // eslint-disable-next-line no-console
    console.log(`Controller connected: ${gamepad.id} (index ${gamepad.index})`);
  }

  /**
   * Handle controller disconnected
   */
  private handleControllerDisconnected(gamepad: Gamepad): void {
    const id = `controller-${gamepad.index}`;
    const controller = this.controllers.get(id);

    if (controller) {
      controller.connected = false;
      this.controllers.delete(id);
      this.states.delete(id);

      // eslint-disable-next-line no-console
      console.log(`Controller disconnected: ${gamepad.id}`);
    }
  }

  /**
   * Get all connected controllers
   */
  getConnectedControllers(): ControllerInfo[] {
    return Array.from(this.controllers.values()).filter((c) => c.connected);
  }

  /**
   * Get controller state
   */
  getControllerState(controllerId: string): ControllerState | null {
    return this.states.get(controllerId) || null;
  }

  /**
   * Create controller mapping for a game
   */
  createMapping(gameId: string, controllerId: string): ControllerMapping {
    const key = `${gameId}-${controllerId}`;

    const mapping: ControllerMapping = {
      gameId,
      controllerId,
      buttonMappings: new Map(
        Object.entries(DEFAULT_BUTTON_MAPPINGS).map(([k, v]) => [Number(k), v])
      ),
      axisMappings: new Map(
        Object.entries(DEFAULT_AXIS_MAPPINGS).map(([k, v]) => [Number(k), v])
      ),
      deadZone: 0.1,
      sensitivity: 1.0,
      invertY: false,
      vibrationEnabled: true,
    };

    this.mappings.set(key, mapping);
    return mapping;
  }

  /**
   * Get controller mapping for a game
   */
  getMapping(gameId: string, controllerId: string): ControllerMapping | null {
    const key = `${gameId}-${controllerId}`;
    return this.mappings.get(key) || null;
  }

  /**
   * Update button mapping
   */
  updateButtonMapping(
    gameId: string,
    controllerId: string,
    buttonIndex: number,
    action: string
  ): void {
    const mapping = this.getMapping(gameId, controllerId);
    if (mapping) {
      mapping.buttonMappings.set(buttonIndex, action);
      // eslint-disable-next-line no-console
      console.log(`Updated button ${buttonIndex} -> ${action} for ${gameId}`);
    }
  }

  /**
   * Update axis mapping
   */
  updateAxisMapping(
    gameId: string,
    controllerId: string,
    axisIndex: number,
    action: string
  ): void {
    const mapping = this.getMapping(gameId, controllerId);
    if (mapping) {
      mapping.axisMappings.set(axisIndex, action);
      // eslint-disable-next-line no-console
      console.log(`Updated axis ${axisIndex} -> ${action} for ${gameId}`);
    }
  }

  /**
   * Update dead zone
   */
  updateDeadZone(gameId: string, controllerId: string, deadZone: number): void {
    const mapping = this.getMapping(gameId, controllerId);
    if (mapping) {
      mapping.deadZone = Math.max(0, Math.min(1, deadZone));
    }
  }

  /**
   * Update sensitivity
   */
  updateSensitivity(
    gameId: string,
    controllerId: string,
    sensitivity: number
  ): void {
    const mapping = this.getMapping(gameId, controllerId);
    if (mapping) {
      mapping.sensitivity = Math.max(0.1, Math.min(2, sensitivity));
    }
  }

  /**
   * Toggle Y-axis inversion
   */
  toggleInvertY(gameId: string, controllerId: string): void {
    const mapping = this.getMapping(gameId, controllerId);
    if (mapping) {
      mapping.invertY = !mapping.invertY;
    }
  }

  /**
   * Toggle vibration
   */
  toggleVibration(gameId: string, controllerId: string): void {
    const mapping = this.getMapping(gameId, controllerId);
    if (mapping) {
      mapping.vibrationEnabled = !mapping.vibrationEnabled;
    }
  }

  /**
   * Test controller vibration
   */
  async testVibration(
    controllerId: string,
    duration: number = 500
  ): Promise<void> {
    const controller = this.controllers.get(controllerId);
    if (!controller || !controller.connected) {
      console.warn('Controller not found or not connected');
      return;
    }

    const gamepad = navigator.getGamepads()[controller.index];
    if (
      gamepad &&
      'vibrationActuator' in gamepad &&
      gamepad.vibrationActuator
    ) {
      try {
        await (gamepad.vibrationActuator as any).playEffect('dual-rumble', {
          startDelay: 0,
          duration: duration,
          weakMagnitude: 0.5,
          strongMagnitude: 1.0,
        });
        // eslint-disable-next-line no-console
        console.log(`Vibration test for ${controllerId}`);
      } catch (error) {
        console.error('Vibration not supported:', error);
      }
    } else {
      console.warn('Vibration not supported on this controller');
    }
  }

  /**
   * Apply dead zone to axis value
   */
  applyDeadZone(value: number, deadZone: number): number {
    if (Math.abs(value) < deadZone) {
      return 0;
    }

    // Scale to maintain full range after dead zone
    const sign = value > 0 ? 1 : -1;
    const scaled = (Math.abs(value) - deadZone) / (1 - deadZone);
    return sign * scaled;
  }

  /**
   * Get processed axis value with dead zone and sensitivity
   */
  getProcessedAxis(
    controllerId: string,
    axisIndex: number,
    mapping?: ControllerMapping
  ): number {
    const state = this.getControllerState(controllerId);
    if (!state || axisIndex >= state.axes.length) {
      return 0;
    }

    let value = state.axes[axisIndex];

    // Apply dead zone
    const deadZone = mapping?.deadZone || 0.1;
    value = this.applyDeadZone(value, deadZone);

    // Apply sensitivity
    const sensitivity = mapping?.sensitivity || 1.0;
    value *= sensitivity;

    // Apply Y-axis inversion if needed
    if (mapping?.invertY && (axisIndex === 1 || axisIndex === 3)) {
      value *= -1;
    }

    return Math.max(-1, Math.min(1, value));
  }

  /**
   * Reset mapping to defaults
   */
  resetMapping(gameId: string, controllerId: string): void {
    const key = `${gameId}-${controllerId}`;
    this.mappings.delete(key);
    this.createMapping(gameId, controllerId);
    // eslint-disable-next-line no-console
    console.log(`Reset mapping for ${gameId}-${controllerId}`);
  }

  /**
   * Export mapping configuration
   */
  exportMapping(gameId: string, controllerId: string): string {
    const mapping = this.getMapping(gameId, controllerId);
    if (!mapping) {
      throw new Error('Mapping not found');
    }

    const exportData = {
      gameId: mapping.gameId,
      controllerId: mapping.controllerId,
      buttonMappings: Array.from(mapping.buttonMappings.entries()),
      axisMappings: Array.from(mapping.axisMappings.entries()),
      deadZone: mapping.deadZone,
      sensitivity: mapping.sensitivity,
      invertY: mapping.invertY,
      vibrationEnabled: mapping.vibrationEnabled,
    };

    return JSON.stringify(exportData, null, 2);
  }

  /**
   * Import mapping configuration
   */
  importMapping(configJson: string): void {
    try {
      const data = JSON.parse(configJson);
      const key = `${data.gameId}-${data.controllerId}`;

      const mapping: ControllerMapping = {
        gameId: data.gameId,
        controllerId: data.controllerId,
        buttonMappings: new Map(data.buttonMappings),
        axisMappings: new Map(data.axisMappings),
        deadZone: data.deadZone,
        sensitivity: data.sensitivity,
        invertY: data.invertY,
        vibrationEnabled: data.vibrationEnabled,
      };

      this.mappings.set(key, mapping);
      // eslint-disable-next-line no-console
      console.log(`Imported mapping for ${key}`);
    } catch (error) {
      console.error('Failed to import mapping:', error);
      throw error;
    }
  }

  /**
   * Cleanup
   */
  cleanup(): void {
    this.stopPolling();
    this.controllers.clear();
    this.states.clear();
    // eslint-disable-next-line no-console
    console.log('Controller service cleaned up');
  }
}

export const controllerConfig = new ControllerConfigService();
