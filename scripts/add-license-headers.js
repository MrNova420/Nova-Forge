#!/usr/bin/env node
/**
 * Add License Headers to Source Files
 * 
 * This script adds the Nova Engine proprietary license header
 * to all TypeScript and TSX source files.
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// License header template
const LICENSE_HEADER = `/**
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

`;

// Check if file already has a license header
function hasLicenseHeader(content) {
  return content.includes('NOVA ENGINE - Proprietary Software') ||
         content.includes('Copyright (c) 2025 Kayden Shawn Massengill');
}

// Add license header to a file
function addLicenseHeader(filePath) {
  try {
    const content = fs.readFileSync(filePath, 'utf8');
    
    // Skip if already has license
    if (hasLicenseHeader(content)) {
      return { status: 'skipped', reason: 'already has license' };
    }
    
    // Skip if file is empty or too small
    if (content.trim().length < 10) {
      return { status: 'skipped', reason: 'file too small' };
    }
    
    // Add license header at the beginning
    const newContent = LICENSE_HEADER + content;
    fs.writeFileSync(filePath, newContent, 'utf8');
    
    return { status: 'added' };
  } catch (error) {
    return { status: 'error', reason: error.message };
  }
}

// Recursively find all TypeScript files
function findTypeScriptFiles(dir, fileList = []) {
  const files = fs.readdirSync(dir);
  
  files.forEach(file => {
    const filePath = path.join(dir, file);
    const stat = fs.statSync(filePath);
    
    if (stat.isDirectory()) {
      // Skip node_modules, dist, build directories
      if (!['node_modules', 'dist', 'build', 'out', '.git'].includes(file)) {
        findTypeScriptFiles(filePath, fileList);
      }
    } else if (file.endsWith('.ts') || file.endsWith('.tsx')) {
      // Skip test files and declaration files
      if (!file.endsWith('.test.ts') && 
          !file.endsWith('.test.tsx') && 
          !file.endsWith('.spec.ts') &&
          !file.endsWith('.spec.tsx') &&
          !file.endsWith('.d.ts')) {
        fileList.push(filePath);
      }
    }
  });
  
  return fileList;
}

// Main execution
function main() {
  const rootDir = path.join(__dirname, '..');
  const packagesDir = path.join(rootDir, 'packages');
  
  console.log('🔍 Finding TypeScript source files...\n');
  
  const files = findTypeScriptFiles(packagesDir);
  console.log(`Found ${files.length} TypeScript files\n`);
  
  console.log('📝 Adding license headers...\n');
  
  let added = 0;
  let skipped = 0;
  let errors = 0;
  
  files.forEach(file => {
    const relativePath = path.relative(rootDir, file);
    const result = addLicenseHeader(file);
    
    if (result.status === 'added') {
      console.log(`✅ Added: ${relativePath}`);
      added++;
    } else if (result.status === 'skipped') {
      // Don't print skipped files to reduce noise
      skipped++;
    } else if (result.status === 'error') {
      console.log(`❌ Error: ${relativePath} - ${result.reason}`);
      errors++;
    }
  });
  
  console.log('\n' + '='.repeat(60));
  console.log('📊 Summary:');
  console.log(`   Total files: ${files.length}`);
  console.log(`   ✅ Headers added: ${added}`);
  console.log(`   ⏭️  Skipped: ${skipped}`);
  console.log(`   ❌ Errors: ${errors}`);
  console.log('='.repeat(60));
  
  if (errors > 0) {
    process.exit(1);
  }
}

main();
