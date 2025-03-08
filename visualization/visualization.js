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

// Stream type constants
const STREAM_TYPES = {
    VIDEO: 'video',
    AUDIO: 'audio'
};

// Application state
let appState = {
    running: false,
    packetCount: 0,
    channelMap: new Map(),  // Stores all channels and their packets
    uniqueChannels: new Set(), // Stores unique channel names
    selectedStreamType: 'all',  // Default to show all stream types
    selectedChannel: 'all',     // Default to show all channels
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
const streamTypeSelector = document.getElementById('stream-type-selector');
const channelSelector = document.getElementById('channel-selector');

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
    
    // Set up filter event listeners
    streamTypeSelector.addEventListener('change', filterChannels);
    channelSelector.addEventListener('change', filterChannels);
    
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
            { name: 'XYZ1', type: 'video', packets: [] },
            { name: 'ABC2', type: 'video', packets: [] },
            { name: 'DEF9', type: 'audio', packets: [] },
            { name: 'GHI3', type: 'audio', packets: [] }
        ]
    };
    
    // Generate mock buffer data - ensure even distribution of channels and stream types
    // Create 50 packets for each channel to ensure good coverage
    for (let i = 0; i < 200; i++) {
        // Deterministic assignment to ensure all channels are represented
        const channelIndex = i % 4; // 4 channels with fixed stream types
        const channelName = appState.sampleData.channels[channelIndex].name;
        const streamType = appState.sampleData.channels[channelIndex].type;
        const sequenceNumber = i + 1;
        
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
        
        // Add to unique channels set for filtering
        appState.uniqueChannels.add(channelName);
    }
    
    // Sort channel packets by sequence number
    appState.sampleData.channels.forEach(channel => {
        channel.packets.sort((a, b) => a.sequenceNumber - b.sequenceNumber);
    });
    
    // Update channel selector with all channel names
    updateChannelSelector();
}

// Start processing visualization
function startProcessing() {
    if (appState.running) return;
    
    appState.running = true;
    appState.processingStartTime = Date.now();
    
    // Ensure we have all 4 channels in the uniqueChannels set
    appState.uniqueChannels.add('XYZ1');
    appState.uniqueChannels.add('DEF9');
    appState.uniqueChannels.add('ABC2');
    appState.uniqueChannels.add('GHI3');
    
    // Update channel selector
    updateChannelSelector();
    
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
    appState.uniqueChannels = new Set();
    appState.processingStartTime = null;
    appState.selectedPacket = null;
    
    // Reset filters
    streamTypeSelector.value = 'all';
    channelSelector.innerHTML = '<option value="all">All Channels</option>';
    appState.selectedStreamType = 'all';
    appState.selectedChannel = 'all';
    
    // Reset counters
    updateStats();
    
    // Clear packet details
    packetDetailsEl.innerHTML = '<p>Click on a packet to see details</p>';
    
    // Recreate mock data
    createMockData();
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
    // Ensure channelName is not 'Unknown'
    if (packet.channelName === 'Unknown' && packet.rawData && packet.rawData.length > 0) {
        // Try to determine channel name from first byte of raw data (for demo purposes)
        const firstByte = packet.rawData[0];
        if (firstByte % 4 === 0) packet.channelName = 'XYZ1';
        else if (firstByte % 4 === 1) packet.channelName = 'DEF9';
        else if (firstByte % 4 === 2) packet.channelName = 'ABC2';
        else packet.channelName = 'GHI3';
    }
    
    // Assign stream types based on channel name
    // XYZ1 and ABC2 are video channels, DEF9 and GHI3 are audio channels
    if (packet.channelName === 'XYZ1' || packet.channelName === 'ABC2') {
        packet.streamType = STREAM_TYPES.VIDEO;
    } else if (packet.channelName === 'DEF9' || packet.channelName === 'GHI3') {
        packet.streamType = STREAM_TYPES.AUDIO;
    } else if (packet.streamType === 'Stream_Video') {
        packet.streamType = STREAM_TYPES.VIDEO;
    } else if (packet.streamType === 'Stream_Audio') {
        packet.streamType = STREAM_TYPES.AUDIO;
    }
    
    // Log the packet being processed for debugging
    console.log('Processing packet:', packet.channelName, packet.streamType);
    
    const channelKey = `${packet.channelName}_${packet.streamType}`;
    
    // Track unique channel names for the filter dropdown
    if (!appState.uniqueChannels.has(packet.channelName)) {
        appState.uniqueChannels.add(packet.channelName);
        updateChannelSelector();
    }
    
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
    
    // Update channel visualization based on current filters
    if (shouldShowChannel(packet.channelName, packet.streamType)) {
        updateChannelVisualization(packet.channelName, packet.streamType, channelPackets);
    }
}

