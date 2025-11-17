/**
 * Login Page
 */

import React, { useState } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface LoginPageProps {
  platform: UnifiedPlatformCore;
}

export const LoginPage: React.FC<LoginPageProps> = ({ platform }) => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setLoading(true);
    setError('');

    try {
      await platform.login(email, password);
    } catch (err: any) {
      setError(err.message || 'Login failed');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div
      style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        minHeight: '100vh',
        background: 'linear-gradient(135deg, #1a1a2e 0%, #16213e 100%)',
      }}
    >
      <div
        style={{
          background: '#2a2a2a',
          padding: '40px',
          borderRadius: '12px',
          width: '100%',
          maxWidth: '400px',
          color: 'white',
        }}
      >
        <h1 style={{ margin: '0 0 10px 0' }}>Welcome to Nova Engine</h1>
        <p style={{ margin: '0 0 30px 0', color: '#888' }}>
          Login to access your account
        </p>

        <form onSubmit={handleSubmit}>
          <div style={{ marginBottom: '20px' }}>
            <label
              style={{ display: 'block', marginBottom: '8px', color: '#aaa' }}
            >
              Email
            </label>
            <input
              type="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              required
              style={{
                width: '100%',
                padding: '12px',
                background: '#1a1a1a',
                border: '1px solid #3a3a3a',
                borderRadius: '4px',
                color: 'white',
              }}
            />
          </div>

          <div style={{ marginBottom: '20px' }}>
            <label
              style={{ display: 'block', marginBottom: '8px', color: '#aaa' }}
            >
              Password
            </label>
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              required
              style={{
                width: '100%',
                padding: '12px',
                background: '#1a1a1a',
                border: '1px solid #3a3a3a',
                borderRadius: '4px',
                color: 'white',
              }}
            />
          </div>

          {error && (
            <div
              style={{
                padding: '10px',
                background: '#f443361a',
                border: '1px solid #f44336',
                borderRadius: '4px',
                color: '#f44336',
                marginBottom: '20px',
              }}
            >
              {error}
            </div>
          )}

          <button
            type="submit"
            disabled={loading}
            style={{
              width: '100%',
              padding: '14px',
              background: '#7b2ff7',
              border: 'none',
              borderRadius: '4px',
              color: 'white',
              fontSize: '16px',
              cursor: loading ? 'not-allowed' : 'pointer',
            }}
          >
            {loading ? 'Logging in...' : 'Login'}
          </button>
        </form>

        <p style={{ marginTop: '20px', textAlign: 'center' }}>
          Don't have an account?{' '}
          <a
            href="/register"
            style={{ color: '#7b2ff7', textDecoration: 'none' }}
          >
            Sign up
          </a>
        </p>
      </div>
    </div>
  );
};
