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
 * Project Storage Service - Complete CRUD for game projects
 */
export interface Project {
  id: string;
  name: string;
  description: string;
  ownerId: string;
  createdAt: Date;
  updatedAt: Date;
  version: string;
  isPublic: boolean;
  tags: string[];
  category: string;
  thumbnail?: string;
}

export interface ProjectFile {
  projectId: string;
  path: string;
  content: string;
  size: number;
  mimeType: string;
  updatedAt: Date;
}

export class ProjectStorageService {
  private projects: Map<string, Project> = new Map();
  private files: Map<string, Map<string, ProjectFile>> = new Map(); // projectId -> files
  private collaborators: Map<string, Set<string>> = new Map(); // projectId -> userIds
  
  async createProject(ownerId: string, name: string, description: string): Promise<Project> {
    const project: Project = {
      id: `proj_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`,
      name,
      description,
      ownerId,
      createdAt: new Date(),
      updatedAt: new Date(),
      version: '1.0.0',
      isPublic: false,
      tags: [],
      category: 'game'
    };
    
    this.projects.set(project.id, project);
    this.files.set(project.id, new Map());
    this.collaborators.set(project.id, new Set([ownerId]));
    
    return project;
  }
  
  async getProject(projectId: string): Promise<Project | null> {
    return this.projects.get(projectId) || null;
  }
  
  async updateProject(projectId: string, updates: Partial<Project>): Promise<Project | null> {
    const project = this.projects.get(projectId);
    if (!project) return null;
    
    Object.assign(project, updates, { updatedAt: new Date() });
    this.projects.set(projectId, project);
    return project;
  }
  
  async deleteProject(projectId: string): Promise<boolean> {
    this.projects.delete(projectId);
    this.files.delete(projectId);
    this.collaborators.delete(projectId);
    return true;
  }
  
  async getUserProjects(userId: string): Promise<Project[]> {
    const userProjects: Project[] = [];
    this.projects.forEach(project => {
      if (project.ownerId === userId || this.collaborators.get(project.id)?.has(userId)) {
        userProjects.push(project);
      }
    });
    return userProjects.sort((a, b) => b.updatedAt.getTime() - a.updatedAt.getTime());
  }
  
  async saveFile(projectId: string, path: string, content: string): Promise<ProjectFile> {
    const file: ProjectFile = {
      projectId,
      path,
      content,
      size: content.length,
      mimeType: this.getMimeType(path),
      updatedAt: new Date()
    };
    
    const projectFiles = this.files.get(projectId) || new Map();
    projectFiles.set(path, file);
    this.files.set(projectId, projectFiles);
    
    const project = this.projects.get(projectId);
    if (project) {
      project.updatedAt = new Date();
      this.projects.set(projectId, project);
    }
    
    return file;
  }
  
  async getFile(projectId: string, path: string): Promise<ProjectFile | null> {
    return this.files.get(projectId)?.get(path) || null;
  }
  
  async listFiles(projectId: string): Promise<ProjectFile[]> {
    const projectFiles = this.files.get(projectId);
    return projectFiles ? Array.from(projectFiles.values()) : [];
  }
  
  async deleteFile(projectId: string, path: string): Promise<boolean> {
    const projectFiles = this.files.get(projectId);
    return projectFiles?.delete(path) || false;
  }
  
  async addCollaborator(projectId: string, userId: string): Promise<boolean> {
    const collaborators = this.collaborators.get(projectId);
    if (!collaborators) return false;
    collaborators.add(userId);
    return true;
  }
  
  async removeCollaborator(projectId: string, userId: string): Promise<boolean> {
    const collaborators = this.collaborators.get(projectId);
    if (!collaborators) return false;
    return collaborators.delete(userId);
  }
  
  async getCollaborators(projectId: string): Promise<string[]> {
    return Array.from(this.collaborators.get(projectId) || []);
  }
  
  private getMimeType(path: string): string {
    const ext = path.split('.').pop()?.toLowerCase();
    const mimeTypes: Record<string, string> = {
      'js': 'application/javascript',
      'ts': 'application/typescript',
      'json': 'application/json',
      'png': 'image/png',
      'jpg': 'image/jpeg',
      'jpeg': 'image/jpeg',
      'gif': 'image/gif',
      'mp3': 'audio/mpeg',
      'wav': 'audio/wav',
      'ogg': 'audio/ogg'
    };
    return mimeTypes[ext || ''] || 'application/octet-stream';
  }
}
