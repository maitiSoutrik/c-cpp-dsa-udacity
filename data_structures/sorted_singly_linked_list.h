#ifndef SORTEDSINGLYLINKEDLIST_H
#define SORTEDSINGLYLINKEDLIST_H
 
 #include "singley_linked_list.h"

class SortedSinglyLinkedList : public SinglyLinkedList {
public:
    // Insert a new node in ascending order based on value
    void insert(ComparableDataWrapperInterface* value) override {
        Node* newNode = new Node(value);
        
        // If the list is empty or the new node should be the new head
        if (head == nullptr || value->isLessThan(*head->data)) {
            newNode->next = head;
            head = newNode;
        } else {
            // Find the appropriate position to insert the new node
            Node* current = head;
            while (current->next != nullptr && !value->isLessThan(*current->next->data)) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
        ++size;
    }
};

#endif