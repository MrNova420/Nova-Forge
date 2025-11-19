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
 * Social Service - Friends, activity feed, and social features
 */
export interface Friend {
  userId: string;
  username: string;
  status: 'online' | 'offline' | 'in-game';
  currentGame?: string;
  addedAt: Date;
}

export interface FriendRequest {
  fromUserId: string;
  toUserId: string;
  sentAt: Date;
  status: 'pending' | 'accepted' | 'rejected';
}

export interface Activity {
  id: string;
  userId: string;
  type: 'game_played' | 'achievement_unlocked' | 'friend_added' | 'game_rated';
  description: string;
  timestamp: Date;
  metadata?: Record<string, unknown>;
}

export class SocialService {
  private friends: Map<string, Set<string>> = new Map(); // userId -> friendIds
  private friendRequests: FriendRequest[] = [];
  private activities: Activity[] = [];
  private userStatus: Map<string, 'online' | 'offline' | 'in-game'> = new Map();

  async sendFriendRequest(
    fromUserId: string,
    toUserId: string
  ): Promise<FriendRequest> {
    const request: FriendRequest = {
      fromUserId,
      toUserId,
      sentAt: new Date(),
      status: 'pending',
    };

    this.friendRequests.push(request);
    return request;
  }

  async acceptFriendRequest(
    fromUserId: string,
    toUserId: string
  ): Promise<boolean> {
    const request = this.friendRequests.find(
      (r) =>
        r.fromUserId === fromUserId &&
        r.toUserId === toUserId &&
        r.status === 'pending'
    );

    if (!request) return false;

    request.status = 'accepted';

    // Add to friends list
    const userFriends = this.friends.get(toUserId) || new Set();
    userFriends.add(fromUserId);
    this.friends.set(toUserId, userFriends);

    const otherFriends = this.friends.get(fromUserId) || new Set();
    otherFriends.add(toUserId);
    this.friends.set(fromUserId, otherFriends);

    // Add activity
    this.addActivity(
      toUserId,
      'friend_added',
      `Became friends with user ${fromUserId}`
    );

    return true;
  }

  async removeFriend(userId: string, friendId: string): Promise<boolean> {
    const userFriends = this.friends.get(userId);
    const otherFriends = this.friends.get(friendId);

    userFriends?.delete(friendId);
    otherFriends?.delete(userId);

    return true;
  }

  getFriends(userId: string): string[] {
    return Array.from(this.friends.get(userId) || []);
  }

  getPendingRequests(userId: string): FriendRequest[] {
    return this.friendRequests.filter(
      (r) => r.toUserId === userId && r.status === 'pending'
    );
  }

  setUserStatus(
    userId: string,
    status: 'online' | 'offline' | 'in-game'
  ): void {
    this.userStatus.set(userId, status);
  }

  getUserStatus(userId: string): 'online' | 'offline' | 'in-game' {
    return this.userStatus.get(userId) || 'offline';
  }

  addActivity(
    userId: string,
    type: Activity['type'],
    description: string,
    metadata?: Record<string, unknown>
  ): void {
    const activity: Activity = {
      id: `activity_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`,
      userId,
      type,
      description,
      timestamp: new Date(),
      metadata,
    };

    this.activities.push(activity);

    // Keep only last 100 activities per user
    const userActivities = this.activities.filter((a) => a.userId === userId);
    if (userActivities.length > 100) {
      const oldestActivity = userActivities[0];
      if (oldestActivity) {
        const index = this.activities.indexOf(oldestActivity);
        if (index > -1) this.activities.splice(index, 1);
      }
    }
  }

  getActivityFeed(userId: string, limit: number = 999999): Activity[] {
    const friendIds = this.getFriends(userId);
    const relevantUsers = [userId, ...friendIds];

    return this.activities
      .filter((a) => relevantUsers.includes(a.userId))
      .sort((a, b) => b.timestamp.getTime() - a.timestamp.getTime())
      .slice(0, limit);
  }
}
