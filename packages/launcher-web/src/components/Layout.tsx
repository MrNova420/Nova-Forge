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

import React from 'react';
import { Outlet, Link } from 'react-router-dom';
import { useAuthStore } from '../store/authStore';

const Layout: React.FC = () => {
  const { user, isAuthenticated, logout } = useAuthStore();

  return (
    <div className="min-h-screen flex flex-col">
      <header className="bg-bg-secondary border-b border-border">
        <nav className="container flex items-center justify-between py-4">
          <Link to="/" className="text-2xl font-bold text-accent">
            Nova Launcher
          </Link>
          
          <div className="flex items-center gap-6">
            <Link to="/games" className="hover:text-accent transition">
              Browse Games
            </Link>
            {isAuthenticated ? (
              <>
                <Link to="/library" className="hover:text-accent transition">
                  My Library
                </Link>
                <Link to="/profile" className="hover:text-accent transition">
                  {user?.username}
                </Link>
                <button onClick={logout} className="btn btn-secondary">
                  Logout
                </button>
              </>
            ) : (
              <>
                <Link to="/?auth=login" className="btn btn-secondary">
                  Login
                </Link>
                <Link to="/?auth=register" className="btn btn-primary">
                  Sign Up
                </Link>
              </>
            )}
          </div>
        </nav>
      </header>
      
      <main className="flex-1">
        <Outlet />
      </main>
      
      <footer className="bg-bg-secondary border-t border-border py-8">
        <div className="container text-center text-text-secondary">
          <p>&copy; 2025 Nova Engine. Built with ❤️ for game developers.</p>
        </div>
      </footer>
    </div>
  );
};

export default Layout;
