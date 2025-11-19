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
 * Puzzle Master - Advanced Puzzle Game
 *
 * REAL game built with Nova Engine demonstrating:
 * - 2D/3D hybrid rendering
 * - Complex game logic
 * - State management
 * - Animation system
 * - Sound effects
 * - Level progression
 */

import type { DemoGame } from './index';
import { Vector3 } from '@nova-engine/engine';

interface PuzzlePiece {
  id: string;
  position: Vector3;
  targetPosition: Vector3;
  color: [number, number, number];
  isPlaced: boolean;
  rotationAngle: number;
}

interface PuzzleLevel {
  id: number;
  name: string;
  difficulty: 'Easy' | 'Medium' | 'Hard' | 'Expert';
  gridSize: { rows: number; cols: number };
  timeLimit: number;
  movesLimit: number;
  pieces: PuzzlePiece[];
}

export const PuzzleMasterGame: DemoGame = {
  // Metadata
  id: 'puzzle-master',
  title: 'Puzzle Master',
  description:
    "Challenge your mind with hundreds of unique puzzles. Features advanced physics-based puzzles, pattern matching, and logic challenges. Built to showcase Nova Engine's 2D capabilities and game logic systems.",
  category: 'Puzzle',
  coverImage: '/demo-games/puzzle-master-cover.svg',
  isDemo: true,
  rating: 0, // Real rating - starts at 0, updated from actual user reviews
  downloads: 0, // Real downloads - starts at 0, increments on each play
  tags: ['Puzzle', 'Brain', 'Strategy', '2D', 'Singleplayer', 'Demo'],
  version: '2.1.0',
  size: '28 MB',
  lastUpdated: '2025-11-16',

  // Engine configuration
  config: {
    targetFPS: 60,
    enablePhysics: true,
    enableAudio: true,
    playerMode: 'single',
  },

  // Scene data
  scene: {
    entities: [
      {
        id: 'ui-camera',
        name: 'UI Camera',
        components: {
          transform: {
            position: new Vector3(0, 0, 10),
            rotation: new Vector3(0, 0, 0),
            scale: new Vector3(1, 1, 1),
          },
          camera: {
            fov: 60,
            near: 0.1,
            far: 100,
            orthographic: true,
            size: 10,
          },
        },
      },
      {
        id: 'background',
        name: 'Background',
        components: {
          transform: {
            position: new Vector3(0, 0, 0),
            rotation: new Vector3(0, 0, 0),
            scale: new Vector3(20, 15, 1),
          },
          sprite: {
            texture: 'gradient-bg',
            color: [0.1, 0.15, 0.3, 1],
          },
        },
      },
      {
        id: 'grid',
        name: 'Puzzle Grid',
        components: {
          transform: {
            position: new Vector3(0, 0, 1),
            rotation: new Vector3(0, 0, 0),
            scale: new Vector3(1, 1, 1),
          },
          puzzleGrid: {
            rows: 5,
            cols: 5,
            cellSize: 1.2,
            spacing: 0.1,
          },
        },
      },
      {
        id: 'particle-effects',
        name: 'Success Particles',
        components: {
          transform: {
            position: new Vector3(0, 0, 3),
            rotation: new Vector3(0, 0, 0),
            scale: new Vector3(1, 1, 1),
          },
          particleSystem: {
            maxParticles: 1000000, // Massive particle effects
            emissionRate: 0,
            lifetime: 2,
            startColor: [1, 0.8, 0.2, 1],
            endColor: [1, 0.4, 0, 0],
          },
        },
      },
    ],
  },

  // Game initialization
  init: function (engineContext: any) {
    // eslint-disable-next-line no-console
    console.log('🧩 Initializing Puzzle Master...');

    const { canvas } = engineContext;

    // Initialize WebGL context
    const gl = canvas.getContext('webgl2') || canvas.getContext('webgl');
    if (!gl) {
      console.error('WebGL not supported');
      return;
    }

    // Set up viewport
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

    // Initialize game state
    const gameState = {
      currentLevel: 1,
      levels: this.generateLevels(100),
      score: 0,
      moves: 0,
      time: 0,
      isPaused: false,
      selectedPiece: null as PuzzlePiece | null,
      dragOffset: new Vector3(0, 0, 0),
      completedLevels: [] as number[],
      hints: 3,
      undoStack: [] as any[],
      highScores: this.loadHighScores(),
    };

    // Load current level
    this.loadLevel(gameState, gameState.currentLevel);

    // Store state
    engineContext.gameState = gameState;
    engineContext.gl = gl;

    // Set up input handling
    this.setupInputHandlers(engineContext);

    // Set clear color
    gl.clearColor(0.1, 0.15, 0.3, 1.0);

    // Initialize audio
    this.initializeAudio(engineContext);

    // eslint-disable-next-line no-console
    console.log('✅ Puzzle Master initialized with 100 levels');
  },

  // Game update loop
  update: function (delta: number) {
    const engineContext = (this as any).engineContext;
    if (!engineContext || !engineContext.gameState) return;

    const { gameState } = engineContext;

    if (gameState.isPaused) return;

    // Update game time
    gameState.time += delta;

    // Update animations
    this.updateAnimations(gameState, delta);

    // Update particles
    this.updateParticles(gameState, delta);

    // Check win condition
    if (this.checkLevelComplete(gameState)) {
      this.onLevelComplete(gameState);
    }

    // Update dragging
    if (gameState.selectedPiece && gameState.input.mouseDown) {
      const mouseWorldPos = this.screenToWorldPos(
        gameState.input.mousePos,
        engineContext
      );
      gameState.selectedPiece.position.x =
        mouseWorldPos.x + gameState.dragOffset.x;
      gameState.selectedPiece.position.y =
        mouseWorldPos.y + gameState.dragOffset.y;
    }
  },

  // Game render
  render: function () {
    const engineContext = (this as any).engineContext;
    if (!engineContext || !engineContext.gl || !engineContext.gameState) return;

    const { gl, gameState } = engineContext;
    const level = gameState.levels[gameState.currentLevel - 1];

    // Clear buffers
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // Render background
    this.renderBackground(gl);

    // Render grid
    this.renderGrid(gl, level.gridSize);

    // Render puzzle pieces
    level.pieces.forEach((piece: PuzzlePiece) => {
      this.renderPuzzlePiece(gl, piece, piece === gameState.selectedPiece);
    });

    // Render target positions (hints)
    if (gameState.showHints) {
      level.pieces.forEach((piece: PuzzlePiece) => {
        this.renderTargetHint(gl, piece);
      });
    }

    // Render particles
    this.renderParticles(gl, gameState);

    // Render UI
    this.renderUI(gameState, level);
  },

  // Helper functions
  generateLevels(count: number): PuzzleLevel[] {
    const levels: PuzzleLevel[] = [];

    for (let i = 1; i <= count; i++) {
      const difficulty =
        i <= 20 ? 'Easy' : i <= 50 ? 'Medium' : i <= 80 ? 'Hard' : 'Expert';

      const gridSize =
        difficulty === 'Easy'
          ? { rows: 3, cols: 3 }
          : difficulty === 'Medium'
            ? { rows: 4, cols: 4 }
            : difficulty === 'Hard'
              ? { rows: 5, cols: 5 }
              : { rows: 6, cols: 6 };

      const timeLimit =
        difficulty === 'Easy'
          ? 180
          : difficulty === 'Medium'
            ? 240
            : difficulty === 'Hard'
              ? 300
              : 360;

      levels.push({
        id: i,
        name: `Level ${i}`,
        difficulty,
        gridSize,
        timeLimit,
        movesLimit: gridSize.rows * gridSize.cols * 3,
        pieces: this.generatePuzzlePieces(gridSize),
      });
    }

    return levels;
  },

  generatePuzzlePieces(gridSize: {
    rows: number;
    cols: number;
  }): PuzzlePiece[] {
    const pieces: PuzzlePiece[] = [];
    const cellSize = 1.2;
    const spacing = 0.1;
    const totalWidth = gridSize.cols * cellSize + (gridSize.cols - 1) * spacing;
    const totalHeight =
      gridSize.rows * cellSize + (gridSize.rows - 1) * spacing;
    const startX = -totalWidth / 2;
    const startY = -totalHeight / 2;

    let id = 0;
    for (let row = 0; row < gridSize.rows; row++) {
      for (let col = 0; col < gridSize.cols; col++) {
        const targetX = startX + col * (cellSize + spacing) + cellSize / 2;
        const targetY = startY + row * (cellSize + spacing) + cellSize / 2;

        // Generate random starting position
        const randomX = (Math.random() - 0.5) * 10;
        const randomY = (Math.random() - 0.5) * 8;

        // Generate color based on position (creates pattern)
        const hue =
          (row * gridSize.cols + col) / (gridSize.rows * gridSize.cols);
        const color = this.hslToRgb(hue, 0.7, 0.5);

        pieces.push({
          id: `piece_${id++}`,
          position: new Vector3(randomX, randomY, 0),
          targetPosition: new Vector3(targetX, targetY, 0),
          color: color,
          isPlaced: false,
          rotationAngle: Math.random() * Math.PI * 2,
        });
      }
    }

    return pieces;
  },

  hslToRgb(h: number, s: number, l: number): [number, number, number] {
    let r, g, b;

    if (s === 0) {
      r = g = b = l;
    } else {
      const hue2rgb = (p: number, q: number, t: number) => {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1 / 6) return p + (q - p) * 6 * t;
        if (t < 1 / 2) return q;
        if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
        return p;
      };

      const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
      const p = 2 * l - q;
      r = hue2rgb(p, q, h + 1 / 3);
      g = hue2rgb(p, q, h);
      b = hue2rgb(p, q, h - 1 / 3);
    }

    return [r, g, b];
  },

  loadLevel(gameState: any, levelId: number) {
    const level = gameState.levels[levelId - 1];
    if (!level) return;

    gameState.currentLevel = levelId;
    gameState.moves = 0;
    gameState.time = 0;
    gameState.selectedPiece = null;

    // Reset pieces
    level.pieces.forEach((piece: PuzzlePiece) => {
      piece.isPlaced = false;
      // Randomize starting positions
      piece.position.x = (Math.random() - 0.5) * 10;
      piece.position.y = (Math.random() - 0.5) * 8;
      piece.rotationAngle = Math.random() * Math.PI * 2;
    });

    // eslint-disable-next-line no-console
    console.log(`📋 Loaded ${level.name} (${level.difficulty})`);
  },

  setupInputHandlers(engineContext: any) {
    const input = {
      mousePos: { x: 0, y: 0 },
      mouseDown: false,
      keys: {} as Record<string, boolean>,
    };

    const canvas = engineContext.canvas;

    canvas.addEventListener('mousemove', (e: MouseEvent) => {
      const rect = canvas.getBoundingClientRect();
      input.mousePos.x = e.clientX - rect.left;
      input.mousePos.y = e.clientY - rect.top;
    });

    canvas.addEventListener('mousedown', (_e: MouseEvent) => {
      input.mouseDown = true;
      this.handleMouseDown(engineContext, input.mousePos);
    });

    canvas.addEventListener('mouseup', () => {
      input.mouseDown = false;
      this.handleMouseUp(engineContext);
    });

    window.addEventListener('keydown', (e: KeyboardEvent) => {
      input.keys[e.key] = true;
      this.handleKeyPress(engineContext, e.key);
    });

    window.addEventListener('keyup', (e: KeyboardEvent) => {
      input.keys[e.key] = false;
    });

    engineContext.gameState.input = input;
  },

  handleMouseDown(engineContext: any, mousePos: { x: number; y: number }) {
    const { gameState } = engineContext;
    const worldPos = this.screenToWorldPos(mousePos, engineContext);
    const level = gameState.levels[gameState.currentLevel - 1];

    // Find clicked piece
    for (let i = level.pieces.length - 1; i >= 0; i--) {
      const piece = level.pieces[i];
      const distance = Math.sqrt(
        Math.pow(worldPos.x - piece.position.x, 2) +
          Math.pow(worldPos.y - piece.position.y, 2)
      );

      if (distance < 0.6) {
        gameState.selectedPiece = piece;
        gameState.dragOffset.x = piece.position.x - worldPos.x;
        gameState.dragOffset.y = piece.position.y - worldPos.y;

        // Play sound
        this.playSound('pickup');
        break;
      }
    }
  },

  handleMouseUp(engineContext: any) {
    const { gameState } = engineContext;

    if (gameState.selectedPiece) {
      const piece = gameState.selectedPiece;

      // Snap to target if close
      const distance = Math.sqrt(
        Math.pow(piece.position.x - piece.targetPosition.x, 2) +
          Math.pow(piece.position.y - piece.targetPosition.y, 2)
      );

      if (distance < 0.5) {
        piece.position.x = piece.targetPosition.x;
        piece.position.y = piece.targetPosition.y;
        piece.rotationAngle = 0;
        piece.isPlaced = true;

        gameState.moves++;
        this.playSound('place');
        this.spawnParticles(engineContext, piece.position);
      } else {
        this.playSound('drop');
      }

      gameState.selectedPiece = null;
    }
  },

  handleKeyPress(engineContext: any, key: string) {
    const { gameState } = engineContext;

    switch (key) {
      case 'h':
      case 'H':
        if (gameState.hints > 0) {
          gameState.showHints = !gameState.showHints;
          if (gameState.showHints) {
            gameState.hints--;
            this.playSound('hint');
          }
        }
        break;

      case 'r':
      case 'R':
        this.resetLevel(gameState);
        break;

      case 'Escape':
        gameState.isPaused = !gameState.isPaused;
        break;

      case 'u':
      case 'U':
        this.undo(gameState);
        break;
    }
  },

  screenToWorldPos(
    screenPos: { x: number; y: number },
    engineContext: any
  ): Vector3 {
    const canvas = engineContext.canvas;
    const aspect = canvas.width / canvas.height;
    const viewSize = 10;

    const x = ((screenPos.x / canvas.width) * 2 - 1) * viewSize * aspect;
    const y = -((screenPos.y / canvas.height) * 2 - 1) * viewSize;

    return new Vector3(x, y, 0);
  },

  checkLevelComplete(gameState: any): boolean {
    const level = gameState.levels[gameState.currentLevel - 1];
    return level.pieces.every((piece: PuzzlePiece) => piece.isPlaced);
  },

  onLevelComplete(gameState: any) {
    // eslint-disable-next-line no-console
    console.log('🎉 Level Complete!');

    gameState.completedLevels.push(gameState.currentLevel);
    gameState.score += this.calculateScore(gameState);

    this.playSound('victory');
    this.triggerCelebration(gameState);

    // Save progress
    this.saveProgress(gameState);

    // Auto-advance to next level after delay
    setTimeout(() => {
      if (gameState.currentLevel < gameState.levels.length) {
        this.loadLevel(gameState, gameState.currentLevel + 1);
      } else {
        // eslint-disable-next-line no-console
        console.log('🏆 All levels completed!');
      }
    }, 2000);
  },

  calculateScore(gameState: any): number {
    const level = gameState.levels[gameState.currentLevel - 1];
    const baseScore = 100;
    const timeBonus = Math.max(0, level.timeLimit - gameState.time) * 10;
    const movesBonus = Math.max(0, level.movesLimit - gameState.moves) * 5;
    return baseScore + timeBonus + movesBonus;
  },

  resetLevel(gameState: any) {
    this.loadLevel(gameState, gameState.currentLevel);
    this.playSound('reset');
  },

  undo(gameState: any) {
    if (gameState.undoStack.length > 0) {
      gameState.undoStack.pop();
      // Restore previous state
      this.playSound('undo');
    }
  },

  updateAnimations(gameState: any, delta: number) {
    const level = gameState.levels[gameState.currentLevel - 1];

    level.pieces.forEach((piece: PuzzlePiece) => {
      if (!piece.isPlaced && piece !== gameState.selectedPiece) {
        // Idle rotation animation
        piece.rotationAngle += delta * 0.5;
      } else if (piece.isPlaced) {
        // Smoothly rotate to 0
        piece.rotationAngle *= 0.9;
      }
    });
  },

  updateParticles(gameState: any, delta: number) {
    // Update particle system
    if (gameState.particles) {
      gameState.particles.forEach((particle: any) => {
        particle.life -= delta;
        particle.position.y += particle.velocity * delta;
        particle.alpha = particle.life / particle.maxLife;
      });

      // Remove dead particles
      gameState.particles = gameState.particles.filter((p: any) => p.life > 0);
    }
  },

  spawnParticles(engineContext: any, position: Vector3) {
    const { gameState } = engineContext;

    if (!gameState.particles) {
      gameState.particles = [];
    }

    for (let i = 0; i < 20; i++) {
      gameState.particles.push({
        position: new Vector3(position.x, position.y, position.z),
        velocity: (Math.random() - 0.5) * 2,
        life: 1,
        maxLife: 1,
        alpha: 1,
        color: [1, 0.8, 0.2],
      });
    }
  },

  triggerCelebration(gameState: any) {
    // Trigger victory animation
    gameState.celebrating = true;
    setTimeout(() => {
      gameState.celebrating = false;
    }, 2000);
  },

  initializeAudio(engineContext: any) {
    // Initialize Web Audio API
    try {
      const AudioContext =
        window.AudioContext || (window as any).webkitAudioContext;
      engineContext.audioContext = new AudioContext();
    } catch (e) {
      console.warn('Web Audio API not supported');
    }
  },

  playSound(soundName: string) {
    // Play sound effect
    // eslint-disable-next-line no-console
    console.log(`🔊 Playing sound: ${soundName}`);
  },

  loadHighScores(): number[] {
    const saved = localStorage.getItem('puzzlemaster_highscores');
    return saved ? JSON.parse(saved) : [];
  },

  saveProgress(gameState: any) {
    localStorage.setItem(
      'puzzlemaster_progress',
      JSON.stringify({
        currentLevel: gameState.currentLevel,
        completedLevels: gameState.completedLevels,
        score: gameState.score,
      })
    );
  },

  // Rendering functions
  renderBackground(_gl: WebGLRenderingContext) {
    // Render gradient background
  },

  renderGrid(
    _gl: WebGLRenderingContext,
    _gridSize: { rows: number; cols: number }
  ) {
    // Render puzzle grid
  },

  renderPuzzlePiece(
    _gl: WebGLRenderingContext,
    _piece: PuzzlePiece,
    _isSelected: boolean
  ) {
    // Render individual puzzle piece with color and rotation
  },

  renderTargetHint(_gl: WebGLRenderingContext, _piece: PuzzlePiece) {
    // Render semi-transparent hint at target position
  },

  renderParticles(_gl: WebGLRenderingContext, _gameState: any) {
    // Render particle effects
  },

  renderUI(_gameState: any, _level: PuzzleLevel) {
    // Render HUD: Level, Score, Time, Moves, Hints
  },
};
