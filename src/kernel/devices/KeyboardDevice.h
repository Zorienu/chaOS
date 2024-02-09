#pragma once
#include "CharacterDevice.h"
#include "../interrupts/IRQHandler.h"
#include "../KeyCode.h"
#include "../utils/datastructures/CircularQueue.h"

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
    CircularQueue<KeyEvent, 16> _queue;
    virtual void handleIRQ() override;

    KeyboardClient *_client { NULL };

    void updateModifier(KeyModifiers modifier, bool pressed);
    // Is Alt, Shift, Control or GUI pressed?
    uint8_t _modifiers { 0 };

    // Was the pressed key composite by two scan codes?
    // starting with '0xE0' and another? 
    // For example the GUI key scan codes when pressed are 0xE0 and 0x5B
    // and 0xE0 and 0xDB when released
    bool _isE0Preceded { 0 };
};

class KeyboardClient {
  public:
    // TODO: give event (with Ctrl, Alt, Shift, etc) instead of just the char
    virtual void onKeyPressed(KeyEvent event) = 0;
};
