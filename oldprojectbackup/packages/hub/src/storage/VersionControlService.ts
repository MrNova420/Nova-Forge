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
 * Version Control Service - Git-like versioning for projects
 */
export interface ProjectVersion {
  id: string;
  projectId: string;
  version: string;
  changelog: string;
  createdBy: string;
  createdAt: Date;
  filesSnapshot: Map<string, string>; // path -> content
  parentVersion?: string;
}

export class VersionControlService {
  private versions: Map<string, ProjectVersion[]> = new Map(); // projectId -> versions
  
  async createVersion(
    projectId: string,
    version: string,
    changelog: string,
    createdBy: string,
    files: Map<string, string>
  ): Promise<ProjectVersion> {
    const projectVersions = this.versions.get(projectId) || [];
    const parentVersion = projectVersions[projectVersions.length - 1]?.id;
    
    const newVersion: ProjectVersion = {
      id: `ver_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`,
      projectId,
      version,
      changelog,
      createdBy,
      createdAt: new Date(),
      filesSnapshot: new Map(files),
      parentVersion
    };
    
    projectVersions.push(newVersion);
    this.versions.set(projectId, projectVersions);
    
    return newVersion;
  }
  
  async getVersions(projectId: string): Promise<ProjectVersion[]> {
    return this.versions.get(projectId) || [];
  }
  
  async getVersion(projectId: string, versionId: string): Promise<ProjectVersion | null> {
    const versions = this.versions.get(projectId) || [];
    return versions.find(v => v.id === versionId) || null;
  }
  
  async getLatestVersion(projectId: string): Promise<ProjectVersion | null> {
    const versions = this.versions.get(projectId) || [];
    return versions[versions.length - 1] || null;
  }
  
  async restoreVersion(projectId: string, versionId: string): Promise<Map<string, string> | null> {
    const version = await this.getVersion(projectId, versionId);
    if (!version) return null;
    return new Map(version.filesSnapshot);
  }
  
  async compareVersions(projectId: string, versionId1: string, versionId2: string): Promise<{
    added: string[];
    modified: string[];
    deleted: string[];
  }> {
    const v1 = await this.getVersion(projectId, versionId1);
    const v2 = await this.getVersion(projectId, versionId2);
    
    if (!v1 || !v2) return { added: [], modified: [], deleted: [] };
    
    const added: string[] = [];
    const modified: string[] = [];
    const deleted: string[] = [];
    
    v2.filesSnapshot.forEach((content, path) => {
      if (!v1.filesSnapshot.has(path)) {
        added.push(path);
      } else if (v1.filesSnapshot.get(path) !== content) {
        modified.push(path);
      }
    });
    
    v1.filesSnapshot.forEach((content, path) => {
      if (!v2.filesSnapshot.has(path)) {
        deleted.push(path);
      }
    });
    
    return { added, modified, deleted };
  }
}
