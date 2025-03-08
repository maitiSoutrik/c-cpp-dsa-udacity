// Visualization data structures and constants
const MAX_BUFFER_SIZE = 16; // Visualization limit for circular buffer
const MAX_PACKET_HISTORY = 20; // Max packets to show per channel
const CHANNELS_COLORS = {
    'XYZ1': '#e74c3c',
    'ABC2': '#3498db',
    'DEF9': '#2ecc71',
    'GHI3': '#f39c12',
    'JKL7': '#9b59b6'
};

// Application state
let appState = {
    running: false,
    packetCount: 0,
    channelMap: new Map(),
    processingStartTime: null,
    currentDataFile: 'sample_data.json', // Default to our sample data
    selectedPacket: null
};

// DOM Elements
const circularBufferEl = document.getElementById('circular-buffer');
const channelStreamsEl = document.getElementById('channel-streams');
const packetsCountEl = document.getElementById('packets-count');
const channelsCountEl = document.getElementById('channels-count');
const processingRateEl = document.getElementById('processing-rate');
const packetDetailsEl = document.getElementById('packet-details-content');
const startBtn = document.getElementById('startBtn');
const pauseBtn = document.getElementById('pauseBtn');
const resetBtn = document.getElementById('resetBtn');

// Initialize the visualization
function initVisualization() {
    // Initialize empty circular buffer visualization
    for (let i = 0; i < MAX_BUFFER_SIZE; i++) {
        const bufferItem = document.createElement('div');
        bufferItem.className = 'buffer-item';
        bufferItem.textContent = '-';
        bufferItem.dataset.index = i;
        circularBufferEl.appendChild(bufferItem);
    }
    
    // Set up event listeners
    startBtn.addEventListener('click', startProcessing);
    pauseBtn.addEventListener('click', pauseProcessing);
    resetBtn.addEventListener('click', resetVisualization);
    
    // Fetch and load the sample data
    fetch(appState.currentDataFile)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            console.log('Successfully loaded data:', data);
            // If we have real data from your parser, we'll use it here
            appState.sampleData = data;
        })
        .catch(error => {
            console.error('Error loading sample data:', error);
            // Create some mock data if the file isn't found
            createMockData();
        });
}

// Create mock data if no real data is available
function createMockData() {
    appState.sampleData = {
        bufferData: [],
        channels: [
            { name: 'XYZ1', type: 'Stream_Video', packets: [] },
            { name: 'DEF9', type: 'Stream_Audio', packets: [] },
            { name: 'ABC2', type: 'Stream_Video', packets: [] }
        ]
    };
    
    // Generate mock buffer data
    for (let i = 0; i < 50; i++) {
        const channelIndex = Math.floor(Math.random() * appState.sampleData.channels.length);
        const channelName = appState.sampleData.channels[channelIndex].name;
        const streamType = appState.sampleData.channels[channelIndex].type;
        const sequenceNumber = Math.floor(Math.random() * 255);
        
        const mockPacket = {
            channelName: channelName,
            streamType: streamType,
            sequenceNumber: sequenceNumber,
            rawData: Array.from({length: 8}, () => Math.floor(Math.random() * 256)),
            payload: Array.from({length: 4}, () => Math.floor(Math.random() * 256))
        };
        
        appState.sampleData.bufferData.push(mockPacket);
        
        // Also add to channel packets
        let channelPackets = appState.sampleData.channels[channelIndex].packets;
        channelPackets.push(mockPacket);
    }
    
    // Sort channel packets by sequence number
    appState.sampleData.channels.forEach(channel => {
        channel.packets.sort((a, b) => a.sequenceNumber - b.sequenceNumber);
    });
}

// Start processing visualization
function startProcessing() {
    if (appState.running) return;
    
    appState.running = true;
    appState.processingStartTime = Date.now();
    
    // If we're restarting after a pause
    if (appState.packetCount > 0) {
        continueProcessing();
        return;
    }
    
    // Start from beginning
    processNextPacket();
}

// Pause the processing visualization
function pauseProcessing() {
    appState.running = false;
}

// Continue processing from current state
function continueProcessing() {
    if (!appState.running) return;
    processNextPacket();
}

// Reset the entire visualization
function resetVisualization() {
    // Stop processing if running
    appState.running = false;
    
    // Clear visualization elements
    clearBufferVisualization();
    clearChannelStreamsVisualization();
    
    // Reset state
    appState.packetCount = 0;
    appState.channelMap = new Map();
    appState.processingStartTime = null;
    appState.selectedPacket = null;
    
    // Reset counters
    updateStats();
    
    // Clear packet details
    packetDetailsEl.innerHTML = '<p>Click on a packet to see details</p>';
}

// Process the next packet in the sequence
function processNextPacket() {
    if (!appState.running || !appState.sampleData) return;
    
    // Get next packet from sample data
    const packetIndex = appState.packetCount % appState.sampleData.bufferData.length;
    const currentPacket = appState.sampleData.bufferData[packetIndex];
    
    // Update buffer visualization
    updateBufferVisualization(currentPacket);
    
    // Process packet into channel streams
    processPacketIntoChannels(currentPacket);
    
    // Update stats
    appState.packetCount++;
    updateStats();
    
    // Continue processing with a delay for visualization
    setTimeout(() => {
        if (appState.running) {
            processNextPacket();
        }
    }, 300); // Adjust speed as needed
}

