#pragma once
#include <kernel/utils/datastructures/CircularQueue.h>

template<typename T, int capacity>
class CircularDeque : public CircularQueue<T, capacity> {
  public:
    T dequeueEnd() {
      ASSERT(!this->empty());

      // Get the index of the last element in the queue
      uint32_t idx = ((this->_front + this->_size - 1) % capacity) * sizeof(T);
      
      this->_size--;
      
      // Get the element in "idx"
      T *element = (T*)&this->_elements[idx];
      
      // Use "move constructor" to move the content of the element into the value to be returned
      T value = move(*element);
      
      // Destroy the stored element (this won't affect "value")
      element->~T();
      
      return value;
    };
};
