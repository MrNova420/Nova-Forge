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
 * Forum System - Discussion forums for games
 */
export interface ForumPost {
  id: string;
  gameId: string;
  authorId: string;
  title: string;
  content: string;
  createdAt: Date;
  replies: number;
}
export interface ForumReply {
  id: string;
  postId: string;
  authorId: string;
  content: string;
  createdAt: Date;
}
export class ForumSystem {
  private posts: Map<string, ForumPost> = new Map();
  private replies: Map<string, ForumReply[]> = new Map();
  createPost(
    gameId: string,
    authorId: string,
    title: string,
    content: string
  ): ForumPost {
    const post: ForumPost = {
      id: Date.now().toString(),
      gameId,
      authorId,
      title,
      content,
      createdAt: new Date(),
      replies: 0,
    };
    this.posts.set(post.id, post);
    return post;
  }
  addReply(postId: string, authorId: string, content: string): ForumReply {
    const reply: ForumReply = {
      id: Date.now().toString(),
      postId,
      authorId,
      content,
      createdAt: new Date(),
    };
    const postReplies = this.replies.get(postId) || [];
    postReplies.push(reply);
    this.replies.set(postId, postReplies);
    const post = this.posts.get(postId);
    if (post) {
      post.replies++;
      this.posts.set(postId, post);
    }
    return reply;
  }
  getPostsByGame(gameId: string): ForumPost[] {
    return Array.from(this.posts.values()).filter((p) => p.gameId === gameId);
  }
  getReplies(postId: string): ForumReply[] {
    return this.replies.get(postId) || [];
  }
}
