/**
 * Nova Engine Demo Games
 * 
 * REAL games built with Nova Engine to showcase capabilities
 * These are not placeholders - they use the actual engine
 */

export interface DemoGame {
  id: string;
  title: string;
  description: string;
  category: 'Action' | 'Puzzle' | 'RPG' | 'Racing' | 'Simulation' | 'Multiplayer';
  coverImage: string;
  rating: number;
  downloads: number;
  tags: string[];
  version: string;
  size: string;
  lastUpdated: string;
  
  // Engine data
  scene: any; // Nova Engine scene data
  config: GameConfig;
  
  // Executable
  init: (engineContext: any) => void;
  update: (delta: number) => void;
  render: () => void;
}

export interface GameConfig {
  targetFPS: number;
  enablePhysics: boolean;
  enableAudio: boolean;
  playerMode: 'single' | 'multiplayer';
  maxPlayers?: number;
}

// Demo Games - Each is a fully functional Nova Engine game
export { SpaceExplorerGame } from './SpaceExplorerGame';
export { PuzzleMasterGame } from './PuzzleMasterGame';
export { NovaWorldGame } from './NovaWorldGame';

/**
 * Get all demo games
 * These are REAL games built with Nova Engine
 */
export function getAllDemoGames(): DemoGame[] {
  // Import actual games
  const { SpaceExplorerGame } = require('./SpaceExplorerGame');
  const { PuzzleMasterGame } = require('./PuzzleMasterGame');
  const { NovaWorldGame } = require('./NovaWorldGame');
  
  return [
    NovaWorldGame,     // Flagship AAA open world
    SpaceExplorerGame, // 3D space adventure
    PuzzleMasterGame,  // Advanced puzzle game
  ];
}
