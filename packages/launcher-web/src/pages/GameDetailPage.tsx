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

import React, { useState } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { useQuery, useMutation } from '@tanstack/react-query';
import { apiService } from '../services/apiService';
import { useAuthStore } from '../store/authStore';

const GameDetailPage: React.FC = () => {
  const { gameId } = useParams<{ gameId: string }>();
  const navigate = useNavigate();
  const { isAuthenticated } = useAuthStore();
  const [selectedScreenshot, setSelectedScreenshot] = useState(0);
  const [userRating, setUserRating] = useState(0);

  const { data: game, isLoading } = useQuery({
    queryKey: ['game', gameId],
    queryFn: () => apiService.getGameById(gameId!),
    enabled: !!gameId,
  });

  const rateMutation = useMutation({
    mutationFn: (rating: number) => apiService.rateGame(gameId!, rating),
  });

  const handlePlay = () => {
    navigate(`/play/${gameId}`);
  };

  const handleDownload = async (platform: string) => {
    try {
      const blob = await apiService.downloadGame(gameId!, platform);
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `${game.title}-${platform}.zip`;
      document.body.appendChild(a);
      a.click();
      window.URL.revokeObjectURL(url);
      document.body.removeChild(a);
    } catch (error) {
      console.error('Download failed:', error);
      alert('Download failed. Please try again.');
    }
  };

  const handleRate = (rating: number) => {
    if (!isAuthenticated) {
      alert('Please login to rate games');
      return;
    }
    setUserRating(rating);
    rateMutation.mutate(rating);
  };

  if (isLoading || !game) {
    return (
      <div className="container py-12">
        <div className="animate-pulse">
          <div className="h-96 bg-bg-secondary rounded-xl mb-8"></div>
          <div className="h-12 bg-bg-secondary rounded w-1/3 mb-4"></div>
          <div className="h-6 bg-bg-secondary rounded w-1/4 mb-8"></div>
          <div className="h-32 bg-bg-secondary rounded"></div>
        </div>
      </div>
    );
  }

  return (
    <div className="container py-8">
      {/* Back Button */}
      <button onClick={() => navigate(-1)} className="btn btn-secondary mb-6">
        ← Back
      </button>

      <div className="grid grid-cols-3 gap-8">
        {/* Left Column - Media */}
        <div className="col-span-2">
          {/* Main Screenshot */}
          <div className="aspect-video bg-bg-secondary rounded-xl overflow-hidden mb-4">
            <img
              src={game.screenshots[selectedScreenshot] || game.thumbnail}
              alt={`${game.title} screenshot ${selectedScreenshot + 1}`}
              className="w-full h-full object-cover"
            />
          </div>

          {/* Thumbnail Gallery */}
          <div className="grid grid-cols-5 gap-2 mb-8">
            {game.screenshots.map((screenshot: string, index: number) => (
              <button
                key={index}
                onClick={() => setSelectedScreenshot(index)}
                className={`aspect-video rounded overflow-hidden border-2 transition ${
                  selectedScreenshot === index
                    ? 'border-accent'
                    : 'border-transparent'
                }`}
              >
                <img
                  src={screenshot}
                  alt={`Thumbnail ${index + 1}`}
                  className="w-full h-full object-cover"
                />
              </button>
            ))}
          </div>

          {/* Description */}
          <div className="mb-8">
            <h2 className="text-2xl font-bold mb-4">About This Game</h2>
            <p className="text-text-secondary leading-relaxed whitespace-pre-wrap">
              {game.description}
            </p>
          </div>

          {/* Tags */}
          <div className="mb-8">
            <h3 className="text-lg font-bold mb-3">Tags</h3>
            <div className="flex gap-2 flex-wrap">
              {game.tags.map((tag: string) => (
                <span
                  key={tag}
                  className="px-3 py-1 bg-bg-secondary rounded-full text-sm"
                >
                  {tag}
                </span>
              ))}
            </div>
          </div>
        </div>

        {/* Right Column - Info & Actions */}
        <div>
          <div className="card p-6 mb-4">
            <h1 className="text-3xl font-bold mb-2">{game.title}</h1>
            <p className="text-text-secondary mb-6">by {game.author}</p>

            {/* Play Button */}
            <button
              onClick={handlePlay}
              className="btn btn-primary w-full mb-3 py-4 text-lg"
            >
              ▶️ Play Now
            </button>

            {/* Download Options */}
            {game.platforms.length > 0 && (
              <div className="mb-6">
                <p className="text-sm text-text-secondary mb-2">
                  Download for:
                </p>
                {game.platforms.map((platform: string) => (
                  <button
                    key={platform}
                    onClick={() => handleDownload(platform)}
                    className="btn btn-secondary w-full mb-2"
                  >
                    ⬇️ {platform.toUpperCase()}
                  </button>
                ))}
              </div>
            )}

            {/* Stats */}
            <div className="space-y-3 py-4 border-y border-border">
              <div className="flex justify-between">
                <span className="text-text-secondary">Rating</span>
                <span className="font-semibold">
                  ★ {game.rating.toFixed(1)} ({game.ratingCount} reviews)
                </span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">Plays</span>
                <span className="font-semibold">
                  {game.playCount.toLocaleString()}
                </span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">Downloads</span>
                <span className="font-semibold">
                  {game.downloadCount.toLocaleString()}
                </span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">Version</span>
                <span className="font-semibold">{game.version}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">Size</span>
                <span className="font-semibold">
                  {(game.size / 1024 / 1024).toFixed(1)} MB
                </span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">Category</span>
                <span className="font-semibold">{game.category}</span>
              </div>
            </div>

            {/* Rating */}
            <div className="mt-6">
              <p className="text-sm mb-2">Rate this game:</p>
              <div className="flex gap-2">
                {[1, 2, 3, 4, 5].map((star) => (
                  <button
                    key={star}
                    onClick={() => handleRate(star)}
                    className={`text-3xl transition ${
                      star <= userRating
                        ? 'text-yellow-400'
                        : 'text-gray-600 hover:text-yellow-200'
                    }`}
                  >
                    ★
                  </button>
                ))}
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default GameDetailPage;
