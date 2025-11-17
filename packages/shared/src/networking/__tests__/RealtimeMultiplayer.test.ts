/**
 * Tests for RealtimeMultiplayer service
 */

import { realtimeMultiplayer, NetworkMessage } from '../RealtimeMultiplayer';

// Mock WebSocket
class MockWebSocket {
  public readyState: number = 1; // OPEN
  public onopen: (() => void) | null = null;
  public onmessage: ((event: { data: string }) => void) | null = null;
  public onerror: ((error: any) => void) | null = null;
  public onclose: (() => void) | null = null;
  private sendQueue: string[] = [];

  static CONNECTING = 0;
  static OPEN = 1;
  static CLOSING = 2;
  static CLOSED = 3;

  constructor(public url: string) {
    setTimeout(() => {
      this.readyState = 1;
      if (this.onopen) {
        this.onopen();
      }
    }, 10);
  }

  send(data: string): void {
    this.sendQueue.push(data);
  }

  close(): void {
    this.readyState = 3;
    if (this.onclose) {
      this.onclose();
    }
  }

  simulateMessage(message: NetworkMessage): void {
    if (this.onmessage) {
      this.onmessage({ data: JSON.stringify(message) });
    }
  }

  getSentMessages(): NetworkMessage[] {
    return this.sendQueue.map((data) => JSON.parse(data));
  }

  clearSendQueue(): void {
    this.sendQueue = [];
  }
}

(global as any).WebSocket = MockWebSocket;

describe('RealtimeMultiplayer', () => {
  beforeEach(async () => {
    realtimeMultiplayer.disconnect();
    await realtimeMultiplayer.initialize('TestPlayer', 'web');
  });

  afterEach(() => {
    realtimeMultiplayer.disconnect();
  });

  describe('Initialization', () => {
    it('should initialize with player information', async () => {
      await realtimeMultiplayer.initialize('Player1', 'desktop');
      const player = realtimeMultiplayer.getCurrentPlayer();

      expect(player).not.toBeNull();
      expect(player?.username).toBe('Player1');
      expect(player?.platform).toBe('desktop');
    });
  });

  describe('Connection', () => {
    it('should connect to server', async () => {
      await realtimeMultiplayer.connect('ws://localhost:8080');
      const player = realtimeMultiplayer.getCurrentPlayer();

      expect(player?.connected).toBe(true);
    });
  });

  describe('Room Management', () => {
    beforeEach(async () => {
      await realtimeMultiplayer.connect('ws://localhost:8080');
    });

    it('should create a room', async () => {
      const room = await realtimeMultiplayer.createRoom('game123', 4);

      expect(room.gameId).toBe('game123');
      expect(room.maxPlayers).toBe(4);
    });
  });
});
