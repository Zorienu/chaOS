#include "TTY.h"

void TTY::setSize(uint8_t rows, uint8_t columns) {
  _rows = rows;
  _columns = columns;
}

bool TTY::canRead(FileDescription&) const {
  return true;
}

size_t TTY::read(FileDescription&, uint8_t *buffer, size_t size) {
  return size;
}

bool TTY::canWrite(FileDescription&) const {
  return false;
}

size_t TTY::write(FileDescription&, const uint8_t *buffer, size_t size) {
  return size;
}
