#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <atomic>
#include <cstddef>  // For size_t
#include <mutex>
#include <stdexcept>
#include <vector>

#include "data_structure.h"

template <typename T>
class CircularBuffer : public DataStructure<T> {
 public:
  CircularBuffer(size_t size=32)
      : m_data(size), m_size(size), m_head(0), m_tail(0), m_count(0) {
      for (size_t i = 0; i < m_size; ++i) {
        m_data[i] = nullptr;
      }
    }

  ~CircularBuffer() {
    std::lock_guard<std::mutex> lock(
        m_mutex);  // Ensure thread safety during destruction
    for (size_t i = 0; i < m_size; ++i) {
      if (m_data[i] != nullptr) {
        delete m_data[i];  // Delete each object if it exists
        m_data[i] = nullptr;
      }
    }
  }

  // Inserts an item into the circular buffer
  void insert(T item) override {
    std::lock_guard<std::mutex> lock(
        m_mutex);  // Lock the mutex for thread safety

    if (isFull()) {
      // If the buffer is full, advance the head to make space (overwrite the
      // oldest element)
      delete m_data[m_head];
      m_head = (m_head + 1) % m_size;
    } else {
      m_count++;  // Increment count only if the buffer isn't full
    }

    // Insert the item at the tail and advance the tail pointer
    m_data[m_tail] = item;
    m_tail = (m_tail + 1) % m_size;
  }

  // Removes and returns the item at the head of the circular buffer
  T remove() override {
    std::lock_guard<std::mutex> lock(
        m_mutex);  // Lock the mutex for thread safety

    if (isEmpty()) {
      throw std::out_of_range("Buffer is empty");
    }

    // Retrieve the item at the head and advance the head pointer
    T item = m_data[m_head];
    m_data[m_head] = nullptr;
    m_head = (m_head + 1) % m_size;
    m_count--;  // Decrement the count since we're removing an item

    return item;
  }

  // Checks if the buffer is empty
  bool isEmpty() const override { return m_count.load() == 0; }

 private:
  std::vector<T> m_data;        // Storage for the buffer items
  size_t m_size;                // Maximum size of the buffer
  size_t m_head;                // Points to the head of the buffer
  size_t m_tail;                // Points to the tail of the buffer
  std::atomic<size_t> m_count;  // Current number of elements in the buffer

  mutable std::mutex m_mutex;  // Mutex to ensure thread safety

  bool isFull() const { return m_count.load() == m_size; }
};

#endif  // CIRCULARBUFFER_H