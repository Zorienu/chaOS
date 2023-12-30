#pragma once
#include "Device.h"

class CharacterDevice : public Device {
  public:
    virtual bool isCharacterDevice() const { return true; }

  private: 
    virtual bool isCharacterDevice() const final { return true; }

  protected: 
    CharacterDevice() : Device() {};
};
