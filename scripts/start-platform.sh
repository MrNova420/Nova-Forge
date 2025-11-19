#!/bin/bash

###############################################################################
# NOVA ENGINE - Platform Auto-Restart Script
# 
# This script ensures the platform stays running even if terminal is closed.
# It starts both backend servers and frontend, with auto-restart on failure.
###############################################################################

# Note: Not using 'set -e' to allow graceful error handling throughout script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"
PID_DIR="$PROJECT_ROOT/.pids"

# Create directories
mkdir -p "$LOG_DIR"
mkdir -p "$PID_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# Log functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_nova() {
    echo -e "${PURPLE}[NOVA]${NC} $1"
}

# Function to check if process is running
is_running() {
    local pid_file=$1
    if [ -f "$pid_file" ]; then
        local pid=$(cat "$pid_file")
        if kill -0 "$pid" 2>/dev/null; then
            return 0
        fi
    fi
    return 1
}

# Function to stop a service
stop_service() {
    local name=$1
    local pid_file="$PID_DIR/${name}.pid"
    
    if is_running "$pid_file"; then
        local pid=$(cat "$pid_file")
        log_info "Stopping $name (PID: $pid)..."
        kill "$pid" 2>/dev/null || true
        sleep 2
        
        # Force kill if still running
        if kill -0 "$pid" 2>/dev/null; then
            log_warning "Force killing $name..."
            kill -9 "$pid" 2>/dev/null || true
        fi
        
        rm -f "$pid_file"
        log_success "$name stopped"
    else
        log_info "$name is not running"
    fi
}

# Function to start a service with auto-restart
start_service() {
    local name=$1
    local command=$2
    local log_file="$LOG_DIR/${name}.log"
    local pid_file="$PID_DIR/${name}.pid"
    
    log_info "Starting $name..."
    
    # Run in background with nohup (persists after terminal close)
    nohup bash -c "
        while true; do
            echo '[$(date)] Starting $name...' >> '$log_file'
            cd '$PROJECT_ROOT'
            $command >> '$log_file' 2>&1
            EXIT_CODE=\$?
            echo '[$(date)] $name exited with code \$EXIT_CODE' >> '$log_file'
            
            if [ \$EXIT_CODE -eq 0 ]; then
                echo '[$(date)] $name stopped gracefully' >> '$log_file'
                break
            else
                echo '[$(date)] $name crashed, restarting in 5 seconds...' >> '$log_file'
                sleep 5
            fi
        done
    " > "$log_file" 2>&1 &
    
    local pid=$!
    echo "$pid" > "$pid_file"
    
    sleep 2
    
    if is_running "$pid_file"; then
        log_success "$name started (PID: $pid)"
        log_info "  Logs: $log_file"
        return 0
    else
        log_error "$name failed to start"
        return 1
    fi
}

# Function to stop all services
stop_all() {
    log_nova "Stopping all Nova Engine services..."
    stop_service "hub-server"
    stop_service "main-server"
    stop_service "frontend"
    log_success "All services stopped"
}

# Function to check service status
status_all() {
    log_nova "Nova Engine Platform Status:"
    echo ""
    
    local all_running=true
    
    # Check Hub Server
    if is_running "$PID_DIR/hub-server.pid"; then
        log_success "Hub Server: RUNNING (PID: $(cat $PID_DIR/hub-server.pid))"
    else
        log_warning "Hub Server: NOT RUNNING"
        all_running=false
    fi
    
    # Check Main Server
    if is_running "$PID_DIR/main-server.pid"; then
        log_success "Main Server: RUNNING (PID: $(cat $PID_DIR/main-server.pid))"
    else
        log_warning "Main Server: NOT RUNNING"
        all_running=false
    fi
    
    # Check Frontend
    if is_running "$PID_DIR/frontend.pid"; then
        log_success "Frontend: RUNNING (PID: $(cat $PID_DIR/frontend.pid))"
    else
        log_warning "Frontend: NOT RUNNING"
        all_running=false
    fi
    
    echo ""
    if $all_running; then
        log_success "All services are running!"
        log_info "Frontend: http://localhost:3000"
        log_info "Backend: http://localhost:3001"
        log_info "Hub API: http://localhost:3001"
    else
        log_warning "Some services are not running"
    fi
}

# Function to start all services
start_all() {
    log_nova "╔═══════════════════════════════════════════════════════════╗"
    log_nova "║                                                           ║"
    log_nova "║        🎮 NOVA ENGINE - UNIFIED PLATFORM 🎮              ║"
    log_nova "║                                                           ║"
    log_nova "║         Starting All Services with Auto-Restart          ║"
    log_nova "║                                                           ║"
    log_nova "╚═══════════════════════════════════════════════════════════╝"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    # Ensure dependencies are installed
    if [ ! -d "node_modules" ]; then
        log_info "Installing dependencies..."
        npm install
    fi
    
    # Start Hub Server (Backend API)
    start_service "hub-server" "npm run dev -w @nova-engine/hub"
    
    # Start Main Server (WebSocket + Additional APIs)
    # start_service "main-server" "npm run dev -w @nova-engine/server"
    
    # Start Frontend (Unified Platform)
    start_service "frontend" "npm run dev -w @nova-engine/unified-platform"
    
    echo ""
    log_success "═══════════════════════════════════════════════════════════"
    log_success "🚀 Nova Engine Platform Started Successfully!"
    log_success "═══════════════════════════════════════════════════════════"
    echo ""
    log_info "📱 Frontend:  http://localhost:3000"
    log_info "🔧 Backend:   http://localhost:3001"
    log_info "📊 Hub API:   http://localhost:3001/api"
    echo ""
    log_info "📝 Logs are available in: $LOG_DIR"
    log_info "🔄 Services will auto-restart if they crash"
    log_info "💾 Services will continue running after terminal closes"
    echo ""
    log_info "Commands:"
    log_info "  • Check status: $0 status"
    log_info "  • Stop all:     $0 stop"
    log_info "  • View logs:    tail -f $LOG_DIR/*.log"
    echo ""
}

# Function to restart all services
restart_all() {
    log_nova "Restarting all services..."
    stop_all
    sleep 2
    start_all
}

# Function to view logs
view_logs() {
    log_info "Viewing logs (Ctrl+C to exit)..."
    tail -f "$LOG_DIR"/*.log 2>/dev/null || log_warning "No logs available yet"
}

# Main command handler
case "${1:-start}" in
    start)
        start_all
        ;;
    stop)
        stop_all
        ;;
    restart)
        restart_all
        ;;
    status)
        status_all
        ;;
    logs)
        view_logs
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|status|logs}"
        echo ""
        echo "Commands:"
        echo "  start   - Start all services with auto-restart"
        echo "  stop    - Stop all services"
        echo "  restart - Restart all services"
        echo "  status  - Check status of all services"
        echo "  logs    - View live logs"
        exit 1
        ;;
esac
