# Nova Launcher Web

The web-based game launcher and player platform for Nova Engine games.

## Features

### Game Discovery
- **Browse Games**: Explore all published games with filters and search
- **Categories**: Filter by Action, Adventure, RPG, Strategy, Puzzle, and more
- **Featured Games**: Curated selection on the homepage
- **New Releases**: Latest games from the community
- **Top Rated**: Highest-rated games by players

### Game Player
- **Instant Play**: Click and play games directly in your browser
- **Fullscreen Mode**: Immersive gaming experience
- **Volume Controls**: Adjust game audio
- **Save Data**: Automatic save/load via IndexedDB
- **Performance Metrics**: Real-time FPS, draw calls, and entity counts

### User Features
- **User Accounts**: Register, login, profile management
- **Game Library**: Track all games you've played
- **Recently Played**: Quick access to recent games
- **Favorites**: Mark games you love
- **Rating System**: Rate games 1-5 stars
- **Play Statistics**: Track playtime and progress

### Social Features (Planned)
- Friends system
- Achievements
- Leaderboards
- Comments and reviews
- Activity feed

## Technology Stack

- **Framework**: React 18 with TypeScript
- **Build Tool**: Vite
- **Router**: React Router v6
- **State**: Zustand for global state
- **Data Fetching**: TanStack Query (React Query)
- **API Client**: Axios
- **Styling**: CSS with CSS variables (dark theme)

## Getting Started

### Installation

```bash
cd packages/launcher-web
npm install
```

### Development

```bash
npm run dev
```

Runs the launcher on `http://localhost:3001`

API requests are proxied to the Nova Hub server at `http://localhost:3000`

### Build

```bash
npm run build
```

Builds for production to the `dist/` directory.

### Type Checking

```bash
npm run typecheck
```

## Project Structure

```
launcher-web/
├── src/
│   ├── components/     # Reusable UI components
│   │   ├── Layout.tsx
│   │   ├── GameCard.tsx
│   │   ├── GameGrid.tsx
│   │   └── GamePlayer.tsx
│   ├── pages/          # Page components
│   │   ├── HomePage.tsx
│   │   ├── GameBrowserPage.tsx
│   │   ├── GameDetailPage.tsx
│   │   ├── GamePlayerPage.tsx
│   │   ├── ProfilePage.tsx
│   │   └── LibraryPage.tsx
│   ├── services/       # API services
│   │   └── apiService.ts
│   ├── store/          # State management
│   │   ├── authStore.ts
│   │   └── gameStore.ts
│   ├── hooks/          # Custom React hooks
│   ├── styles/         # Global styles
│   │   └── index.css
│   ├── types/          # TypeScript types
│   ├── App.tsx         # Main app component
│   └── main.tsx        # App entry point
├── index.html
├── vite.config.ts
├── tsconfig.json
└── package.json
```

## API Integration

The launcher communicates with the Nova Hub backend through REST APIs:

### Authentication
- `POST /api/auth/register` - Register new user
- `POST /api/auth/login` - Login
- `GET /api/auth/me` - Get current user

### Games
- `GET /api/games` - Browse all games (with filters)
- `GET /api/games/featured` - Featured games
- `GET /api/games/new` - New releases
- `GET /api/games/top-rated` - Top rated games
- `GET /api/games/:id` - Game details
- `GET /api/games/:id/play` - Play game (increments counter)
- `GET /api/games/:id/download/:platform` - Download game
- `POST /api/games/:id/rate` - Rate game

### Library
- `GET /api/library` - User's game library
- `POST /api/library` - Add game to library
- `DELETE /api/library/:id` - Remove from library

### Multiplayer
- `POST /api/multiplayer/rooms` - Create room
- `GET /api/multiplayer/rooms/:gameId` - List rooms
- `POST /api/multiplayer/rooms/join` - Join room
- `POST /api/multiplayer/quickmatch` - Quick match

## Game Player Integration

The launcher integrates with Nova Engine Runtime to play games:

```typescript
// Load game from Hub CDN
const gameData = await fetch(`/cdn/games/${gameId}/game.json`);

// Initialize Nova Runtime
const runtime = new NovaRuntime(canvas);
await runtime.load(gameData);

// Start game
runtime.start();

// Handle saves
runtime.on('save', (data) => {
  localStorage.setItem(`game_${gameId}_save`, JSON.stringify(data));
});
```

## Progressive Web App (PWA)

The launcher can be installed as a PWA:

- Offline game playing (for downloaded games)
- Desktop/mobile app experience
- Push notifications for game updates
- Background sync for save data

## Deployment

### Production Build

```bash
npm run build
```

### Deploy to CDN

The built files can be deployed to any static hosting:

- Vercel
- Netlify
- CloudFlare Pages
- AWS S3 + CloudFront
- GitHub Pages

### Environment Variables

Create `.env.production`:

```env
VITE_API_URL=https://api.novaengine.com
VITE_CDN_URL=https://cdn.novaengine.com
```

## Browser Support

- Chrome/Edge 90+
- Firefox 88+
- Safari 14+
- Opera 76+

Requires:
- ES2020 support
- WebGL 2.0 or WebGPU
- Web Audio API
- IndexedDB

## Performance

- Initial bundle: ~210 KB (gzipped: ~67 KB)
- Code splitting per route
- Lazy loading for game assets
- Service worker caching
- Image optimization

## Roadmap

### Phase 8.1 - Launch (Current)
- [x] Game browser interface
- [x] Game detail pages
- [x] Game player integration
- [x] User accounts
- [x] Game library
- [x] Rating system

### Phase 8.2 - Social Features
- [ ] Friends system
- [ ] Activity feed
- [ ] Comments and reviews
- [ ] Game sharing
- [ ] Achievements

### Phase 8.3 - Multiplayer
- [ ] Lobby browser
- [ ] Quick match
- [ ] Friends invites
- [ ] Voice chat
- [ ] Spectator mode

## Contributing

See [CONTRIBUTING.md](../../CONTRIBUTING.md) for development guidelines.

## License

MIT License - see [LICENSE](../../LICENSE) for details.

---

**Nova Launcher Web** - Play amazing games built with Nova Engine! 🎮
