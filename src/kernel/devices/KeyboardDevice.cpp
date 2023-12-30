#include "KeyboardDevice.h"
#include "../../include/io/io.h"
#include "../../include/c/stdio.h"
#include "../../include/interrupts/pic.h"

#define IRQ_KEYBOARD 1

void KeyboardDevice::handleIRQ () {
  uint8_t key = IO::inb(0x60);

  // Scancode set 1 -> Ascii lookup table
  const char scancode_to_ascii[] = "\x00\x1B" "1234567890-=" "\x08"
  "\x00" "qwertyuiop[]" "\x0D\x1D" "asdfghjkl;'`" "\x00" "\\"
  "zxcvbnm,./" "\x00\x00\x00" " ";

  // printf("\nkey: %d %c", key, scancode_to_ascii[key]);

  if (scancode_to_ascii[key] == '1') cls();

  if (key == 28) printf("\n");
  else if (key < 100) printf("%c", scancode_to_ascii[key]);
}

KeyboardDevice::KeyboardDevice() : IRQHandler(IRQ_KEYBOARD), CharacterDevice() {
  enableIRQ();
}
