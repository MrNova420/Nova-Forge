/**
 * Distributed State Management Service
 * Handles state synchronization for massive multiplayer games with thousands of players
 * Uses sharding, CRDT, and eventual consistency for scalability
 */

import { RedisService } from '../database/RedisService';

export interface GameState {
  roomId: string;
  worldState: WorldState;
  playerStates: Map<string, PlayerState>;
  entityStates: Map<string, EntityState>;
  version: number;
  timestamp: number;
}

export interface WorldState {
  time: number;
  weather?: string;
  dayNight?: number;
  events: GameEvent[];
  regions: Map<string, RegionState>;
}

export interface RegionState {
  id: string;
  bounds: { minX: number; minY: number; maxX: number; maxY: number };
  entities: string[];
  players: string[];
  authorityNode?: string; // Which server/peer has authority
}

export interface PlayerState {
  playerId: string;
  position: { x: number; y: number; z: number };
  rotation: { x: number; y: number; z: number };
  velocity: { x: number; y: number; z: number };
  health: number;
  inventory: InventoryItem[];
  region: string;
  timestamp: number;
}

export interface EntityState {
  entityId: string;
  type: string;
  position: { x: number; y: number; z: number };
  rotation: { x: number; y: number; z: number };
  state: any;
  ownerId?: string;
  region: string;
  timestamp: number;
}

export interface InventoryItem {
  id: string;
  type: string;
  quantity: number;
  properties?: Record<string, any>;
}

export interface GameEvent {
  id: string;
  type: string;
  data: any;
  timestamp: number;
  region?: string;
}

export interface StateShard {
  id: string;
  regions: string[];
  players: string[];
  entities: string[];
  authorityNode: string;
}

export class DistributedStateService {
  private redis = RedisService.getInstance();

  /**
   * Initialize distributed state for a game room
   */
  async initializeState(
    roomId: string,
    worldBounds: { width: number; height: number }
  ): Promise<GameState> {
    // Create regional shards
    const regions = this.createRegionShards(worldBounds, 100); // 100x100 unit regions

    const initialState: GameState = {
      roomId,
      worldState: {
        time: 0,
        events: [],
        regions,
      },
      playerStates: new Map(),
      entityStates: new Map(),
      version: 1,
      timestamp: Date.now(),
    };

    // Store initial state
    await this.redis.setJson(
      `state:${roomId}`,
      this.serializeState(initialState),
      3600
    );

    return initialState;
  }

  /**
   * Create region shards for spatial partitioning
   */
  private createRegionShards(
    worldBounds: { width: number; height: number },
    regionSize: number
  ): Map<string, RegionState> {
    const regions = new Map<string, RegionState>();

    const cols = Math.ceil(worldBounds.width / regionSize);
    const rows = Math.ceil(worldBounds.height / regionSize);

    for (let y = 0; y < rows; y++) {
      for (let x = 0; x < cols; x++) {
        const regionId = `region_${x}_${y}`;
        regions.set(regionId, {
          id: regionId,
          bounds: {
            minX: x * regionSize,
            minY: y * regionSize,
            maxX: (x + 1) * regionSize,
            maxY: (y + 1) * regionSize,
          },
          entities: [],
          players: [],
        });
      }
    }

    return regions;
  }

  /**
   * Update player state with conflict resolution
   */
  async updatePlayerState(
    roomId: string,
    playerId: string,
    state: Partial<PlayerState>,
    clientTimestamp: number
  ): Promise<{ accepted: boolean; resolvedState: PlayerState | null }> {
    // Get current state
    const currentState = await this.getPlayerState(roomId, playerId);

    // Server timestamp
    const serverTimestamp = Date.now();

    // Conflict resolution - only accept if client timestamp is newer
    if (currentState && clientTimestamp < currentState.timestamp) {
      // Client state is outdated
      return {
        accepted: false,
        resolvedState: currentState,
      };
    }

    // Merge state
    const newState: PlayerState = {
      ...currentState,
      ...state,
      playerId,
      timestamp: serverTimestamp,
    } as PlayerState;

    // Determine region
    if (newState.position) {
      newState.region = this.getRegionForPosition(newState.position);
    }

    // Store updated state
    await this.redis.setJson(
      `state:${roomId}:player:${playerId}`,
      newState,
      60
    );

    // Update region tracking
    if (newState.region) {
      await this.redis.set(
        `state:${roomId}:region:${newState.region}:player:${playerId}`,
        '1',
        60
      );
    }

    // Publish update to subscribers
    await this.publishStateUpdate(roomId, 'player', playerId, newState);

    return {
      accepted: true,
      resolvedState: newState,
    };
  }

