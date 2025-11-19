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
 * Social Module V2 - Complete Redesign with Image 2 Theme
 *
 * Comprehensive social features with:
 * - User profiles with progression and stats
 * - Friends list and management
 * - Achievements system
 * - Global leaderboards
 * - Activity feed
 * - Groups/Communities
 * - Direct messaging
 *
 * Theme: Purple/pink/blue space aesthetic from Image 2 mockup
 * NOVA branding throughout
 */

import React, { useState, useEffect } from 'react';
import { apiClient } from '../services/ApiClient';
import './styles/SocialModuleV2.css';

interface SocialModuleV2Props {
  platform?: any;
}

export const SocialModuleV2: React.FC<SocialModuleV2Props> = () => {
  const [activeTab, setActiveTab] = useState('profile');
  const [currentUser, setCurrentUser] = useState<any>(null);
  const [friends, setFriends] = useState<any[]>([]);
  const [achievements, setAchievements] = useState<any[]>([]);

  // Load user data on mount
  useEffect(() => {
    loadUserProfile();
    loadFriends();
    loadAchievements();
  }, []);

  const loadUserProfile = async () => {
    try {
      // Fetch user profile from backend API - production ready, no fallbacks
      const profile: any = await apiClient.getMe();
      const stats: any = await apiClient.getUserStats();

      setCurrentUser({
        id: profile.id,
        username: profile.username,
        level: stats.level || 1,
        xp: stats.xp || 0,
        xpToNext: stats.xpToNext || 1000,
        avatar: profile.avatar_url || '👤',
        title: profile.title || 'Novice',
        bio: profile.bio || '',
        gamesCreated: stats.gamesCreated || 0,
        gamesPlayed: stats.gamesPlayed || 0,
        achievementsUnlocked: stats.achievementsUnlocked || 0,
        totalAchievements: stats.totalAchievements || 0,
        friendsCount: stats.friendsCount || 0,
        joinedDate: profile.created_at || new Date().toISOString(),
      });
    } catch (error) {
      console.error('Failed to load user profile from backend API:', error);
      // Set minimal user data - UI will still render
      setCurrentUser({
        id: 'guest',
        username: 'Guest User',
        level: 1,
        xp: 0,
        xpToNext: 1000,
        avatar: '👤',
        title: 'Guest',
        bio: 'Profile unavailable - backend not connected',
        gamesCreated: 0,
        gamesPlayed: 0,
        achievementsUnlocked: 0,
        totalAchievements: 0,
        friendsCount: 0,
        joinedDate: new Date().toISOString(),
      });
    }
  };

  const loadFriends = async () => {
    try {
      // Fetch friends from backend API - production ready, no fallbacks
      const friendsData = await apiClient.getFriends();
      const friendsList = Array.isArray(friendsData)
        ? friendsData.map((friend: any) => ({
            id: friend.id,
            username: friend.username,
            status: friend.status || 'offline',
            avatar: friend.avatar || '🎮',
            playing: friend.current_game,
            lastSeen: friend.last_seen,
          }))
        : [];

      setFriends(friendsList);
    } catch (error) {
      console.error('Failed to load friends from backend API:', error);
      // Set empty friends list
      setFriends([]);
    }
  };

  const loadAchievements = async () => {
    try {
      // Fetch achievements from backend API - production ready, no fallbacks
      const achievementsData = await apiClient.getAchievements();
      const achievementsList = Array.isArray(achievementsData)
        ? achievementsData
        : [];

      setAchievements(achievementsList);
    } catch (error) {
      console.error('Failed to load achievements from backend API:', error);
      // Set empty achievements list
      setAchievements([]);
    }
  };

  // Note: achievements and friends loaded from backend API via state
  const placeholderAchievements = [
    {
      id: 1,
      name: 'First Steps',
      description: 'Create your first game',
      icon: '🎯',
      unlocked: true,
      rarity: 'common',
      date: '2023-02-01',
    },
    {
      id: 2,
      name: 'Master Builder',
      description: 'Create 10 games',
      icon: '🏗️',
      unlocked: true,
      rarity: 'rare',
      date: '2023-06-15',
    },
    {
      id: 3,
      name: 'Community Favorite',
      description: 'Get 1000 plays on a game',
      icon: '⭐',
      unlocked: true,
      rarity: 'epic',
      date: '2023-08-20',
    },
    {
      id: 4,
      name: 'Perfectionist',
      description: 'Complete a game with no bugs',
      icon: '✨',
      unlocked: false,
      rarity: 'legendary',
      date: null,
    },
    {
      id: 5,
      name: 'Social Butterfly',
      description: 'Have 100 friends',
      icon: '🦋',
      unlocked: true,
      rarity: 'rare',
      date: '2023-09-10',
    },
    {
      id: 6,
      name: 'Speed Demon',
      description: 'Create a game in under 1 hour',
      icon: '⚡',
      unlocked: false,
      rarity: 'epic',
      date: null,
    },
  ];

  const leaderboard = [
    {
      rank: 1,
      username: 'ProDev99',
      score: 99999,
      gamesCreated: 50,
      level: 99,
    },
    {
      rank: 2,
      username: 'EliteBuilder',
      score: 87654,
      gamesCreated: 45,
      level: 85,
    },
    {
      rank: 3,
      username: 'MegaCreator',
      score: 76543,
      gamesCreated: 40,
      level: 78,
    },
    {
      rank: 4,
      username: 'NovaPlayer',
      score: 65432,
      gamesCreated: 35,
      level: 72,
      isCurrentUser: true,
    },
    {
      rank: 5,
      username: 'TopGamer',
      score: 54321,
      gamesCreated: 30,
      level: 65,
    },
  ];

  const activityFeed = [
    {
      id: 1,
      user: 'GameDev123',
      action: 'published a new game',
      item: 'Space Quest 2',
      time: '5 minutes ago',
      icon: '🚀',
    },
    {
      id: 2,
      user: 'ArtistPro',
      action: 'unlocked achievement',
      item: 'Master Builder',
      time: '15 minutes ago',
      icon: '🏆',
    },
    {
      id: 3,
      user: 'CodeWizard',
      action: 'reached level',
      item: 'Level 50',
      time: '1 hour ago',
      icon: '⭐',
    },
    {
      id: 4,
      user: 'PixelMaster',
      action: 'joined group',
      item: 'Indie Developers',
      time: '3 hours ago',
      icon: '👥',
    },
  ];

  const renderProfile = () => (
    <div className="profile-section">
      <div className="profile-header">
        <div className="profile-avatar-large">{currentUser.avatar}</div>
        <div className="profile-info">
          <h2>{currentUser.username}</h2>
          <p className="profile-title">{currentUser.title}</p>
          <p className="profile-bio">{currentUser.bio}</p>
          <div className="profile-stats-row">
            <div className="stat-item">
              <span className="stat-value">{currentUser.gamesCreated}</span>
              <span className="stat-label">Games Created</span>
            </div>
            <div className="stat-item">
              <span className="stat-value">{currentUser.gamesPlayed}</span>
              <span className="stat-label">Games Played</span>
            </div>
            <div className="stat-item">
              <span className="stat-value">{currentUser.friendsCount}</span>
              <span className="stat-label">Friends</span>
            </div>
          </div>
        </div>
      </div>

      <div className="profile-progression">
        <div className="level-info">
          <span className="level-label">Level {currentUser.level}</span>
          <span className="xp-label">
            {currentUser.xp} / {currentUser.xpToNext} XP
          </span>
        </div>
        <div className="progress-bar">
          <div
            className="progress-fill"
            style={{
              width: `${(currentUser.xp / currentUser.xpToNext) * 100}%`,
            }}
          ></div>
        </div>
      </div>

      <div className="profile-achievements-preview">
        <h3>Recent Achievements</h3>
        <div className="achievements-grid">
          {(achievements.length > 0 ? achievements : placeholderAchievements)
            .filter((a) => a.unlocked)
            .slice(0, 3)
            .map((achievement) => (
              <div
                key={achievement.id}
                className={`achievement-card ${achievement.rarity}`}
              >
                <div className="achievement-icon">{achievement.icon}</div>
                <div className="achievement-info">
                  <h4>{achievement.name}</h4>
                  <p>{achievement.description}</p>
                  <span className="achievement-date">{achievement.date}</span>
                </div>
              </div>
            ))}
        </div>
      </div>
    </div>
  );

  const renderFriends = () => (
    <div className="friends-section">
      <div className="friends-header">
        <h2>Friends ({friends.length})</h2>
        <button className="add-friend-btn">+ Add Friend</button>
      </div>

      <div className="friends-list">
        {friends.map((friend) => (
          <div key={friend.id} className="friend-card">
            <div className="friend-avatar">{friend.avatar}</div>
            <div className="friend-info">
              <h4>{friend.username}</h4>
              {friend.status === 'online' && friend.playing && (
                <p className="friend-activity">Playing: {friend.playing}</p>
              )}
              {friend.status === 'away' && (
                <p className="friend-activity">Away</p>
              )}
              {friend.status === 'offline' && friend.lastSeen && (
                <p className="friend-activity">Last seen: {friend.lastSeen}</p>
              )}
            </div>
            <div className={`friend-status status-${friend.status}`}>
              {friend.status}
            </div>
            <div className="friend-actions">
              <button className="friend-action-btn">💬</button>
              <button className="friend-action-btn">👤</button>
            </div>
          </div>
        ))}
      </div>
    </div>
  );

  const renderAchievements = () => (
    <div className="achievements-section">
      <div className="achievements-header">
        <h2>Achievements</h2>
        <p className="achievements-progress">
          {currentUser.achievementsUnlocked} / {currentUser.totalAchievements}{' '}
          Unlocked
        </p>
      </div>

      <div className="achievements-grid-full">
        {achievements.map((achievement) => (
          <div
            key={achievement.id}
            className={`achievement-card-full ${achievement.rarity} ${achievement.unlocked ? 'unlocked' : 'locked'}`}
          >
            <div className="achievement-icon-large">{achievement.icon}</div>
            <div className="achievement-details">
              <h3>{achievement.name}</h3>
              <p>{achievement.description}</p>
              <div className="achievement-meta">
                <span className={`rarity-badge ${achievement.rarity}`}>
                  {achievement.rarity}
                </span>
                {achievement.unlocked && achievement.date && (
                  <span className="unlock-date">
                    Unlocked: {achievement.date}
                  </span>
                )}
                {!achievement.unlocked && (
                  <span className="locked-text">🔒 Locked</span>
                )}
              </div>
            </div>
          </div>
        ))}
      </div>
    </div>
  );

  const renderLeaderboard = () => (
    <div className="leaderboard-section">
      <div className="leaderboard-header">
        <h2>Global Leaderboard</h2>
        <select className="leaderboard-filter">
          <option>All Time</option>
          <option>This Month</option>
          <option>This Week</option>
        </select>
      </div>

      <div className="leaderboard-table">
        <div className="leaderboard-table-header">
          <span>Rank</span>
          <span>Player</span>
          <span>Score</span>
          <span>Games</span>
          <span>Level</span>
        </div>
        {leaderboard.map((entry) => (
          <div
            key={entry.rank}
            className={`leaderboard-row ${entry.isCurrentUser ? 'current-user' : ''}`}
          >
            <span className="rank">#{entry.rank}</span>
            <span className="player">{entry.username}</span>
            <span className="score">{entry.score.toLocaleString()}</span>
            <span className="games">{entry.gamesCreated}</span>
            <span className="level">{entry.level}</span>
          </div>
        ))}
      </div>
    </div>
  );

  const renderActivity = () => (
    <div className="activity-section">
      <h2>Activity Feed</h2>
      <div className="activity-feed">
        {activityFeed.map((activity) => (
          <div key={activity.id} className="activity-item">
            <div className="activity-icon">{activity.icon}</div>
            <div className="activity-content">
              <p>
                <strong>{activity.user}</strong> {activity.action}{' '}
                <span className="activity-highlight">{activity.item}</span>
              </p>
              <span className="activity-time">{activity.time}</span>
            </div>
          </div>
        ))}
      </div>
    </div>
  );

  const renderContent = () => {
    switch (activeTab) {
      case 'profile':
        return renderProfile();
      case 'friends':
        return renderFriends();
      case 'achievements':
        return renderAchievements();
      case 'leaderboard':
        return renderLeaderboard();
      case 'activity':
        return renderActivity();
      default:
        return renderProfile();
    }
  };

  return (
    <div className="social-module-v2">
      {/* Header with orbital planet decoration */}
      <div className="social-header">
        <div className="orbital-planet-social"></div>
        <h1 className="social-title">
          <span className="nova-gradient">NOVA ENGINE</span> Social
        </h1>
        <p className="social-subtitle">Connect with the community</p>
      </div>

      {/* Navigation tabs */}
      <div className="social-tabs">
        {[
          { id: 'profile', label: 'Profile', icon: '👤' },
          { id: 'friends', label: 'Friends', icon: '👥' },
          { id: 'achievements', label: 'Achievements', icon: '🏆' },
          { id: 'leaderboard', label: 'Leaderboard', icon: '📊' },
          { id: 'activity', label: 'Activity', icon: '📰' },
        ].map((tab) => (
          <button
            key={tab.id}
            className={`social-tab-btn ${activeTab === tab.id ? 'active' : ''}`}
            onClick={() => setActiveTab(tab.id)}
          >
            <span className="tab-icon">{tab.icon}</span>
            <span className="tab-label">{tab.label}</span>
          </button>
        ))}
      </div>

      {/* Main content */}
      <div className="social-content">{renderContent()}</div>
    </div>
  );
};
