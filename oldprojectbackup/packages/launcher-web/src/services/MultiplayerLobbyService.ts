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
 * Multiplayer Lobby Service - Complete lobby browser and matchmaking
 * Phase 8.3: Multiplayer - Lobby browser, Quick match, Voice chat
 */

export interface GameLobby {
  id: string;
  gameId: string;
  gameName: string;
  hostId: string;
  hostName: string;
  name: string;
  description?: string;
  maxPlayers: number;
  currentPlayers: number;
  players: LobbyPlayer[];
  region: string;
  gameMode: string;
  difficulty?: 'easy' | 'normal' | 'hard' | 'expert';
  isPrivate: boolean;
  hasPassword: boolean;
  status: 'waiting' | 'starting' | 'in-progress' | 'finished';
  createdAt: Date;
  settings: Record<string, any>;
}

export interface LobbyPlayer {
  id: string;
  username: string;
  displayName: string;
  avatarUrl?: string;
  level?: number;
  rank?: string;
  isReady: boolean;
  isHost: boolean;
  team?: number;
  ping: number;
  joinedAt: Date;
}

export interface QuickMatchPreferences {
  gameId: string;
  region: 'auto' | 'na' | 'eu' | 'asia' | 'sa' | 'oce';
  gameMode?: string;
  skillLevel?: 'any' | 'beginner' | 'intermediate' | 'advanced' | 'expert';
  maxPing?: number;
}

export interface VoiceChannel {
  id: string;
  lobbyId: string;
  participants: string[];
  audioStream?: MediaStream;
  peerConnections: Map<string, RTCPeerConnection>;
}

class MultiplayerLobbyService {
  private lobbies: Map<string, GameLobby> = new Map();
  private currentLobby: GameLobby | null = null;
  private ws: WebSocket | null = null;
  private voiceChannel: VoiceChannel | null = null;
  private onLobbyUpdateCallbacks: ((lobby: GameLobby) => void)[] = [];

  /**
   * Browse all lobbies for a game
   */
  async browseLobies(
    gameId: string,
    filters?: {
      region?: string;
      gameMode?: string;
      hasSlots?: boolean;
      notFull?: boolean;
      notStarted?: boolean;
    }
  ): Promise<GameLobby[]> {
    try {
      const params = new URLSearchParams({ gameId, ...filters } as any);
      const response = await fetch(`/api/multiplayer/lobbies?${params}`, {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to fetch lobbies');

      const lobbies: GameLobby[] = await response.json();
      lobbies.forEach((lobby) => this.lobbies.set(lobby.id, lobby));

      return lobbies;
    } catch (error) {
      console.error('Error browsing lobbies:', error);
      return [];
    }
  }

  /**
   * Create a new lobby
   */
  async createLobby(config: {
    gameId: string;
    name: string;
    description?: string;
    maxPlayers: number;
    region: string;
    gameMode: string;
    difficulty?: GameLobby['difficulty'];
    isPrivate: boolean;
    password?: string;
    settings?: Record<string, any>;
  }): Promise<GameLobby> {
    try {
      const response = await fetch('/api/multiplayer/lobbies', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${this.getToken()}`,
        },
        body: JSON.stringify(config),
      });

      if (!response.ok) throw new Error('Failed to create lobby');

      const lobby: GameLobby = await response.json();
      this.currentLobby = lobby;
      this.lobbies.set(lobby.id, lobby);

      // Connect to lobby WebSocket
      this.connectToLobby(lobby.id);

      // eslint-disable-next-line no-console
      console.log(`Created lobby: ${lobby.name} (${lobby.id})`);
      return lobby;
    } catch (error) {
      console.error('Error creating lobby:', error);
      throw error;
    }
  }

  /**
   * Join a lobby
   */
  async joinLobby(lobbyId: string, password?: string): Promise<GameLobby> {
    try {
      const response = await fetch(`/api/multiplayer/lobbies/${lobbyId}/join`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${this.getToken()}`,
        },
        body: JSON.stringify({ password }),
      });

      if (!response.ok) {
        const error = await response.json();
        throw new Error(error.message || 'Failed to join lobby');
      }

      const lobby: GameLobby = await response.json();
      this.currentLobby = lobby;
      this.lobbies.set(lobby.id, lobby);

      // Connect to lobby WebSocket
      this.connectToLobby(lobby.id);

      // eslint-disable-next-line no-console
      console.log(`Joined lobby: ${lobby.name}`);
      return lobby;
    } catch (error) {
      console.error('Error joining lobby:', error);
      throw error;
    }
  }

  /**
   * Leave current lobby
   */
  async leaveLobby(): Promise<void> {
    if (!this.currentLobby) return;

    try {
      const response = await fetch(
        `/api/multiplayer/lobbies/${this.currentLobby.id}/leave`,
        {
          method: 'POST',
          headers: { Authorization: `Bearer ${this.getToken()}` },
        }
      );

      if (!response.ok) throw new Error('Failed to leave lobby');

      // Disconnect WebSocket
      if (this.ws) {
        this.ws.close();
        this.ws = null;
      }

      // Leave voice channel
      if (this.voiceChannel) {
        await this.leaveVoiceChannel();
      }

      // eslint-disable-next-line no-console
      console.log(`Left lobby: ${this.currentLobby.name}`);
      this.currentLobby = null;
    } catch (error) {
      console.error('Error leaving lobby:', error);
      throw error;
    }
  }

