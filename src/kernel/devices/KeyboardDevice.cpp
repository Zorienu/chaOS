#include "KeyboardDevice.h"
#include "../../include/io/io.h"
#include "../../include/c/stdio.h"
#include "../../include/interrupts/pic.h"
#include "../tty/VirtualConsole.h"
#include "../utils/kprintf.h"
#include "../KeyCode.h"

#define IRQ_KEYBOARD 1

/*
 * Scan code from which keys are no longer pressed but released
 */
#define BREAK_CODE_OFFSET 0x80

// We can only have one instance of KeyboardDevice
static KeyboardDevice *s_the;

/*
 * Scan code set obtained from http://www.brokenthorn.com/Resources/OSDevScanCodes.html
 * Original XT Scan Code Set (Make codes for each key)
 */
static KeyCode scancodeToKeyCode[BREAK_CODE_OFFSET] = {
  Key_Invalid, // 0x0
  Key_Escape, // 0x1
  Key_1, // 0x2
  Key_2, // 0x3
  Key_3, // 0x4
  Key_4, // 0x5
  Key_5, // 0x6
  Key_6, // 0x7
  Key_7, // 0x8
  Key_8, // 0x9
  Key_9, // 0xA
  Key_0, // 0xB
  Key_Minus, // 0xC
  Key_Equal, // 0xD
  Key_Backspace, // 0xE
  Key_Tab, // 0xF
  Key_Q, // 0x10
  Key_W, // 0x11
  Key_E, // 0x12
  Key_R, // 0x13
  Key_T, // 0x14
  Key_Y, // 0x15
  Key_U, // 0x16
  Key_I, // 0x17
  Key_O, // 0x18
  Key_P, // 0x19
  Key_LeftBracket, // 0x1A
  Key_RightBracket, // 0x1B
  Key_Return,  // 0x1C
  Key_Control, // 0x1D
  Key_A, // 0x1E
  Key_S, // 0x1F
  Key_D, // 0x20
  Key_F, // 0x21
  Key_G, // 0x22
  Key_H, // 0x23
  Key_J, // 0x24
  Key_K, // 0x25
  Key_L, // 0x26
  Key_Semicolon, // 0x27
  Key_Apostrophe, // 0x28
  Key_Backtick, // 0x29
  Key_LeftShift, // 0x2A
  Key_Backslash, // 0x2B
  Key_Z, // 0x2C
  Key_X, // 0x2D
  Key_C, // 0x2E
  Key_V, // 0x2F
  Key_B, // 0x30
  Key_N, // 0x31
  Key_M, // 0x32
  Key_Comma, // 0x33
  Key_Period, // 0x34
  Key_Slash, // 0x35
  Key_RightShift, // 0x36
  Key_Asterisk, // 0x37
  Key_Alt,     // 0x38
  Key_Space,   // 0x39
  Key_CapsLock, // 0x3A
  Key_F1, // 0x3B
  Key_F2, // 0x3C
  Key_F3, // 0x3D
  Key_F4, // 0x3E
  Key_F5, // 0x3F
  Key_F6, // 0x40
  Key_F7, // 0x41
  Key_F8, // 0x42
  Key_F9, // 0x43
  Key_F10, // 0x44
  Key_NumLock, // 0x45
  Key_Invalid, // 0x46
  Key_Home, // 0x47
  Key_Up, // 0x48
  Key_PageUp, // 0x49
  Key_Minus, // 0x4A
  Key_Left, // 0x4B
  Key_Invalid, // 0x4C
  Key_Right, // 0x4D
  Key_Plus, // 0x4E
  Key_End, // 0x4F
  Key_Down, // 0x50
  Key_PageDown, // 0x51
  Key_Invalid, // 0x52
  Key_Delete, // 0x53
  Key_Invalid, // 0x54
  Key_Invalid, // 0x55
  Key_Backslash, // 0x56
  Key_F11, // 0x57
  Key_F12, // 0x58
  Key_Invalid, // 0x59
  Key_Invalid, // 0x5A
  Key_GUI, // 0x5B 
};

/*
 * The key code that will be obtained if the "Shift" key is pressed
 */
