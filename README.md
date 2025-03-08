# TV Audio and Video Parser

## Project Overview

This C/C++ project demonstrates advanced data structures and algorithms in action through a TV audio and video parser application. The parser processes binary data streams from different TV channels, separating audio and video content using efficient data structures.

## Key Features

### Data Structures Implementation

- **Circular Buffer**: Efficient storage of incoming raw data packets
- **HashMap**: Fast O(1) lookup of channels and stream types
- **Sorted Singly Linked List**: Maintains packet sequence order for proper playback
- **Stack & Queue**: Used for various processing operations

### Parser Capabilities

- Processes binary TV packet data
- Separates audio and video streams
- Organizes data by channel and stream type
- Maintains sequence ordering

## 🌟 Interactive Visualization

The project includes an interactive web-based visualization that provides a compelling visual representation of the parsing process. This visualization allows viewers to:

- **See the Circular Buffer in action**: Watch as raw data moves through the buffer
- **View channel streams**: Observe how audio and video packets are organized by channel
- **Examine packet details**: Click on individual packets to see their byte-level contents
- **Track statistics**: Monitor processing rates and packet counts

![TV Parser Visualization](visualization/screenshot.png)

### Online Demo

A standalone version of the visualization is deployed and available online:

**[View Live Demo](https://maitisoutrik.github.io/tv-parser-visualization/)**

This deployed version uses sample data and doesn't require the C++ backend to run.

### Local Visualization with Live Data

To run the visualization with live data from the C++ application:

1. Compile and run the main application:

   ```bash
   make
   ./tv_parser_app
   ```

2. Use one of the provided scripts to start a local server:

   ```bash
   # For Bash users
   ./run_visualization.sh
   
   # For Fish shell users
   ./run_visualization.fish
   ```

3. Or manually start a simple HTTP server in the project directory:

   ```bash
   python -m http.server
   ```

4. Open your browser and navigate to:

   ```text
   http://localhost:8000/
   ```

5. Use the interactive controls to start, pause, and reset the visualization

### Architecture

The visualization has two components:

1. **C++ Backend**: The `DataExporter` class in `visualization/data_exporter.h` exports processed data as JSON files
2. **JavaScript Frontend**: The visualization reads these JSON files and renders the interactive UI

## Technical Implementation

The project implements a robust pipeline for processing TV data:

1. **Data Ingestion**: Raw binary packets are read from a data file and stored in a circular buffer
2. **Data Parsing**: Each packet is parsed to extract channel name, stream type, sequence number, and payload
3. **Data Organization**: Processed packets are stored in a hashmap of sorted linked lists, organized by channel and stream type
4. **Data Visualization**: Processed data is exported to JSON format and visualized in a web interface

## Learning Outcomes

This project demonstrates advanced understanding of:

- Data structure selection and implementation
- Memory management in C/C++
- Binary data parsing
- Data visualization techniques
- Algorithm complexity analysis and optimization

## Credits

This project was completed as part of the Udacity C/C++ Nanodegree program, with additional visualization enhancements to showcase the functionality.
