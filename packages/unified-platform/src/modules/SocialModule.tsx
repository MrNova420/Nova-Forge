/**
 * Social Module - Friends & Achievements
 */

import React, { useState, useEffect } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface SocialModuleProps {
  platform: UnifiedPlatformCore;
}

export const SocialModule: React.FC<SocialModuleProps> = ({
  platform: _platform,
}) => {
  const [friends, setFriends] = useState<any[]>([]);
  const [achievements, setAchievements] = useState<any[]>([]);
  const [activeTab, setActiveTab] = useState<'friends' | 'achievements'>(
    'friends'
  );

  useEffect(() => {
    loadFriends();
    loadAchievements();
  }, []);

  const loadFriends = async () => {
    try {
      const response = await fetch('/api/friends');
      const data = await response.json();
      setFriends(data.friends || []);
    } catch (error) {
      console.error('Failed to load friends:', error);
    }
  };

  const loadAchievements = async () => {
    try {
      const response = await fetch('/api/achievements');
      const data = await response.json();
      setAchievements(data.achievements || []);
    } catch (error) {
      console.error('Failed to load achievements:', error);
    }
  };

  return (
    <div className="social-module">
      <div className="tabs">
        <button
          className={activeTab === 'friends' ? 'active' : ''}
          onClick={() => setActiveTab('friends')}
        >
          Friends
        </button>
        <button
          className={activeTab === 'achievements' ? 'active' : ''}
          onClick={() => setActiveTab('achievements')}
        >
          Achievements
        </button>
      </div>

      {activeTab === 'friends' && (
        <div className="friends-list">
          {friends.map((friend) => (
            <div key={friend.id} className="friend-card">
              <div className="avatar">{friend.name[0]}</div>
              <div className="info">
                <h3>{friend.name}</h3>
                <span className={friend.online ? 'online' : 'offline'}>
                  {friend.online ? 'Online' : 'Offline'}
                </span>
              </div>
            </div>
          ))}
        </div>
      )}

      {activeTab === 'achievements' && (
        <div className="achievements-list">
          {achievements.map((achievement) => (
            <div key={achievement.id} className="achievement-card">
              <div className="icon">{achievement.icon}</div>
              <div className="info">
                <h3>{achievement.name}</h3>
                <p>{achievement.description}</p>
                <div className="progress">
                  <div
                    className="bar"
                    style={{ width: `${achievement.progress}%` }}
                  />
                </div>
              </div>
            </div>
          ))}
        </div>
      )}

      <style>{`
        .social-module {
          padding: 20px;
          background: #1a1a1a;
          color: white;
          height: 100%;
          overflow-y: auto;
        }
        .tabs {
          display: flex;
          gap: 10px;
          margin-bottom: 20px;
        }
        .tabs button {
          padding: 10px 20px;
          background: #2a2a2a;
          border: none;
          color: white;
          cursor: pointer;
          border-radius: 4px;
        }
        .tabs button.active {
          background: #7b2ff7;
        }
        .friends-list, .achievements-list {
          display: flex;
          flex-direction: column;
          gap: 10px;
        }
        .friend-card, .achievement-card {
          display: flex;
          gap: 15px;
          padding: 15px;
          background: #2a2a2a;
          border-radius: 8px;
        }
        .avatar, .icon {
          width: 50px;
          height: 50px;
          background: #7b2ff7;
          border-radius: 50%;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 1.5em;
        }
        .online {
          color: #4caf50;
        }
        .offline {
          color: #666;
        }
        .progress {
          width: 100%;
          height: 8px;
          background: #1a1a1a;
          border-radius: 4px;
          margin-top: 10px;
        }
        .bar {
          height: 100%;
          background: #7b2ff7;
          border-radius: 4px;
        }
      `}</style>
    </div>
  );
};