  /**
   * Quick match - find or create a suitable lobby
   */
  async quickMatch(preferences: QuickMatchPreferences): Promise<GameLobby> {
    try {
      const response = await fetch('/api/multiplayer/quickmatch', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${this.getToken()}`,
        },
        body: JSON.stringify(preferences),
      });

      if (!response.ok) throw new Error('Quick match failed');

      const result = await response.json();
      const lobby: GameLobby = result.lobby;

      this.currentLobby = lobby;
      this.lobbies.set(lobby.id, lobby);

      // Connect to lobby
      this.connectToLobby(lobby.id);

      // eslint-disable-next-line no-console
      console.log(`Quick match found: ${lobby.name}`);
      return lobby;
    } catch (error) {
      console.error('Error in quick match:', error);
      throw error;
    }
  }

  /**
   * Set player ready status
   */
  async setReady(ready: boolean): Promise<void> {
    if (!this.currentLobby) throw new Error('Not in a lobby');

    try {
      const response = await fetch(
        `/api/multiplayer/lobbies/${this.currentLobby.id}/ready`,
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            Authorization: `Bearer ${this.getToken()}`,
          },
          body: JSON.stringify({ ready }),
        }
      );

      if (!response.ok) throw new Error('Failed to set ready status');

      // eslint-disable-next-line no-console
      console.log(`Set ready status: ${ready}`);
    } catch (error) {
      console.error('Error setting ready status:', error);
      throw error;
    }
  }

  /**
   * Start game (host only)
   */
  async startGame(): Promise<void> {
    if (!this.currentLobby) throw new Error('Not in a lobby');

    try {
      const response = await fetch(
        `/api/multiplayer/lobbies/${this.currentLobby.id}/start`,
        {
          method: 'POST',
          headers: { Authorization: `Bearer ${this.getToken()}` },
        }
      );

      if (!response.ok) throw new Error('Failed to start game');

      // eslint-disable-next-line no-console
      console.log('Game starting...');
    } catch (error) {
      console.error('Error starting game:', error);
      throw error;
    }
  }

  /**
   * Invite friend to lobby
   */
  async inviteFriend(friendId: string): Promise<void> {
    if (!this.currentLobby) throw new Error('Not in a lobby');

    try {
      const response = await fetch(
        `/api/multiplayer/lobbies/${this.currentLobby.id}/invite`,
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            Authorization: `Bearer ${this.getToken()}`,
          },
          body: JSON.stringify({ friendId }),
        }
      );

      if (!response.ok) throw new Error('Failed to send invite');

      // eslint-disable-next-line no-console
      console.log(`Invited friend ${friendId} to lobby`);
    } catch (error) {
      console.error('Error inviting friend:', error);
      throw error;
    }
  }

  /**
   * Kick player from lobby (host only)
   */
  async kickPlayer(playerId: string): Promise<void> {
    if (!this.currentLobby) throw new Error('Not in a lobby');

    try {
      const response = await fetch(
        `/api/multiplayer/lobbies/${this.currentLobby.id}/kick`,
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            Authorization: `Bearer ${this.getToken()}`,
          },
          body: JSON.stringify({ playerId }),
        }
      );

      if (!response.ok) throw new Error('Failed to kick player');

      // eslint-disable-next-line no-console
      console.log(`Kicked player ${playerId}`);
    } catch (error) {
      console.error('Error kicking player:', error);
      throw error;
    }
  }

  /**
   * Connect to lobby WebSocket for real-time updates
   */
  private connectToLobby(lobbyId: string): void {
    const token = this.getToken();
    this.ws = new WebSocket(
      `ws://localhost:3000/api/multiplayer/lobbies/${lobbyId}/ws?token=${token}`
    );

    this.ws.onmessage = (event) => {
      const message = JSON.parse(event.data);

      switch (message.type) {
        case 'lobby_update':
          this.handleLobbyUpdate(message.lobby);
          break;
        case 'player_joined':
          // eslint-disable-next-line no-console
          console.log(`${message.player.username} joined the lobby`);
          break;
        case 'player_left':
          // eslint-disable-next-line no-console
          console.log(`${message.player.username} left the lobby`);
          break;
        case 'player_ready':
          // eslint-disable-next-line no-console
          console.log(
            `${message.player.username} is ${message.ready ? 'ready' : 'not ready'}`
          );
          break;
        case 'game_starting':
          // eslint-disable-next-line no-console
          console.log('Game is starting!');
          break;
        case 'chat_message':
          // eslint-disable-next-line no-console
          console.log(`[Chat] ${message.player.username}: ${message.message}`);
          break;
      }
    };

    this.ws.onerror = (error) => {
      console.error('Lobby WebSocket error:', error);
    };

    this.ws.onclose = () => {
      // eslint-disable-next-line no-console
      console.log('Disconnected from lobby');
    };
  }

  /**
   * Handle lobby update
   */
  private handleLobbyUpdate(lobby: GameLobby): void {
    this.currentLobby = lobby;
    this.lobbies.set(lobby.id, lobby);

    // Notify callbacks
    this.onLobbyUpdateCallbacks.forEach((callback) => callback(lobby));
  }

  /**
   * Send chat message in lobby
   */
  async sendChatMessage(message: string): Promise<void> {
    if (!this.ws || !this.currentLobby) throw new Error('Not in a lobby');

    this.ws.send(
      JSON.stringify({
        type: 'chat_message',
        message,
      })
    );
  }

  /**
   * Join voice channel
   */
  async joinVoiceChannel(): Promise<void> {
    if (!this.currentLobby) throw new Error('Not in a lobby');

    try {
      // Request microphone access
      const stream = await navigator.mediaDevices.getUserMedia({ audio: true });

      this.voiceChannel = {
        id: `voice_${this.currentLobby.id}`,
        lobbyId: this.currentLobby.id,
        participants: [],
        audioStream: stream,
        peerConnections: new Map(),
      };

      // Set up WebRTC connections for each player
      for (const player of this.currentLobby.players) {
        if (player.id !== this.getCurrentUserId()) {
          await this.createPeerConnection(player.id);
        }
      }

      // eslint-disable-next-line no-console
      console.log('Joined voice channel');
    } catch (error) {
      console.error('Error joining voice channel:', error);
      throw error;
    }
  }

  /**
   * Leave voice channel
   */
  async leaveVoiceChannel(): Promise<void> {
    if (!this.voiceChannel) return;

    // Stop audio stream
    if (this.voiceChannel.audioStream) {
      this.voiceChannel.audioStream
        .getTracks()
        .forEach((track) => track.stop());
    }

    // Close all peer connections
    this.voiceChannel.peerConnections.forEach((pc) => pc.close());

    this.voiceChannel = null;
    // eslint-disable-next-line no-console
    console.log('Left voice channel');
  }

  /**
   * Create WebRTC peer connection
   */
  private async createPeerConnection(peerId: string): Promise<void> {
    if (!this.voiceChannel) return;

    const pc = new RTCPeerConnection({
      iceServers: [
        { urls: 'stun:stun.l.google.com:19302' },
        { urls: 'stun:stun1.l.google.com:19302' },
      ],
    });

    // Add local stream
    if (this.voiceChannel.audioStream) {
      this.voiceChannel.audioStream.getTracks().forEach((track) => {
        pc.addTrack(track, this.voiceChannel!.audioStream!);
      });
    }

    // Handle remote stream
    pc.ontrack = (event) => {
      const [remoteStream] = event.streams;
      if (remoteStream) {
        this.playRemoteAudio(peerId, remoteStream);
      }
    };

    // Handle ICE candidates
    pc.onicecandidate = (event) => {
      if (event.candidate && this.ws) {
        this.ws.send(
          JSON.stringify({
            type: 'ice_candidate',
            peerId,
            candidate: event.candidate,
          })
        );
      }
    };

    this.voiceChannel.peerConnections.set(peerId, pc);

    // Create and send offer
    const offer = await pc.createOffer();
    await pc.setLocalDescription(offer);

    if (this.ws) {
      this.ws.send(
        JSON.stringify({
          type: 'voice_offer',
          peerId,
          offer,
        })
      );
    }
  }

  /**
   * Play remote audio stream
   */
  private playRemoteAudio(peerId: string, stream: MediaStream): void {
    const audio = new Audio();
    audio.srcObject = stream;
    audio.play();

    // eslint-disable-next-line no-console
    console.log(`Playing audio from ${peerId}`);
  }

  /**
   * Toggle mute
   */
  toggleMute(muted: boolean): void {
    if (!this.voiceChannel || !this.voiceChannel.audioStream) return;

    this.voiceChannel.audioStream.getAudioTracks().forEach((track) => {
      track.enabled = !muted;
    });

    // eslint-disable-next-line no-console
    console.log(`Microphone ${muted ? 'muted' : 'unmuted'}`);
  }

  /**
   * Subscribe to lobby updates
   */
  onLobbyUpdate(callback: (lobby: GameLobby) => void): () => void {
    this.onLobbyUpdateCallbacks.push(callback);

    return () => {
      const index = this.onLobbyUpdateCallbacks.indexOf(callback);
      if (index > -1) {
        this.onLobbyUpdateCallbacks.splice(index, 1);
      }
    };
  }

  /**
   * Get current lobby
   */
  getCurrentLobby(): GameLobby | null {
    return this.currentLobby;
  }

  /**
   * Check if player is host
   */
  isHost(): boolean {
    if (!this.currentLobby) return false;

    const currentUserId = this.getCurrentUserId();
    return this.currentLobby.hostId === currentUserId;
  }

  /**
   * Get auth token
   */
  private getToken(): string {
    return localStorage.getItem('auth_token') || '';
  }

  /**
   * Get current user ID
   */
  private getCurrentUserId(): string {
    return localStorage.getItem('user_id') || '';
  }
}

export const multiplayerLobbyService = new MultiplayerLobbyService();
