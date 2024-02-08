#pragma once
#include "stdint.h"
#include <stddef.h>
#include "../devices/CharacterDevice.h"
#include "../utils/datastructures/CircularQueue.h"

class TTY : public CharacterDevice {
  public:
    virtual bool canRead (FileDescription&) const override;
    virtual bool canWrite (FileDescription&) const override;

    // TODO: use ssize_t instead
    virtual size_t read(FileDescription&, uint8_t*, size_t) override;
    virtual size_t write(FileDescription&, const uint8_t*, size_t) override;

    virtual bool isTTY() const final override { return true; } 

    void setSize(uint8_t rows, uint8_t columns);

    uint8_t rows() { return _rows; }
    uint8_t columns() { return _columns; }

  protected:
    // TODO: use ssize_t instead
    // Will be implemented by VirtualConsole
    virtual size_t onTTYWrite(const uint8_t *buffer, size_t size) = 0;

    void emit(uint8_t ch);

  private:
    CircularQueue<uint8_t, 1024> _input_buffer;
    uint8_t _rows;
    uint8_t _columns;
};
