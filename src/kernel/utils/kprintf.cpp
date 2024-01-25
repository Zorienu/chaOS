#include "../tty/VirtualConsole.h"

extern "C" void x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t *quotientOut, uint32_t *remainderOut);

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

void puts (char *string, VirtualConsole& console) {
  while (*string) console.onChar(*string++);
}

int *printfNumber(int *argp, int length, bool sign, int radix, VirtualConsole& console);

void kprintf(char *format, ...) {
  int *argp = (int *)&format;
  argp++;
  
  int state = PRINTF_STATE_NORMAL;
  int length = PRINTF_LENGTH_DEFAULT;
  int radix = 10;
  bool sign = false;

  VirtualConsole *console = VirtualConsole::getCurrentConsole();

  while (*format) {
    switch (state) {
      case PRINTF_STATE_NORMAL:
        switch (*format) {
          case '%': state = PRINTF_STATE_LENGTH;
                    break;
          default: console->onChar(*format);
                   break;
        }
        break;

      case PRINTF_STATE_LENGTH:
        switch (*format) {
          case 'h': length = PRINTF_LENGTH_SHORT;
                    state = PRINTF_STATE_LENGTH_SHORT;
                    break;
          case 'l': length = PRINTF_LENGTH_LONG;
                    state = PRINTF_STATE_LENGTH_LONG;
                    break;
          default:  goto PRINTF_STATE_SPEC_;
        }
        break;

      case PRINTF_STATE_LENGTH_SHORT:
        if (*format == 'h') {
          length = PRINTF_LENGTH_SHORT_SHORT;
          state = PRINTF_STATE_SPEC;
        }
        else goto PRINTF_STATE_SPEC_;
        break;

      case PRINTF_STATE_LENGTH_LONG:
        if (*format == 'l') {
          length = PRINTF_LENGTH_LONG_LONG;
          state = PRINTF_STATE_SPEC;
        }
        else goto PRINTF_STATE_SPEC_;
        break;

      case PRINTF_STATE_SPEC:
      PRINTF_STATE_SPEC_:
        switch (*format) {
          // Specifier characters
          case 'c': console->onChar((char)*argp);
                    argp++;
                    break;

          case 's': puts(*(char **)argp, *console);
                    argp++;
                    break;

          case '%': console->onChar('%');
                    break;

          // Signed decimal number
          case 'd': 
          case 'i': radix = 10; sign = true;
                    argp = printfNumber(argp, length, sign, radix, *console);
                    break;

          // Unsigned decimal number
          case 'u': radix = 10; sign = false;
                    argp = printfNumber(argp, length, sign, radix, *console);
                    break;

          // Unsigned hexadecimal number
          case 'X':  
          case 'x':  
          case 'p': radix = 16; sign = false;
                    argp = printfNumber(argp, length, sign, radix, *console);
                    break;
          
          // Unsigned octal number
          case 'o': radix = 8; sign = false;
                    argp = printfNumber(argp, length, sign, radix, *console);
                    break;

          case 'b': radix = 2; sign = false;
                    argp = printfNumber(argp, length, sign, radix, *console);
                    break;

          // Ignore invalid characters
          default:  break;
        }

        // Return to normal state
        state = PRINTF_STATE_NORMAL; 
        length = PRINTF_LENGTH_DEFAULT;
        radix = 10;
        sign = false;
    }

    format++;
  }
}

int *printfNumber(int *argp, int length, bool sign, int radix, VirtualConsole& console) {
  char *g_HexChars = "0123456789ABCDEF";
  char buffer[32] = { 0 };
  unsigned long long int number = 0;
  int number_sign = 1;

  // Process length
  switch (length) {
    case PRINTF_LENGTH_SHORT_SHORT:
    case PRINTF_LENGTH_SHORT:
    case PRINTF_LENGTH_DEFAULT:
    case PRINTF_LENGTH_LONG:
      if (sign) {
        int n = *argp;
        if (n < 0) {
          n = -n;
          number_sign = -1;
        }
        number = (unsigned long long)n;
      }
      else {
        number = *(unsigned int*)argp;
      }

      argp++;
      break;

    case PRINTF_LENGTH_LONG_LONG:
      if (sign) {
        long long int n = *(long long int *)argp;
        if (n < 0) {
          n = -n;
          number_sign = -1;
        }
        number = (unsigned long long)n;
      }
      else {
        number = *(unsigned long long int*)argp;
      }

      argp += 2;
      break;

  }

  // Convert number to ASCII
  int pos = 0;
  do {
    uint32_t remainder;
    x86_div64_32(number, radix, &number, &remainder);
    buffer[pos++] = g_HexChars[remainder];
  } while (number > 0);

  if (radix == 16) {
    buffer[pos++] = 'x';
    buffer[pos++] = '0';
  }
  else if (radix == 2) {
    buffer[pos++] = 'B';
    buffer[pos++] = '0';
  }

  // Add sign
  if (sign && number_sign < 0) {
    buffer[pos++] = '-';
  }


  // Print number in reverse order
  while (--pos >= 0) {
    console.onChar(buffer[pos]);
  }

  return argp;
}
