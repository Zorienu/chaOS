#pragma once
#include "CharacterDevice.h"
#include "../irqHandler.h"

class KeyboardDevice final : public IRQHandler, public CharacterDevice {
  public: 
    KeyboardDevice();

    // TODO: implement
    virtual bool canRead (FileDescription&) const override { return false; }
    virtual bool canWrite (FileDescription&) const override { return false; }

    // TODO: use ssize_t instead
    // TODO: implement
    virtual size_t read(FileDescription&, uint8_t*, size_t) const override { return 0; }
    virtual size_t write(FileDescription&, const uint8_t*, size_t) const override { return 1; }

  private:
    virtual void handleIRQ() override;
};
