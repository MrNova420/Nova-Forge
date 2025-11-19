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
 * @fileoverview Server build pipeline for multiplayer game servers
 */

import * as fs from 'fs';
import * as path from 'path';
import {
  BuildConfig,
  BuildResult,
  BuildArtifact,
  BuildStats,
  BuildTarget,
  DEFAULT_BUILD_CONFIG,
} from './BuildConfig';

/**
 * Server platforms
 */
export enum ServerPlatform {
  Linux = 'linux',
  Windows = 'windows',
  Docker = 'docker',
}

/**
 * Server build configuration
 */
export interface ServerBuildConfig extends BuildConfig {
  /** Target server platform */
  platform: ServerPlatform;
  /** Server name */
  serverName: string;
  /** Server version */
  serverVersion: string;
  /** Port number */
  port: number;
  /** Maximum players */
  maxPlayers: number;
  /** Enable clustering */
  clustering: boolean;
  /** Include database */
  includeDatabase: boolean;
}

/**
 * Server builder for dedicated game servers
 * Creates optimized server builds for multiplayer games
 */
export class ServerBuilder {
  /** Build configuration */
  private config: ServerBuildConfig;

  constructor(config?: Partial<ServerBuildConfig>) {
    this.config = {
      ...DEFAULT_BUILD_CONFIG,
      target: BuildTarget.Server,
      platform: ServerPlatform.Linux,
      serverName: 'Nova Engine Server',
      serverVersion: '1.0.0',
      port: 8080,
      maxPlayers: 1000000, // Unlimited players
      clustering: false,
      includeDatabase: false,
      ...config,
    };
  }

  /**
   * Build server application
   */
  public async build(projectPath: string): Promise<BuildResult> {
    const startTime = Date.now();
    const errors: string[] = [];
    const warnings: string[] = [];
    const artifacts: BuildArtifact[] = [];

    try {
      // Validate project
      if (!this.validateProject(projectPath)) {
        errors.push('Invalid project structure');
        return {
          success: false,
          errors,
        };
      }

      // Create output directory
      const outDir = path.join(projectPath, this.config.outDir);
      this.ensureDirectory(outDir);

      // Build steps
      // eslint-disable-next-line no-console
      console.log('Starting server build...');

      // 1. Bundle server code
      // eslint-disable-next-line no-console
      console.log('1/7 Bundling server code...');
      const serverArtifacts = await this.bundleServerCode(projectPath, outDir);
      artifacts.push(...serverArtifacts);

      // 2. Bundle game logic
      // eslint-disable-next-line no-console
      console.log('2/7 Bundling game logic...');
      const gameArtifacts = await this.bundleGameLogic(projectPath, outDir);
      artifacts.push(...gameArtifacts);

      // 3. Copy configuration files
      // eslint-disable-next-line no-console
      console.log('3/7 Copying configuration...');
      const configArtifacts = await this.copyConfiguration(projectPath, outDir);
      artifacts.push(...configArtifacts);

      // 4. Setup networking
      // eslint-disable-next-line no-console
      console.log('4/7 Setting up networking...');
      const networkArtifacts = await this.setupNetworking(outDir);
      artifacts.push(...networkArtifacts);

      // 5. Setup database (if enabled)
      if (this.config.includeDatabase) {
        // eslint-disable-next-line no-console
        console.log('5/7 Setting up database...');
        const dbArtifacts = await this.setupDatabase(outDir);
        artifacts.push(...dbArtifacts);
      } else {
        // eslint-disable-next-line no-console
        console.log('5/7 Skipping database setup...');
      }

      // 6. Generate deployment files
      // eslint-disable-next-line no-console
      console.log('6/7 Generating deployment files...');
      const deployArtifacts = await this.generateDeploymentFiles(outDir);
      artifacts.push(...deployArtifacts);

      // 7. Generate metadata
      // eslint-disable-next-line no-console
      console.log('7/7 Generating metadata...');
      const metadataArtifact = await this.generateMetadata(
        outDir,
        artifacts,
        startTime
      );
      artifacts.push(metadataArtifact);

      // eslint-disable-next-line no-console
      console.log('Server build completed successfully!');

      // Calculate stats
      const stats: BuildStats = {
        buildTime: Date.now() - startTime,
        bundleSize: artifacts.reduce((sum, a) => sum + a.size, 0),
        assetCount: artifacts.length,
      };

      return {
        success: true,
        artifacts,
        stats,
      };
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : 'Unknown error';
      errors.push(errorMsg);
      console.error('Server build failed:', errorMsg);

      return {
        success: false,
        errors,
        warnings,
      };
    }
  }

