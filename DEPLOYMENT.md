# Nova Engine - Deployment Guide

**Version**: 0.1.0  
**Last Updated**: 2025-11-19  
**Status**: Production-Ready Infrastructure

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Environment Setup](#environment-setup)
3. [Build & Deployment](#build--deployment)
4. [Platform-Specific Deployment](#platform-specific-deployment)
5. [Monitoring & Maintenance](#monitoring--maintenance)
6. [Rollback Procedures](#rollback-procedures)
7. [Security Considerations](#security-considerations)

---

## Prerequisites

### System Requirements

**Development Environment**:

- Node.js >= 18.0.0
- npm >= 9.0.0
- Git >= 2.30.0
- TypeScript >= 5.0.0

**Production Environment**:

- Node.js 18 LTS (recommended)
- PostgreSQL >= 14.0
- Redis >= 6.2 (optional, for rate limiting)
- Nginx >= 1.20 (reverse proxy)
- SSL Certificate (Let's Encrypt recommended)

### Required Secrets

Ensure all environment variables are configured:

```bash
# Critical - Must be set
JWT_SECRET=<minimum 32 characters>
SESSION_SECRET=<minimum 32 characters>
DATABASE_PASSWORD=<strong password>

# Database
DATABASE_HOST=<database host>
DATABASE_PORT=5432
DATABASE_NAME=nova_engine
DATABASE_USER=nova_engine_user

# Optional but recommended
SENTRY_DSN=<error tracking>
AWS_ACCESS_KEY_ID=<for S3 storage>
AWS_SECRET_ACCESS_KEY=<for S3 storage>
```

---

## Environment Setup

### 1. Clone Repository

```bash
git clone https://github.com/MrNova420/Nova-Engine.git
cd Nova-Engine
```

### 2. Install Dependencies

```bash
npm install
```

### 3. Configure Environment

```bash
# Copy environment template
cp .env.example .env

# Edit .env with production values
nano .env
```

### 4. Database Setup

```bash
# Create PostgreSQL database
psql -U postgres
CREATE DATABASE nova_engine;
CREATE USER nova_engine_user WITH PASSWORD 'your_secure_password';
GRANT ALL PRIVILEGES ON DATABASE nova_engine TO nova_engine_user;
\q

# Run migrations (when available)
npm run db:migrate
```

---

## Build & Deployment

### Production Build

```bash
# Build all packages
npm run build

# Verify build
npm test

# Check for security vulnerabilities
npm audit
```

### Environment-Specific Builds

```bash
# Development
NODE_ENV=development npm run build

# Staging
NODE_ENV=staging npm run build

# Production
NODE_ENV=production npm run build
```

---

## Platform-Specific Deployment

### Web Platform (Unified Platform)

**Build**:

```bash
npm run build -w @nova-engine/unified-platform
```

**Deploy to Nginx**:

```bash
# Copy build artifacts
sudo cp -r packages/unified-platform/dist/* /var/www/nova-engine/

# Nginx configuration example
sudo nano /etc/nginx/sites-available/nova-engine
```

**Nginx Config**:

```nginx
server {
    listen 80;
    listen [::]:80;
    server_name nova-engine.com www.nova-engine.com;

    # Redirect to HTTPS
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    listen [::]:443 ssl http2;
    server_name nova-engine.com www.nova-engine.com;

    # SSL Configuration
    ssl_certificate /etc/letsencrypt/live/nova-engine.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/nova-engine.com/privkey.pem;
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers HIGH:!aNULL:!MD5;

    # Security Headers
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-XSS-Protection "1; mode=block" always;
    add_header Referrer-Policy "strict-origin-when-cross-origin" always;
    add_header Strict-Transport-Security "max-age=31536000; includeSubDomains; preload" always;

    # Root directory
    root /var/www/nova-engine;
    index index.html;

    # Static files caching
    location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg|woff|woff2|ttf|eot)$ {
        expires 1y;
        add_header Cache-Control "public, immutable";
    }

    # API proxy
    location /api {
        proxy_pass http://localhost:3001;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_cache_bypass $http_upgrade;
    }

    # WebSocket support
    location /ws {
        proxy_pass http://localhost:3001;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }

    # SPA fallback
    location / {
        try_files $uri $uri/ /index.html;
    }
}
```

### Backend Server (Hub/Server)

**Using PM2 (Recommended)**:

```bash
# Install PM2 globally
npm install -g pm2

# Start server
pm2 start packages/server/dist/index.js --name "nova-engine-server"

# Save PM2 configuration
pm2 save

# Setup PM2 startup script
pm2 startup
```

**PM2 Ecosystem File** (`ecosystem.config.js`):

```javascript
module.exports = {
  apps: [
    {
      name: 'nova-engine-server',
      script: './packages/server/dist/index.js',
      instances: 'max',
      exec_mode: 'cluster',
      env: {
        NODE_ENV: 'production',
        PORT: 3001,
      },
      error_file: './logs/err.log',
      out_file: './logs/out.log',
      log_file: './logs/combined.log',
      time: true,
    },
  ],
};
```

**Start with Ecosystem**:

```bash
pm2 start ecosystem.config.js
```

### Desktop Launcher (Tauri)

```bash
# Build desktop application
npm run build -w @nova-engine/launcher-desktop

# Installers will be in packages/launcher-desktop/src-tauri/target/release/bundle/
```

**Supported Platforms**:

- Windows: `.msi`, `.exe`
- macOS: `.dmg`, `.app`
- Linux: `.deb`, `.AppImage`

### Mobile Launcher

```bash
# Android
npm run build -w @nova-engine/launcher-mobile
cd packages/launcher-mobile/android
./gradlew assembleRelease

# iOS (requires macOS)
cd packages/launcher-mobile/ios
pod install
xcodebuild -workspace NovaEngine.xcworkspace -scheme NovaEngine -configuration Release
```

---

## Monitoring & Maintenance

### Health Checks

**Endpoint**: `GET /api/health`

```bash
curl https://nova-engine.com/api/health
```

**Expected Response**:

```json
{
  "status": "healthy",
  "timestamp": "2025-11-19T18:00:00.000Z",
  "version": "0.1.0",
  "uptime": 123456
}
```

### Monitoring Services

**Sentry (Error Tracking)**:

```bash
# Already integrated in code
# Configure SENTRY_DSN in .env
```

**PM2 Monitoring**:

```bash
pm2 monit              # Real-time monitoring
pm2 logs               # View logs
pm2 status             # Check status
pm2 restart all        # Restart all services
```

### Log Management

**Log Locations**:

- PM2 Logs: `./logs/`
- Nginx Logs: `/var/log/nginx/`
- Application Logs: `./logs/application.log`

**Log Rotation**:

```bash
# Install logrotate configuration
sudo nano /etc/logrotate.d/nova-engine
```

**Logrotate Config**:

```
/var/www/nova-engine/logs/*.log {
    daily
    rotate 14
    compress
    delaycompress
    notifempty
    create 0640 www-data www-data
    sharedscripts
    postrotate
        pm2 reloadLogs
    endscript
}
```

---

## Rollback Procedures

### Quick Rollback

```bash
# Using PM2
pm2 stop nova-engine-server
git checkout <previous-commit>
npm run build
pm2 restart nova-engine-server
```

### Database Rollback

```bash
# If migrations exist
npm run db:rollback

# Manual rollback
psql -U nova_engine_user nova_engine < backup.sql
```

### Backup Strategy

**Database Backup**:

```bash
# Create backup
pg_dump -U nova_engine_user nova_engine > backup_$(date +%Y%m%d_%H%M%S).sql

# Automated daily backups (crontab)
0 2 * * * pg_dump -U nova_engine_user nova_engine > /backups/nova_engine_$(date +\%Y\%m\%d).sql
```

**File Backup**:

```bash
# Backup uploads and user data
tar -czf backup_files_$(date +%Y%m%d).tar.gz uploads/ user-data/
```

---

## Security Considerations

### SSL/TLS

**Let's Encrypt Setup**:

```bash
sudo apt install certbot python3-certbot-nginx
sudo certbot --nginx -d nova-engine.com -d www.nova-engine.com
```

**Auto-renewal**:

```bash
sudo certbot renew --dry-run
```

### Firewall Configuration

```bash
# UFW (Ubuntu)
sudo ufw allow 22/tcp      # SSH
sudo ufw allow 80/tcp      # HTTP
sudo ufw allow 443/tcp     # HTTPS
sudo ufw enable
```

### Environment Security

**Never commit**:

- `.env` files
- Private keys
- Database credentials
- API secrets

**Use Secrets Management**:

- AWS Secrets Manager
- HashiCorp Vault
- GitHub Secrets (for CI/CD)

### Security Checklist

- [ ] All secrets in environment variables
- [ ] JWT_SECRET minimum 32 characters
- [ ] SESSION_SECRET minimum 32 characters
- [ ] SSL/TLS certificates installed
- [ ] Firewall configured
- [ ] Database user with limited privileges
- [ ] Rate limiting enabled
- [ ] Security headers configured
- [ ] Regular security audits (`npm audit`)
- [ ] Dependencies up to date
- [ ] Backups configured
- [ ] Monitoring enabled

---

## CI/CD Integration

### GitHub Actions

Workflow file already configured in `.github/workflows/ci.yml`

**Automatic Deployment**:

```yaml
# Add to workflow for auto-deploy
- name: Deploy to Production
  if: github.ref == 'refs/heads/main'
  run: |
    ssh deploy@nova-engine.com "cd /var/www/nova-engine && ./deploy.sh"
```

### Deploy Script Example

Create `deploy.sh`:

```bash
#!/bin/bash
set -e

echo "Starting deployment..."

# Pull latest changes
git pull origin main

# Install dependencies
npm install

# Build
npm run build

# Run tests
npm test

# Restart services
pm2 restart all

# Reload nginx
sudo nginx -t && sudo systemctl reload nginx

echo "Deployment complete!"
```

---

## Troubleshooting

### Common Issues

**Build Fails**:

```bash
# Clear cache and rebuild
rm -rf node_modules package-lock.json
npm install
npm run build
```

**Database Connection Issues**:

```bash
# Check PostgreSQL is running
sudo systemctl status postgresql

# Test connection
psql -U nova_engine_user -h localhost nova_engine
```

**High Memory Usage**:

```bash
# Check PM2 processes
pm2 list
pm2 monit

# Restart if needed
pm2 restart all
```

---

## Support

**Documentation**: See README.md, CONTRIBUTING.md  
**Issues**: https://github.com/MrNova420/Nova-Engine/issues  
**Security**: See SECURITY_AUDIT.md

---

**Last Updated**: 2025-11-19  
**Version**: 0.1.0  
**Status**: Production-Ready
