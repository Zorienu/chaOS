#pragma once
#include "CharacterDevice.h"
#include "../irqHandler.h"

class KeyboardClient;

class KeyboardDevice final : public IRQHandler, public CharacterDevice {
  public: 
    KeyboardDevice();

    static KeyboardDevice& the();

    // TODO: implement
    virtual bool canRead (FileDescription&) const override { return false; }
    virtual bool canWrite (FileDescription&) const override { return false; }

    // TODO: use ssize_t instead
    // TODO: implement
    virtual size_t read(FileDescription&, uint8_t*, size_t) override { return 0; }
    virtual size_t write(FileDescription&, const uint8_t*, size_t) override { return 1; }

    void setClient(KeyboardClient *client) { _client = client; }

  private:
    virtual void handleIRQ() override;

    KeyboardClient *_client { NULL };
};

class KeyboardClient {
  public:
    // TODO: give event (with Ctrl, Alt, Shift, etc) instead of just the char
    virtual void onKeyPressed(char c) = 0;
};
