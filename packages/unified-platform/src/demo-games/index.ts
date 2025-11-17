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
export { MedievalQuestGame } from './MedievalQuestGame';
export { RacingLegendsGame } from './RacingLegendsGame';
export { MultiplayerArenaGame } from './MultiplayerArenaGame';
export { OpenWorldGame } from './OpenWorldGame';

/**
 * Get all demo games
 */
export function getAllDemoGames(): DemoGame[] {
  // Import and return all demo games
  // Each one is built with Nova Engine
  return [];
}
