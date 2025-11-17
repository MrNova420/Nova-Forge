import { Game } from '../store/gameStore';

export const mockGames: Game[] = [
  {
    id: '1',
    title: 'Space Explorer',
    description: 'Explore the vast universe in this epic space adventure',
    thumbnail:
      'https://via.placeholder.com/400x300/1a1a1a/ffffff?text=Space+Explorer',
    screenshots: [],
    author: 'Nova Studios',
    rating: 4.5,
    ratingCount: 125,
    downloadCount: 12500,
    playCount: 25000,
    category: 'Action',
    tags: ['Space', 'Adventure', 'Sci-Fi'],
    version: '1.2.0',
    size: 524288000, // 500 MB in bytes
    platforms: ['web', 'desktop'],
    publishedAt: '2025-11-15',
  },
  {
    id: '2',
    title: 'Puzzle Master',
    description: 'Challenge your mind with hundreds of unique puzzles',
    thumbnail:
      'https://via.placeholder.com/400x300/2a2a2a/ffffff?text=Puzzle+Master',
    screenshots: [],
    author: 'Brain Games Inc',
    rating: 4.8,
    ratingCount: 250,
    downloadCount: 25000,
    playCount: 50000,
    category: 'Puzzle',
    tags: ['Puzzle', 'Strategy', 'Brain'],
    version: '2.0.1',
    size: 157286400, // 150 MB in bytes
    platforms: ['web', 'mobile'],
    publishedAt: '2025-11-12',
  },
  {
    id: '3',
    title: 'Racing Legends',
    description: 'Feel the speed in this high-octane racing game',
    thumbnail:
      'https://via.placeholder.com/400x300/3a3a3a/ffffff?text=Racing+Legends',
    screenshots: [],
    author: 'Speed Demons',
    rating: 4.6,
    ratingCount: 180,
    downloadCount: 18000,
    playCount: 36000,
    category: 'Racing',
    tags: ['Racing', 'Multiplayer', 'Action'],
    version: '1.5.2',
    size: 838860800, // 800 MB in bytes
    platforms: ['web', 'desktop'],
    publishedAt: '2025-11-10',
  },
  {
    id: '4',
    title: 'Medieval Quest',
    description: 'Embark on an epic fantasy adventure',
    thumbnail:
      'https://via.placeholder.com/400x300/4a4a4a/ffffff?text=Medieval+Quest',
    screenshots: [],
    author: 'Fantasy Games',
    rating: 4.7,
    ratingCount: 150,
    downloadCount: 15000,
    playCount: 30000,
    category: 'RPG',
    tags: ['RPG', 'Fantasy', 'Adventure'],
    version: '3.1.0',
    size: 1288490189, // 1.2 GB in bytes
    platforms: ['web', 'desktop', 'console'],
    publishedAt: '2025-11-08',
  },
  {
    id: '5',
    title: 'City Builder Pro',
    description: 'Build and manage your dream city',
    thumbnail:
      'https://via.placeholder.com/400x300/5a5a5a/ffffff?text=City+Builder+Pro',
    screenshots: [],
    author: 'Urban Dev',
    rating: 4.4,
    ratingCount: 200,
    downloadCount: 20000,
    playCount: 40000,
    category: 'Simulation',
    tags: ['Simulation', 'Strategy', 'Building'],
    version: '2.3.0',
    size: 629145600, // 600 MB in bytes
    platforms: ['web', 'desktop', 'mobile'],
    publishedAt: '2025-11-05',
  },
  {
    id: '6',
    title: 'Shooter Arena',
    description: 'Fast-paced multiplayer shooter action',
    thumbnail:
      'https://via.placeholder.com/400x300/6a6a6a/ffffff?text=Shooter+Arena',
    screenshots: [],
    author: 'Combat Studios',
    rating: 4.3,
    ratingCount: 300,
    downloadCount: 30000,
    playCount: 60000,
    category: 'Action',
    tags: ['Shooter', 'Multiplayer', 'Competitive'],
    version: '1.8.5',
    size: 995932160, // 950 MB in bytes
    platforms: ['web', 'desktop'],
    publishedAt: '2025-11-03',
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
      new Date(b.publishedAt).getTime() - new Date(a.publishedAt).getTime()
  );
};

export const getMockTopRatedGames = (): Game[] => {
  return [...mockGames].sort((a, b) => b.rating - a.rating);
};

export const getMockGameById = (id: string): Game | undefined => {
  return mockGames.find((game) => game.id === id);
};
