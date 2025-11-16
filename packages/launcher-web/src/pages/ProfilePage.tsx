import React from 'react';
import { useAuthStore } from '../store/authStore';

const ProfilePage: React.FC = () => {
  const { user } = useAuthStore();

  if (!user) return null;

  return (
    <div className="container py-8">
      <h1 className="text-4xl font-bold mb-8">My Profile</h1>

      <div className="grid grid-cols-3 gap-8">
        {/* Profile Info */}
        <div className="col-span-1">
          <div className="card p-6">
            <div className="w-32 h-32 bg-bg-tertiary rounded-full mb-4 mx-auto flex items-center justify-center text-6xl">
              👤
            </div>
            <h2 className="text-2xl font-bold text-center mb-2">
              {user.username}
            </h2>
            <p className="text-text-secondary text-center mb-6">{user.email}</p>
            
            <div className="space-y-3 text-sm">
              <div className="flex justify-between">
                <span className="text-text-secondary">Member since</span>
                <span>{new Date(user.createdAt).toLocaleDateString()}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">Games played</span>
                <span>42</span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">Total playtime</span>
                <span>126 hours</span>
              </div>
            </div>

            <button className="btn btn-secondary w-full mt-6">
              Edit Profile
            </button>
          </div>
        </div>

        {/* Activity & Stats */}
        <div className="col-span-2">
          <div className="card p-6 mb-6">
            <h3 className="text-xl font-bold mb-4">Recent Activity</h3>
            <div className="space-y-4">
              {[...Array(5)].map((_, i) => (
                <div key={i} className="flex items-center gap-4 p-4 bg-bg-tertiary rounded">
                  <div className="w-16 h-16 bg-bg-primary rounded"></div>
                  <div className="flex-1">
                    <p className="font-semibold">Played Example Game</p>
                    <p className="text-sm text-text-secondary">2 hours ago</p>
                  </div>
                </div>
              ))}
            </div>
          </div>

          <div className="card p-6">
            <h3 className="text-xl font-bold mb-4">Achievements</h3>
            <div className="grid grid-cols-4 gap-4">
              {[...Array(8)].map((_, i) => (
                <div key={i} className="text-center">
                  <div className="w-16 h-16 bg-bg-tertiary rounded-full mx-auto mb-2 flex items-center justify-center text-2xl">
                    🏆
                  </div>
                  <p className="text-xs">Achievement {i + 1}</p>
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default ProfilePage;
