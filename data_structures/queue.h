#ifndef QUEUE_H
#define QUEUE_H

#include <vector>
#include <stdexcept>
#include "data_structure.h"

// Templated Queue derived from DataStructure using std::vector
template <typename T>
class Queue : public DataStructure<T> {
 public:
  // Constructor: Default
  Queue() = default;

  // Destructor: Default
  ~Queue() override = default;

  // Enqueues (inserts) an element into the queue
  void insert(T data) override {
    m_data.push_back(
        data);  // Add data to the end of the vector (back of the queue)
  }

  // Dequeues (removes) an element from the queue and returns it
  T remove() override {
    if (isEmpty()) {
      throw std::out_of_range("Queue is empty");
    }
    T data = m_data.front();       // Get the first element (front of the queue)
    m_data.erase(m_data.begin());  // Remove the first element
    return data;
  }

  // Checks if the queue is empty
  bool isEmpty() const override { return m_data.empty(); }

 private:
  std::vector<T> m_data;  // Underlying container to hold the queue elements
};

#endif  // QUEUE_H
