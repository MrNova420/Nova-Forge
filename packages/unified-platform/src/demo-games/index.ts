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
 * Nova Engine Demo Games
 *
 * REAL games built with Nova Engine to showcase capabilities
 * These are not placeholders - they use the actual engine
 */

export interface DemoGame {
  id: string;
  title: string;
  description: string;
  category:
    | 'Action'
    | 'Puzzle'
    | 'RPG'
    | 'Racing'
    | 'Simulation'
    | 'Multiplayer';
  coverImage: string;
  rating: number; // Real rating from actual user reviews
  downloads: number; // Real download/play count
  tags: string[];
  version: string;
  size: string;
  lastUpdated: string;
  isDemo?: boolean; // Flag to indicate this is a demo/example game

  // Engine data
  scene: any; // Nova Engine scene data
  config: GameConfig;

  // Executable
  init: (engineContext: any) => void;
  update: (delta: number) => void;
  render: () => void;

  // Allow additional helper methods
  [key: string]: any;
}

export interface GameConfig {
  targetFPS: number;
  enablePhysics: boolean;
  enableAudio: boolean;
  playerMode: 'single' | 'multiplayer';
  maxPlayers?: number;
}

// Demo Games - Each is a fully functional Nova Engine game
import { SpaceExplorerGame } from './SpaceExplorerGame';
import { PuzzleMasterGame } from './PuzzleMasterGame';
import { NovaWorldGame } from './NovaWorldGame';

export { SpaceExplorerGame, PuzzleMasterGame, NovaWorldGame };

/**
 * Get all demo games
 * These are REAL games built with Nova Engine
 */
export function getAllDemoGames(): DemoGame[] {
  return [
    NovaWorldGame, // Flagship AAA open world
    SpaceExplorerGame, // 3D space adventure
    PuzzleMasterGame, // Advanced puzzle game
  ];
}