static KeyCode scancodeToKeyCodeShifted[BREAK_CODE_OFFSET] = {
  Key_Invalid, // 0x0
  Key_Escape, // 0x1
  Key_ExclamationPoint, // 0x2
  Key_AtSign, // 0x3
  Key_Hashtag, // 0x4
  Key_Dollar, // 0x5
  Key_Percent, // 0x6
  Key_Ampersand, // 0x7
  Key_Apostrophe, // 0x8
  Key_LeftParenthesis, // 0x9
  Key_RightParenthesis, // 0xA
  Key_Equal, // 0xB
  Key_Underscore, // 0xC
  Key_Equal, // 0xD
  Key_Backspace, // 0xE
  Key_Tab, // 0xF
  Key_Q, // 0x10
  Key_W, // 0x11
  Key_E, // 0x12
  Key_R, // 0x13
  Key_T, // 0x14
  Key_Y, // 0x15
  Key_U, // 0x16
  Key_I, // 0x17
  Key_O, // 0x18
  Key_P, // 0x19
  Key_LeftBrace, // 0x1A
  Key_RightBrace, // 0x1B
  Key_Return, // 0x1C
  Key_Control, // 0x1D
  Key_A, // 0x1E
  Key_S, // 0x1F
  Key_D, // 0x20
  Key_F, // 0x21
  Key_G, // 0x22
  Key_H, // 0x23
  Key_J, // 0x24
  Key_K, // 0x25
  Key_L, // 0x26
  Key_Colon, // 0x27
  Key_DoubleQuote, // 0x28
  Key_Tilde, // 0x29
  Key_LeftShift, // 0x2A
  Key_Pipe, // 0x2B
  Key_Z, // 0x2C
  Key_X, // 0x2D
  Key_C, // 0x2E
  Key_V, // 0x2F
  Key_B, // 0x30
  Key_N, // 0x31
  Key_M, // 0x32
  Key_LessThan, // 0x33
  Key_GreaterThan, // 0x34
  Key_Slash, // 0x35
  Key_QuestionMark, // 0x36
  Key_Asterisk, // 0x37
  Key_Alt,     // 0x38
  Key_Space,   // 0x39
  Key_CapsLock, // 0x3A
  Key_F1, // 0x3B
  Key_F2, // 0x3C
  Key_F3, // 0x3D
  Key_F4, // 0x3E
  Key_F5, // 0x3F
  Key_F6, // 0x40
  Key_F7, // 0x41
  Key_F8, // 0x42
  Key_F9, // 0x43
  Key_F10, // 0x44
  Key_NumLock, // 0x45
  Key_Invalid, // 0x46
  Key_Home, // 0x47
  Key_Up, // 0x48
  Key_PageUp, // 0x49
  Key_Minus, // 0x4A
  Key_Left, // 0x4B
  Key_Invalid, // 0x4C
  Key_Right, // 0x4D
  Key_Plus, // 0x4E
  Key_End, // 0x4F
  Key_Down, // 0x50
  Key_PageDown, // 0x51
  Key_Invalid, // 0x52
  Key_Delete, // 0x53
  Key_Invalid, // 0x54
  Key_Invalid, // 0x55
  Key_Backslash, // 0x56
  Key_F11, // 0x57
  Key_F12, // 0x58
  Key_Invalid, // 0x59
  Key_Invalid, // 0x5A
  Key_GUI, // 0x5B 
};

/*
 * Make codes: sent when a key is pressed or held down
 * Break codes: sent when a key is released
 * There is a unique make code and break code for each key on the keyboard
 */
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

  // if (key == 28) kprintf("\n");
  // else if (key < 100) kprintf("%c", scancode_to_ascii[key]);

  if (_client && key<100) _client->onKeyPressed(scancode_to_ascii[key]);
}

KeyboardDevice::KeyboardDevice() : IRQHandler(IRQ_KEYBOARD), CharacterDevice() {
  s_the = this;
  enableIRQ();
}

KeyboardDevice& KeyboardDevice::the() {
  return *s_the;
}

void KeyboardDevice::updateModifier(KeyModifiers modifier, bool pressed) {
  if (pressed) _modifiers |= modifier;
  else _modifiers &= ~modifier;
}