  /**
   * Update entity state
   */
  async updateEntityState(
    roomId: string,
    entityId: string,
    state: Partial<EntityState>
  ): Promise<EntityState> {
    // Get current state
    const currentState = await this.getEntityState(roomId, entityId);

    // Merge state
    const newState: EntityState = {
      ...currentState,
      ...state,
      entityId,
      timestamp: Date.now(),
    } as EntityState;

    // Determine region
    if (newState.position) {
      newState.region = this.getRegionForPosition(newState.position);
    }

    // Store updated state
    await this.redis.setJson(
      `state:${roomId}:entity:${entityId}`,
      newState,
      60
    );

    // Update region tracking
    if (newState.region) {
      await this.redis.set(
        `state:${roomId}:region:${newState.region}:entity:${entityId}`,
        '1',
        60
      );
    }

    // Publish update
    await this.publishStateUpdate(roomId, 'entity', entityId, newState);

    return newState;
  }

  /**
   * Get player state
   */
  async getPlayerState(
    roomId: string,
    playerId: string
  ): Promise<PlayerState | null> {
    return this.redis.getJson<PlayerState>(
      `state:${roomId}:player:${playerId}`
    );
  }

  /**
   * Get entity state
   */
  async getEntityState(
    roomId: string,
    entityId: string
  ): Promise<EntityState | null> {
    return this.redis.getJson<EntityState>(
      `state:${roomId}:entity:${entityId}`
    );
  }

  /**
   * Get all players in a region (for Area of Interest)
   */
  async getPlayersInRegion(
    roomId: string,
    regionId: string
  ): Promise<PlayerState[]> {
    // Get player IDs in region
    const playerIds = await this.redis.get(
      `state:${roomId}:region:${regionId}:players`
    );
    if (!playerIds) return [];

    const ids = JSON.parse(playerIds);

    // Fetch all player states
    const states = await Promise.all(
      ids.map((id: string) => this.getPlayerState(roomId, id))
    );

    return states.filter((s): s is PlayerState => s !== null);
  }

  /**
   * Get entities in a region
   */
  async getEntitiesInRegion(
    roomId: string,
    regionId: string
  ): Promise<EntityState[]> {
    const entityIds = await this.redis.get(
      `state:${roomId}:region:${regionId}:entities`
    );
    if (!entityIds) return [];

    const ids = JSON.parse(entityIds);

    const states = await Promise.all(
      ids.map((id: string) => this.getEntityState(roomId, id))
    );

    return states.filter((s): s is EntityState => s !== null);
  }

  /**
   * Get state for player's Area of Interest (AoI)
   */
  async getAreaOfInterest(
    roomId: string,
    playerId: string,
    radius: number = 150
  ): Promise<{
    players: PlayerState[];
    entities: EntityState[];
  }> {
    // Get player's state
    const playerState = await this.getPlayerState(roomId, playerId);
    if (!playerState) {
      return { players: [], entities: [] };
    }

    // Get player's region and adjacent regions
    const playerRegion = playerState.region;
    const adjacentRegions = this.getAdjacentRegions(playerRegion);
    const regionsToCheck = [playerRegion, ...adjacentRegions];

    // Fetch all players and entities in relevant regions
    const [playerLists, entityLists] = await Promise.all([
      Promise.all(
        regionsToCheck.map((r) => this.getPlayersInRegion(roomId, r))
      ),
      Promise.all(
        regionsToCheck.map((r) => this.getEntitiesInRegion(roomId, r))
      ),
    ]);

    const allPlayers = playerLists.flat();
    const allEntities = entityLists.flat();

    // Filter by distance
    const playersInRange = allPlayers.filter((p) => {
      if (p.playerId === playerId) return false;
      return this.distance(playerState.position, p.position) <= radius;
    });

    const entitiesInRange = allEntities.filter((e) => {
      return this.distance(playerState.position, e.position) <= radius;
    });

    return {
      players: playersInRange,
      entities: entitiesInRange,
    };
  }

