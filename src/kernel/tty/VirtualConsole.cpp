#include "VirtualConsole.h"

VirtualConsole::VirtualConsole() : TTY() {
  setSize(80, 25);
}

size_t VirtualConsole::onTTYWrite(const uint8_t *buffer, size_t size) {

  return 0;
}

void VirtualConsole::onChar(char c) {
  // TODO: implement "printf"
}
