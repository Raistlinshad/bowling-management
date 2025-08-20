#!/bin/bash

# Bowling Management Update Script
echo "========================================="
echo "Updating Bowling Management System..."
echo "========================================="

# Kill any running instances
if pgrep BowlingManagement > /dev/null; then
    echo "Stopping running BowlingManagement instances..."
    pkill BowlingManagement
    sleep 2
fi

# Run the build script
/home/centrebowl/build_bowling.sh

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "🎳 Update complete!"
    echo ""
    read -p "Would you like to start the application now? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Starting Bowling Management..."
        cd /home/centrebowl/Desktop
        ./BowlingManagement &
        echo "Application started!"
    fi
else
    echo "❌ Update failed!"
    exit 1
fi