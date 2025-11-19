/**
 * CommandManager
 * Manages command history and implements undo/redo functionality
 */

import { Command } from './Command';

export class CommandManager {
  private history: Command[] = [];
  private currentIndex: number = -1;
  private maxHistorySize: number = 999999; // Essentially unlimited undo
  private mergingEnabled: boolean = true;

  /**
   * Execute a command and add it to history
   */
  execute(command: Command): void {
    // Try to merge with previous command
    if (this.mergingEnabled && this.currentIndex >= 0) {
      const lastCommand = this.history[this.currentIndex];
      if (
        lastCommand &&
        lastCommand.canMergeWith &&
        lastCommand.canMergeWith(command)
      ) {
        lastCommand.mergeWith && lastCommand.mergeWith(command);
        command.execute();
        return;
      }
    }

    // Remove any commands after current index (if we undid some commands)
    this.history = this.history.slice(0, this.currentIndex + 1);

    // Execute the command
    command.execute();

    // Add to history
    this.history.push(command);
    this.currentIndex++;

    // Limit history size
    if (this.history.length > this.maxHistorySize) {
      this.history.shift();
      this.currentIndex--;
    }
  }

  /**
   * Undo the last command
   */
  undo(): boolean {
    if (!this.canUndo()) {
      return false;
    }

    const command = this.history[this.currentIndex];
    if (!command) {
      return false;
    }

    command.undo();
    this.currentIndex--;

    // eslint-disable-next-line no-console
    console.log(`Undid: ${command.name}`);
    return true;
  }

  /**
   * Redo the last undone command
   */
  redo(): boolean {
    if (!this.canRedo()) {
      return false;
    }

    this.currentIndex++;
    const command = this.history[this.currentIndex];

    if (!command) {
      return false;
    }

    if (command.redo) {
      command.redo();
    } else {
      command.execute();
    }

    // eslint-disable-next-line no-console
    console.log(`Redid: ${command.name}`);
    return true;
  }

  /**
   * Check if undo is available
   */
  canUndo(): boolean {
    return this.currentIndex >= 0;
  }

  /**
   * Check if redo is available
   */
  canRedo(): boolean {
    return this.currentIndex < this.history.length - 1;
  }

  /**
   * Get command history
   */
  getHistory(): ReadonlyArray<Command> {
    return this.history;
  }

  /**
   * Get current index in history
   */
  getCurrentIndex(): number {
    return this.currentIndex;
  }

  /**
   * Clear all history
   */
  clear(): void {
    this.history = [];
    this.currentIndex = -1;
  }

  /**
   * Set maximum history size
   */
  setMaxHistorySize(size: number): void {
    this.maxHistorySize = Math.max(1, size);

    // Trim history if needed
    if (this.history.length > this.maxHistorySize) {
      const removeCount = this.history.length - this.maxHistorySize;
      this.history.splice(0, removeCount);
      this.currentIndex = Math.max(-1, this.currentIndex - removeCount);
    }
  }

  /**
   * Enable or disable command merging
   */
  setMergingEnabled(enabled: boolean): void {
    this.mergingEnabled = enabled;
  }
}
