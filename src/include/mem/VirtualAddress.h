#pragma once

#include <stdint.h>
#include <virtualMem.h>

/* 
 * TODO: remove the "2" when removing the typedef
 */
class VirtualAddress2 {
  public:
    VirtualAddress2 () {}
    explicit VirtualAddress2(uint32_t address) 
      : _address(address)
    {}

  void set(uint32_t address) { _address = address; }
  uint32_t get() const { return _address; }
  
  VirtualAddress2 offset(uint32_t o) { return VirtualAddress2(_address + o); }
  VirtualAddress2 alignUp() { return VirtualAddress2((_address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)); }

  private:
    uint32_t _address { 0 };
};
