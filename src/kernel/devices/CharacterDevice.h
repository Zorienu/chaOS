#pragma once
#include <kernel/devices/Device.h>

class CharacterDevice : public Device {
  public:

  private: 
    virtual bool isCharacterDevice() const final { return true; }

  protected: 
    CharacterDevice() : Device() {};
};
