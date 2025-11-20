# Nova Engine Backend Server

Complete backend system for Nova Engine platform with authentication, database, and API endpoints.

## Features

- ✅ User authentication (register/login) with JWT
- ✅ PostgreSQL database with comprehensive schema
- ✅ Secure password hashing with bcrypt
- ✅ CORS and security middleware
- ✅ TypeScript support
- ✅ Environment configuration

## Setup

1. Install dependencies:

```bash
npm install
```

2. Set up PostgreSQL database:

```bash
createdb nova_engine
```

3. Configure environment variables:

```bash
cp .env.example .env
# Edit .env with your database credentials
```

4. Run in development:

```bash
npm run dev
```

5. Build for production:

```bash
npm run build
npm start
```

## API Endpoints

### Authentication

- `POST /api/auth/register` - Register new user
- `POST /api/auth/login` - Login user

### Health Check

- `GET /health` - Server health status

## Database Schema

- **users** - User accounts and profiles
- **games** - Game information
- **user_sessions** - JWT refresh tokens
- More tables for achievements, friends, multiplayer, etc.

## Technologies

- Express.js
- PostgreSQL
- JWT
- bcrypt
- TypeScript
- Helmet (security)
- CORS
