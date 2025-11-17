/**
 * Notification Center - Dropdown notifications
 */

import React from 'react';

interface NotificationCenterProps {
  notifications: any[];
  onClear: () => void;
}

export const NotificationCenter: React.FC<NotificationCenterProps> = ({
  notifications,
  onClear,
}) => {
  return (
    <div className="notification-center">
      <div className="notification-header">
        <h3>Notifications</h3>
        <button onClick={onClear}>Clear All</button>
      </div>
      <div className="notification-list">
        {notifications.length === 0 ? (
          <div className="no-notifications">No notifications</div>
        ) : (
          notifications.map((notification, index) => (
            <div
              key={index}
              className={`notification notification-${notification.type}`}
            >
              <span className="notification-icon">
                {notification.type === 'success' && '✓'}
                {notification.type === 'error' && '✗'}
                {notification.type === 'info' && 'ℹ'}
                {notification.type === 'warning' && '⚠'}
              </span>
              <span className="notification-message">
                {notification.message}
              </span>
            </div>
          ))
        )}
      </div>

      <style>{`
        .notification-center {
          position: absolute;
          top: 60px;
          right: 20px;
          width: 300px;
          background: #2a2a2a;
          border: 1px solid #3a3a3a;
          border-radius: 8px;
          color: white;
          max-height: 400px;
          display: flex;
          flex-direction: column;
        }

        .notification-header {
          display: flex;
          justify-content: space-between;
          padding: 15px;
          border-bottom: 1px solid #3a3a3a;
        }

        .notification-header h3 {
          margin: 0;
        }

        .notification-header button {
          background: transparent;
          border: none;
          color: #7b2ff7;
          cursor: pointer;
        }

        .notification-list {
          overflow-y: auto;
        }

        .notification {
          display: flex;
          gap: 10px;
          padding: 15px;
          border-bottom: 1px solid #3a3a3a;
        }

        .notification-success {
          border-left: 3px solid #4caf50;
        }

        .notification-error {
          border-left: 3px solid #f44336;
        }

        .notification-info {
          border-left: 3px solid #2196f3;
        }

        .notification-warning {
          border-left: 3px solid #ff9800;
        }

        .no-notifications {
          padding: 40px;
          text-align: center;
          color: #666;
        }
      `}</style>
    </div>
  );
};
