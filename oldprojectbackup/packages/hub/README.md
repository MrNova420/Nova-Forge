# Nova Hub - Platform Server

Backend server for the Nova Engine ecosystem, providing game hosting, distribution, and collaboration features.

## Features

- **User Authentication**: JWT-based authentication with bcrypt password hashing
- **Project Management**: Store and version control game projects
- **Asset Storage**: Cloud storage for game assets with optimization
- **Build Pipeline**: Automated builds for web, desktop, and mobile
- **API**: RESTful API with FastifyJS
- **Caching**: Redis for high-performance caching
- **Database**: PostgreSQL for reliable data storage

## Architecture

```
packages/hub/
├── src/
│   ├── api/           # API route handlers
│   ├── auth/          # Authentication services
│   ├── database/      # Database and Redis services
│   ├── services/      # Business logic services
│   ├── storage/       # File storage management
│   ├── utils/         # Utility functions
│   ├── index.ts       # Server entry point
│   └── server.ts      # Fastify server configuration
├── package.json
├── tsconfig.json
└── README.md
```

## Setup

### Prerequisites

- Node.js 18+
- PostgreSQL 14+
- Redis 6+

### Environment Variables

Create a `.env` file:

```env
# Server
PORT=3001
HOST=0.0.0.0
NODE_ENV=development

# Database
DB_HOST=localhost
DB_PORT=5432
DB_NAME=nova_hub
DB_USER=postgres
DB_PASSWORD=your_password

# Redis
REDIS_URL=redis://localhost:6379

# JWT
JWT_SECRET=your-secret-key-change-in-production

# CORS
CORS_ORIGIN=*

# Logging
LOG_LEVEL=info
```

### Installation

```bash
# Install dependencies
npm install

# Run database migrations (creates tables)
npm run dev  # First run initializes schema

# Start development server
npm run dev
```

## Development

```bash
# Development with hot reload
npm run dev

# Build
npm run build

# Production
npm start

# Tests
npm test

# Linting
npm run lint

# Format code
npm run format
```

## API Endpoints

### Authentication

- `POST /api/auth/register` - Register new user
- `POST /api/auth/login` - Login user
- `GET /api/auth/me` - Get current user (requires auth)
- `POST /api/auth/logout` - Logout user

### Projects

- `GET /api/projects` - List user projects
- `POST /api/projects` - Create new project
- `GET /api/projects/:id` - Get project details
- `PUT /api/projects/:id` - Update project
- `DELETE /api/projects/:id` - Delete project

### Assets

- `POST /api/assets/upload` - Upload asset
- `GET /api/assets/:id` - Download asset
- `DELETE /api/assets/:id` - Delete asset

### Builds

- `POST /api/builds` - Create build job
- `GET /api/builds/:id` - Get build status
- `GET /api/builds/:id/download` - Download build

## Database Schema

### Users

- id (UUID)
- email (unique)
- username (unique)
- password_hash
- display_name
- avatar_url
- created_at, updated_at, last_login
- is_active, is_verified

### Projects

- id (UUID)
- owner_id (FK to users)
- name
- description
- visibility (public/private/unlisted)
- created_at, updated_at
- size_bytes, asset_count

### Assets

- id (UUID)
- project_id (FK to projects)
- path
- type
- size_bytes
- hash
- storage_url
- created_at, updated_at

### Builds

- id (UUID)
- project_id (FK to projects)
- target (web/desktop/mobile)
- status (queued/building/success/failed)
- build_url
- created_at, started_at, completed_at

## Security

- Passwords hashed with bcrypt (10 rounds)
- JWT tokens for authentication
- Rate limiting on all endpoints
- CORS configuration
- Input validation with Zod schemas
- SQL injection prevention (parameterized queries)

## Performance

- Redis caching for frequently accessed data
- PostgreSQL connection pooling
- Efficient database indexes
- Multipart file uploads with streaming
- Gzip compression for API responses

## Deployment

### Docker

```dockerfile
# Coming soon
```

### Cloud Platforms

Compatible with:

- AWS (EC2, RDS, ElastiCache)
- Google Cloud Platform
- Azure
- DigitalOcean
- Heroku

## License

Proprietary - See [LICENSE](../../LICENSE) file

Nova Engine is closed-source proprietary software. See [LEGAL.md](../../LEGAL.md) for complete licensing terms.

## Contributing

See CONTRIBUTING.md in the root directory

---

**Status**: Phase 7 In Development
**Version**: 0.1.0
