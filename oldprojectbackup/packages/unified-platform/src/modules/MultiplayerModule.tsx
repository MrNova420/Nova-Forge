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
 * NOVA ENGINE - MULTIPLAYER MODULE
 * Complete multiplayer system with matchmaking, friends, lobbies, chat
 */

import React, { useState, useEffect } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface MultiplayerModuleProps {
  platform: UnifiedPlatformCore;
}

interface Player {
  id: string;
  username: string;
  level: number;
  avatar: string;
  status: 'online' | 'in-game' | 'away';
  rank: string;
}

interface Lobby {
  id: string;
  name: string;
  game: string;
  host: string;
  players: number;
  maxPlayers: number;
  mode: string;
  region: string;
  ping: number;
}

interface Message {
  id: string;
  username: string;
  message: string;
  timestamp: string;
  type: 'chat' | 'system' | 'friend';
}

export const MultiplayerModuleRedesigned: React.FC<MultiplayerModuleProps> = ({
  platform,
}) => {
  const [activeTab, setActiveTab] = useState<
    'matchmaking' | 'lobbies' | 'friends' | 'party'
  >('matchmaking');
  const [onlinePlayers, setOnlinePlayers] = useState(12847);
  const [selectedGame, setSelectedGame] = useState('');
  const [selectedMode, setSelectedMode] = useState('');
  const [selectedRegion, setSelectedRegion] = useState('auto');
  const [isSearching, setIsSearching] = useState(false);
  const [lobbies, setLobbies] = useState<Lobby[]>([]);
  const [friends, setFriends] = useState<Player[]>([]);
  const [party] = useState<Player[]>([]);
  const [messages, setMessages] = useState<Message[]>([
    {
      id: '1',
      username: 'System',
      message: 'Welcome to Nova Engine Multiplayer!',
      timestamp: new Date().toLocaleTimeString(),
      type: 'system',
    },
  ]);
  const [newMessage, setNewMessage] = useState('');
  const [playerStats] = useState({
    gamesPlayed: 156,
    wins: 89,
    losses: 67,
    winRate: 57,
    avgScore: 2450,
    rank: 'Diamond III',
  });

  useEffect(() => {
    loadLobbies();
    loadFriends();

    // Simulate online players counter
    const interval = setInterval(() => {
      setOnlinePlayers((prev) => prev + Math.floor(Math.random() * 20 - 10));
    }, 3000);

    return () => clearInterval(interval);
  }, []);

  const loadLobbies = () => {
    const mockLobbies: Lobby[] = [
      {
        id: '1',
        name: 'Epic Battle Arena',
        game: 'Space Warriors',
        host: 'ProGamer123',
        players: 3,
        maxPlayers: 4,
        mode: 'Team Deathmatch',
        region: 'US-East',
        ping: 25,
      },
      {
        id: '2',
        name: 'Chill Session',
        game: 'Space Warriors',
        host: 'CasualPlayer',
        players: 2,
        maxPlayers: 8,
        mode: 'Free for All',
        region: 'EU-West',
        ping: 45,
      },
      {
        id: '3',
        name: 'Ranked Match',
        game: 'Cyber Racers',
        host: 'RaceKing',
        players: 6,
        maxPlayers: 8,
        mode: 'Circuit Race',
        region: 'Asia',
        ping: 78,
      },
      {
        id: '4',
        name: 'Beginners Welcome',
        game: 'Space Warriors',
        host: 'NewbieFriend',
        players: 1,
        maxPlayers: 4,
        mode: 'Co-op',
        region: 'US-West',
        ping: 35,
      },
      {
        id: '5',
        name: 'Hardcore Only',
        game: 'Cyber Racers',
        host: 'ElitePro',
        players: 4,
        maxPlayers: 4,
        mode: 'Elimination',
        region: 'EU-North',
        ping: 52,
      },
    ];
    setLobbies(mockLobbies);
  };

  const loadFriends = () => {
    const mockFriends: Player[] = [
      {
        id: '1',
        username: 'SkillMaster',
        level: 45,
        avatar: '👨',
        status: 'online',
        rank: 'Platinum I',
      },
      {
        id: '2',
        username: 'NovaQueen',
        level: 67,
        avatar: '👩',
        status: 'in-game',
        rank: 'Diamond II',
      },
      {
        id: '3',
        username: 'SpeedDemon',
        level: 34,
        avatar: '😎',
        status: 'online',
        rank: 'Gold III',
      },
      {
        id: '4',
        username: 'TacticalMind',
        level: 52,
        avatar: '🧠',
        status: 'away',
        rank: 'Platinum II',
      },
      {
        id: '5',
        username: 'CyberNinja',
        level: 78,
        avatar: '🥷',
        status: 'online',
        rank: 'Diamond I',
      },
    ];
    setFriends(mockFriends);
  };

  const handleStartMatchmaking = () => {
    if (!selectedGame || !selectedMode) {
      platform.showNotification({
        type: 'error',
        message: 'Please select game and mode',
      });
      return;
    }

    setIsSearching(true);
    addSystemMessage('Searching for match...');

    // Simulate matchmaking
    setTimeout(() => {
      setIsSearching(false);
      addSystemMessage('Match found! Connecting to server...');
      platform.showNotification({ type: 'success', message: 'Match found!' });
    }, 5000);
  };

  const handleCancelMatchmaking = () => {
    setIsSearching(false);
    addSystemMessage('Matchmaking cancelled');
  };

  const handleJoinLobby = (lobby: Lobby) => {
    addSystemMessage(`Joining ${lobby.name}...`);
    // TODO: Implement actual lobby join
    platform.showNotification({
      type: 'info',
      message: `Joining ${lobby.name}`,
    });
  };

  const handleCreateLobby = () => {
    // TODO: Implement create lobby modal
    platform.showNotification({
      type: 'info',
      message: 'Create lobby feature coming soon',
    });
  };

  const handleInviteFriend = (friend: Player) => {
    addSystemMessage(`Invited ${friend.username} to party`);
    platform.showNotification({
      type: 'success',
      message: `Invited ${friend.username}`,
    });
  };

  const handleSendMessage = () => {
    if (!newMessage.trim()) return;

    const msg: Message = {
      id: Date.now().toString(),
      username: 'You',
      message: newMessage,
      timestamp: new Date().toLocaleTimeString(),
      type: 'chat',
    };

    setMessages([...messages, msg]);
    setNewMessage('');
  };

  const addSystemMessage = (text: string) => {
    const msg: Message = {
      id: Date.now().toString(),
      username: 'System',
      message: text,
      timestamp: new Date().toLocaleTimeString(),
      type: 'system',
    };
    setMessages([...messages, msg]);
  };

  return (
    <div className="multiplayer-redesigned">
      {/* Header with Stats */}
      <div className="mp-header">
        <div className="mp-branding">
          <h1 className="mp-title">MULTIPLAYER</h1>
          <div className="online-indicator">
            <span className="pulse-dot"></span>
            <span>{onlinePlayers.toLocaleString()} players online</span>
          </div>
        </div>

        <div className="player-quick-stats">
          <div className="quick-stat">
            <span className="stat-label">Rank</span>
            <span className="stat-value rank-badge">{playerStats.rank}</span>
          </div>
          <div className="quick-stat">
            <span className="stat-label">Win Rate</span>
            <span className="stat-value">{playerStats.winRate}%</span>
          </div>
          <div className="quick-stat">
            <span className="stat-label">Games</span>
            <span className="stat-value">{playerStats.gamesPlayed}</span>
          </div>
        </div>
      </div>

      {/* Navigation Tabs */}
      <div className="mp-tabs">
        <button
          className={`mp-tab ${activeTab === 'matchmaking' ? 'active' : ''}`}
          onClick={() => setActiveTab('matchmaking')}
        >
          <span className="tab-icon">🎯</span>
          <span>Matchmaking</span>
        </button>
        <button
          className={`mp-tab ${activeTab === 'lobbies' ? 'active' : ''}`}
          onClick={() => setActiveTab('lobbies')}
        >
          <span className="tab-icon">🏠</span>
          <span>Lobbies</span>
        </button>
        <button
          className={`mp-tab ${activeTab === 'friends' ? 'active' : ''}`}
          onClick={() => setActiveTab('friends')}
        >
          <span className="tab-icon">👥</span>
          <span>Friends</span>
        </button>
        <button
          className={`mp-tab ${activeTab === 'party' ? 'active' : ''}`}
          onClick={() => setActiveTab('party')}
        >
          <span className="tab-icon">🎉</span>
          <span>Party</span>
        </button>
      </div>

      {/* Main Content Area */}
      <div className="mp-content">
        <div className="mp-main">
          {/* Matchmaking Tab */}
          {activeTab === 'matchmaking' && (
            <div className="matchmaking-panel">
              <div className="matchmaking-config">
                <h3>Quick Match</h3>

                <div className="config-section">
                  <label>Select Game</label>
                  <select
                    value={selectedGame}
                    onChange={(e) => setSelectedGame(e.target.value)}
                  >
                    <option value="">Choose a game...</option>
                    <option value="space-warriors">Space Warriors</option>
                    <option value="cyber-racers">Cyber Racers</option>
                    <option value="nova-battle">Nova Battle Royale</option>
                  </select>
                </div>

                <div className="config-section">
                  <label>Game Mode</label>
                  <select
                    value={selectedMode}
                    onChange={(e) => setSelectedMode(e.target.value)}
                  >
                    <option value="">Choose mode...</option>
                    <option value="tdm">Team Deathmatch</option>
                    <option value="ffa">Free for All</option>
                    <option value="coop">Co-op</option>
                    <option value="ranked">Ranked</option>
                  </select>
                </div>

                <div className="config-section">
                  <label>Region</label>
                  <select
                    value={selectedRegion}
                    onChange={(e) => setSelectedRegion(e.target.value)}
                  >
                    <option value="auto">Auto (Best Ping)</option>
                    <option value="us-east">US East</option>
                    <option value="us-west">US West</option>
                    <option value="eu-west">EU West</option>
                    <option value="asia">Asia</option>
                  </select>
                </div>

                {isSearching ? (
                  <div className="searching-status">
                    <div className="search-spinner"></div>
                    <p>Searching for players...</p>
                    <p className="search-time">Estimated wait: 30s</p>
                    <button
                      onClick={handleCancelMatchmaking}
                      className="cancel-btn"
                    >
                      Cancel Search
                    </button>
                  </div>
                ) : (
                  <button
                    onClick={handleStartMatchmaking}
                    className="start-match-btn"
                  >
                    <span>🎮</span> Find Match
                  </button>
                )}
              </div>

              {/* Player Stats Detail */}
              <div className="stats-detail">
                <h3>Your Statistics</h3>
                <div className="stats-grid">
                  <div className="stat-card">
                    <div className="stat-icon">🏆</div>
                    <div className="stat-info">
                      <span className="stat-number">{playerStats.wins}</span>
                      <span className="stat-text">Wins</span>
                    </div>
                  </div>
                  <div className="stat-card">
                    <div className="stat-icon">❌</div>
                    <div className="stat-info">
                      <span className="stat-number">{playerStats.losses}</span>
                      <span className="stat-text">Losses</span>
                    </div>
                  </div>
                  <div className="stat-card">
                    <div className="stat-icon">📊</div>
                    <div className="stat-info">
                      <span className="stat-number">
                        {playerStats.avgScore}
                      </span>
                      <span className="stat-text">Avg Score</span>
                    </div>
                  </div>
                  <div className="stat-card">
                    <div className="stat-icon">⭐</div>
                    <div className="stat-info">
                      <span className="stat-number">
                        {playerStats.winRate}%
                      </span>
                      <span className="stat-text">Win Rate</span>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          )}

          {/* Lobbies Tab */}
          {activeTab === 'lobbies' && (
            <div className="lobbies-panel">
              <div className="lobbies-header">
                <h3>Available Lobbies</h3>
                <button
                  onClick={handleCreateLobby}
                  className="create-lobby-btn"
                >
                  + Create Lobby
                </button>
              </div>

              <div className="lobbies-filters">
                <input
                  type="text"
                  placeholder="Search lobbies..."
                  className="lobby-search"
                />
                <select className="lobby-filter">
                  <option>All Games</option>
                  <option>Space Warriors</option>
                  <option>Cyber Racers</option>
                </select>
                <select className="lobby-filter">
                  <option>All Regions</option>
                  <option>US East</option>
                  <option>EU West</option>
                </select>
              </div>

              <div className="lobbies-list">
                {lobbies.map((lobby) => (
                  <div key={lobby.id} className="lobby-card">
                    <div className="lobby-main-info">
                      <div className="lobby-title-section">
                        <h4>{lobby.name}</h4>
                        <span className="lobby-game">{lobby.game}</span>
                      </div>
                      <div className="lobby-details">
                        <span className="lobby-detail">
                          <span className="detail-icon">👤</span>
                          {lobby.players}/{lobby.maxPlayers}
                        </span>
                        <span className="lobby-detail">
                          <span className="detail-icon">🎮</span>
                          {lobby.mode}
                        </span>
                        <span className="lobby-detail">
                          <span className="detail-icon">🌍</span>
                          {lobby.region}
                        </span>
                        <span
                          className={`lobby-ping ${lobby.ping < 50 ? 'good' : lobby.ping < 100 ? 'medium' : 'bad'}`}
                        >
                          {lobby.ping}ms
                        </span>
                      </div>
                    </div>
                    <button
                      onClick={() => handleJoinLobby(lobby)}
                      className="join-lobby-btn"
                    >
                      Join
                    </button>
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* Friends Tab */}
          {activeTab === 'friends' && (
            <div className="friends-panel">
              <div className="friends-header">
                <h3>Friends List</h3>
                <button className="add-friend-btn">+ Add Friend</button>
              </div>

              <div className="friends-search">
                <input type="text" placeholder="Search friends..." />
              </div>

              <div className="friends-list">
                {friends.map((friend) => (
                  <div key={friend.id} className="friend-card">
                    <div className="friend-avatar">
                      <span className="avatar-emoji">{friend.avatar}</span>
                      <span
                        className={`status-indicator ${friend.status}`}
                      ></span>
                    </div>
                    <div className="friend-info">
                      <div className="friend-name-section">
                        <span className="friend-name">{friend.username}</span>
                        <span className="friend-level">Lvl {friend.level}</span>
                      </div>
                      <div className="friend-status-section">
                        <span className={`status-text ${friend.status}`}>
                          {friend.status === 'online'
                            ? '🟢 Online'
                            : friend.status === 'in-game'
                              ? '🎮 In Game'
                              : '🌙 Away'}
                        </span>
                        <span className="friend-rank">{friend.rank}</span>
                      </div>
                    </div>
                    <div className="friend-actions">
                      <button
                        onClick={() => handleInviteFriend(friend)}
                        className="friend-action-btn"
                        title="Invite"
                      >
                        📨
                      </button>
                      <button className="friend-action-btn" title="Message">
                        💬
                      </button>
                      <button
                        className="friend-action-btn"
                        title="View Profile"
                      >
                        👤
                      </button>
                    </div>
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* Party Tab */}
          {activeTab === 'party' && (
            <div className="party-panel">
              <div className="party-header">
                <h3>Your Party</h3>
                <button className="party-settings-btn">⚙️</button>
              </div>

              {party.length === 0 ? (
                <div className="empty-party">
                  <span className="empty-icon">🎉</span>
                  <h4>No Party Yet</h4>
                  <p>Invite friends to create a party and play together</p>
                  <button className="invite-friends-btn">Invite Friends</button>
                </div>
              ) : (
                <div className="party-members">
                  {party.map((member) => (
                    <div key={member.id} className="party-member">
                      <span className="member-avatar">{member.avatar}</span>
                      <div className="member-info">
                        <span className="member-name">{member.username}</span>
                        <span className="member-status">{member.status}</span>
                      </div>
                      <button className="kick-btn">✕</button>
                    </div>
                  ))}
                </div>
              )}

              <div className="party-chat">
                <h4>Party Chat</h4>
                <div className="chat-messages-small">
                  <p className="system-msg">Party chat is ready!</p>
                </div>
              </div>
            </div>
          )}
        </div>

        {/* Chat Sidebar */}
        <div className="mp-chat">
          <div className="chat-header">
            <h4>💬 Chat</h4>
            <div className="chat-tabs">
              <button className="chat-tab active">Global</button>
              <button className="chat-tab">Party</button>
            </div>
          </div>

          <div className="chat-messages">
            {messages.map((msg) => (
              <div key={msg.id} className={`chat-message ${msg.type}`}>
                <span className="msg-time">[{msg.timestamp}]</span>
                <span className="msg-user">{msg.username}:</span>
                <span className="msg-text">{msg.message}</span>
              </div>
            ))}
          </div>

          <div className="chat-input">
            <input
              type="text"
              placeholder="Type a message..."
              value={newMessage}
              onChange={(e) => setNewMessage(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleSendMessage()}
            />
            <button onClick={handleSendMessage}>Send</button>
          </div>
        </div>
      </div>

      <style>{`
        .multiplayer-redesigned {
          width: 100%;
          min-height: 100vh;
          background: radial-gradient(ellipse at center, #1a0033 0%, #000000 70%);
          color: white;
          display: flex;
          flex-direction: column;
        }

        /* Header */
        .mp-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 30px 40px;
          background: linear-gradient(180deg, rgba(26,0,51,0.8) 0%, transparent 100%);
          border-bottom: 2px solid rgba(123, 47, 247, 0.3);
        }

        .mp-branding {
          display: flex;
          align-items: center;
          gap: 30px;
        }

        .mp-title {
          font-size: 36px;
          font-weight: 900;
          letter-spacing: 4px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          margin: 0;
        }

        .online-indicator {
          display: flex;
          align-items: center;
          gap: 10px;
          padding: 8px 16px;
          background: rgba(26, 0, 51, 0.6);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
        }

        .pulse-dot {
          width: 10px;
          height: 10px;
          background: #4ade80;
          border-radius: 50%;
          animation: pulse 2s infinite;
        }

        @keyframes pulse {
          0%, 100% { opacity: 1; transform: scale(1); }
          50% { opacity: 0.5; transform: scale(1.2); }
        }

        .player-quick-stats {
          display: flex;
          gap: 30px;
        }

        .quick-stat {
          display: flex;
          flex-direction: column;
          align-items: center;
          gap: 5px;
        }

        .stat-label {
          font-size: 12px;
          color: rgba(255, 255, 255, 0.6);
          text-transform: uppercase;
          letter-spacing: 1px;
        }

        .stat-value {
          font-size: 20px;
          font-weight: 700;
          color: #4cc9f0;
        }

        .rank-badge {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
        }

        /* Tabs */
        .mp-tabs {
          display: flex;
          gap: 5px;
          padding: 0 40px;
          background: rgba(26, 0, 51, 0.4);
          border-bottom: 2px solid rgba(123, 47, 247, 0.2);
        }

        .mp-tab {
          display: flex;
          align-items: center;
          gap: 10px;
          padding: 16px 24px;
          background: transparent;
          border: none;
          color: rgba(255, 255, 255, 0.6);
          cursor: pointer;
          font-size: 15px;
          font-weight: 600;
          border-bottom: 3px solid transparent;
          transition: all 0.3s ease;
        }

        .mp-tab:hover {
          color: white;
          background: rgba(123, 47, 247, 0.1);
        }

        .mp-tab.active {
          color: white;
          border-bottom-color: #7b2ff7;
          background: rgba(123, 47, 247, 0.2);
        }

        .tab-icon {
          font-size: 20px;
        }

        /* Content */
        .mp-content {
          flex: 1;
          display: flex;
          overflow: hidden;
        }

        .mp-main {
          flex: 1;
          overflow-y: auto;
          padding: 30px 40px;
        }

        /* Matchmaking */
        .matchmaking-panel {
          display: grid;
          grid-template-columns: 1fr 1fr;
          gap: 30px;
          max-width: 1400px;
          margin: 0 auto;
        }

        .matchmaking-config {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
          padding: 30px;
        }

        .matchmaking-config h3 {
          margin: 0 0 25px 0;
          font-size: 24px;
        }

        .config-section {
          margin-bottom: 20px;
        }

        .config-section label {
          display: block;
          margin-bottom: 8px;
          font-size: 14px;
          font-weight: 600;
          color: rgba(255, 255, 255, 0.9);
        }

        .config-section select {
          width: 100%;
          padding: 12px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          font-size: 15px;
          cursor: pointer;
          transition: all 0.3s ease;
        }

        .config-section select:focus {
          outline: none;
          border-color: rgba(123, 47, 247, 0.8);
          box-shadow: 0 0 20px rgba(123, 47, 247, 0.3);
        }

        .start-match-btn {
          width: 100%;
          padding: 16px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 12px;
          color: white;
          font-size: 18px;
          font-weight: 700;
          cursor: pointer;
          margin-top: 10px;
          display: flex;
          align-items: center;
          justify-content: center;
          gap: 10px;
          transition: transform 0.2s ease;
        }

        .start-match-btn:hover {
          transform: scale(1.02);
        }

        .searching-status {
          text-align: center;
          padding: 30px 0;
        }

        .search-spinner {
          width: 60px;
          height: 60px;
          border: 4px solid rgba(123, 47, 247, 0.3);
          border-top-color: #7b2ff7;
          border-radius: 50%;
          animation: spin 1s linear infinite;
          margin: 0 auto 20px;
        }

        @keyframes spin {
          to { transform: rotate(360deg); }
        }

        .search-time {
          font-size: 14px;
          color: rgba(255, 255, 255, 0.6);
          margin: 10px 0 20px 0;
        }

        .cancel-btn {
          padding: 12px 30px;
          background: rgba(239, 68, 68, 0.3);
          border: 2px solid rgba(239, 68, 68, 0.5);
          border-radius: 12px;
          color: white;
          cursor: pointer;
          transition: all 0.2s ease;
        }

        .cancel-btn:hover {
          background: rgba(239, 68, 68, 0.5);
        }

        /* Stats Detail */
        .stats-detail {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
          padding: 30px;
        }

        .stats-detail h3 {
          margin: 0 0 25px 0;
          font-size: 24px;
        }

        .stats-grid {
          display: grid;
          grid-template-columns: repeat(2, 1fr);
          gap: 20px;
        }

        .stat-card {
          background: rgba(123, 47, 247, 0.1);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          padding: 20px;
          display: flex;
          align-items: center;
          gap: 15px;
        }

        .stat-icon {
          font-size: 36px;
        }

        .stat-info {
          display: flex;
          flex-direction: column;
        }

        .stat-number {
          font-size: 28px;
          font-weight: 700;
          color: #4cc9f0;
        }

        .stat-text {
          font-size: 13px;
          color: rgba(255, 255, 255, 0.6);
          text-transform: uppercase;
        }

        /* Lobbies */
        .lobbies-panel {
          max-width: 1400px;
          margin: 0 auto;
        }

        .lobbies-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 25px;
        }

        .lobbies-header h3 {
          font-size: 28px;
          margin: 0;
        }

        .create-lobby-btn {
          padding: 12px 24px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 12px;
          color: white;
          font-weight: 700;
          cursor: pointer;
        }

        .lobbies-filters {
          display: flex;
          gap: 15px;
          margin-bottom: 25px;
        }

        .lobby-search {
          flex: 1;
          padding: 12px 18px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          font-size: 15px;
        }

        .lobby-filter {
          padding: 12px 18px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          cursor: pointer;
        }

        .lobbies-list {
          display: flex;
          flex-direction: column;
          gap: 15px;
        }

        .lobby-card {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          padding: 20px;
          display: flex;
          justify-content: space-between;
          align-items: center;
          transition: all 0.3s ease;
        }

        .lobby-card:hover {
          border-color: rgba(123, 47, 247, 0.6);
          box-shadow: 0 4px 20px rgba(123, 47, 247, 0.3);
        }

        .lobby-main-info {
          flex: 1;
        }

        .lobby-title-section h4 {
          margin: 0 0 5px 0;
          font-size: 18px;
        }

        .lobby-game {
          font-size: 13px;
          color: rgba(123, 47, 247, 0.9);
        }

        .lobby-details {
          display: flex;
          gap: 20px;
          margin-top: 12px;
          font-size: 14px;
        }

        .lobby-detail {
          display: flex;
          align-items: center;
          gap: 5px;
          color: rgba(255, 255, 255, 0.7);
        }

        .detail-icon {
          font-size: 16px;
        }

        .lobby-ping {
          padding: 4px 10px;
          border-radius: 12px;
          font-weight: 600;
        }

        .lobby-ping.good {
          background: rgba(74, 222, 128, 0.2);
          color: #4ade80;
        }

        .lobby-ping.medium {
          background: rgba(251, 191, 36, 0.2);
          color: #fbbf24;
        }

        .lobby-ping.bad {
          background: rgba(239, 68, 68, 0.2);
          color: #ef4444;
        }

        .join-lobby-btn {
          padding: 12px 30px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 12px;
          color: white;
          font-weight: 700;
          cursor: pointer;
        }

        /* Friends */
        .friends-panel {
          max-width: 1200px;
          margin: 0 auto;
        }

        .friends-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 20px;
        }

        .friends-header h3 {
          font-size: 28px;
          margin: 0;
        }

        .add-friend-btn {
          padding: 12px 24px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 12px;
          color: white;
          font-weight: 700;
          cursor: pointer;
        }

        .friends-search {
          margin-bottom: 25px;
        }

        .friends-search input {
          width: 100%;
          padding: 12px 18px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          font-size: 15px;
        }

        .friends-list {
          display: flex;
          flex-direction: column;
          gap: 15px;
        }

        .friend-card {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          padding: 20px;
          display: flex;
          align-items: center;
          gap: 20px;
          transition: all 0.3s ease;
        }

        .friend-card:hover {
          border-color: rgba(123, 47, 247, 0.6);
          box-shadow: 0 4px 20px rgba(123, 47, 247, 0.3);
        }

        .friend-avatar {
          position: relative;
          width: 60px;
          height: 60px;
          background: rgba(123, 47, 247, 0.2);
          border-radius: 50%;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 30px;
        }

        .status-indicator {
          position: absolute;
          bottom: 2px;
          right: 2px;
          width: 14px;
          height: 14px;
          border-radius: 50%;
          border: 2px solid rgba(26, 0, 51, 0.8);
        }

        .status-indicator.online {
          background: #4ade80;
        }

        .status-indicator.in-game {
          background: #fbbf24;
        }

        .status-indicator.away {
          background: #94a3b8;
        }

        .friend-info {
          flex: 1;
        }

        .friend-name-section {
          display: flex;
          align-items: baseline;
          gap: 10px;
          margin-bottom: 5px;
        }

        .friend-name {
          font-size: 18px;
          font-weight: 700;
        }

        .friend-level {
          font-size: 13px;
          color: rgba(123, 47, 247, 0.9);
        }

        .friend-status-section {
          display: flex;
          align-items: center;
          gap: 15px;
          font-size: 14px;
        }

        .status-text {
          color: rgba(255, 255, 255, 0.7);
        }

        .status-text.online {
          color: #4ade80;
        }

        .friend-rank {
          color: rgba(255, 255, 255, 0.6);
        }

        .friend-actions {
          display: flex;
          gap: 10px;
        }

        .friend-action-btn {
          width: 40px;
          height: 40px;
          background: rgba(123, 47, 247, 0.2);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 10px;
          color: white;
          font-size: 18px;
          cursor: pointer;
          transition: all 0.2s ease;
        }

        .friend-action-btn:hover {
          background: rgba(123, 47, 247, 0.4);
        }

        /* Party */
        .party-panel {
          max-width: 800px;
          margin: 0 auto;
        }

        .party-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 25px;
        }

        .party-header h3 {
          font-size: 28px;
          margin: 0;
        }

        .party-settings-btn {
          width: 40px;
          height: 40px;
          background: rgba(123, 47, 247, 0.2);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 10px;
          color: white;
          font-size: 20px;
          cursor: pointer;
        }

        .empty-party {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
          padding: 60px 40px;
          text-align: center;
        }

        .empty-icon {
          font-size: 80px;
          display: block;
          margin-bottom: 20px;
        }

        .empty-party h4 {
          font-size: 24px;
          margin: 0 0 10px 0;
        }

        .empty-party p {
          font-size: 16px;
          color: rgba(255, 255, 255, 0.6);
          margin: 0 0 30px 0;
        }

        .invite-friends-btn {
          padding: 14px 32px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 12px;
          color: white;
          font-size: 16px;
          font-weight: 700;
          cursor: pointer;
        }

        /* Chat */
        .mp-chat {
          width: 350px;
          background: rgba(26, 0, 51, 0.8);
          border-left: 2px solid rgba(123, 47, 247, 0.3);
          display: flex;
          flex-direction: column;
        }

        .chat-header {
          padding: 20px;
          border-bottom: 1px solid rgba(123, 47, 247, 0.3);
        }

        .chat-header h4 {
          margin: 0 0 15px 0;
          font-size: 18px;
        }

        .chat-tabs {
          display: flex;
          gap: 5px;
        }

        .chat-tab {
          flex: 1;
          padding: 8px;
          background: transparent;
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 8px;
          color: rgba(255, 255, 255, 0.6);
          cursor: pointer;
          font-size: 13px;
          transition: all 0.2s ease;
        }

        .chat-tab.active {
          background: rgba(123, 47, 247, 0.3);
          color: white;
        }

        .chat-messages {
          flex: 1;
          overflow-y: auto;
          padding: 15px;
          font-size: 13px;
        }

        .chat-message {
          margin-bottom: 10px;
          padding: 8px;
          border-radius: 8px;
          line-height: 1.5;
        }

        .chat-message.system {
          background: rgba(123, 47, 247, 0.1);
          color: rgba(123, 47, 247, 0.9);
        }

        .msg-time {
          color: rgba(255, 255, 255, 0.4);
          font-size: 11px;
          margin-right: 5px;
        }

        .msg-user {
          font-weight: 700;
          margin-right: 5px;
        }

        .msg-text {
          color: rgba(255, 255, 255, 0.9);
        }

        .chat-input {
          display: flex;
          gap: 10px;
          padding: 15px;
          border-top: 1px solid rgba(123, 47, 247, 0.3);
        }

        .chat-input input {
          flex: 1;
          padding: 10px;
          background: rgba(26, 0, 51, 0.6);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 8px;
          color: white;
          font-size: 14px;
        }

        .chat-input button {
          padding: 10px 20px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 8px;
          color: white;
          font-weight: 700;
          cursor: pointer;
        }

        @media (max-width: 1200px) {
          .matchmaking-panel {
            grid-template-columns: 1fr;
          }
          
          .mp-content {
            flex-direction: column;
          }
          
          .mp-chat {
            width: 100%;
            height: 300px;
          }
        }
      `}</style>
    </div>
  );
};
export { MultiplayerModuleRedesigned as MultiplayerModule } from './MultiplayerModule';
