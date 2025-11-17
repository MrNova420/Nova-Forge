import { Game } from '../store/gameStore';

export const mockGames: Game[] = [
  {
    id: '1',
    title: 'Space Explorer',
    description: 'Explore the vast universe in this epic space adventure',
    developer: 'Nova Studios',
    coverImage:
      'https://via.placeholder.com/400x300/1a1a1a/ffffff?text=Space+Explorer',
    rating: 4.5,
    downloads: 12500,
    category: 'Action',
    tags: ['Space', 'Adventure', 'Sci-Fi'],
    version: '1.2.0',
    size: '500 MB',
    lastUpdated: '2025-11-15',
  },
  {
    id: '2',
    title: 'Puzzle Master',
    description: 'Challenge your mind with hundreds of unique puzzles',
    developer: 'Brain Games Inc',
    coverImage:
      'https://via.placeholder.com/400x300/2a2a2a/ffffff?text=Puzzle+Master',
    rating: 4.8,
    downloads: 25000,
    category: 'Puzzle',
    tags: ['Puzzle', 'Strategy', 'Brain'],
    version: '2.0.1',
    size: '150 MB',
    lastUpdated: '2025-11-12',
  },
  {
    id: '3',
    title: 'Racing Legends',
    description: 'Feel the speed in this high-octane racing game',
    developer: 'Speed Demons',
    coverImage:
      'https://via.placeholder.com/400x300/3a3a3a/ffffff?text=Racing+Legends',
    rating: 4.6,
    downloads: 18000,
    category: 'Racing',
    tags: ['Racing', 'Multiplayer', 'Action'],
    version: '1.5.2',
    size: '800 MB',
    lastUpdated: '2025-11-10',
  },
  {
    id: '4',
    title: 'Medieval Quest',
    description: 'Embark on an epic fantasy adventure',
    developer: 'Fantasy Games',
    coverImage:
      'https://via.placeholder.com/400x300/4a4a4a/ffffff?text=Medieval+Quest',
    rating: 4.7,
    downloads: 15000,
    category: 'RPG',
    tags: ['RPG', 'Fantasy', 'Adventure'],
    version: '3.1.0',
    size: '1.2 GB',
    lastUpdated: '2025-11-08',
  },
  {
    id: '5',
    title: 'City Builder Pro',
    description: 'Build and manage your dream city',
    developer: 'Urban Dev',
    coverImage:
      'https://via.placeholder.com/400x300/5a5a5a/ffffff?text=City+Builder+Pro',
    rating: 4.4,
    downloads: 20000,
    category: 'Simulation',
    tags: ['Simulation', 'Strategy', 'Building'],
    version: '2.3.0',
    size: '600 MB',
    lastUpdated: '2025-11-05',
  },
  {
    id: '6',
    title: 'Shooter Arena',
    description: 'Fast-paced multiplayer shooter action',
    developer: 'Combat Studios',
    coverImage:
      'https://via.placeholder.com/400x300/6a6a6a/ffffff?text=Shooter+Arena',
    rating: 4.3,
    downloads: 30000,
    category: 'Action',
    tags: ['Shooter', 'Multiplayer', 'Competitive'],
    version: '1.8.5',
    size: '950 MB',
    lastUpdated: '2025-11-03',
  },
];

export const getMockGames = (category?: string, search?: string): Game[] => {
  let filtered = [...mockGames];

  if (category) {
    filtered = filtered.filter((game) => game.category === category);
  }

  if (search) {
    const searchLower = search.toLowerCase();
    filtered = filtered.filter(
      (game) =>
        game.title.toLowerCase().includes(searchLower) ||
        game.description.toLowerCase().includes(searchLower) ||
        game.tags.some((tag) => tag.toLowerCase().includes(searchLower))
    );
  }

  return filtered;
};

export const getMockFeaturedGames = (): Game[] => {
  return mockGames.filter((game) => game.rating >= 4.5);
};

export const getMockNewGames = (): Game[] => {
  return [...mockGames].sort(
    (a, b) =>
      new Date(b.lastUpdated).getTime() - new Date(a.lastUpdated).getTime()
  );
};

export const getMockTopRatedGames = (): Game[] => {
  return [...mockGames].sort((a, b) => b.rating - a.rating);
};

export const getMockGameById = (id: string): Game | undefined => {
  return mockGames.find((game) => game.id === id);
};
