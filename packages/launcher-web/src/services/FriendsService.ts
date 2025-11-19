/**
 * Friends Service - Complete social features implementation
 * Phase 8.2: Social Features - Friends system
 */

export interface Friend {
  id: string;
  username: string;
  displayName: string;
  avatarUrl?: string;
  status: 'online' | 'offline' | 'in-game' | 'away';
  currentGame?: {
    id: string;
    name: string;
    startedAt: Date;
  };
  lastSeen: Date;
  addedAt: Date;
}

export interface FriendRequest {
  id: string;
  from: Friend;
  message?: string;
  sentAt: Date;
}

export interface FriendActivity {
  id: string;
  friend: Friend;
  type: 'game_start' | 'game_finish' | 'achievement' | 'status_change';
  data: any;
  timestamp: Date;
}

class FriendsService {
  private friends: Map<string, Friend> = new Map();
  private pendingRequests: Map<string, FriendRequest> = new Map();
  private blockedUsers: Set<string> = new Set();
  private activityFeed: FriendActivity[] = [];

  /**
   * Get all friends
   */
  async getFriends(): Promise<Friend[]> {
    try {
      const response = await fetch('/api/friends', {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to fetch friends');

      const friends: Friend[] = await response.json();
      friends.forEach((friend) => this.friends.set(friend.id, friend));

      return friends;
    } catch (error) {
      console.error('Error fetching friends:', error);
      return Array.from(this.friends.values());
    }
  }

  /**
   * Search for users to add as friends
   */
  async searchUsers(query: string): Promise<Friend[]> {
    try {
      const response = await fetch(
        `/api/users/search?q=${encodeURIComponent(query)}`,
        {
          headers: { Authorization: `Bearer ${this.getToken()}` },
        }
      );

      if (!response.ok) throw new Error('Search failed');

      return await response.json();
    } catch (error) {
      console.error('Error searching users:', error);
      return [];
    }
  }

  /**
   * Send friend request
   */
  async sendFriendRequest(userId: string, message?: string): Promise<void> {
    try {
      const response = await fetch('/api/friends/request', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${this.getToken()}`,
        },
        body: JSON.stringify({ userId, message }),
      });

      if (!response.ok) throw new Error('Failed to send friend request');

      // eslint-disable-next-line no-console
      console.log(`Friend request sent to user ${userId}`);
    } catch (error) {
      console.error('Error sending friend request:', error);
      throw error;
    }
  }

  /**
   * Get pending friend requests
   */
  async getPendingRequests(): Promise<FriendRequest[]> {
    try {
      const response = await fetch('/api/friends/requests', {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to fetch requests');

      const requests: FriendRequest[] = await response.json();
      requests.forEach((req) => this.pendingRequests.set(req.id, req));

      return requests;
    } catch (error) {
      console.error('Error fetching requests:', error);
      return Array.from(this.pendingRequests.values());
    }
  }

  /**
   * Accept friend request
   */
  async acceptFriendRequest(requestId: string): Promise<void> {
    try {
      const response = await fetch(`/api/friends/request/${requestId}/accept`, {
        method: 'POST',
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to accept request');

      this.pendingRequests.delete(requestId);
      await this.getFriends(); // Refresh friends list

      // eslint-disable-next-line no-console
      console.log(`Accepted friend request ${requestId}`);
    } catch (error) {
      console.error('Error accepting friend request:', error);
      throw error;
    }
  }

  /**
   * Decline friend request
   */
  async declineFriendRequest(requestId: string): Promise<void> {
    try {
      const response = await fetch(
        `/api/friends/request/${requestId}/decline`,
        {
          method: 'POST',
          headers: { Authorization: `Bearer ${this.getToken()}` },
        }
      );

      if (!response.ok) throw new Error('Failed to decline request');

      this.pendingRequests.delete(requestId);
      // eslint-disable-next-line no-console
      console.log(`Declined friend request ${requestId}`);
    } catch (error) {
      console.error('Error declining friend request:', error);
      throw error;
    }
  }

  /**
   * Remove friend
   */
  async removeFriend(friendId: string): Promise<void> {
    try {
      const response = await fetch(`/api/friends/${friendId}`, {
        method: 'DELETE',
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to remove friend');

      this.friends.delete(friendId);
      // eslint-disable-next-line no-console
      console.log(`Removed friend ${friendId}`);
    } catch (error) {
      console.error('Error removing friend:', error);
      throw error;
    }
  }

  /**
   * Block user
   */
  async blockUser(userId: string): Promise<void> {
    try {
      const response = await fetch(`/api/users/${userId}/block`, {
        method: 'POST',
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to block user');

      this.blockedUsers.add(userId);
      this.friends.delete(userId);
      // eslint-disable-next-line no-console
      console.log(`Blocked user ${userId}`);
    } catch (error) {
      console.error('Error blocking user:', error);
      throw error;
    }
  }

  /**
   * Unblock user
   */
  async unblockUser(userId: string): Promise<void> {
    try {
      const response = await fetch(`/api/users/${userId}/unblock`, {
        method: 'POST',
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to unblock user');

      this.blockedUsers.delete(userId);
      // eslint-disable-next-line no-console
      console.log(`Unblocked user ${userId}`);
    } catch (error) {
      console.error('Error unblocking user:', error);
      throw error;
    }
  }

  /**
   * Get friend activity feed
   */
  async getActivityFeed(limit: number = 999999): Promise<FriendActivity[]> {
    try {
      const response = await fetch(`/api/friends/activity?limit=${limit}`, {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to fetch activity');

      this.activityFeed = await response.json();
      return this.activityFeed;
    } catch (error) {
      console.error('Error fetching activity:', error);
      return this.activityFeed;
    }
  }

  /**
   * Get online friends
   */
  getOnlineFriends(): Friend[] {
    return Array.from(this.friends.values()).filter(
      (friend) => friend.status === 'online' || friend.status === 'in-game'
    );
  }

  /**
   * Get friends playing specific game
   */
  getFriendsPlayingGame(gameId: string): Friend[] {
    return Array.from(this.friends.values()).filter(
      (friend) => friend.currentGame?.id === gameId
    );
  }

  /**
   * Invite friend to game
   */
  async inviteFriendToGame(
    friendId: string,
    gameId: string,
    roomId?: string
  ): Promise<void> {
    try {
      const response = await fetch('/api/friends/invite', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${this.getToken()}`,
        },
        body: JSON.stringify({ friendId, gameId, roomId }),
      });

      if (!response.ok) throw new Error('Failed to send invite');

      // eslint-disable-next-line no-console
      console.log(`Invited friend ${friendId} to game ${gameId}`);
    } catch (error) {
      console.error('Error inviting friend:', error);
      throw error;
    }
  }

