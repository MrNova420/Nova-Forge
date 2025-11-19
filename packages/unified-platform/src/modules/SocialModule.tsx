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
 * NOVA ENGINE - SOCIAL MODULE
 * Complete social system with profiles, achievements, leaderboards, activity feed, groups
 */

import React, { useState } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface SocialModuleProps {
  platform: UnifiedPlatformCore;
}

interface Achievement {
  id: string;
  name: string;
  description: string;
  icon: string;
  rarity: 'common' | 'rare' | 'epic' | 'legendary';
  progress: number;
  maxProgress: number;
  unlocked: boolean;
  unlockedDate?: string;
  points: number;
}

interface Activity {
  id: string;
  user: string;
  avatar: string;
  action: string;
  timestamp: string;
  type: 'achievement' | 'game' | 'friend' | 'milestone';
}

interface LeaderboardEntry {
  rank: number;
  user: string;
  avatar: string;
  score: number;
  level: number;
  trend: 'up' | 'down' | 'same';
}

export const SocialModuleRedesigned: React.FC<SocialModuleProps> = ({
  platform: _platform,
}) => {
  const [activeTab, setActiveTab] = useState<
    'profile' | 'achievements' | 'leaderboard' | 'activity' | 'groups'
  >('profile');

  const [userProfile] = useState({
    username: 'NovaPlayer',
    level: 45,
    experience: 15670,
    nextLevelXP: 20000,
    rank: 'Diamond III',
    joinDate: 'January 2024',
    gamesPlayed: 156,
    hoursPlayed: 234,
    achievements: 47,
    totalAchievements: 150,
    friends: 23,
    bio: 'Passionate game developer and Nova Engine enthusiast! Always looking for new challenges.',
    favoriteGames: ['Space Warriors', 'Cyber Racers', 'Nova Battle Royale'],
  });

  const [achievements] = useState<Achievement[]>([
    {
      id: '1',
      name: 'First Steps',
      description: 'Play your first game',
      icon: '🎮',
      rarity: 'common',
      progress: 1,
      maxProgress: 1,
      unlocked: true,
      unlockedDate: 'Jan 15, 2024',
      points: 10,
    },
    {
      id: '2',
      name: 'Victory!',
      description: 'Win 10 matches',
      icon: '🏆',
      rarity: 'rare',
      progress: 10,
      maxProgress: 10,
      unlocked: true,
      unlockedDate: 'Jan 20, 2024',
      points: 25,
    },
    {
      id: '3',
      name: 'Social Butterfly',
      description: 'Add 20 friends',
      icon: '🦋',
      rarity: 'rare',
      progress: 23,
      maxProgress: 20,
      unlocked: true,
      unlockedDate: 'Feb 5, 2024',
      points: 25,
    },
    {
      id: '4',
      name: 'Legendary Player',
      description: 'Reach Diamond rank',
      icon: '💎',
      rarity: 'epic',
      progress: 1,
      maxProgress: 1,
      unlocked: true,
      unlockedDate: 'Mar 1, 2024',
      points: 50,
    },
    {
      id: '5',
      name: 'Game Developer',
      description: 'Create 5 games in editor',
      icon: '👨‍💻',
      rarity: 'epic',
      progress: 3,
      maxProgress: 5,
      unlocked: false,
      points: 50,
    },
    {
      id: '6',
      name: 'Marathon Runner',
      description: 'Play for 100 hours',
      icon: '⏱️',
      rarity: 'rare',
      progress: 234,
      maxProgress: 100,
      unlocked: true,
      unlockedDate: 'Feb 28, 2024',
      points: 30,
    },
    {
      id: '7',
      name: 'Perfect Score',
      description: 'Get a perfect game score',
      icon: '💯',
      rarity: 'legendary',
      progress: 0,
      maxProgress: 1,
      unlocked: false,
      points: 100,
    },
    {
      id: '8',
      name: 'Team Player',
      description: 'Win 50 team matches',
      icon: '🤝',
      rarity: 'epic',
      progress: 28,
      maxProgress: 50,
      unlocked: false,
      points: 50,
    },
  ]);

  const [activities] = useState<Activity[]>([
    {
      id: '1',
      user: 'SkillMaster',
      avatar: '👨',
      action: 'unlocked Achievement: Victory Master',
      timestamp: '2 hours ago',
      type: 'achievement',
    },
    {
      id: '2',
      user: 'NovaQueen',
      avatar: '👩',
      action: 'reached level 70',
      timestamp: '3 hours ago',
      type: 'milestone',
    },
    {
      id: '3',
      user: 'You',
      avatar: '😎',
      action: 'won a match in Space Warriors',
      timestamp: '5 hours ago',
      type: 'game',
    },
    {
      id: '4',
      user: 'CyberNinja',
      avatar: '🥷',
      action: 'became friends with TacticalMind',
      timestamp: '1 day ago',
      type: 'friend',
    },
    {
      id: '5',
      user: 'SpeedDemon',
      avatar: '⚡',
      action: 'set a new high score: 9,450',
      timestamp: '1 day ago',
      type: 'game',
    },
  ]);

  const [leaderboard] = useState<LeaderboardEntry[]>([
    {
      rank: 1,
      user: 'ProGamer123',
      avatar: '👑',
      score: 15840,
      level: 89,
      trend: 'same',
    },
    {
      rank: 2,
      user: 'ElitePlayer',
      avatar: '⭐',
      score: 15320,
      level: 85,
      trend: 'up',
    },
    {
      rank: 3,
      user: 'MasterChief',
      avatar: '🎖️',
      score: 14990,
      level: 82,
      trend: 'down',
    },
    {
      rank: 4,
      user: 'SkillMaster',
      avatar: '👨',
      score: 14560,
      level: 80,
      trend: 'up',
    },
    {
      rank: 5,
      user: 'NovaQueen',
      avatar: '👩',
      score: 13870,
      level: 78,
      trend: 'up',
    },
    {
      rank: 12,
      user: 'You (NovaPlayer)',
      avatar: '😎',
      score: 11240,
      level: 45,
      trend: 'up',
    },
  ]);

  const rarityColors = {
    common: '#94a3b8',
    rare: '#3b82f6',
    epic: '#a855f7',
    legendary: '#f59e0b',
  };

  return (
    <div className="social-redesigned">
      {/* Header */}
      <div className="social-header">
        <div className="social-branding">
          <h1 className="social-title">SOCIAL</h1>
          <p className="social-subtitle">Connect, Compete, Achieve</p>
        </div>
      </div>

      {/* Tabs */}
      <div className="social-tabs">
        <button
          className={`social-tab ${activeTab === 'profile' ? 'active' : ''}`}
          onClick={() => setActiveTab('profile')}
        >
          <span className="tab-icon">👤</span>
          <span>Profile</span>
        </button>
        <button
          className={`social-tab ${activeTab === 'achievements' ? 'active' : ''}`}
          onClick={() => setActiveTab('achievements')}
        >
          <span className="tab-icon">🏆</span>
          <span>Achievements</span>
        </button>
        <button
          className={`social-tab ${activeTab === 'leaderboard' ? 'active' : ''}`}
          onClick={() => setActiveTab('leaderboard')}
        >
          <span className="tab-icon">📊</span>
          <span>Leaderboard</span>
        </button>
        <button
          className={`social-tab ${activeTab === 'activity' ? 'active' : ''}`}
          onClick={() => setActiveTab('activity')}
        >
          <span className="tab-icon">📰</span>
          <span>Activity Feed</span>
        </button>
        <button
          className={`social-tab ${activeTab === 'groups' ? 'active' : ''}`}
          onClick={() => setActiveTab('groups')}
        >
          <span className="tab-icon">👥</span>
          <span>Groups</span>
        </button>
      </div>

      {/* Content */}
      <div className="social-content">
        {/* Profile Tab */}
        {activeTab === 'profile' && (
          <div className="profile-panel">
            <div className="profile-hero">
              <div className="profile-banner"></div>
              <div className="profile-info-card">
                <div className="profile-avatar">
                  <span className="avatar-large">😎</span>
                  <span className="profile-level">Lvl {userProfile.level}</span>
                </div>
                <div className="profile-details">
                  <h2>{userProfile.username}</h2>
                  <p className="profile-rank">{userProfile.rank}</p>
                  <p className="profile-join-date">
                    Joined {userProfile.joinDate}
                  </p>
                  <p className="profile-bio">{userProfile.bio}</p>
                  <button className="edit-profile-btn">✏️ Edit Profile</button>
                </div>
              </div>
            </div>

            {/* XP Progress */}
            <div className="xp-section">
              <div className="xp-header">
                <span>Level {userProfile.level}</span>
                <span>
                  {userProfile.experience.toLocaleString()} /{' '}
                  {userProfile.nextLevelXP.toLocaleString()} XP
                </span>
              </div>
              <div className="xp-bar">
                <div
                  className="xp-fill"
                  style={{
                    width: `${(userProfile.experience / userProfile.nextLevelXP) * 100}%`,
                  }}
                ></div>
              </div>
            </div>

            {/* Stats Grid */}
            <div className="stats-grid-social">
              <div className="stat-card-social">
                <div className="stat-icon-social">🎮</div>
                <div className="stat-info-social">
                  <span className="stat-number-social">
                    {userProfile.gamesPlayed}
                  </span>
                  <span className="stat-label-social">Games Played</span>
                </div>
              </div>
              <div className="stat-card-social">
                <div className="stat-icon-social">⏱️</div>
                <div className="stat-info-social">
                  <span className="stat-number-social">
                    {userProfile.hoursPlayed}
                  </span>
                  <span className="stat-label-social">Hours Played</span>
                </div>
              </div>
              <div className="stat-card-social">
                <div className="stat-icon-social">🏆</div>
                <div className="stat-info-social">
                  <span className="stat-number-social">
                    {userProfile.achievements}/{userProfile.totalAchievements}
                  </span>
                  <span className="stat-label-social">Achievements</span>
                </div>
              </div>
              <div className="stat-card-social">
                <div className="stat-icon-social">👥</div>
                <div className="stat-info-social">
                  <span className="stat-number-social">
                    {userProfile.friends}
                  </span>
                  <span className="stat-label-social">Friends</span>
                </div>
              </div>
            </div>

            {/* Favorite Games */}
            <div className="favorite-games-section">
              <h3>Favorite Games</h3>
              <div className="favorite-games-grid">
                {userProfile.favoriteGames.map((game, idx) => (
                  <div key={idx} className="favorite-game-card">
                    <div className="game-icon">🎮</div>
                    <span>{game}</span>
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}

        {/* Achievements Tab */}
        {activeTab === 'achievements' && (
          <div className="achievements-panel">
            <div className="achievements-header">
              <h3>Your Achievements</h3>
              <div className="achievement-stats">
                <span>
                  {achievements.filter((a) => a.unlocked).length} /{' '}
                  {achievements.length} Unlocked
                </span>
                <span>
                  {achievements.reduce(
                    (sum, a) => sum + (a.unlocked ? a.points : 0),
                    0
                  )}{' '}
                  Points
                </span>
              </div>
            </div>

            {/* Achievement Filters */}
            <div className="achievement-filters">
              <button className="filter-btn active">All</button>
              <button className="filter-btn">Unlocked</button>
              <button className="filter-btn">Locked</button>
              <button className="filter-btn">Common</button>
              <button className="filter-btn">Rare</button>
              <button className="filter-btn">Epic</button>
              <button className="filter-btn">Legendary</button>
            </div>

            {/* Achievements Grid */}
            <div className="achievements-grid">
              {achievements.map((achievement) => (
                <div
                  key={achievement.id}
                  className={`achievement-card ${achievement.unlocked ? 'unlocked' : 'locked'}`}
                  style={
                    {
                      '--rarity-color': rarityColors[achievement.rarity],
                    } as any
                  }
                >
                  <div className="achievement-icon">{achievement.icon}</div>
                  <div className="achievement-info">
                    <h4>{achievement.name}</h4>
                    <p>{achievement.description}</p>
                    <div className="achievement-meta">
                      <span
                        className="achievement-rarity"
                        style={{ color: rarityColors[achievement.rarity] }}
                      >
                        {achievement.rarity.toUpperCase()}
                      </span>
                      <span className="achievement-points">
                        {achievement.points} pts
                      </span>
                    </div>
                    {achievement.unlocked ? (
                      <span className="achievement-date">
                        Unlocked: {achievement.unlockedDate}
                      </span>
                    ) : (
                      <div className="achievement-progress">
                        <div className="progress-bar-small">
                          <div
                            className="progress-fill-small"
                            style={{
                              width: `${(achievement.progress / achievement.maxProgress) * 100}%`,
                            }}
                          ></div>
                        </div>
                        <span>
                          {achievement.progress} / {achievement.maxProgress}
                        </span>
                      </div>
                    )}
                  </div>
                  {achievement.unlocked && (
                    <div className="unlock-badge">✓</div>
                  )}
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Leaderboard Tab */}
        {activeTab === 'leaderboard' && (
          <div className="leaderboard-panel">
            <div className="leaderboard-header">
              <h3>Global Leaderboard</h3>
              <select className="leaderboard-filter">
                <option>All Time</option>
                <option>This Month</option>
                <option>This Week</option>
                <option>Today</option>
              </select>
            </div>

            <div className="leaderboard-table">
              <div className="leaderboard-table-header">
                <span className="col-rank">Rank</span>
                <span className="col-player">Player</span>
                <span className="col-score">Score</span>
                <span className="col-level">Level</span>
                <span className="col-trend">Trend</span>
              </div>
              {leaderboard.map((entry) => (
                <div
                  key={entry.rank}
                  className={`leaderboard-row ${entry.user.includes('You') ? 'highlight' : ''} ${entry.rank <= 3 ? 'top-three' : ''}`}
                >
                  <span className="col-rank">
                    {entry.rank <= 3 ? (
                      <span className={`medal medal-${entry.rank}`}>
                        {entry.rank === 1
                          ? '🥇'
                          : entry.rank === 2
                            ? '🥈'
                            : '🥉'}
                      </span>
                    ) : (
                      `#${entry.rank}`
                    )}
                  </span>
                  <span className="col-player">
                    <span className="player-avatar">{entry.avatar}</span>
                    <span className="player-name">{entry.user}</span>
                  </span>
                  <span className="col-score">
                    {entry.score.toLocaleString()}
                  </span>
                  <span className="col-level">{entry.level}</span>
                  <span className="col-trend">
                    {entry.trend === 'up' && (
                      <span className="trend-up">↑</span>
                    )}
                    {entry.trend === 'down' && (
                      <span className="trend-down">↓</span>
                    )}
                    {entry.trend === 'same' && (
                      <span className="trend-same">━</span>
                    )}
                  </span>
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Activity Feed Tab */}
        {activeTab === 'activity' && (
          <div className="activity-panel">
            <div className="activity-header">
              <h3>Recent Activity</h3>
              <div className="activity-filters">
                <button className="activity-filter-btn active">All</button>
                <button className="activity-filter-btn">Friends</button>
                <button className="activity-filter-btn">Achievements</button>
                <button className="activity-filter-btn">Games</button>
              </div>
            </div>

            <div className="activity-feed">
              {activities.map((activity) => (
                <div
                  key={activity.id}
                  className={`activity-item ${activity.type}`}
                >
                  <div className="activity-avatar">{activity.avatar}</div>
                  <div className="activity-content">
                    <p>
                      <span className="activity-user">{activity.user}</span>{' '}
                      <span className="activity-action">{activity.action}</span>
                    </p>
                    <span className="activity-time">{activity.timestamp}</span>
                  </div>
                  <div className="activity-type-icon">
                    {activity.type === 'achievement' && '🏆'}
                    {activity.type === 'game' && '🎮'}
                    {activity.type === 'friend' && '👥'}
                    {activity.type === 'milestone' && '⭐'}
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Groups Tab */}
        {activeTab === 'groups' && (
          <div className="groups-panel">
            <div className="groups-header">
              <h3>Groups & Communities</h3>
              <button className="create-group-btn">+ Create Group</button>
            </div>

            <div className="groups-placeholder">
              <span className="placeholder-icon">👥</span>
              <h4>Coming Soon</h4>
              <p>
                Create and join groups to connect with players who share your
                interests
              </p>
            </div>
          </div>
        )}
      </div>

      <style>{`
        .social-redesigned {
          width: 100%;
          min-height: 100vh;
          background: radial-gradient(ellipse at center, #1a0033 0%, #000000 70%);
          color: white;
          display: flex;
          flex-direction: column;
        }

        /* Header */
        .social-header {
          padding: 30px 40px;
          background: linear-gradient(180deg, rgba(26,0,51,0.8) 0%, transparent 100%);
          border-bottom: 2px solid rgba(123, 47, 247, 0.3);
        }

        .social-title {
          font-size: 36px;
          font-weight: 900;
          letter-spacing: 4px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          margin: 0 0 5px 0;
        }

        .social-subtitle {
          font-size: 16px;
          color: rgba(255, 255, 255, 0.7);
          margin: 0;
        }

        /* Tabs */
        .social-tabs {
          display: flex;
          gap: 5px;
          padding: 0 40px;
          background: rgba(26, 0, 51, 0.4);
          border-bottom: 2px solid rgba(123, 47, 247, 0.2);
        }

        .social-tab {
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

        .social-tab:hover {
          color: white;
          background: rgba(123, 47, 247, 0.1);
        }

        .social-tab.active {
          color: white;
          border-bottom-color: #7b2ff7;
          background: rgba(123, 47, 247, 0.2);
        }

        /* Content */
        .social-content {
          flex: 1;
          overflow-y: auto;
          padding: 30px 40px;
        }

        /* Profile */
        .profile-panel {
          max-width: 1200px;
          margin: 0 auto;
        }

        .profile-hero {
          position: relative;
          margin-bottom: 30px;
        }

        .profile-banner {
          height: 200px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border-radius: 20px 20px 0 0;
        }

        .profile-info-card {
          display: flex;
          gap: 30px;
          padding: 30px;
          background: linear-gradient(135deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
          margin-top: -80px;
          position: relative;
          z-index: 1;
        }

        .profile-avatar {
          position: relative;
          display: flex;
          flex-direction: column;
          align-items: center;
        }

        .avatar-large {
          width: 140px;
          height: 140px;
          background: linear-gradient(135deg, rgba(123, 47, 247, 0.3) 0%, rgba(255, 110, 196, 0.3) 100%);
          border: 4px solid rgba(123, 47, 247, 0.5);
          border-radius: 50%;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 70px;
        }

        .profile-level {
          margin-top: 10px;
          padding: 6px 16px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border-radius: 20px;
          font-weight: 700;
          font-size: 14px;
        }

        .profile-details {
          flex: 1;
        }

        .profile-details h2 {
          font-size: 32px;
          margin: 0 0 8px 0;
        }

        .profile-rank {
          font-size: 18px;
          color: #4cc9f0;
          margin: 0 0 5px 0;
        }

        .profile-join-date {
          font-size: 14px;
          color: rgba(255, 255, 255, 0.6);
          margin: 0 0 15px 0;
        }

        .profile-bio {
          font-size: 15px;
          line-height: 1.6;
          color: rgba(255, 255, 255, 0.8);
          margin: 0 0 20px 0;
        }

        .edit-profile-btn {
          padding: 10px 24px;
          background: rgba(123, 47, 247, 0.3);
          border: 2px solid rgba(123, 47, 247, 0.5);
          border-radius: 12px;
          color: white;
          font-weight: 600;
          cursor: pointer;
          transition: all 0.2s ease;
        }

        .edit-profile-btn:hover {
          background: rgba(123, 47, 247, 0.5);
        }

        .xp-section {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          padding: 20px;
          margin-bottom: 30px;
        }

        .xp-header {
          display: flex;
          justify-content: space-between;
          margin-bottom: 12px;
          font-size: 14px;
          font-weight: 600;
        }

        .xp-bar {
          height: 24px;
          background: rgba(26, 0, 51, 0.6);
          border-radius: 12px;
          overflow: hidden;
          border: 1px solid rgba(123, 47, 247, 0.3);
        }

        .xp-fill {
          height: 100%;
          background: linear-gradient(90deg, #7b2ff7 0%, #ff6ec4 100%);
          transition: width 0.3s ease;
        }

        .stats-grid-social {
          display: grid;
          grid-template-columns: repeat(4, 1fr);
          gap: 20px;
          margin-bottom: 30px;
        }

        .stat-card-social {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          padding: 24px;
          display: flex;
          align-items: center;
          gap: 16px;
        }

        .stat-icon-social {
          font-size: 36px;
        }

        .stat-info-social {
          display: flex;
          flex-direction: column;
        }

        .stat-number-social {
          font-size: 24px;
          font-weight: 700;
          color: #4cc9f0;
        }

        .stat-label-social {
          font-size: 13px;
          color: rgba(255, 255, 255, 0.6);
          text-transform: uppercase;
        }

        .favorite-games-section {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          padding: 24px;
        }

        .favorite-games-section h3 {
          margin: 0 0 20px 0;
          font-size: 20px;
        }

        .favorite-games-grid {
          display: grid;
          grid-template-columns: repeat(3, 1fr);
          gap: 15px;
        }

        .favorite-game-card {
          background: rgba(123, 47, 247, 0.2);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          padding: 16px;
          display: flex;
          align-items: center;
          gap: 12px;
          font-size: 14px;
          font-weight: 600;
        }

        .game-icon {
          font-size: 24px;
        }

        /* Achievements */
        .achievements-panel {
          max-width: 1400px;
          margin: 0 auto;
        }

        .achievements-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 25px;
        }

        .achievements-header h3 {
          font-size: 28px;
          margin: 0;
        }

        .achievement-stats {
          display: flex;
          gap: 30px;
          font-size: 15px;
          color: rgba(255, 255, 255, 0.7);
        }

        .achievement-filters {
          display: flex;
          gap: 10px;
          margin-bottom: 25px;
          flex-wrap: wrap;
        }

        .filter-btn {
          padding: 8px 18px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
          color: white;
          cursor: pointer;
          font-size: 14px;
          transition: all 0.2s ease;
        }

        .filter-btn:hover,
        .filter-btn.active {
          background: rgba(123, 47, 247, 0.3);
          border-color: rgba(123, 47, 247, 0.6);
        }

        .achievements-grid {
          display: grid;
          grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
          gap: 20px;
        }

        .achievement-card {
          position: relative;
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid var(--rarity-color);
          border-radius: 16px;
          padding: 20px;
          display: flex;
          gap: 16px;
          transition: all 0.3s ease;
        }

        .achievement-card:hover {
          box-shadow: 0 4px 20px var(--rarity-color);
          transform: translateY(-2px);
        }

        .achievement-card.locked {
          opacity: 0.5;
          filter: grayscale(0.7);
        }

        .achievement-icon {
          font-size: 48px;
          flex-shrink: 0;
        }

        .achievement-info {
          flex: 1;
        }

        .achievement-info h4 {
          margin: 0 0 8px 0;
          font-size: 18px;
        }

        .achievement-info p {
          margin: 0 0 12px 0;
          font-size: 14px;
          color: rgba(255, 255, 255, 0.7);
        }

        .achievement-meta {
          display: flex;
          gap: 15px;
          font-size: 12px;
          margin-bottom: 8px;
        }

        .achievement-rarity {
          font-weight: 700;
          text-transform: uppercase;
        }

        .achievement-points {
          color: rgba(255, 255, 255, 0.6);
        }

        .achievement-date {
          font-size: 12px;
          color: rgba(255, 255, 255, 0.5);
        }

        .achievement-progress {
          display: flex;
          align-items: center;
          gap: 10px;
          font-size: 12px;
        }

        .progress-bar-small {
          flex: 1;
          height: 8px;
          background: rgba(26, 0, 51, 0.6);
          border-radius: 4px;
          overflow: hidden;
        }

        .progress-fill-small {
          height: 100%;
          background: linear-gradient(90deg, #7b2ff7 0%, #ff6ec4 100%);
          transition: width 0.3s ease;
        }

        .unlock-badge {
          position: absolute;
          top: 12px;
          right: 12px;
          width: 32px;
          height: 32px;
          background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
          border-radius: 50%;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 18px;
          font-weight: 700;
          color: white;
        }

        /* Leaderboard */
        .leaderboard-panel {
          max-width: 1200px;
          margin: 0 auto;
        }

        .leaderboard-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 25px;
        }

        .leaderboard-header h3 {
          font-size: 28px;
          margin: 0;
        }

        .leaderboard-filter {
          padding: 10px 18px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          cursor: pointer;
        }

        .leaderboard-table {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          overflow: hidden;
        }

        .leaderboard-table-header {
          display: grid;
          grid-template-columns: 80px 1fr 150px 100px 80px;
          padding: 16px 20px;
          background: rgba(123, 47, 247, 0.2);
          font-weight: 700;
          font-size: 14px;
          text-transform: uppercase;
          border-bottom: 2px solid rgba(123, 47, 247, 0.3);
        }

        .leaderboard-row {
          display: grid;
          grid-template-columns: 80px 1fr 150px 100px 80px;
          padding: 16px 20px;
          border-bottom: 1px solid rgba(123, 47, 247, 0.2);
          align-items: center;
          transition: background 0.2s ease;
        }

        .leaderboard-row:hover {
          background: rgba(123, 47, 247, 0.1);
        }

        .leaderboard-row.highlight {
          background: rgba(255, 110, 196, 0.1);
          border: 2px solid rgba(255, 110, 196, 0.3);
        }

        .leaderboard-row.top-three {
          background: rgba(251, 191, 36, 0.05);
        }

        .col-rank {
          font-weight: 700;
          font-size: 18px;
        }

        .medal {
          font-size: 28px;
        }

        .col-player {
          display: flex;
          align-items: center;
          gap: 12px;
        }

        .player-avatar {
          font-size: 24px;
        }

        .player-name {
          font-weight: 600;
        }

        .col-score {
          font-weight: 700;
          color: #4cc9f0;
        }

        .trend-up {
          color: #4ade80;
          font-size: 20px;
        }

        .trend-down {
          color: #ef4444;
          font-size: 20px;
        }

        .trend-same {
          color: rgba(255, 255, 255, 0.4);
        }

        /* Activity */
        .activity-panel {
          max-width: 1000px;
          margin: 0 auto;
        }

        .activity-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 25px;
        }

        .activity-header h3 {
          font-size: 28px;
          margin: 0;
        }

        .activity-filters {
          display: flex;
          gap: 10px;
        }

        .activity-filter-btn {
          padding: 8px 16px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
          color: white;
          cursor: pointer;
          font-size: 14px;
          transition: all 0.2s ease;
        }

        .activity-filter-btn:hover,
        .activity-filter-btn.active {
          background: rgba(123, 47, 247, 0.3);
        }

        .activity-feed {
          display: flex;
          flex-direction: column;
          gap: 15px;
        }

        .activity-item {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          padding: 20px;
          display: flex;
          gap: 16px;
          align-items: start;
          transition: all 0.3s ease;
        }

        .activity-item:hover {
          border-color: rgba(123, 47, 247, 0.6);
          box-shadow: 0 4px 20px rgba(123, 47, 247, 0.2);
        }

        .activity-avatar {
          width: 48px;
          height: 48px;
          background: rgba(123, 47, 247, 0.2);
          border-radius: 50%;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 24px;
          flex-shrink: 0;
        }

        .activity-content {
          flex: 1;
        }

        .activity-content p {
          margin: 0 0 5px 0;
          font-size: 15px;
        }

        .activity-user {
          font-weight: 700;
          color: #4cc9f0;
        }

        .activity-action {
          color: rgba(255, 255, 255, 0.9);
        }

        .activity-time {
          font-size: 13px;
          color: rgba(255, 255, 255, 0.5);
        }

        .activity-type-icon {
          font-size: 24px;
          opacity: 0.7;
        }

        /* Groups */
        .groups-panel {
          max-width: 1000px;
          margin: 0 auto;
        }

        .groups-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 25px;
        }

        .groups-header h3 {
          font-size: 28px;
          margin: 0;
        }

        .create-group-btn {
          padding: 12px 24px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 12px;
          color: white;
          font-weight: 700;
          cursor: pointer;
        }

        .groups-placeholder {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 20px;
          padding: 80px 40px;
          text-align: center;
        }

        .placeholder-icon {
          font-size: 80px;
          display: block;
          margin-bottom: 20px;
        }

        .groups-placeholder h4 {
          font-size: 24px;
          margin: 0 0 10px 0;
        }

        .groups-placeholder p {
          font-size: 16px;
          color: rgba(255, 255, 255, 0.6);
          max-width: 500px;
          margin: 0 auto;
        }

        @media (max-width: 1024px) {
          .stats-grid-social {
            grid-template-columns: repeat(2, 1fr);
          }
          
          .favorite-games-grid {
            grid-template-columns: 1fr;
          }
          
          .achievements-grid {
            grid-template-columns: 1fr;
          }
          
          .leaderboard-table-header,
          .leaderboard-row {
            font-size: 13px;
          }
        }
      `}</style>
    </div>
  );
};
export { SocialModuleRedesigned as SocialModule } from './SocialModule';
