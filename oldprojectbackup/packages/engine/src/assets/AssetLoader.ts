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
 * AssetLoader
 *
 * Base class and types for asset loaders.
 */

export enum AssetType {
  Texture = 'texture',
  Model = 'model',
  Audio = 'audio',
  Shader = 'shader',
  Material = 'material',
  Scene = 'scene',
  Font = 'font',
  JSON = 'json',
  Binary = 'binary',
  Text = 'text',
}

export interface AssetLoader<T> {
  load(path: string): Promise<T>;
  supports?(path: string): boolean;
}

export class TextLoader implements AssetLoader<string> {
  async load(path: string): Promise<string> {
    const response = await fetch(path);
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    return response.text();
  }

  supports(path: string): boolean {
    return path.endsWith('.txt') || path.endsWith('.text');
  }
}

export class JSONLoader implements AssetLoader<any> {
  async load(path: string): Promise<any> {
    const response = await fetch(path);
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    return response.json();
  }

  supports(path: string): boolean {
    return path.endsWith('.json');
  }
}

export class BinaryLoader implements AssetLoader<ArrayBuffer> {
  async load(path: string): Promise<ArrayBuffer> {
    const response = await fetch(path);
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    return response.arrayBuffer();
  }
}

export class ImageLoader implements AssetLoader<HTMLImageElement> {
  async load(path: string): Promise<HTMLImageElement> {
    return new Promise((resolve, reject) => {
      const image = new Image();
      image.crossOrigin = 'anonymous';
      image.onload = () => resolve(image);
      image.onerror = () => reject(new Error(`Failed to load image: ${path}`));
      image.src = path;
    });
  }

  supports(path: string): boolean {
    const ext = path.toLowerCase();
    return (
      ext.endsWith('.png') ||
      ext.endsWith('.jpg') ||
      ext.endsWith('.jpeg') ||
      ext.endsWith('.gif') ||
      ext.endsWith('.webp') ||
      ext.endsWith('.bmp')
    );
  }
}