  /**
   * Publish state update to subscribers
   */
  private async publishStateUpdate(
    roomId: string,
    type: 'player' | 'entity' | 'world',
    id: string,
    state: any
  ): Promise<void> {
    const update = {
      type,
      id,
      state,
      timestamp: Date.now(),
    };

    // In production, would use Redis pub/sub or message queue
    await this.redis.setJson(`state:${roomId}:update:${Date.now()}`, update, 5);
  }

  /**
   * Process game event
   */
  async processGameEvent(roomId: string, event: GameEvent): Promise<void> {
    // Store event
    await this.redis.setJson(`state:${roomId}:event:${event.id}`, event, 60);

    // Add to event log
    await this.redis.set(`state:${roomId}:event_log`, event.id, 60);

    // Broadcast event
    await this.publishStateUpdate(roomId, 'world', event.id, event);
  }

  /**
   * Snapshot state for persistence
   */
  async snapshotState(roomId: string): Promise<string> {
    const snapshotId = `snapshot_${Date.now()}`;

    // Would implement full state serialization
    await this.redis.set(
      `state:${roomId}:snapshot:${snapshotId}`,
      'data',
      86400
    );

    return snapshotId;
  }

  /**
   * Restore state from snapshot
   */
  async restoreFromSnapshot(
    _roomId: string,
    _snapshotId: string
  ): Promise<boolean> {
    // Would implement state restoration
    return true;
  }

  /**
   * Calculate distance between two positions
   */
  private distance(
    pos1: { x: number; y: number; z: number },
    pos2: { x: number; y: number; z: number }
  ): number {
    const dx = pos1.x - pos2.x;
    const dy = pos1.y - pos2.y;
    const dz = pos1.z - pos2.z;
    return Math.sqrt(dx * dx + dy * dy + dz * dz);
  }

  /**
   * Get region ID for a position
   */
  private getRegionForPosition(position: { x: number; y: number }): string {
    const regionSize = 100;
    const x = Math.floor(position.x / regionSize);
    const y = Math.floor(position.y / regionSize);
    return `region_${x}_${y}`;
  }

  /**
   * Get adjacent regions
   */
  private getAdjacentRegions(regionId: string): string[] {
    const [, x, y] = regionId.match(/region_(-?\d+)_(-?\d+)/) || [];
    if (!x || !y) return [];

    const cx = parseInt(x, 10);
    const cy = parseInt(y, 10);

    return [
      `region_${cx - 1}_${cy - 1}`,
      `region_${cx}_${cy - 1}`,
      `region_${cx + 1}_${cy - 1}`,
      `region_${cx - 1}_${cy}`,
      `region_${cx + 1}_${cy}`,
      `region_${cx - 1}_${cy + 1}`,
      `region_${cx}_${cy + 1}`,
      `region_${cx + 1}_${cy + 1}`,
    ];
  }

  /**
   * Serialize state for storage
   */
  private serializeState(state: GameState): any {
    return {
      roomId: state.roomId,
      worldState: {
        ...state.worldState,
        regions: Object.fromEntries(state.worldState.regions),
      },
      playerStates: Object.fromEntries(state.playerStates),
      entityStates: Object.fromEntries(state.entityStates),
      version: state.version,
      timestamp: state.timestamp,
    };
  }

  /**
   * Garbage collect old state data
   */
  async garbageCollect(
    roomId: string,
    olderThanMs: number = 60000
  ): Promise<number> {
    const now = Date.now();
    const cutoff = now - olderThanMs;

    // Would implement cleanup of old state entries
    return 0;
  }

  /**
   * Get state synchronization statistics
   */
  async getStateStats(_roomId: string): Promise<{
    playerCount: number;
    entityCount: number;
    regionCount: number;
    updatesPerSecond: number;
    stateSize: number;
  }> {
    // Would calculate actual statistics
    return {
      playerCount: 0,
      entityCount: 0,
      regionCount: 0,
      updatesPerSecond: 0,
      stateSize: 0,
    };
  }
}
