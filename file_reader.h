#ifndef FILEREADER_H
#define FILEREADER_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>  // For std::hex and std::setw
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "data_structure.h"  // Include your DataStructure interface

template <typename T>
class FileReader {
 public:
  // Constructor that initializes the FileReader with a DataStructure instance,
  // and chunk size
  FileReader(DataStructure<T>* dataStructure, const std::string& filePath,
             size_t chunkSize=128, bool debug = false)
      : m_dataStructure(dataStructure),
        m_filePath(filePath),
        m_chunkSize(chunkSize),
        m_debug(debug),
        m_terminate(false),
        m_isRunning(true) {
    // Open the file
    m_file.open(filePath, std::ios::binary);
    if (!m_file) {
      std::cerr << "Failed to open the file: " << filePath << std::endl;
      m_isRunning =
          false;  // If the file can't be opened, set state to not busy
      return;
    }

    // Determine file size by seeking to the end and using tellg
    m_file.seekg(0, std::ios::end);
    m_fileSize = m_file.tellg();
    m_file.seekg(0, std::ios::beg);  // Reset back to the beginning of the file

    std::cout << "File size: " << m_fileSize << " bytes" << std::endl
              << std::endl;
  }

  // Destructor
  ~FileReader() {
    if (m_file.is_open()) {
      m_file.close();
    }
    if (m_thread.joinable()) {
      m_terminate = true;
      m_thread.join();
    }
  }

  // Starts the file reading process in a separate thread
  void startReading() {
    if (!m_file.is_open()) {
      std::cerr << "Cannot start reading, file is not open." << std::endl;
      return;
    }
    m_thread = std::thread(&FileReader::readFile, this);
  }

  // Check if the reader is still running
  bool isRunning() const { return m_isRunning; }

 private:
  DataStructure<std::vector<uint8_t>*>* m_dataStructure;
  std::ifstream m_file;
  std::string m_filePath;
  size_t m_chunkSize;
  bool m_debug;
  std::thread m_thread;
  std::atomic<bool> m_terminate;
  std::atomic<bool>
      m_isRunning;  // Atomic flag to indicate whether the reader is busy
  size_t m_fileSize;

  // Reads the file in chunks and pushes data into the DataStructure
  void readFile() {
    size_t totalBytesRead = 0;

    while (!m_terminate && totalBytesRead < m_fileSize) {
      // Calculate how much data we can read
      size_t bytesLeft = m_fileSize - totalBytesRead;
      size_t currentChunkSize = std::min(m_chunkSize, bytesLeft);

      // Read the data into a buffer
      T data = nullptr;

      if constexpr (std::is_pointer<T>::value) {
        using ValueType = typename std::remove_pointer<T>::type;
        data = new ValueType(
            currentChunkSize);  // Allocate for vector or other types
      } else {
        throw std::runtime_error("Non-pointer data type");
      }

      m_file.read(reinterpret_cast<char*>(data->data()), currentChunkSize);
      size_t bytesRead =
          m_file.gcount();  // Get how many bytes were actually read

      if (bytesRead > 0) {
        totalBytesRead += bytesRead;

        // Push the data to the DataStructure
        m_dataStructure->insert(data);

        if (m_debug) {
          const unsigned char* bytes =
              reinterpret_cast<const unsigned char*>(data->data());
          std::cout << "First 8 bytes as hex: ";
          for (int i = 0; i < 8; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(bytes[i]) << " ";
          }
          std::cout << std::dec << " @ " << totalBytesRead << " - " << bytesLeft
                    << std::endl;  // Switch back to decimal output
        }
      }

      // Terminate if the end of the file is reached
      if (bytesRead < m_chunkSize) {
        if (m_debug) {
          std::cout << "End of file reached. Stopping reading." << std::endl;
        }
        break;
      } else {
        // Sleep for 100ms between reads
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
    // Close the file and stop the thread when finished
    if (m_file.is_open()) {
      m_file.close();
    }

    // Mark as not busy after finishing reading
    m_isRunning = false;
  }
};

#endif  // FILEREADER_H
