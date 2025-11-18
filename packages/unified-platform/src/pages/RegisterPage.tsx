/**
 * Register Page
 */

import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface RegisterPageProps {
  platform: UnifiedPlatformCore;
}

export const RegisterPage: React.FC<RegisterPageProps> = ({ platform }) => {
  const [email, setEmail] = useState('');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');

    if (password !== confirmPassword) {
      setError('Passwords do not match');
      return;
    }

    setLoading(true);

    try {
      await platform.register(username, email, password);
      navigate('/hub');
    } catch (err: any) {
      setError(err.message || 'Registration failed');
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
        <h1 style={{ margin: '0 0 10px 0' }}>Join Nova Engine</h1>
        <p style={{ margin: '0 0 30px 0', color: '#888' }}>
          Create your account
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
              Username
            </label>
            <input
              type="text"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
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
              autoComplete="new-password"
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
              Confirm Password
            </label>
            <input
              type="password"
              value={confirmPassword}
              onChange={(e) => setConfirmPassword(e.target.value)}
              autoComplete="new-password"
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
            {loading ? 'Creating account...' : 'Sign Up'}
          </button>
        </form>

        <p style={{ marginTop: '20px', textAlign: 'center' }}>
          Already have an account?{' '}
          <a href="/login" style={{ color: '#7b2ff7', textDecoration: 'none' }}>
            Login
          </a>
        </p>
      </div>
    </div>
  );
};
