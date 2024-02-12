#pragma once
#include <stdint.h>
#include "../kprintf.h"
#include "../Assertions.h"
#include "../utility.h"

template<typename T, int capacity>
class CircularQueue {
  public:
    CircularQueue() { kprintf("Elements address: %lx", _elements); }
    ~CircularQueue() { clear(); }

    // We only recieve rvalues to avoid the caller keep using the object
    // Chat GPT says: When you pass an object by value or by value reference, 
    // you provide a copy of the object to the function. 
    // This means that any modifications made to the object within the function are local 
    // to that function and do not affect the original object in the caller's scope.
    // Here's a good explanation differentiating:
    // void foo(Widget w); lvalues and rvalues
    // void foo(Widget& w); lvalues only
    // void foo(const Widget& w); lvalues and "rvalues"
    // void foo(Widget&& w);  rvalues only
    // https://stackoverflow.com/questions/37935393/pass-by-value-vs-pass-by-rvalue-reference
    void enqueue(T&& value) {
      // if we exceed the capacity, we simply go back to 0 and lost the first element
      uint32_t idx = ((_front + _size) % capacity) * sizeof(T);
      
      // Queue already full? we will lost the first element and must destroy them
      if (full()) first().~T();

      // Save new item at the current index
      // Using move since "value" is now considered an l-value
      // instead of an r-value, then we need to tell to consider it
      // as an r-value again to use the move constructor instead of the copy constructor
      new (&_elements[idx]) T(move(value));

      // The queue is full now, then we should move the head to the next element
      // (losing the previously "first" element)
      if (full()) _front = (_front + 1) % capacity;
      // Otherwise we simply say that the queue has 1 more element
      else _size++;
    };

    // Overload to receive lvalues (constant, to avoid modifying it) 
    // and create a copy of them
    void enqueue(const T& value) {
      enqueue(T(value));
    }

    T dequeue() {
      ASSERT(!empty());

      // Get the index of the first element in the queue
      uint32_t idx = _front * sizeof(T);
      
      // Move the head to the next element
      _front = (_front + 1) % capacity;
      
      if (!empty()) _size--;
      
      // Get the address of the "idx" element 
      T *element = (T*)&_elements[idx];
      
      // Use "copy constructor" to copy the content of the element into the value to be returned
      T value = move(*element);
      
      // Destroy the stored element (this won't affect "value")
      element->~T();
      
      return value;
    };

    void clear() {
      while (!empty()) {
        elements()[_front++].~T();
        _size--;
      }
    }
    
    bool full() {
        return _size == capacity;
    }
    
    bool empty() {
        return _size == 0;
    }
    
    int size() {
        return _size;
    }

  private: 
    T* elements() { return (T*)_elements; }

    const T& at(int index) { return elements()[(_front + index) % capacity]; };
    const T& first() { return at(0); }
    const T& last() { return at(size() - 1); }

    uint8_t _elements[sizeof(T) * capacity];
    int _size;
    int _front;
};
