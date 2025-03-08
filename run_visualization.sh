#!/bin/bash

# Script to run the TV Parser Visualization

# Kill any existing Python HTTP server
pkill -f 'python3 -m http.server' 2>/dev/null

# Wait a moment for the port to be released
sleep 1

# Start the HTTP server in the background
cd "$(dirname "$0")"
python3 -m http.server &

# Print a message with the URL
echo "TV Parser Visualization is running at http://localhost:8000"
echo "Press Ctrl+C to stop the server"

# Wait for user to press Ctrl+C
trap "pkill -f 'python3 -m http.server'; echo 'Server stopped'; exit 0" INT
while true; do
    sleep 1
done
