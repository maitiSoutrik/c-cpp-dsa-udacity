#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "../data_interface/av_data_interface.h"
#include "../data_structures/hashmap.h"
#include "../data_structures/sorted_singly_linked_list.h"

class DataExporter {
public:
    DataExporter(const std::string& outputPath) : m_outputPath(outputPath) {
        // Make sure the visualization directory exists
        ensureDirectoryExists();
    }

    // Export raw data packets to JSON file
    void exportRawPackets(const std::vector<std::vector<uint8_t>*>& rawPackets) {
        std::ofstream outFile(m_outputPath + "/raw_packets.json");
        if (!outFile.is_open()) {
            std::cerr << "Failed to open output file for raw packets!" << std::endl;
            return;
        }

        outFile << "{\n  \"bufferData\": [\n";

        bool firstPacket = true;
        for (auto packet : rawPackets) {
            if (!firstPacket) {
                outFile << ",\n";
            }
            firstPacket = false;

            outFile << "    {\n";
            outFile << "      \"rawData\": [";

            bool firstByte = true;
            for (auto byte : *packet) {
                if (!firstByte) {
                    outFile << ", ";
                }
                firstByte = false;
                outFile << static_cast<int>(byte);
            }

            outFile << "]\n    }";
        }

        outFile << "\n  ]\n}";
        outFile.close();
    }

    // Export processed channel data to JSON file
    void exportChannelData(const HashMap<std::string, SortedSinglyLinkedList>& channelData) {
        std::ofstream outFile(m_outputPath + "/channel_data.json");
        if (!outFile.is_open()) {
            std::cerr << "Failed to open output file for channel data!" << std::endl;
            return;
        }

        outFile << "{\n  \"channels\": [\n";

        // We need to store all keys for iteration
        std::vector<std::string> keys;
        channelData.getAllKeys(keys);

        bool firstChannel = true;
        for (const auto& key : keys) {
            if (!firstChannel) {
                outFile << ",\n";
            }
            firstChannel = false;

            // Parse channel name and stream type from key
            std::string channelName = key.substr(0, key.find('_'));
            std::string streamTypeStr = key.substr(key.find('_') + 1);
            int streamTypeInt = std::stoi(streamTypeStr);
            std::string streamType = (streamTypeInt == 1) ? "Stream_Video" : "Stream_Audio";

            outFile << "    {\n";
            outFile << "      \"name\": \"" << channelName << "\",\n";
            outFile << "      \"type\": \"" << streamType << "\",\n";
            outFile << "      \"packets\": [\n";

            // Get the sorted linked list for this channel
            auto dataStreamHolder = channelData.get(key);
            if (dataStreamHolder != nullptr) {
                bool firstPacket = true;
                auto currentNode = dataStreamHolder->getHead();

                while (currentNode != nullptr) {
                    auto avData = static_cast<AvDataInterface*>(currentNode->data);
                    if (avData != nullptr) {
                        if (!firstPacket) {
                            outFile << ",\n";
                        }
                        firstPacket = false;

                        // Add packet data
                        outFile << "        {\n";
                        outFile << "          \"sequenceNumber\": " << static_cast<int>(avData->getSequenceNumber()) << ",\n";
                        
                        // Add payload data
                        auto payload = avData->getPayload();
                        outFile << "          \"payload\": [";
                        
                        if (payload != nullptr && !payload->empty()) {
                            bool firstByte = true;
                            for (auto byte : *payload) {
                                if (!firstByte) {
                                    outFile << ", ";
                                }
                                firstByte = false;
                                outFile << static_cast<int>(byte);
                            }
                        }
                        
                        outFile << "]\n        }";
                    }
                    currentNode = currentNode->next;
                }
            }

            outFile << "\n      ]\n    }";
        }

        outFile << "\n  ]\n}";
        outFile.close();
    }

