#ifndef HASHMAP_H
#define HASHMAP_H

#include <iostream>
#include <vector>
#include <optional>

template <typename Key, typename Value>
class HashMap {
 private:
  struct Entry {
    Key key;
    Value* value = nullptr;  // Store the pointer to Value
    bool isOccupied = false;
    bool isDeleted = false;
  };

  std::vector<Entry> table;
  size_t capacity;
  size_t size;

  // Hash function (using std::hash)
  size_t hash(const Key& key) const { return std::hash<Key>{}(key) % capacity; }

  // Function to find the index of a key (used for get, and remove)
  std::optional<size_t> findIndex(const Key& key) const {
    size_t index = hash(key);
    size_t originalIndex = index;
    while (table[index].isOccupied) {
      if (table[index].key == key && !table[index].isDeleted) {
        return index;
      }
      index = (index + 1) % capacity;
      if (index == originalIndex) {
        break;  // We've looped through the entire table
      }
    }
    return std::nullopt;
  }

 public:
  // Constructor
  HashMap(size_t initCapacity = 16)
      : table(initCapacity), capacity(initCapacity), size(0) {}

  // Destructor: Clean up dynamically allocated memory
  ~HashMap() {
    for (auto& entry : table) {
      if (entry.isOccupied && !entry.isDeleted) {
        remove(entry.key);  // Use the remove method to handle deletion
      }
    }
  }

  // Insert or update an entry in the HashMap, 
  // accepting Value* instead of const Value&
  void insert(const Key& key, Value* value) {
    size_t index = hash(key);
    size_t attempts = 0;       // Counter for probing attempts

    while (table[index].isOccupied && !table[index].isDeleted) {
      if (table[index].key == key) {
        if (nullptr != table[index].value) {
          delete table[index].value;  // Delete the old value to avoid memory leaks
        }
        table[index].value = value;  // Update with the new pointer, no copying
        return;
      }
      index = (index + 1) % capacity;
      attempts++;
      if (attempts == capacity) {
        throw std::runtime_error("Hash table is full");
      }
    }

    // Insert new entry and set isOccupied flag
    table[index] = {key, value, true, false};
    ++size;

    // Optional: Implement resizing logic if the load factor is too high
  }

  // Retrieve a pointer to the value associated with a key
  Value* get(const Key& key) const {
    auto index = findIndex(key);
    if (index && table[*index].value) {
      return table[*index].value;  // Return the pointer directly
    }
    return nullptr;  // Return nullptr if key not found
  }

  // Remove an entry with the given key
  bool remove(const Key& key) {
    auto index = findIndex(key);
    if (index) {
      if (nullptr != table[*index].value) {
        delete table[*index].value;  // Delete the dynamically allocated value
        table[*index].value = nullptr;  // Set pointer to nullptr for safety
      }
      table[*index].isDeleted = true;
      --size;
      return true;
    }
    return false;
  }

  // Check if the HashMap contains a key
  bool contains(const Key& key) const { return findIndex(key).has_value(); }

  // Get the number of elements in the HashMap
  size_t getSize() const { return size; }
  
  // Get all keys in the HashMap
  void getAllKeys(std::vector<Key>& keys) const {
    keys.clear();
    for (const auto& entry : table) {
      if (entry.isOccupied && !entry.isDeleted) {
        keys.push_back(entry.key);
      }
    }
  }
};

#endif
