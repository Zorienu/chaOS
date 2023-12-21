#include "Device.h"

class CharacterDevice : public Device {
  public:
    virtual bool isCharacterDevice() const { return true; }

  private: 
};