// Update circular buffer visualization
function updateBufferVisualization(packet) {
    // Clear active state from all buffer items
    const bufferItems = circularBufferEl.querySelectorAll('.buffer-item');
    bufferItems.forEach(item => item.classList.remove('active'));
    
    // Update the buffer item at the current position
    const bufferPosition = appState.packetCount % MAX_BUFFER_SIZE;
    const bufferItem = bufferItems[bufferPosition];
    if (bufferItem) {
        bufferItem.textContent = packet.sequenceNumber;
        bufferItem.classList.add('active');
        bufferItem.dataset.packetData = JSON.stringify(packet);
        
        // Add click handler
        bufferItem.onclick = () => showPacketDetails(packet);
    }
}

// Clear buffer visualization
function clearBufferVisualization() {
    const bufferItems = circularBufferEl.querySelectorAll('.buffer-item');
    bufferItems.forEach(item => {
        item.textContent = '-';
        item.classList.remove('active');
        item.onclick = null;
        delete item.dataset.packetData;
    });
}

// Process a packet into the appropriate channel stream
function processPacketIntoChannels(packet) {
    const channelKey = `${packet.channelName}_${packet.streamType === 'Stream_Video' ? 'video' : 'audio'}`;
    
    // Check if we already have this channel
    if (!appState.channelMap.has(channelKey)) {
        // Create new channel visualization
        createChannelVisualization(packet.channelName, packet.streamType);
        appState.channelMap.set(channelKey, []);
    }
    
    // Add packet to channel
    const channelPackets = appState.channelMap.get(channelKey);
    channelPackets.push(packet);
    
    // Keep only the latest MAX_PACKET_HISTORY packets
    if (channelPackets.length > MAX_PACKET_HISTORY) {
        channelPackets.shift();
    }
    
    // Update channel visualization
    updateChannelVisualization(packet.channelName, packet.streamType, channelPackets);
}

// Create a new channel visualization
function createChannelVisualization(channelName, streamType) {
    const typeClass = streamType === 'Stream_Video' ? 'video' : 'audio';
    const typeName = streamType === 'Stream_Video' ? 'Video' : 'Audio';
    
    const channelEl = document.createElement('div');
    channelEl.className = 'channel';
    channelEl.id = `channel-${channelName}-${typeClass}`;
    
    // Use a color from our palette or default
    const channelColor = CHANNELS_COLORS[channelName] || '#7f8c8d';
    channelEl.style.borderLeftColor = channelColor;
    channelEl.style.borderLeftWidth = '4px';
    channelEl.style.borderLeftStyle = 'solid';
    
    channelEl.innerHTML = `
        <div class="channel-header">
            <div class="channel-name">${channelName}</div>
            <div class="channel-type ${typeClass}">${typeName}</div>
        </div>
        <div class="stream-packets"></div>
    `;
    
    channelStreamsEl.appendChild(channelEl);
}

// Update an existing channel visualization with new packets
function updateChannelVisualization(channelName, streamType, packets) {
    const typeClass = streamType === 'Stream_Video' ? 'video' : 'audio';
    const channelId = `channel-${channelName}-${typeClass}`;
    const channelEl = document.getElementById(channelId);
    
    if (!channelEl) return;
    
    const streamPacketsEl = channelEl.querySelector('.stream-packets');
    streamPacketsEl.innerHTML = ''; // Clear existing packets
    
    // Add packets (most recent first)
    packets.slice().reverse().forEach(packet => {
        const packetEl = document.createElement('div');
        packetEl.className = `packet ${typeClass}`;
        packetEl.textContent = packet.sequenceNumber;
        packetEl.dataset.packetData = JSON.stringify(packet);
        packetEl.onclick = () => showPacketDetails(packet);
        streamPacketsEl.appendChild(packetEl);
    });
}

// Clear all channel streams visualization
function clearChannelStreamsVisualization() {
    channelStreamsEl.innerHTML = '';
    appState.channelMap.clear();
}

// Show detailed information about a selected packet
function showPacketDetails(packet) {
    appState.selectedPacket = packet;
    
    const streamType = packet.streamType === 'Stream_Video' ? 'Video' : 'Audio';
    
    let detailsHtml = `
        <div class="packet-detail-item"><span>Channel:</span> ${packet.channelName}</div>
        <div class="packet-detail-item"><span>Type:</span> ${streamType}</div>
        <div class="packet-detail-item"><span>Sequence Number:</span> ${packet.sequenceNumber}</div>
        <div class="packet-detail-item">
            <span>Raw Data (bytes):</span>
            <div class="byte-display">
    `;
    
    // Display raw data bytes
    packet.rawData.forEach(byte => {
        detailsHtml += `<div class="byte">${byte.toString(16).padStart(2, '0')}</div>`;
    });
    
    detailsHtml += `
            </div>
        </div>
        <div class="packet-detail-item">
            <span>Payload (bytes):</span>
            <div class="byte-display">
    `;
    
    // Display payload bytes
    packet.payload.forEach(byte => {
        detailsHtml += `<div class="byte">${byte.toString(16).padStart(2, '0')}</div>`;
    });
    
    detailsHtml += `
            </div>
        </div>
    `;
    
    packetDetailsEl.innerHTML = detailsHtml;
}

// Update statistics display
function updateStats() {
    packetsCountEl.textContent = appState.packetCount;
    channelsCountEl.textContent = appState.channelMap.size;
    
    // Calculate processing rate
    if (appState.processingStartTime && appState.packetCount > 0) {
        const elapsedSeconds = (Date.now() - appState.processingStartTime) / 1000;
        const rate = (appState.packetCount / elapsedSeconds).toFixed(2);
        processingRateEl.textContent = rate;
    } else {
        processingRateEl.textContent = '0';
    }
}

// Initialize the visualization when the page loads
document.addEventListener('DOMContentLoaded', initVisualization);
