#pragma once
#include "TTY.h"

// Possible background and foreground colors in text-mode in text-mode
enum Color {
  BLACK = 0,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,	
  MAGENTA = 5,
  BROWN = 6,
  LIGHT_GREY = 7,
  DARK_GREY = 8,
  LIGHT_BLUE = 9,
  LIGHT_GREEN = 10,
  LIGHT_CYAN = 11,
  LIGHT_RED = 12,
  LIGHT_MAGENTA = 13,
  LIGHT_BROWN = 14,
  WHITE = 15,
};

class VirtualConsole final : public TTY {
  public:
    VirtualConsole(unsigned int index);
    void static initialize();

    void clear();

  protected: 
    virtual size_t onTTYWrite(const uint8_t *buffer, size_t size) override;

  private: 
    void onChar(char c);
    unsigned int _index;

    uint8_t _currentRow { 0 };
    uint8_t _currentColumn { 0 }; 

    uint8_t _currentBackgroundColor { Color::BLUE };
    uint8_t _currentForegroundColor { Color::WHITE };

    void putCharAt(uint8_t row, uint8_t column, char c);
    void horizontalScroll();

    void setBackgroundColor(Color color);
    void setForegroundColor(Color color);

};
