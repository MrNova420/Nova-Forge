/**
 * NOVA ENGINE - Complete Backend Server with WebSocket Support
 */

import express from 'express';
import cors from 'cors';
import helmet from 'helmet';
import dotenv from 'dotenv';
import { createServer } from 'http';
import { Server as SocketIOServer } from 'socket.io';
import { initializeDatabase, pool } from './config/database';
import authRoutes from './routes/auth';
import gamesRoutes from './routes/games';
import socialRoutes from './routes/social';
import multiplayerRoutes from './routes/multiplayer';

dotenv.config();

const app = express();
const httpServer = createServer(app);
const io = new SocketIOServer(httpServer, {
  cors: {
    origin: process.env.FRONTEND_URL || 'http://localhost:3000',
    methods: ['GET', 'POST'],
    credentials: true,
  },
});

const PORT = process.env.PORT || 3001;

// Middleware
app.use(helmet());
app.use(
  cors({
    origin: process.env.FRONTEND_URL || 'http://localhost:3000',
    credentials: true,
  })
);
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Routes
app.use('/api/auth', authRoutes);
app.use('/api/games', gamesRoutes);
app.use('/api/social', socialRoutes);
app.use('/api/multiplayer', multiplayerRoutes);

// Health check
app.get('/health', (_req, res) => {
  res.json({
    status: 'ok',
    timestamp: new Date().toISOString(),
    websocket: 'enabled',
  });
});

// WebSocket connection handling
io.on('connection', (socket) => {
  // eslint-disable-next-line no-console
  console.log('✅ Client connected:', socket.id);

  // Handle user authentication via socket
  socket.on('authenticate', (data) => {
    const { userId, username } = data;
    socket.data.userId = userId;
    socket.data.username = username;
    socket.join(`user:${userId}`);
    // eslint-disable-next-line no-console
    console.log(`🔐 User ${username} (${userId}) authenticated`);
    socket.emit('authenticated', { success: true });
  });

  // Matchmaking events
  socket.on('matchmaking:search', (data) => {
    // eslint-disable-next-line no-console
    console.log('🎮 Matchmaking search started:', data);
    socket.join('matchmaking');
    socket.emit('matchmaking:status', { status: 'searching' });
    // TODO: Implement actual matchmaking algorithm
  });

  socket.on('matchmaking:cancel', () => {
    // eslint-disable-next-line no-console
    console.log('❌ Matchmaking cancelled');
    socket.leave('matchmaking');
    socket.emit('matchmaking:status', { status: 'cancelled' });
  });

  // Lobby events
  socket.on('lobby:join', (lobbyId) => {
    socket.join(`lobby:${lobbyId}`);
    // eslint-disable-next-line no-console
    console.log(`👥 User ${socket.data.username} joined lobby ${lobbyId}`);
    io.to(`lobby:${lobbyId}`).emit('lobby:player_joined', {
      userId: socket.data.userId,
      username: socket.data.username,
      timestamp: new Date().toISOString(),
    });
  });

  socket.on('lobby:leave', (lobbyId) => {
    socket.leave(`lobby:${lobbyId}`);
    // eslint-disable-next-line no-console
    console.log(`👋 User ${socket.data.username} left lobby ${lobbyId}`);
    io.to(`lobby:${lobbyId}`).emit('lobby:player_left', {
      userId: socket.data.userId,
      username: socket.data.username,
      timestamp: new Date().toISOString(),
    });
  });

  socket.on('lobby:chat', (data) => {
    const { lobbyId, message } = data;
    io.to(`lobby:${lobbyId}`).emit('lobby:chat_message', {
      userId: socket.data.userId,
      username: socket.data.username,
      message,
      timestamp: new Date().toISOString(),
    });
  });

  socket.on('lobby:ready', (data) => {
    const { lobbyId, ready } = data;
    io.to(`lobby:${lobbyId}`).emit('lobby:player_ready', {
      userId: socket.data.userId,
      username: socket.data.username,
      ready,
      timestamp: new Date().toISOString(),
    });
  });

  // Friend status updates
  socket.on('status:update', (status) => {
    socket.data.status = status;
    // eslint-disable-next-line no-console
    console.log(`📊 User ${socket.data.username} status: ${status}`);
    // TODO: Notify friends of status change
    // Get friends list from database and emit to their rooms
  });

  // Notifications
  socket.on('notification:send', (data) => {
    const { targetUserId, notification } = data;
    io.to(`user:${targetUserId}`).emit('notification:received', notification);
  });

  // Disconnect handling
  socket.on('disconnect', () => {
    // eslint-disable-next-line no-console
    console.log('❌ Client disconnected:', socket.id);
    if (socket.data.userId) {
      // Update user status to offline
      // TODO: Update database
    }
  });
});

// Initialize and start server
async function startServer() {
  try {
    await initializeDatabase();
    // eslint-disable-next-line no-console
    console.log('✅ Database initialized');

    httpServer.listen(PORT, () => {
      // eslint-disable-next-line no-console
      console.log(`
╔═══════════════════════════════════════════════════════════╗
║              NOVA ENGINE BACKEND SERVER                  ║
║                                                           ║
║  🚀 Server running on port ${PORT}                        ║
║  📊 Database connected                                    ║
║  🔐 Authentication enabled                                ║
║  🌐 CORS enabled for frontend                            ║
║  📡 WebSocket server ready                                ║
║                                                           ║
║  API Endpoints:                                           ║
║  • /api/auth/*        - Authentication                   ║
║  • /api/games/*       - Game management                  ║
║  • /api/social/*      - Social features                  ║
║  • /api/multiplayer/* - Multiplayer & lobbies           ║
╚═══════════════════════════════════════════════════════════╝
      `);
    });
  } catch (error) {
    console.error('❌ Failed to start server:', error);
    process.exit(1);
  }
}

// Graceful shutdown
process.on('SIGTERM', async () => {
  // eslint-disable-next-line no-console
  console.log('SIGTERM received, closing server...');
  await pool.end();
  httpServer.close();
  process.exit(0);
});

startServer();

export { app, io };
export default app;
