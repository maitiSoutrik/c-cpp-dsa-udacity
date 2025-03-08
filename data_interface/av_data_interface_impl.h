#ifndef AVDATAINTERFACEIMPL_H
#define AVDATAINTERFACEIMPL_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "av_data_interface.h"

class AvDataInterfaceImpl : public AvDataInterface {
 public:
  // Constructor takes the raw data pointer and calls the base class constructor
  AvDataInterfaceImpl(std::vector<uint8_t>* rawDataPtr)
      : AvDataInterface(rawDataPtr) {}

  // Implement the pure virtual function to parse the raw data block
  void parseData(std::string* channelName, StreamType* streamType) override {
    // Check if the pointers passed in are valid
    if (!channelName || !streamType || !m_rawDataPtr) {
      throw std::invalid_argument("Null pointer(s) detected");
    }

    // Ensure that the header is of valid length
    const size_t minRequiredSize = 4 + 1 + 1;
    if (m_rawDataPtr->size() < minRequiredSize) {
      throw std::runtime_error("Invalid header to parse");
    }

    // ########################################################################
    // Implement the logic to parse the raw data and populate the 
    // channelName and streamType pointers' content 
    // and the m_sequenceNumber class variable.

    // Convert the first 4 bytes of the raw data to a string by using the
    // std::string() constructor that takes two iterators as arguments, one for
    // the beginning and the other for the end of the range. 
    *channelName = std::string(m_rawDataPtr->begin(), m_rawDataPtr->begin() + 4);

    // Parse the content type (audio/video), by static_casting the 5th byte
    // (index 4) of the raw data to StreamType
    *streamType = static_cast<StreamType>((*m_rawDataPtr)[4]);

    // Extract the sequence number, which is the 6th byte (index 5) of the raw
    // data and assign it to m_sequenceNumber
    m_sequenceNumber = (*m_rawDataPtr)[5];

    // end of implementation
    // ########################################################################    

    // Extract the payload by erasing the header
    m_rawDataPtr->erase(m_rawDataPtr->begin(),
                        m_rawDataPtr->begin() + minRequiredSize);
    m_payload = m_rawDataPtr;
    m_rawDataPtr = nullptr;
  }

  // Default destructor since the base class handles memory management
  ~AvDataInterfaceImpl() = default;
};

#endif  // AVDATAINTERFACEIMPL_H
