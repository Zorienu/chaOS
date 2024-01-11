#include "VirtualConsole.h"
#include "../../include/c/string.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_BUFFER_ADDRESS 0xB8000

static uint8_t *vgaBuffer;
static VirtualConsole *consoles[6];


VirtualConsole::VirtualConsole() : TTY() {
  setSize(SCREEN_WIDTH,SCREEN_HEIGHT);
}

void VirtualConsole::initialize() {
  vgaBuffer = (uint8_t *)VGA_BUFFER_ADDRESS;
  memset(consoles, 0x0, sizeof(consoles));
}

size_t VirtualConsole::onTTYWrite(const uint8_t *buffer, size_t size) {
  for (size_t i = 0; i < size; i++) 
    onChar(buffer[i]);


  return size;
}



void VirtualConsole::onChar(char c) {
  // TODO: implement "printf"
  putCharAt(_currentRow, _currentColumn, c);
}

void VirtualConsole::putCharAt(uint8_t row, uint8_t column, char c) {
  int offset = ((SCREEN_WIDTH * row) + column);
  uint16_t *location = (uint16_t *)vgaBuffer + offset;

  // 15            12 11            8 7              0
  // -------------------------------------------------
  // |   Backcolor   |   Forecolor   |   Character   |
  // -------------------------------------------------
  *location = _currentBackgroundColor << 12 | _currentForegroundColor << 8 | c;
}

void VirtualConsole::horizontalScroll() {
  // We do not need to scroll yet
  if (_currentRow < SCREEN_HEIGHT) return;

  int bytesPerLine = SCREEN_WIDTH * 2;
  int screenBytes = bytesPerLine * SCREEN_HEIGHT;
  uint8_t *secondLine = vgaBuffer + bytesPerLine;

  memcpy(vgaBuffer, secondLine, screenBytes - bytesPerLine);

  // Clear the last line
  uint8_t *lastLine = vgaBuffer + (SCREEN_HEIGHT - 1) * bytesPerLine;
  memset(lastLine, 0x0, bytesPerLine);

  _currentRow = SCREEN_HEIGHT - 1;
}

void VirtualConsole::setBackgroundColor(enum Color color) {
  _currentBackgroundColor = color;
}

void VirtualConsole::setForegroundColor(enum Color color) {
  _currentForegroundColor = color;
}

void VirtualConsole::clear() {
  uint8_t *vga = vgaBuffer;

  memset(vgaBuffer, 0x0, SCREEN_WIDTH * SCREEN_HEIGHT);

  // Update virtual cursor
  _currentRow = 0;
  _currentColumn = 0;

  // Move hardware cursor
  // TODO: moveCsr();
}
