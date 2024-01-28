#include "KeyboardDevice.h"
#include "../../include/io/io.h"
#include "../../include/c/stdio.h"
#include "../../include/interrupts/pic.h"
#include "../tty/VirtualConsole.h"
#include "../utils/kprintf.h"

#define IRQ_KEYBOARD 1

void KeyboardDevice::handleIRQ () {
  uint8_t key = IO::inb(0x60);

  // Scancode set 1 -> Ascii lookup table
  const char scancode_to_ascii[] = "\x00\x1B" "1234567890-=" "\x08"
  "\x00" "qwertyuiop[]" "\x0D\x1D" "asdfghjkl;'`" "\x00" "\\"
  "zxcvbnm,./" "\x00\x00\x00" " ";

  // printf("\nkey: %d %c", key, scancode_to_ascii[key]);

  if (scancode_to_ascii[key] == '1') VirtualConsole::switchTo(0);
  else if (scancode_to_ascii[key] == '2') VirtualConsole::switchTo(1);
  else if (scancode_to_ascii[key] == '3') cls();

  if (key == 28) kprintf("\n");
  else if (key < 100) kprintf("%c", scancode_to_ascii[key]);
}

KeyboardDevice::KeyboardDevice() : IRQHandler(IRQ_KEYBOARD), CharacterDevice() {
  enableIRQ();
}
