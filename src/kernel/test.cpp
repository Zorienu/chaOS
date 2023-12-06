#include "test.h"

int TestClass::getCounter() {
  return this->counter;
}

void TestClass::setCounter(int n) {
  this->counter = n;
}

void TestClass::incrementCounter() {
  this->counter++;
}
