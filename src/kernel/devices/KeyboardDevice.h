#pragma once
#include "CharacterDevice.h"
#include "../irqHandler.h"
#include "../KeyCode.h"

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

    void updateModifier(KeyModifiers modifier, bool pressed);
    // Is Alt, Shift, Control or GUI pressed?
    uint8_t _modifiers { 0 };
};

class KeyboardClient {
  public:
    // TODO: give event (with Ctrl, Alt, Shift, etc) instead of just the char
    virtual void onKeyPressed(KeyEvent event) = 0;
};
