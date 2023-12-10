#pragma once

#include "PhysicalAddress.h"
#include <stdint.h>

class PageTableEntry2 {
  public:
    enum Flags {
      Present =        1 << 0,
      ReadWrite =      1 << 1,
      UserSupervisor = 1 << 2,
      WriteThrough =   1 << 3,
      CacheDisabled =  1 << 4,
    };

  bool isPresent() { return _entry & Present; }
  void setPresent(bool value) { setBit(Present, value); }

  bool isWritable() { return _entry & ReadWrite; }
  void setWritable(bool value) { setBit(ReadWrite, value); }

  bool isUserAllowed() { return _entry & UserSupervisor; }
  void setUserAllowed(bool value) { setBit(UserSupervisor, value); }
  
  bool isWriteThrough() { return _entry & WriteThrough; }
  void setWriteThrough(bool value) { setBit(WriteThrough, value); }

  bool isCacheDisabled () { return _entry & CacheDisabled; }
  void setCacheDisabled (bool value) { setBit(CacheDisabled, value); }

  void setBit (uint8_t bit, bool value) {
    if (value)
      _entry |= bit;
    else 
      _entry &= ~bit;
  }

  // Ox7FF = bits sets from bit 0 to bit 11 (flag bits)
  // Preserve flags and remove old frame
  // Set new frame
  void setPhysicalFrame (uint32_t pa) { _entry = (_entry & 0x7FF) | pa; }

  private:
    uint32_t _entry;
};
