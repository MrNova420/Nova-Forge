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

import React, { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import { apiService } from '../services/apiService';
import { Game } from '../store/gameStore';
import GameCard from '../components/GameCard';

const HomePage: React.FC = () => {
  const [featuredGames, setFeaturedGames] = useState<Game[]>([]);
  const [newGames, setNewGames] = useState<Game[]>([]);
  const [topRated, setTopRated] = useState<Game[]>([]);
  const [, setIsLoading] = useState(true);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [featured, recent, top] = await Promise.all([
          apiService.getFeaturedGames(),
          apiService.getNewGames(),
          apiService.getTopRatedGames(),
        ]);
        setFeaturedGames(featured.slice(0, 3));
        setNewGames(recent.slice(0, 6));
        setTopRated(top.slice(0, 6));
      } catch (error) {
        console.error('Failed to fetch games:', error);
      } finally {
        setIsLoading(false);
      }
    };

    fetchData();
  }, []);

  return (
    <div className="container py-12">
      {/* Hero Section */}
      <section className="text-center mb-16">
        <h1 className="text-5xl font-bold mb-4">
          Play Amazing Games Built with{' '}
          <span className="text-accent">Nova Engine</span>
        </h1>
        <p className="text-xl text-text-secondary mb-8">
          Discover, play, and download games created by our community
        </p>
        <Link to="/games" className="btn btn-primary btn-lg">
          Browse All Games
        </Link>
      </section>

      {/* Featured Games */}
      <section className="mb-16">
        <div className="flex items-center justify-between mb-6">
          <h2 className="text-3xl font-bold">Featured Games</h2>
          <Link
            to="/games?filter=featured"
            className="text-accent hover:underline"
          >
            View All →
          </Link>
        </div>
        <div className="grid grid-cols-3 gap-6">
          {featuredGames.map((game) => (
            <GameCard key={game.id} game={game} />
          ))}
        </div>
      </section>

      {/* New Releases */}
      <section className="mb-16">
        <div className="flex items-center justify-between mb-6">
          <h2 className="text-3xl font-bold">New Releases</h2>
          <Link to="/games?filter=new" className="text-accent hover:underline">
            View All →
          </Link>
        </div>
        <div className="grid grid-cols-3 gap-6">
          {newGames.map((game) => (
            <GameCard key={game.id} game={game} />
          ))}
        </div>
      </section>

      {/* Top Rated */}
      <section>
        <div className="flex items-center justify-between mb-6">
          <h2 className="text-3xl font-bold">Top Rated</h2>
          <Link
            to="/games?filter=top-rated"
            className="text-accent hover:underline"
          >
            View All →
          </Link>
        </div>
        <div className="grid grid-cols-3 gap-6">
          {topRated.map((game) => (
            <GameCard key={game.id} game={game} />
          ))}
        </div>
      </section>

      {/* Call to Action */}
      <section className="mt-16 text-center bg-bg-secondary rounded-xl p-12">
        <h2 className="text-3xl font-bold mb-4">
          Ready to Create Your Own Game?
        </h2>
        <p className="text-xl text-text-secondary mb-8">
          Start building with Nova Engine Editor today
        </p>
        <a href="/editor" className="btn btn-primary btn-lg">
          Open Editor
        </a>
      </section>
    </div>
  );
};

export default HomePage;
