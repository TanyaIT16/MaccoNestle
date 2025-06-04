#ifndef MOTORCOMMANDS_H
#define MOTORCOMMANDS_H

#include <Arduino.h>
#include "stepperDriver.h"
// Enum defining the behavior of the stepper motor

// Struct to hold motor data


// Class to manage motor commands using a stack
class MotorCommands {
public:
  // Constructor
  MotorCommands() {
    top = nullptr;  // Initialize top pointer to nullptr
  }

  // Push a MotorData object onto the stack
  void push(const MotorData& data) {
    // Create a new node
    Node* newNode = new Node;
    newNode->data = data;  // Assign data to the new node
    newNode->next = top;   // Link the new node to the current top
    top = newNode;         // Update top to point to the new node
  }

  // Pop a MotorData object from the stack
  MotorData pop() {
    if (isEmpty()) {
      // Stack is empty, return a default-initialized MotorData
      return MotorData();
    } else {
      // Retrieve data from the top node
      MotorData data = top->data;
      // Move top to the next node
      Node* temp = top;
      top = top->next;
      delete temp;  // Free memory allocated for the top node
      return data;  // Return the retrieved data
    }
  }

  // Check if the stack is empty
  bool isEmpty() const {
    return top == nullptr;
  }

  int size() const {
    int count = 0;
    Node* current = top;
    while (current != nullptr) {
      count++;
      current = current->next;
    }
    return count;
  }

private:
  // Node structure for the stack
  struct Node {
    MotorData data;
    Node* next;
  };

  Node* top;  // Pointer to the top of the stack
};

#endif  // MOTORCOMMANDS_H