#include <iostream>
#include <thread>
#include <iomanip>
#include <vector>

#include "av_data_interface_impl.h"
#include "circular_buffer.h"
#include "file_reader.h"
#include "hashmap.h"
#include "queue.h"
#include "sorted_singly_linked_list.h"
#include "stack.h"
#include "visualization/data_exporter.h"

int main() {
  // Set the debug flag to true to print out extra information
  bool debug = false;
  
  // Store processed packets for visualization
  std::vector<std::pair<std::vector<uint8_t>*, AvDataInterface*>> processedPackets;

  // A CircularBuffer is chosen for raw data storage since:
  // 1. It efficiently handles continuous data arrival
  // 2. It automatically overwrites oldest data when full (FIFO)
  // 3. It works well in memory-constrained environments
  // 4. It handles the dynamic nature of incoming data packets
  auto dsInstance = CircularBuffer<std::vector<uint8_t>*>();

  // Instantiate the FileReader with the created DataStructure instance,
  // path to the data file, chunk size, and debug flag
  FileReader fr = FileReader(&dsInstance, "data/tv_packets.bin", 128, debug);
  fr.startReading();

  // HashMap is chosen for the high-level data structure because:
  // 1. It provides O(1) average case lookup by unique identifier
  // 2. Efficient for frequent retrievals based on channel + stream type
  // 3. Dynamically expands to hold all required channel/stream combinations
  // 4. Perfect for mapping string keys to data structure pointers
  HashMap<std::string, SortedSinglyLinkedList> highLevelDataStructureInstance;

  // Continue reading as long as the FileReader is running or the dsInstance
  // still has data to parse
  while (fr.isRunning() || false == dsInstance.isEmpty()) {
    // Wait for the reader to finish
    while (false == dsInstance.isEmpty()) {
      // Parse each data chunk into an AvDataInterface object
      auto rawData = dsInstance.remove();
      auto avDataInterface = new AvDataInterfaceImpl(rawData);
      
      // Store the raw data and its parsed version for visualization
      processedPackets.push_back(std::make_pair(rawData, avDataInterface));

      std::string channelName;
      StreamType streamType;
      avDataInterface->parseData(&channelName, &streamType);

      // Concatenate channelName and streamType to create the unique identifier
      std::string uniqueIdentifier =
          channelName + "_" + std::to_string(static_cast<int>(streamType));

      // Check if the uniqueIdentifier exists in the hashmap
      auto dataStreamHolderPtr =
          highLevelDataStructureInstance.get(uniqueIdentifier);
      if (nullptr == dataStreamHolderPtr) {
        // SortedSinglyLinkedList is chosen for each stream because:
        // 1. It automatically sorts data by sequence number during insertion
        // 2. Maintains ordered data for proper playback sequence
        // 3. Efficient for sequential access during playback
        // 4. Implements the ComparableDataWrapperInterface for sorting
        auto newDsh = new SortedSinglyLinkedList();

        // Add the new sorted list to the hashmap with the unique identifier
        highLevelDataStructureInstance.insert(uniqueIdentifier, newDsh);

        dataStreamHolderPtr = highLevelDataStructureInstance.get(uniqueIdentifier);
      }
      dataStreamHolderPtr->insert(avDataInterface);

      if (debug) {
        std::cout << channelName << " " << static_cast<int>(streamType) << " "
                  << static_cast<int>(avDataInterface->getSequenceNumber())
                  << std::endl;
      }
    }

    // Allow the file reader to read and push data
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Standout task: Extract and print specific bytes from streams
  std::cout << "\n===== Standout Task Results =====\n";
  
  // Get XYZ1 video stream (XYZ1_1)
  auto xyz1VideoStream = highLevelDataStructureInstance.get("XYZ1_1");
  if (xyz1VideoStream != nullptr) {
    std::cout << "Channel-XYZ_Video: ";
    
    // Search for sequence numbers 1, 50, and 100
    bool first = true;
    for (int seq : {1, 50, 100}) {
      auto node = xyz1VideoStream->search(seq);
      if (node != nullptr) {
        auto data = static_cast<AvDataInterface*>(node->data);
        auto payload = data->getPayload();
        if (payload != nullptr && !payload->empty()) {
          if (!first) {
            std::cout << ",";
          }
          std::cout << std::hex << std::setw(2) << std::setfill('0') 
                    << static_cast<int>((*payload)[0]);
          first = false;
        } else {
          if (!first) {
            std::cout << ",";
          }
          std::cout << "??";
          first = false;
        }
      } else {
        if (!first) {
          std::cout << ",";
        }
        std::cout << "??";
        first = false;
      }
    }
    std::cout << std::dec << std::endl;
  }
  
  // Get DEF9 audio stream (DEF9_0)
  auto def9AudioStream = highLevelDataStructureInstance.get("DEF9_0");
  if (def9AudioStream != nullptr) {
    std::cout << "Channel-DEF_Audio: ";
    
    // Search for sequence numbers 1, 50, and 100
    bool first = true;
    for (int seq : {1, 50, 100}) {
      auto node = def9AudioStream->search(seq);
      if (node != nullptr) {
        auto data = static_cast<AvDataInterface*>(node->data);
        auto payload = data->getPayload();
        if (payload != nullptr && !payload->empty()) {
          if (!first) {
            std::cout << ",";
          }
          std::cout << std::hex << std::setw(2) << std::setfill('0') 
                    << static_cast<int>((*payload)[0]);
          first = false;
        } else {
          if (!first) {
            std::cout << ",";
          }
          std::cout << "??";
          first = false;
        }
      } else {
        if (!first) {
          std::cout << ",";
        }
        std::cout << "??";
        first = false;
      }
    }
    std::cout << std::dec << std::endl;
  }

  // Export data for visualization
  std::cout << "\n===== Exporting Data for Visualization =====\n";
  DataExporter exporter("visualization");
  exporter.exportVisualizationData(processedPackets, highLevelDataStructureInstance);
  
  // Display instructions for visualization
  std::cout << "\n===== Interactive Visualization =====\n";
  std::cout << "To view the interactive visualization:\n";
  std::cout << "1. Navigate to the project directory in a terminal\n";
  std::cout << "2. Run a simple HTTP server:\n";
  std::cout << "   python3 -m http.server\n";
  std::cout << "   or if that doesn't work:\n";
  std::cout << "   python -m http.server\n";
  std::cout << "3. Open your browser to http://localhost:8000\n";
  std::cout << "   (Note: we created a redirect from the root page to the visualization)\n";
  std::cout << "4. Interact with the visualization to see your data in action!\n";

  return 0;
}
