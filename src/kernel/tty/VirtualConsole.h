#pragma once
#include "TTY.h"

class VirtualConsole final : public TTY {
  public:
    VirtualConsole();

  protected: 
    virtual size_t onTTYWrite(const uint8_t *buffer, size_t size) override;

  private: 
    void onChar(char c);
};
