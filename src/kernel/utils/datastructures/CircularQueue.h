#pragma once
#include <stdint.h>

template<typename T, int capacity>
class CircularQueue {
  public:
    CircularQueue() {}

    void enqueue(T value) {
      // if we exceed the capacity, we simply go back to 0 and lost the first element
      uint32_t idx = ((front + m_size) % capacity) * sizeof(T);
      
      // Queue already full? we will lost the first element and must destroy them
      if (full()) {
        T element = elements()[(front + m_size) % capacity];
        element.~T();
      }

      // Save new item at the current index
      new (&_elements[idx]) T(value);

      // The queue is full now, then we should move the head to the next element
      if (full()) front = (front + 1) % capacity;
      // Otherwise we simply say that the queue has 1 more element
      else m_size++;
    };

    T dequeue() {
      // Get the index of the first element in the queue
      uint32_t idx = front * sizeof(T);
      
      // Move the head to the next element
      front = (front + 1) % capacity;
      
      if (!empty()) m_size--;
      
      // Get the address of the "idx" element and cast it
      T *element = (T*)&_elements[idx];
      
      // Use "copy constructor" to copy the content of the element into the value to be returned
      T value = *element;
      
      // Destroy the stored element (this won't affect "value")
      element->~T();
      
      return value;
    };
    
    bool full() {
        return m_size == capacity;
    }
    
    bool empty() {
        return m_size == 0;
    }
    
    int size() {
        return m_size;
    }

  private: 
    T* elements() { return (T*)_elements; }

    uint8_t _elements[sizeof(T) * capacity];
    int m_size;
    int front;
};
