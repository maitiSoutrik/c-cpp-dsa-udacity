#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

template <typename T>
class DataStructure {
 public:
  virtual ~DataStructure() = default;

  // Insert a data pointer into the structure
  virtual void insert(T data) = 0;

  // Remove data from the structure and return the pointer
  virtual T remove() = 0;

  // Optional: Method to check if the container is empty
  virtual bool isEmpty() const = 0;
};

#endif  // DATASTRUCTURE_H