// Create a new channel visualization
function createChannelVisualization(channelName, streamType) {
    const typeClass = streamType;
    const typeName = streamType === STREAM_TYPES.VIDEO ? 'Video' : 'Audio';
    
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
    const typeClass = streamType;
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
    
    const streamType = packet.streamType === STREAM_TYPES.VIDEO ? 'Video' : 'Audio';
    
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
    // Show the correct number of channels (4 channels, not channel-stream combinations)
    channelsCountEl.textContent = appState.uniqueChannels.size;
    
    // Calculate processing rate
    if (appState.processingStartTime && appState.packetCount > 0) {
        const elapsedSeconds = (Date.now() - appState.processingStartTime) / 1000;
        const rate = (appState.packetCount / elapsedSeconds).toFixed(2);
        processingRateEl.textContent = rate;
    } else {
        processingRateEl.textContent = '0';
    }
}

// Update channel selector dropdown with unique channel names
function updateChannelSelector() {
    // Save current selection
    const currentSelection = channelSelector.value;
    
    // Clear existing options except 'All Channels'
    channelSelector.innerHTML = '<option value="all">All Channels</option>';
    
    // Add each unique channel
    appState.uniqueChannels.forEach(channelName => {
        const option = document.createElement('option');
        option.value = channelName;
        option.textContent = channelName;
        channelSelector.appendChild(option);
    });
    
    // Restore selection if it still exists
    if (Array.from(channelSelector.options).some(opt => opt.value === currentSelection)) {
        channelSelector.value = currentSelection;
    }
}

// Filter channels based on selected stream type and channel
function filterChannels() {
    // Update app state with current selections
    appState.selectedStreamType = streamTypeSelector.value;
    appState.selectedChannel = channelSelector.value;
    
    console.log('Filtering channels:', appState.selectedStreamType, appState.selectedChannel);
    
    // Hide all channels first
    const allChannelElements = channelStreamsEl.querySelectorAll('.channel');
    allChannelElements.forEach(el => {
        el.style.display = 'none';
    });
    
    // Show only channels that match the filter criteria
    appState.channelMap.forEach((packets, key) => {
        const [channelName, streamType] = key.split('_');
        
        console.log('Checking channel:', channelName, streamType, 'against filters:', appState.selectedChannel, appState.selectedStreamType);
        
        if (shouldShowChannel(channelName, streamType)) {
            const channelEl = document.getElementById(`channel-${channelName}-${streamType}`);
            if (channelEl) {
                console.log('Showing channel:', channelName, streamType);
                channelEl.style.display = 'block';
            } else {
                console.log('Channel element not found:', `channel-${channelName}-${streamType}`);
            }
        }
    });
    
    // If no channels are visible after filtering, show a message
    const visibleChannels = channelStreamsEl.querySelectorAll('.channel[style="display: block;"]');
    if (visibleChannels.length === 0) {
        // Create a message element if none exists
        let noChannelsMsg = document.getElementById('no-channels-message');
        if (!noChannelsMsg) {
            noChannelsMsg = document.createElement('div');
            noChannelsMsg.id = 'no-channels-message';
            noChannelsMsg.className = 'no-data-message';
            channelStreamsEl.appendChild(noChannelsMsg);
        }
        noChannelsMsg.textContent = `No ${appState.selectedStreamType === 'all' ? '' : appState.selectedStreamType} channels found for ${appState.selectedChannel === 'all' ? 'any channel' : appState.selectedChannel}`;
        noChannelsMsg.style.display = 'block';
        
        // Debug info
        console.log('No visible channels found for filter:', appState.selectedStreamType, appState.selectedChannel);
        console.log('Available channels:', Array.from(appState.channelMap.keys()));
    } else {
        // Hide the message if channels are visible
        const noChannelsMsg = document.getElementById('no-channels-message');
        if (noChannelsMsg) {
            noChannelsMsg.style.display = 'none';
        }
    }
}

// Determine if a channel should be shown based on current filters
function shouldShowChannel(channelName, streamType) {
    // Check channel filter
    const channelMatches = appState.selectedChannel === 'all' || appState.selectedChannel === channelName;
    
    // Check stream type filter
    let streamTypeMatches = false;
    
    if (appState.selectedStreamType === 'all') {
        streamTypeMatches = true;
    } else {
        // Direct string comparison since we've standardized stream types
        streamTypeMatches = appState.selectedStreamType === streamType;
    }
    
    console.log(`Checking if should show channel: ${channelName}, ${streamType}, channelMatches: ${channelMatches}, streamTypeMatches: ${streamTypeMatches}`);
    
    return channelMatches && streamTypeMatches;
}

// Clear all channel streams visualization
function clearChannelStreamsVisualization() {
    channelStreamsEl.innerHTML = '';
    appState.channelMap.clear();
}

// Initialize the visualization when the page loads
document.addEventListener('DOMContentLoaded', initVisualization);
