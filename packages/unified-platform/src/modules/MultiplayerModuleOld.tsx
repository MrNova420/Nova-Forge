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
 * Multiplayer Module - Online Lobbies & Matchmaking
 */

import React, { useState, useEffect } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface MultiplayerModuleProps {
  platform: UnifiedPlatformCore;
}

export const MultiplayerModule: React.FC<MultiplayerModuleProps> = ({
  platform,
}) => {
  const [lobbies, setLobbies] = useState<any[]>([]);
  const [currentLobby, setCurrentLobby] = useState<any>(null);

  useEffect(() => {
    loadLobbies();
  }, []);

  const loadLobbies = async () => {
    try {
      const response = await fetch('/api/multiplayer/lobbies');
      const data = await response.json();
      setLobbies(data.lobbies || []);
    } catch (error) {
      console.error('Failed to load lobbies:', error);
    }
  };

  const createLobby = async () => {
    const name = prompt('Lobby name:');
    if (!name) return;

    try {
      const response = await fetch('/api/multiplayer/lobbies', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name }),
      });
      const data = await response.json();
      setCurrentLobby(data.lobby);
      platform.showNotification({
        type: 'success',
        message: 'Lobby created!',
      });
    } catch (error) {
      console.error('Failed to create lobby:', error);
    }
  };

  const joinLobby = async (lobbyId: string) => {
    try {
      const response = await fetch(`/api/multiplayer/lobbies/${lobbyId}/join`, {
        method: 'POST',
      });
      const data = await response.json();
      setCurrentLobby(data.lobby);
    } catch (error) {
      console.error('Failed to join lobby:', error);
    }
  };

  if (currentLobby) {
    return (
      <div className="lobby-view">
        <button onClick={() => setCurrentLobby(null)}>← Back</button>
        <h2>{currentLobby.name}</h2>
        <div className="players">
          {currentLobby.players?.map((player: any) => (
            <div key={player.id} className="player">
              {player.name} {player.ready && '✓'}
            </div>
          ))}
        </div>
        <button>Ready</button>
        <style>{`
          .lobby-view {
            padding: 20px;
            background: #1a1a1a;
            color: white;
          }
          .players {
            margin: 20px 0;
          }
          .player {
            padding: 10px;
            background: #2a2a2a;
            margin: 5px 0;
            border-radius: 4px;
          }
        `}</style>
      </div>
    );
  }

  return (
    <div className="multiplayer-module">
      <div className="header">
        <h2>Multiplayer Lobbies</h2>
        <button onClick={createLobby}>+ Create Lobby</button>
      </div>
      <div className="lobby-list">
        {lobbies.map((lobby) => (
          <div
            key={lobby.id}
            className="lobby-card"
            onClick={() => joinLobby(lobby.id)}
          >
            <h3>{lobby.name}</h3>
            <p>
              {lobby.players?.length || 0}/{lobby.maxPlayers} players
            </p>
          </div>
        ))}
      </div>
      <style>{`
        .multiplayer-module {
          padding: 20px;
          background: #1a1a1a;
          color: white;
          height: 100%;
        }
        .header {
          display: flex;
          justify-content: space-between;
          margin-bottom: 20px;
        }
        .lobby-list {
          display: grid;
          grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
          gap: 15px;
        }
        .lobby-card {
          padding: 20px;
          background: #2a2a2a;
          border-radius: 8px;
          cursor: pointer;
          transition: transform 0.3s;
        }
        .lobby-card:hover {
          transform: scale(1.05);
        }
      `}</style>
    </div>
  );
};
