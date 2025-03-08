#ifndef AVDATAINTERFACE_H
#define AVDATAINTERFACE_H

#include <cstdint>
#include <string>
#include <vector>

#include "comparable_data_interface.h"

enum class StreamType : uint8_t { Stream_Audio = 0, Stream_Video = 1 };

// Abstract base class for handling A/V data
class AvDataInterface : public ComparableDataWrapperInterface {
 public:
  // Virtual destructor for the abstract base class
  ~AvDataInterface() override {
    if (m_payload != nullptr) {
      delete m_payload;
      m_payload = nullptr ;
    }

    if (m_rawDataPtr != nullptr) {
      delete m_rawDataPtr;
      m_rawDataPtr = nullptr ;
    }
  }

  // Constructor takes a vector of uint8_t (immutable raw data)
  AvDataInterface(std::vector<uint8_t>* rawDataPtr)
      : m_rawDataPtr(rawDataPtr), m_payload(nullptr), m_sequenceNumber(0) {}

  // Delete copy constructor and assignment operator to enforce immutability
  AvDataInterface(const AvDataInterface&) = delete;
  AvDataInterface& operator=(const AvDataInterface&) = delete;

  // Delete move constructor and assignment operator as well
  AvDataInterface(AvDataInterface&&) = delete;
  AvDataInterface& operator=(AvDataInterface&&) = delete;

  // Pure virtual function to parse the raw data block into components (channel
  // name, content type, sequence number, payload)
  virtual void parseData(std::string* channelName, StreamType* streamType) = 0;

  // Get the payload (parsed from the raw data)
  virtual std::vector<uint8_t>* getPayload() const { return m_payload; }

  // Get the sequence number (parsed from the raw data)
  virtual uint8_t getSequenceNumber() const { return m_sequenceNumber; }

  virtual const std::vector<uint8_t>* const getRawData() const {
    return m_rawDataPtr;
  }

  int getValue() const override { return m_sequenceNumber; }

  // Comparison methods for equality and ordering
  bool isEqual(const ComparableDataWrapperInterface& other) const override {
    return m_sequenceNumber == other.getValue();
  }

  bool isLessThan(
      const ComparableDataWrapperInterface& other) const override {
    return m_sequenceNumber < other.getValue();
  }  

 protected:
  std::vector<uint8_t>* m_rawDataPtr;
  std::vector<uint8_t>* m_payload;
  uint8_t m_sequenceNumber;
};

#endif  // AVDATAINTERFACE_H