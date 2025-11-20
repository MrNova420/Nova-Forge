-- ============================================
-- NOVA ENGINE - Complete Database Schema
-- PostgreSQL Initialization Script
-- ============================================

-- Enable UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ============================================
-- USERS & AUTHENTICATION
-- ============================================

-- Users table - Core user accounts
CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    username VARCHAR(255) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    display_name VARCHAR(255),
    avatar_url TEXT,
    bio TEXT,
    level INTEGER DEFAULT 1,
    experience INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    last_login TIMESTAMP,
    is_verified BOOLEAN DEFAULT FALSE,
    is_banned BOOLEAN DEFAULT FALSE,
    role VARCHAR(50) DEFAULT 'user' -- 'user', 'admin', 'moderator', 'developer'
);

-- Sessions table - JWT session management
CREATE TABLE IF NOT EXISTS sessions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    token TEXT NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    ip_address VARCHAR(45),
    user_agent TEXT
);

-- ============================================
-- GAMES & CONTENT
-- ============================================

-- Games table - Game catalog
CREATE TABLE IF NOT EXISTS games (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    title VARCHAR(255) NOT NULL,
    slug VARCHAR(255) UNIQUE NOT NULL,
    description TEXT,
    creator_id UUID REFERENCES users(id) ON DELETE SET NULL,
    thumbnail_url TEXT,
    banner_url TEXT,
    trailer_url TEXT,
    category VARCHAR(100),
    tags TEXT[], -- Array of tags
    version VARCHAR(50) DEFAULT '1.0.0',
    file_size_mb INTEGER,
    download_url TEXT,
    play_count INTEGER DEFAULT 0,
    rating_average DECIMAL(3,2) DEFAULT 0,
    rating_count INTEGER DEFAULT 0,
    published BOOLEAN DEFAULT FALSE,
    is_featured BOOLEAN DEFAULT FALSE,
    release_date TIMESTAMP,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Game ratings - User ratings for games
CREATE TABLE IF NOT EXISTS game_ratings (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    rating INTEGER CHECK (rating >= 1 AND rating <= 5),
    review TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(game_id, user_id)
);

-- Game plays - Track game play sessions
CREATE TABLE IF NOT EXISTS game_plays (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    session_duration INTEGER, -- Duration in seconds
    score INTEGER,
    level_reached INTEGER,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Game assets - Store game files and assets
CREATE TABLE IF NOT EXISTS game_assets (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    asset_type VARCHAR(50), -- 'texture', 'model', 'audio', 'script', etc.
    file_name VARCHAR(255),
    file_path TEXT,
    file_size_bytes BIGINT,
    mime_type VARCHAR(100),
    created_at TIMESTAMP DEFAULT NOW()
);

-- ============================================
-- SOCIAL FEATURES
-- ============================================

-- Friendships table
CREATE TABLE IF NOT EXISTS friendships (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    friend_id UUID REFERENCES users(id) ON DELETE CASCADE,
    status VARCHAR(50) DEFAULT 'pending', -- 'pending', 'accepted', 'blocked'
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(user_id, friend_id)
);

-- Achievements definitions
CREATE TABLE IF NOT EXISTS achievements (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    title VARCHAR(255) NOT NULL,
    description TEXT,
    icon_url TEXT,
    points INTEGER DEFAULT 0,
    rarity VARCHAR(50) DEFAULT 'common', -- 'common', 'rare', 'epic', 'legendary'
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    requirement_type VARCHAR(50), -- 'play_count', 'score', 'level', etc.
    requirement_value INTEGER,
    created_at TIMESTAMP DEFAULT NOW()
);

-- User achievements - Track unlocked achievements
CREATE TABLE IF NOT EXISTS user_achievements (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    achievement_id UUID REFERENCES achievements(id) ON DELETE CASCADE,
    unlocked_at TIMESTAMP DEFAULT NOW(),
    progress INTEGER DEFAULT 0,
    UNIQUE(user_id, achievement_id)
);

-- Activity feed
CREATE TABLE IF NOT EXISTS activities (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    activity_type VARCHAR(50), -- 'game_play', 'achievement', 'friend_add', etc.
    content TEXT,
    metadata JSONB, -- Store additional data as JSON
    created_at TIMESTAMP DEFAULT NOW()
);

-- ============================================
-- MULTIPLAYER FEATURES
-- ============================================

-- Lobbies table
CREATE TABLE IF NOT EXISTS lobbies (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    host_id UUID REFERENCES users(id) ON DELETE CASCADE,
    max_players INTEGER DEFAULT 4,
    current_players INTEGER DEFAULT 0,
    game_mode VARCHAR(100),
    map VARCHAR(100),
    region VARCHAR(50),
    status VARCHAR(50) DEFAULT 'waiting', -- 'waiting', 'starting', 'in_progress', 'finished'
    is_private BOOLEAN DEFAULT FALSE,
    password_hash VARCHAR(255),
    created_at TIMESTAMP DEFAULT NOW(),
    started_at TIMESTAMP,
    ended_at TIMESTAMP
);

-- Lobby players
CREATE TABLE IF NOT EXISTS lobby_players (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    lobby_id UUID REFERENCES lobbies(id) ON DELETE CASCADE,
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    is_ready BOOLEAN DEFAULT FALSE,
    team INTEGER,
    joined_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(lobby_id, user_id)
);

-- Matchmaking queue
CREATE TABLE IF NOT EXISTS matchmaking_queue (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    game_mode VARCHAR(100),
    region VARCHAR(50),
    skill_rating INTEGER DEFAULT 1000,
    status VARCHAR(50) DEFAULT 'searching', -- 'searching', 'matched', 'cancelled'
    created_at TIMESTAMP DEFAULT NOW(),
    matched_at TIMESTAMP
);

-- Matches - Completed game matches
CREATE TABLE IF NOT EXISTS matches (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    lobby_id UUID REFERENCES lobbies(id) ON DELETE SET NULL,
    game_mode VARCHAR(100),
    map VARCHAR(100),
    duration_seconds INTEGER,
    winner_team INTEGER,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Match players - Player performance in matches
CREATE TABLE IF NOT EXISTS match_players (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    match_id UUID REFERENCES matches(id) ON DELETE CASCADE,
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    team INTEGER,
    score INTEGER DEFAULT 0,
    kills INTEGER DEFAULT 0,
    deaths INTEGER DEFAULT 0,
    assists INTEGER DEFAULT 0,
    result VARCHAR(50), -- 'win', 'loss', 'draw'
    UNIQUE(match_id, user_id)
);

-- ============================================
-- COMMUNICATION
-- ============================================

-- Messages - Direct messages between users
CREATE TABLE IF NOT EXISTS messages (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    sender_id UUID REFERENCES users(id) ON DELETE CASCADE,
    recipient_id UUID REFERENCES users(id) ON DELETE CASCADE,
    content TEXT NOT NULL,
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Notifications
CREATE TABLE IF NOT EXISTS notifications (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    type VARCHAR(50), -- 'friend_request', 'achievement', 'message', etc.
    title VARCHAR(255),
    content TEXT,
    link TEXT,
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW()
);

-- ============================================
-- STORAGE & FILES
-- ============================================

-- User uploads - Track all user uploaded files
CREATE TABLE IF NOT EXISTS user_uploads (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    file_name VARCHAR(255),
    file_path TEXT,
    file_size_bytes BIGINT,
    mime_type VARCHAR(100),
    upload_type VARCHAR(50), -- 'avatar', 'game_asset', 'screenshot', etc.
    is_public BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Save games - User game progress saves
CREATE TABLE IF NOT EXISTS save_games (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    save_name VARCHAR(255),
    save_data JSONB, -- Store save data as JSON
    screenshot_url TEXT,
    level INTEGER,
    playtime_seconds INTEGER,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- ============================================
-- ANALYTICS & METRICS
-- ============================================

-- User stats - Track user statistics
CREATE TABLE IF NOT EXISTS user_stats (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    total_playtime_seconds BIGINT DEFAULT 0,
    games_played INTEGER DEFAULT 0,
    achievements_unlocked INTEGER DEFAULT 0,
    friends_count INTEGER DEFAULT 0,
    matches_won INTEGER DEFAULT 0,
    matches_lost INTEGER DEFAULT 0,
    skill_rating INTEGER DEFAULT 1000,
    rank VARCHAR(50) DEFAULT 'Unranked',
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Game analytics - Track game performance
CREATE TABLE IF NOT EXISTS game_analytics (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_id UUID REFERENCES games(id) ON DELETE CASCADE,
    date DATE NOT NULL,
    play_count INTEGER DEFAULT 0,
    unique_players INTEGER DEFAULT 0,
    avg_session_duration INTEGER DEFAULT 0,
    total_revenue DECIMAL(10,2) DEFAULT 0,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(game_id, date)
);

-- ============================================
-- INDEXES for Performance
-- ============================================

-- Users indexes
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_created_at ON users(created_at);

-- Games indexes
CREATE INDEX IF NOT EXISTS idx_games_creator_id ON games(creator_id);
CREATE INDEX IF NOT EXISTS idx_games_category ON games(category);
CREATE INDEX IF NOT EXISTS idx_games_published ON games(published);
CREATE INDEX IF NOT EXISTS idx_games_featured ON games(is_featured);
CREATE INDEX IF NOT EXISTS idx_games_rating ON games(rating_average);

-- Friendships indexes
CREATE INDEX IF NOT EXISTS idx_friendships_user_id ON friendships(user_id);
CREATE INDEX IF NOT EXISTS idx_friendships_friend_id ON friendships(friend_id);
CREATE INDEX IF NOT EXISTS idx_friendships_status ON friendships(status);

-- Activities indexes
CREATE INDEX IF NOT EXISTS idx_activities_user_id ON activities(user_id);
CREATE INDEX IF NOT EXISTS idx_activities_created_at ON activities(created_at);

-- Lobbies indexes
CREATE INDEX IF NOT EXISTS idx_lobbies_game_id ON lobbies(game_id);
CREATE INDEX IF NOT EXISTS idx_lobbies_status ON lobbies(status);
CREATE INDEX IF NOT EXISTS idx_lobbies_created_at ON lobbies(created_at);

-- Matches indexes
CREATE INDEX IF NOT EXISTS idx_matches_game_id ON matches(game_id);
CREATE INDEX IF NOT EXISTS idx_matches_created_at ON matches(created_at);

-- Messages indexes
CREATE INDEX IF NOT EXISTS idx_messages_sender_id ON messages(sender_id);
CREATE INDEX IF NOT EXISTS idx_messages_recipient_id ON messages(recipient_id);
CREATE INDEX IF NOT EXISTS idx_messages_created_at ON messages(created_at);

-- Notifications indexes
CREATE INDEX IF NOT EXISTS idx_notifications_user_id ON notifications(user_id);
CREATE INDEX IF NOT EXISTS idx_notifications_is_read ON notifications(is_read);

-- ============================================
-- SEED DATA - Initial content
-- ============================================

-- Insert demo user
INSERT INTO users (username, email, password_hash, display_name, level, experience, is_verified)
VALUES 
    ('demo_user', 'demo@novaengine.com', '$2a$10$YourHashedPasswordHere', 'Demo User', 10, 5000, TRUE),
    ('nova_admin', 'admin@novaengine.com', '$2a$10$YourHashedPasswordHere', 'Nova Admin', 50, 100000, TRUE)
ON CONFLICT (username) DO NOTHING;

-- Insert demo games
INSERT INTO games (title, slug, description, thumbnail_url, category, tags, published, is_featured, rating_average)
VALUES 
    ('Space Warriors', 'space-warriors', 'Epic space combat game with stunning graphics', '/assets/games/space-warriors.jpg', 'Action', ARRAY['space', 'shooter', 'multiplayer'], TRUE, TRUE, 4.8),
    ('Nova Racer', 'nova-racer', 'High-speed racing through neon-lit cities', '/assets/games/nova-racer.jpg', 'Racing', ARRAY['racing', 'competitive'], TRUE, TRUE, 4.6),
    ('Puzzle Master', 'puzzle-master', 'Mind-bending puzzles that challenge your logic', '/assets/games/puzzle-master.jpg', 'Puzzle', ARRAY['puzzle', 'casual'], TRUE, FALSE, 4.5),
    ('Fantasy Quest', 'fantasy-quest', 'Embark on an epic fantasy adventure', '/assets/games/fantasy-quest.jpg', 'RPG', ARRAY['rpg', 'adventure', 'fantasy'], TRUE, TRUE, 4.9),
    ('Arena Legends', 'arena-legends', 'Battle royale with unique abilities', '/assets/games/arena-legends.jpg', 'Battle Royale', ARRAY['battle-royale', 'pvp'], TRUE, TRUE, 4.7)
ON CONFLICT (slug) DO NOTHING;

-- Insert achievements
INSERT INTO achievements (title, description, icon_url, points, rarity, requirement_type, requirement_value)
VALUES 
    ('First Steps', 'Create your first game', '/assets/achievements/first-steps.png', 10, 'common', 'game_created', 1),
    ('Play Master', 'Play 100 games', '/assets/achievements/play-master.png', 50, 'rare', 'play_count', 100),
    ('Social Butterfly', 'Add 50 friends', '/assets/achievements/social-butterfly.png', 30, 'rare', 'friends_count', 50),
    ('Champion', 'Win 100 matches', '/assets/achievements/champion.png', 100, 'epic', 'matches_won', 100),
    ('Legend', 'Reach max level', '/assets/achievements/legend.png', 500, 'legendary', 'level', 100)
ON CONFLICT DO NOTHING;

-- ============================================
-- FUNCTIONS & TRIGGERS
-- ============================================

-- Function to update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Triggers for updated_at
CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_games_updated_at BEFORE UPDATE ON games
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_sessions_updated_at BEFORE UPDATE ON sessions
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Function to update game ratings
CREATE OR REPLACE FUNCTION update_game_rating()
RETURNS TRIGGER AS $$
BEGIN
    UPDATE games
    SET rating_average = (
        SELECT AVG(rating)::DECIMAL(3,2)
        FROM game_ratings
        WHERE game_id = NEW.game_id
    ),
    rating_count = (
        SELECT COUNT(*)
        FROM game_ratings
        WHERE game_id = NEW.game_id
    )
    WHERE id = NEW.game_id;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Trigger for game ratings
CREATE TRIGGER update_game_rating_trigger AFTER INSERT OR UPDATE ON game_ratings
    FOR EACH ROW EXECUTE FUNCTION update_game_rating();

-- ============================================
-- VIEWS for Common Queries
-- ============================================

-- View: User profiles with stats
CREATE OR REPLACE VIEW user_profiles AS
SELECT 
    u.id,
    u.username,
    u.display_name,
    u.avatar_url,
    u.bio,
    u.level,
    u.experience,
    u.created_at,
    COALESCE(s.total_playtime_seconds, 0) as total_playtime,
    COALESCE(s.games_played, 0) as games_played,
    COALESCE(s.achievements_unlocked, 0) as achievements_unlocked,
    COALESCE(s.friends_count, 0) as friends_count,
    COALESCE(s.matches_won, 0) as matches_won,
    COALESCE(s.matches_lost, 0) as matches_lost,
    COALESCE(s.skill_rating, 1000) as skill_rating,
    COALESCE(s.rank, 'Unranked') as rank
FROM users u
LEFT JOIN user_stats s ON u.id = s.user_id;

-- View: Popular games
CREATE OR REPLACE VIEW popular_games AS
SELECT 
    g.*,
    u.username as creator_username,
    u.display_name as creator_display_name
FROM games g
LEFT JOIN users u ON g.creator_id = u.id
WHERE g.published = TRUE
ORDER BY g.rating_average DESC, g.play_count DESC;

-- ============================================
-- PERMISSIONS (Optional - for production)
-- ============================================

-- Grant permissions to application user
-- GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO nova_engine_app;
-- GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO nova_engine_app;

-- ============================================
-- COMPLETE
-- ============================================

-- Vacuum and analyze for optimal performance
VACUUM ANALYZE;

SELECT 'Nova Engine Database Initialized Successfully!' as status;
SELECT COUNT(*) as table_count FROM information_schema.tables WHERE table_schema = 'public';