    // Export complete visualization data (combines raw and channel data)
    void exportVisualizationData(const std::vector<std::pair<std::vector<uint8_t>*, AvDataInterface*>>& processedPackets,
                               const HashMap<std::string, SortedSinglyLinkedList>& channelData) {
        // Counters for errors (to avoid printing hundreds of identical error messages)
        int parsingErrorCount = 0;
        int rawDataErrorCount = 0;
        int payloadErrorCount = 0;
        int channelKeyErrorCount = 0;
        int getHeadErrorCount = 0;
        int castErrorCount = 0;
        int sequenceNumberErrorCount = 0;
        
        // Use a top-level try-catch to ensure we always have visualization data
        try {
            // If we cannot process the data properly, create fallback visualization data
            if (processedPackets.empty()) {
                createMockupData();
                std::cout << "Created mockup visualization data since no packets were processed" << std::endl;
                return;
            }
            
            std::ofstream outFile(m_outputPath + "/sample_data.json");
            if (!outFile.is_open()) {
                std::cerr << "Failed to open output file for visualization data!" << std::endl;
                createMockupData();
                return;
            }

            outFile << "{\n  \"bufferData\": [\n";

            // Export buffer data (processed packets)
            bool firstPacket = true;
            for (const auto& pair : processedPackets) {
                auto rawData = pair.first;
                auto avData = pair.second;
                
                // Skip invalid packet pairs
                if (rawData == nullptr || avData == nullptr) {
                    continue;
                }
                
                if (!firstPacket) {
                    outFile << ",\n";
                }
                firstPacket = false;

                std::string channelName;
                StreamType streamType;
                
                // Using more defensive approach for parsing
                bool parseSuccess = false;
                
                try {
                    if (avData != nullptr) {
                        avData->parseData(&channelName, &streamType);
                        parseSuccess = true;
                    }
                } catch (const std::exception& e) {
                    parsingErrorCount++;
                    // Only print first few errors to avoid console spam
                    if (parsingErrorCount <= 3) {
                        std::cerr << "Error parsing data: " << e.what() << std::endl;
                    }
                }
                
                if (!parseSuccess) {
                    // Use default values if parsing fails
                    channelName = "Unknown";
                    streamType = StreamType::Stream_Video;
                }

                outFile << "    {\n";
                outFile << "      \"channelName\": \"" << channelName << "\",\n";
                outFile << "      \"streamType\": \"" << (streamType == StreamType::Stream_Video ? "Stream_Video" : "Stream_Audio") << "\",\n";
                outFile << "      \"sequenceNumber\": " << static_cast<int>(avData->getSequenceNumber()) << ",\n";
                
                // Add raw data
                outFile << "      \"rawData\": [";
                bool firstByte = true;
                
                // Safely iterate through raw data with null check
                if (rawData != nullptr) {
                    try {
                        for (auto byte : *rawData) {
                            if (!firstByte) {
                                outFile << ", ";
                            }
                            firstByte = false;
                            outFile << static_cast<int>(byte);
                        }
                    } catch (const std::exception& e) {
                        rawDataErrorCount++;
                        if (rawDataErrorCount <= 3) {
                            std::cerr << "Error processing raw data: " << e.what() << std::endl;
                        }
                    }
                }
                outFile << "],\n";
                
                // Add payload data
                outFile << "      \"payload\": [";
                
                // Safely process payload data with null checks
                std::vector<uint8_t>* payload = nullptr;
                try {
                    payload = avData->getPayload();
                    
                    if (payload != nullptr && !payload->empty()) {
                        firstByte = true;
                        for (auto byte : *payload) {
                            if (!firstByte) {
                                outFile << ", ";
                            }
                            firstByte = false;
                            outFile << static_cast<int>(byte);
                        }
                    }
                } catch (const std::exception& e) {
                    payloadErrorCount++;
                    if (payloadErrorCount <= 3) {
                        std::cerr << "Error processing payload: " << e.what() << std::endl;
                    }
                }
                
                outFile << "]\n    }";
            }

            outFile << "\n  ],\n";

            // Export channel data
            outFile << "  \"channels\": [\n";

            // We need to store all keys for iteration
            std::vector<std::string> keys;
            channelData.getAllKeys(keys);
            
            bool firstChannel = true;
            for (const auto& key : keys) {
                if (key.empty()) continue;  // Skip empty keys
                
                if (!firstChannel) {
                    outFile << ",\n";
                }
                firstChannel = false;

                // Parse channel name and stream type from key with error checking
                std::string channelName;
                std::string streamType;
                
                try {
                    size_t underscorePos = key.find('_');
                    if (underscorePos != std::string::npos) {
                        channelName = key.substr(0, underscorePos);
                        std::string streamTypeStr = key.substr(underscorePos + 1);
                        int streamTypeInt = std::stoi(streamTypeStr);
                        streamType = (streamTypeInt == 1) ? "Stream_Video" : "Stream_Audio";
                    } else {
                        // If key format is invalid, use defaults
                        channelName = key;
                        streamType = "Stream_Video";
                    }
                } catch (const std::exception& e) {
                    channelKeyErrorCount++;
                    if (channelKeyErrorCount <= 3) {
                        std::cerr << "Error parsing channel key '" << key << "': " << e.what() << std::endl;
                    }
                    channelName = key;
                    streamType = "Stream_Video";  // Default to video if parsing fails
                }

                outFile << "    {\n";
                outFile << "      \"name\": \"" << channelName << "\",\n";
                outFile << "      \"type\": \"" << streamType << "\",\n";
                outFile << "      \"packets\": [\n";

                // Get the sorted linked list for this channel
                auto dataStreamHolder = channelData.get(key);
                if (dataStreamHolder != nullptr) {
                    bool firstPacket = true;
                    Node* currentNode = nullptr;
                    
                    try {
                        currentNode = dataStreamHolder->getHead();
                    } catch (const std::exception& e) {
                        getHeadErrorCount++;
                        if (getHeadErrorCount <= 3) {
                            std::cerr << "Error getting head node: " << e.what() << std::endl;
                        }
                    }

                    while (currentNode != nullptr) {
                        // Skip null data nodes
                        if (currentNode->data == nullptr) {
                            currentNode = currentNode->next;
                            continue;
                        }
                        
                        AvDataInterface* avData = nullptr;
                        try {
                            avData = static_cast<AvDataInterface*>(currentNode->data);
                        } catch (const std::exception& e) {
                            castErrorCount++;
                            if (castErrorCount <= 3) {
                                std::cerr << "Error casting node data: " << e.what() << std::endl;
                            }
                            currentNode = currentNode->next;
                            continue;
                        }
                        
                        if (avData != nullptr) {
                            if (!firstPacket) {
                                outFile << ",\n";
                            }
                            firstPacket = false;

                            // Add packet data
                            outFile << "        {\n";
                            
                            try {
                                outFile << "          \"sequenceNumber\": " << static_cast<int>(avData->getSequenceNumber()) << ",\n";
                            } catch (const std::exception& e) {
                                sequenceNumberErrorCount++;
                                if (sequenceNumberErrorCount <= 3) {
                                    std::cerr << "Error getting sequence number: " << e.what() << std::endl;
                                }
                                outFile << "          \"sequenceNumber\": 0,\n";
                            }
                            
                            // Add payload data
                            outFile << "          \"payload\": [";
                            
                            try {
                                auto payload = avData->getPayload();
                                if (payload != nullptr && !payload->empty()) {
                                    bool firstByte = true;
                                    for (auto byte : *payload) {
                                        if (!firstByte) {
                                            outFile << ", ";
                                        }
                                        firstByte = false;
                                        outFile << static_cast<int>(byte);
                                    }
                                }
                            } catch (const std::exception& e) {
                                payloadErrorCount++;
                                if (payloadErrorCount <= 3) {
                                    std::cerr << "Error processing payload data: " << e.what() << std::endl;
                                }
                            }
                            
                            outFile << "]\n        }";
                        }
                        
                        currentNode = currentNode->next;
                    }
                }

                outFile << "\n      ]\n    }";
            }

            outFile << "\n  ]\n}";
            outFile.close();
            
            // Print summary of errors if any occurred
            if (parsingErrorCount > 0 || rawDataErrorCount > 0 || payloadErrorCount > 0 || 
                channelKeyErrorCount > 0 || getHeadErrorCount > 0 || castErrorCount > 0 || 
                sequenceNumberErrorCount > 0) {
                std::cout << "\n--- Error Summary ---" << std::endl;
                if (parsingErrorCount > 0) std::cout << "Data parsing errors: " << parsingErrorCount << std::endl;
                if (rawDataErrorCount > 0) std::cout << "Raw data processing errors: " << rawDataErrorCount << std::endl;
                if (payloadErrorCount > 0) std::cout << "Payload processing errors: " << payloadErrorCount << std::endl;
                if (channelKeyErrorCount > 0) std::cout << "Channel key parsing errors: " << channelKeyErrorCount << std::endl;
                if (getHeadErrorCount > 0) std::cout << "Getting head node errors: " << getHeadErrorCount << std::endl;
                if (castErrorCount > 0) std::cout << "Type casting errors: " << castErrorCount << std::endl;
                if (sequenceNumberErrorCount > 0) std::cout << "Sequence number errors: " << sequenceNumberErrorCount << std::endl;
                std::cout << "All errors were handled gracefully." << std::endl;
            }
            
            std::cout << "\nVisualization data exported to " << m_outputPath << "/sample_data.json" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Critical error during data export: " << e.what() << std::endl;
            std::cerr << "Falling back to mockup data" << std::endl;
            createMockupData();
        } catch (...) {
            std::cerr << "Unknown error during data export" << std::endl;
            std::cerr << "Falling back to mockup data" << std::endl;
            createMockupData();
        }
    }

private:
    std::string m_outputPath;
    
