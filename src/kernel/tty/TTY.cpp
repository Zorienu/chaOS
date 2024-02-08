#include "TTY.h"
#include "../heap/kmalloc.h"

void TTY::setSize(uint8_t rows, uint8_t columns) {
  _rows = rows;
  _columns = columns;
}

bool TTY::canRead(FileDescription&) const {
  return true;
}

size_t TTY::read(FileDescription&, uint8_t *buffer, size_t size) {
  if (size > _input_buffer.size()) size = _input_buffer.size();

  for (int i = 0; i < size; i++) 
    buffer[i] = _input_buffer.dequeue();

  return size;
}

bool TTY::canWrite(FileDescription&) const {
  return false;
}

size_t TTY::write(FileDescription&, const uint8_t *buffer, size_t size) {
  return size;
}

void TTY::emit(uint8_t ch) {
  // TODO: handle events like Ctrl + W for erasing word
  _input_buffer.enqueue(ch);
}
