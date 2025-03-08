#ifndef SINGLYLINKEDLIST_H
#define SINGLYLINKEDLIST_H

#include "node.h"

class SinglyLinkedList {
 public:
  SinglyLinkedList() : head(nullptr), size(0) {}

  virtual ~SinglyLinkedList() {
    Node* current = head;
    while (current != nullptr) {
      Node* next = current->next;
      delete current;  // Node's destructor will delete the data
      current = next;
    }
  }

  // Pure virtual insert method
  virtual void insert(ComparableDataWrapperInterface* value) = 0;

  // Search for a node by value
  Node* search(int value) {
    Node* current = head;
    while (current != nullptr) {
      if (current->data->getValue() == value) {
        return current;
      }
      current = current->next;
    }
    return nullptr;
  }

  // Remove a node by value
  void remove(int value) {
    Node* current = head;
    Node* previous = nullptr;
    while (current != nullptr) {
      if (current->data->getValue() == value) {
        if (previous == nullptr) {
          head = current->next;
        } else {
          previous->next = current->next;
        }
        delete current;  // Node's destructor will delete the data
        --size;
        return;
      }
      previous = current;
      current = current->next;
    }
  }

  // Print all nodes in the list
  void printList() const {
    Node* current = head;
    while (current != nullptr) {
      std::cout << "Node with value: " << current->data->getValue()
                << std::endl;
      current = current->next;
    }
  }

  // Get the size of the list
  size_t getSize() const { return size; }

  // Get the head node for iteration
  Node* getHead() const { return head; }

 protected:
  Node* head;
  size_t size;
};

#endif