  /**
   * Join friend's game
   */
  async joinFriendGame(
    friendId: string
  ): Promise<{ gameId: string; roomId: string }> {
    try {
      const friend = this.friends.get(friendId);
      if (!friend || !friend.currentGame) {
        throw new Error('Friend is not in a game');
      }

      const response = await fetch(`/api/friends/${friendId}/join`, {
        method: 'POST',
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to join friend');

      return await response.json();
    } catch (error) {
      console.error('Error joining friend:', error);
      throw error;
    }
  }

  /**
   * Update friend status (called by WebSocket)
   */
  updateFriendStatus(
    friendId: string,
    status: Friend['status'],
    currentGame?: Friend['currentGame']
  ): void {
    const friend = this.friends.get(friendId);
    if (friend) {
      friend.status = status;
      friend.currentGame = currentGame;
      friend.lastSeen = new Date();

      // Add to activity feed
      this.activityFeed.unshift({
        id: `${friendId}_${Date.now()}`,
        friend,
        type: 'status_change',
        data: { status, currentGame },
        timestamp: new Date(),
      });

      // Keep feed limited
      if (this.activityFeed.length > 100) {
        this.activityFeed.pop();
      }
    }
  }

  /**
   * Get auth token from storage
   */
  private getToken(): string {
    return localStorage.getItem('auth_token') || '';
  }

  /**
   * Initialize WebSocket connection for real-time updates
   */
  initializeRealtimeUpdates(): void {
    const token = this.getToken();
    if (!token) return;

    const ws = new WebSocket(
      `ws://localhost:3000/api/friends/ws?token=${token}`
    );

    ws.onmessage = (event) => {
      const message = JSON.parse(event.data);

      switch (message.type) {
        case 'friend_status':
          this.updateFriendStatus(
            message.friendId,
            message.status,
            message.currentGame
          );
          break;
        case 'friend_request':
          this.pendingRequests.set(message.request.id, message.request);
          break;
        case 'friend_activity':
          this.activityFeed.unshift(message.activity);
          if (this.activityFeed.length > 100) {
            this.activityFeed.pop();
          }
          break;
      }
    };

    ws.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    ws.onclose = () => {
      // eslint-disable-next-line no-console
      console.log('WebSocket closed, reconnecting in 5s...');
      setTimeout(() => this.initializeRealtimeUpdates(), 5000);
    };
  }
}

export const friendsService = new FriendsService();
