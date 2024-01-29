#pragma once
#include "TTY.h"
#include "../devices/KeyboardDevice.h"

// Possible background and foreground colors in text-mode in text-mode
enum Color {
  C_BLACK = 0,
  C_BLUE = 1,
  C_GREEN = 2,
  C_CYAN = 3,
  C_RED = 4,	
  C_MAGENTA = 5,
  C_BROWN = 6,
  C_LIGHT_GREY = 7,
  C_DARK_GREY = 8,
  C_LIGHT_BLUE = 9,
  C_LIGHT_GREEN = 10,
  C_LIGHT_CYAN = 11,
  C_LIGHT_RED = 12,
  C_LIGHT_MAGENTA = 13,
  C_LIGHT_BROWN = 14,
  C_WHITE = 15,
};

class VirtualConsole final : public TTY, KeyboardClient {
  public:
    VirtualConsole(unsigned int index);
    void static initialize();

    void clear();
    void onChar(char c);
    static void switchTo(uint8_t consoleIndex);
    void setActive(bool active);

    static VirtualConsole* getCurrentConsole();

    virtual void onKeyPressed(char c) override;

  protected: 
    virtual size_t onTTYWrite(const uint8_t *buffer, size_t size) override;

  private: 
    uint8_t *_buffer;

    unsigned int _index;
    bool _active;

    uint8_t _currentRow { 0 };
    uint8_t _currentColumn { 0 }; 

    uint8_t _currentBackgroundColor { Color::C_BLUE };
    uint8_t _currentForegroundColor { Color::C_WHITE };

    void putCharAt(uint8_t row, uint8_t column, char c);
    void horizontalScroll();

    void setBackgroundColor(Color color);
    void setForegroundColor(Color color);
};