  /**
   * Validate project structure
   */
  private validateProject(projectPath: string): boolean {
    try {
      // Check if project path exists
      if (!fs.existsSync(projectPath)) {
        return false;
      }

      // Check for required files
      const packageJsonPath = path.join(projectPath, 'package.json');
      if (!fs.existsSync(packageJsonPath)) {
        return false;
      }

      return true;
    } catch {
      return false;
    }
  }

  /**
   * Ensure directory exists
   */
  private ensureDirectory(dir: string): void {
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir, { recursive: true });
    }
  }

  /**
   * Bundle server code
   */
  private async bundleServerCode(
    _projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];

    // Create server entry point
    const serverPath = path.join(outDir, 'server.js');
    const serverContent = `
/**
 * Nova Engine Dedicated Server
 * Version: ${this.config.serverVersion}
 */

const http = require('http');
const WebSocket = require('ws');

const PORT = ${this.config.port};
const MAX_PLAYERS = ${this.config.maxPlayers};

// Server state
const players = new Map();
const rooms = new Map();

// HTTP server
const server = http.createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'application/json' });
  res.end(JSON.stringify({
    name: '${this.config.serverName}',
    version: '${this.config.serverVersion}',
    players: players.size,
    maxPlayers: MAX_PLAYERS,
    rooms: rooms.size,
    uptime: process.uptime(),
  }));
});

// WebSocket server
const wss = new WebSocket.Server({ server });

wss.on('connection', (ws, req) => {
  const playerId = generatePlayerId();
  // eslint-disable-next-line no-console
  console.log(\`Player \${playerId} connected from \${req.socket.remoteAddress}\`);
  
  players.set(playerId, {
    id: playerId,
    ws,
    connected: true,
    connectedAt: Date.now(),
  });
  
  // Send welcome message
  ws.send(JSON.stringify({
    type: 'welcome',
    playerId,
    serverInfo: {
      name: '${this.config.serverName}',
      version: '${this.config.serverVersion}',
    },
  }));
  
  // Handle messages
  ws.on('message', (data) => {
    try {
      const message = JSON.parse(data.toString());
      handleMessage(playerId, message);
    } catch (error) {
      console.error('Failed to parse message:', error);
    }
  });
  
  // Handle disconnection
  ws.on('close', () => {
    // eslint-disable-next-line no-console
    console.log(\`Player \${playerId} disconnected\`);
    handleDisconnect(playerId);
  });
  
  ws.on('error', (error) => {
    console.error(\`WebSocket error for player \${playerId}:\`, error);
  });
});

// Message handler
function handleMessage(playerId, message) {
  const player = players.get(playerId);
  if (!player) return;
  
  switch (message.type) {
    case 'createRoom':
      handleCreateRoom(playerId, message.data);
      break;
    case 'joinRoom':
      handleJoinRoom(playerId, message.data);
      break;
    case 'leaveRoom':
      handleLeaveRoom(playerId);
      break;
    case 'state':
      broadcastToRoom(playerId, message);
      break;
    case 'input':
      broadcastToRoom(playerId, message);
      break;
    case 'chat':
      broadcastToRoom(playerId, message);
      break;
    case 'ping':
      player.ws.send(JSON.stringify({ type: 'pong', timestamp: Date.now() }));
      break;
    default:
      console.warn(\`Unknown message type: \${message.type}\`);
  }
}

// Room management
function handleCreateRoom(playerId, data) {
  const roomId = generateRoomId();
  const room = {
    id: roomId,
    hostId: playerId,
    players: [playerId],
    maxPlayers: data.maxPlayers || 1000000, // Default to unlimited
    gameId: data.gameId,
    createdAt: Date.now(),
  };
  
  rooms.set(roomId, room);
  
  const player = players.get(playerId);
  player.roomId = roomId;
  
  player.ws.send(JSON.stringify({
    type: 'roomCreated',
    room,
  }));
  
  // eslint-disable-next-line no-console
  console.log(\`Room \${roomId} created by player \${playerId}\`);
}

function handleJoinRoom(playerId, data) {
  const room = rooms.get(data.roomId);
  if (!room) {
    const player = players.get(playerId);
    player.ws.send(JSON.stringify({
      type: 'error',
      message: 'Room not found',
    }));
    return;
  }
  
  if (room.players.length >= room.maxPlayers) {
    const player = players.get(playerId);
    player.ws.send(JSON.stringify({
      type: 'error',
      message: 'Room is full',
    }));
    return;
  }
  
  room.players.push(playerId);
  
  const player = players.get(playerId);
  player.roomId = data.roomId;
  
  // Notify player
  player.ws.send(JSON.stringify({
    type: 'roomJoined',
    room,
  }));
  
  // Notify other players in room
  broadcastToRoom(playerId, {
    type: 'playerJoined',
    playerId,
  });
  
  // eslint-disable-next-line no-console
  console.log(\`Player \${playerId} joined room \${data.roomId}\`);
}

function handleLeaveRoom(playerId) {
  const player = players.get(playerId);
  if (!player || !player.roomId) return;
  
  const room = rooms.get(player.roomId);
  if (!room) return;
  
  // Remove player from room
  room.players = room.players.filter(id => id !== playerId);
  
  // Notify other players
  broadcastToRoom(playerId, {
    type: 'playerLeft',
    playerId,
  });
  
  // Delete room if empty
  if (room.players.length === 0) {
    rooms.delete(player.roomId);
    // eslint-disable-next-line no-console
    console.log(\`Room \${player.roomId} deleted (empty)\`);
  }
  
  player.roomId = null;
  // eslint-disable-next-line no-console
  console.log(\`Player \${playerId} left room\`);
}

function handleDisconnect(playerId) {
  handleLeaveRoom(playerId);
  players.delete(playerId);
}

// Broadcasting
function broadcastToRoom(senderId, message) {
  const player = players.get(senderId);
  if (!player || !player.roomId) return;
  
  const room = rooms.get(player.roomId);
  if (!room) return;
  
  const data = JSON.stringify(message);
  
  for (const recipientId of room.players) {
    if (recipientId !== senderId) {
      const recipient = players.get(recipientId);
      if (recipient && recipient.ws.readyState === WebSocket.OPEN) {
        recipient.ws.send(data);
      }
    }
  }
}

// Utilities
function generatePlayerId() {
  return 'player_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
}

function generateRoomId() {
  return 'room_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
}

// Start server
server.listen(PORT, () => {
  // eslint-disable-next-line no-console
  console.log(\`\${' ='.repeat(40)}\`);
  // eslint-disable-next-line no-console
  console.log(\`  Nova Engine Dedicated Server v\${${this.config.serverVersion}}\`);
  // eslint-disable-next-line no-console
  console.log(\`\${' ='.repeat(40)}\`);
  // eslint-disable-next-line no-console
  console.log(\`  Status: Running\`);
  // eslint-disable-next-line no-console
  console.log(\`  Port: \${PORT}\`);
  // eslint-disable-next-line no-console
  console.log(\`  Max Players: \${MAX_PLAYERS}\`);
  // eslint-disable-next-line no-console
  console.log(\`  Time: \${new Date().toLocaleString()}\`);
  // eslint-disable-next-line no-console
  console.log(\`\${' ='.repeat(40)}\`);
});

// Graceful shutdown
process.on('SIGTERM', () => {
  // eslint-disable-next-line no-console
  console.log('Shutting down server...');
  wss.close(() => {
    server.close(() => {
      // eslint-disable-next-line no-console
      console.log('Server stopped');
      process.exit(0);
    });
  });
});
    `.trim();

    fs.writeFileSync(serverPath, serverContent);

    artifacts.push({
      path: serverPath,
      size: Buffer.byteLength(serverContent),
      type: 'javascript',
    });

    return artifacts;
  }

  /**
   * Bundle game logic
   */
  private async bundleGameLogic(
    projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];
    const gameLogicPath = path.join(projectPath, 'src', 'server');

    if (!fs.existsSync(gameLogicPath)) {
      return artifacts;
    }

    // Copy game logic files
    const targetDir = path.join(outDir, 'game');
    this.ensureDirectory(targetDir);

    // In a real implementation, would bundle all game logic
    // For now, create placeholder
    const logicPath = path.join(targetDir, 'game-logic.js');
    const logicContent = `
/**
 * Game-specific server logic
 */
module.exports = {
  validatePlayerAction: (action) => {
    // Validate player actions
    return true;
  },
  
  updateGameState: (state, action) => {
    // Update game state based on action
    return state;
  },
  
  checkWinCondition: (state) => {
    // Check if game has ended
    return false;
  },
};
    `.trim();

    fs.writeFileSync(logicPath, logicContent);

    artifacts.push({
      path: logicPath,
      size: Buffer.byteLength(logicContent),
      type: 'javascript',
    });

    return artifacts;
  }

  /**
   * Copy configuration files
   */
  private async copyConfiguration(
    _projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];

    // Create server configuration
    const configPath = path.join(outDir, 'config.json');
    const config = {
      server: {
        name: this.config.serverName,
        version: this.config.serverVersion,
        port: this.config.port,
        maxPlayers: this.config.maxPlayers,
        clustering: this.config.clustering,
      },
      game: {
        tickRate: 60,
        maxLatency: 300,
        autoSave: true,
        autoSaveInterval: 60000,
      },
      security: {
        rateLimit: {
          enabled: true,
          maxRequests: 100,
          windowMs: 60000,
        },
        cors: {
          enabled: true,
          origins: ['*'],
        },
      },
    };

    fs.writeFileSync(configPath, JSON.stringify(config, null, 2));

    artifacts.push({
      path: configPath,
      size: Buffer.byteLength(JSON.stringify(config)),
      type: 'config',
    });

    return artifacts;
  }

  /**
   * Setup networking
   */
  private async setupNetworking(outDir: string): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];

    // Create package.json for server
    const packagePath = path.join(outDir, 'package.json');
    const packageJson = {
      name: this.config.serverName.toLowerCase().replace(/\s+/g, '-'),
      version: this.config.serverVersion,
      description: 'Nova Engine Dedicated Server',
      main: 'server.js',
      scripts: {
        start: 'node server.js',
        dev: 'nodemon server.js',
      },
      dependencies: {
        ws: '^8.14.0',
      },
      devDependencies: {
        nodemon: '^3.0.1',
      },
      engines: {
        node: '>=18.0.0',
      },
    };

    fs.writeFileSync(packagePath, JSON.stringify(packageJson, null, 2));

    artifacts.push({
      path: packagePath,
      size: Buffer.byteLength(JSON.stringify(packageJson)),
      type: 'config',
    });

    return artifacts;
  }

  /**
   * Setup database
   */
  private async setupDatabase(outDir: string): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];

    // Create database initialization script
    const dbPath = path.join(outDir, 'init-db.js');
    const dbContent = `
/**
 * Database initialization
 */

const fs = require('fs');
const path = require('path');

// Initialize SQLite database
const dbFile = path.join(__dirname, 'game.db');

// Create tables
const schema = \`
CREATE TABLE IF NOT EXISTS players (
  id TEXT PRIMARY KEY,
  username TEXT NOT NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS game_sessions (
  id TEXT PRIMARY KEY,
  player_id TEXT NOT NULL,
  started_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  ended_at TIMESTAMP,
  FOREIGN KEY (player_id) REFERENCES players(id)
);

CREATE TABLE IF NOT EXISTS game_state (
  session_id TEXT PRIMARY KEY,
  state TEXT NOT NULL,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (session_id) REFERENCES game_sessions(id)
);
\`;

// eslint-disable-next-line no-console
console.log('Database initialized');
    `.trim();

    fs.writeFileSync(dbPath, dbContent);

    artifacts.push({
      path: dbPath,
      size: Buffer.byteLength(dbContent),
      type: 'javascript',
    });

    return artifacts;
  }

  /**
   * Generate deployment files
   */
  private async generateDeploymentFiles(
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];

    if (this.config.platform === ServerPlatform.Docker) {
      // Create Dockerfile
      const dockerfilePath = path.join(outDir, 'Dockerfile');
      const dockerfile = `
FROM node:18-alpine

WORKDIR /app

COPY package*.json ./
RUN npm ci --only=production

COPY . .

EXPOSE ${this.config.port}

CMD ["node", "server.js"]
      `.trim();

      fs.writeFileSync(dockerfilePath, dockerfile);

      artifacts.push({
        path: dockerfilePath,
        size: Buffer.byteLength(dockerfile),
        type: 'config',
      });

      // Create docker-compose.yml
      const composePath = path.join(outDir, 'docker-compose.yml');
      const compose = `
version: '3.8'

services:
  server:
    build: .
    ports:
      - "${this.config.port}:${this.config.port}"
    environment:
      - NODE_ENV=production
    restart: unless-stopped
      `.trim();

      fs.writeFileSync(composePath, compose);

      artifacts.push({
        path: composePath,
        size: Buffer.byteLength(compose),
        type: 'config',
      });
    }

    // Create start script
    const startScriptPath = path.join(outDir, 'start.sh');
    const startScript = `#!/bin/bash
# Nova Engine Server Start Script

echo "Starting ${this.config.serverName}..."

# Install dependencies if needed
if [ ! -d "node_modules" ]; then
  echo "Installing dependencies..."
  npm install
fi

# Start server
node server.js
    `.trim();

    fs.writeFileSync(startScriptPath, startScript);
    fs.chmodSync(startScriptPath, '755');

    artifacts.push({
      path: startScriptPath,
      size: Buffer.byteLength(startScript),
      type: 'script',
    });

    return artifacts;
  }

  /**
   * Generate build metadata
   */
  private async generateMetadata(
    outDir: string,
    artifacts: BuildArtifact[],
    startTime: number
  ): Promise<BuildArtifact> {
    const metadata = {
      version: this.config.serverVersion,
      buildDate: new Date().toISOString(),
      buildTime: Date.now() - startTime,
      platform: this.config.platform,
      artifacts: artifacts.map((a) => ({
        path: path.relative(outDir, a.path),
        size: a.size,
        type: a.type,
      })),
      config: {
        port: this.config.port,
        maxPlayers: this.config.maxPlayers,
        clustering: this.config.clustering,
        database: this.config.includeDatabase,
        minify: this.config.minify,
        sourceMaps: this.config.sourceMaps,
      },
    };

    const metadataPath = path.join(outDir, 'build-metadata.json');
    fs.writeFileSync(metadataPath, JSON.stringify(metadata, null, 2));

    return {
      path: metadataPath,
      size: Buffer.byteLength(JSON.stringify(metadata)),
      type: 'metadata',
    };
  }
}
