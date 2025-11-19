/**
 * Command Interface
 * Base interface for all editor commands (Command Pattern)
 */

export interface Command {
  /**
   * Name of the command for display in history
   */
  readonly name: string;

  /**
   * Execute the command
   */
  execute(): void;

  /**
   * Undo the command
   */
  undo(): void;

  /**
   * Redo the command (optional, defaults to execute())
   */
  redo?(): void;

  /**
   * Check if command can be merged with another command
   * Useful for continuous operations like dragging
   */
  canMergeWith?(other: Command): boolean;

  /**
   * Merge this command with another command
   */
  mergeWith?(other: Command): void;
}

export abstract class BaseCommand implements Command {
  abstract readonly name: string;

  abstract execute(): void;
  abstract undo(): void;

  redo(): void {
    this.execute();
  }

  canMergeWith(_other: Command): boolean {
    return false;
  }

  mergeWith(_other: Command): void {
    // Override in subclasses that support merging
  }
}
