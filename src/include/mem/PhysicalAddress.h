#pragma once

#include <stdint.h>
#include "virtualMem.h"

/* 
 * TODO: remove the "2" when removing the typedef
 */
class PhysicalAddress2 {
  public:
    PhysicalAddress2 () {}
    explicit PhysicalAddress2(uint32_t address) 
      : _address(address)
    {}

  uint32_t get() const { return _address; }
  
  PhysicalAddress2 offset(uint32_t o) { return PhysicalAddress2(_address + o); }
  PhysicalAddress2 alignUp() { return PhysicalAddress2((_address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)); }

  private:
    uint32_t _address { 0 };
};