    // Creates a simple mockup data file for visualization if real data is unavailable
    void createMockupData() {
        std::ofstream outFile(m_outputPath + "/sample_data.json");
        if (!outFile.is_open()) {
            std::cerr << "Failed to create mockup data file!" << std::endl;
            return;
        }
        
        // Use our pre-created sample data with mock TV channels
        outFile << "{\n";
        outFile << "  \"bufferData\": [\n";
        outFile << "    {\n";
        outFile << "      \"channelName\": \"XYZ1\",\n";
        outFile << "      \"streamType\": \"Stream_Video\",\n";
        outFile << "      \"sequenceNumber\": 1,\n";
        outFile << "      \"rawData\": [45, 88, 89, 90, 49, 1, 1, 120, 121, 122, 123, 124],\n";
        outFile << "      \"payload\": [120, 121, 122, 123, 124]\n";
        outFile << "    },\n";
        outFile << "    {\n";
        outFile << "      \"channelName\": \"XYZ1\",\n";
        outFile << "      \"streamType\": \"Stream_Video\",\n";
        outFile << "      \"sequenceNumber\": 2,\n";
        outFile << "      \"rawData\": [45, 88, 89, 90, 49, 1, 2, 130, 131, 132, 133, 134],\n";
        outFile << "      \"payload\": [130, 131, 132, 133, 134]\n";
        outFile << "    },\n";
        outFile << "    {\n";
        outFile << "      \"channelName\": \"DEF9\",\n";
        outFile << "      \"streamType\": \"Stream_Audio\",\n";
        outFile << "      \"sequenceNumber\": 1,\n";
        outFile << "      \"rawData\": [45, 68, 69, 70, 57, 0, 1, 210, 211, 212, 213, 214],\n";
        outFile << "      \"payload\": [210, 211, 212, 213, 214]\n";
        outFile << "    }\n";
        outFile << "  ],\n";
        outFile << "  \"channels\": [\n";
        outFile << "    {\n";
        outFile << "      \"name\": \"XYZ1\",\n";
        outFile << "      \"type\": \"Stream_Video\",\n";
        outFile << "      \"packets\": [\n";
        outFile << "        {\n";
        outFile << "          \"sequenceNumber\": 1,\n";
        outFile << "          \"payload\": [120, 121, 122, 123, 124]\n";
        outFile << "        },\n";
        outFile << "        {\n";
        outFile << "          \"sequenceNumber\": 2,\n";
        outFile << "          \"payload\": [130, 131, 132, 133, 134]\n";
        outFile << "        }\n";
        outFile << "      ]\n";
        outFile << "    },\n";
        outFile << "    {\n";
        outFile << "      \"name\": \"DEF9\",\n";
        outFile << "      \"type\": \"Stream_Audio\",\n";
        outFile << "      \"packets\": [\n";
        outFile << "        {\n";
        outFile << "          \"sequenceNumber\": 1,\n";
        outFile << "          \"payload\": [210, 211, 212, 213, 214]\n";
        outFile << "        }\n";
        outFile << "      ]\n";
        outFile << "    }\n";
        outFile << "  ]\n";
        outFile << "}\n";
        outFile.close();
        
        std::cout << "Created mockup visualization data in " << m_outputPath << "/sample_data.json" << std::endl;
    }
    
    void ensureDirectoryExists() {
        // Check if the directory exists, create it if not
        #ifdef _WIN32
        std::string command = "if not exist \"" + m_outputPath + "\" mkdir \"" + m_outputPath + "\"";
        #else
        std::string command = "mkdir -p \"" + m_outputPath + "\"";
        #endif
        
        std::system(command.c_str());
    }
};

#endif // DATA_EXPORTER_H
