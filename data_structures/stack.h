#ifndef STACK_H
#define STACK_H

#include <vector>
#include <stdexcept>
#include "data_structure.h"

// Templated Stack derived from DataStructure
template <typename T>
class Stack : public DataStructure<T> {
 public:
  // Constructor: Optional, using default vector
  Stack() = default;

  // Destructor: Default
  ~Stack() override = default;

  // Pushes (inserts) an element onto the stack
  void insert(T data) override {
    m_data.push_back(data);  // Add data to the end of the vector
  }

  // Pops (removes) an element from the stack and returns it
  T remove() override {
    if (isEmpty()) {
      throw std::out_of_range("Stack is empty");
    }
    T data = m_data.back();  // Get the last element (top of the stack)
    m_data.pop_back();       // Remove the last element
    return data;
  }

  // Checks if the stack is empty
  bool isEmpty() const override { return m_data.empty(); }

 private:
  std::vector<T> m_data;  // Underlying container to hold the stack elements
};

#endif  // STACK_H
