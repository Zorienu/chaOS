#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_BUFFER_ADDRESS 0xB8000

// Possible background and foreground colors
enum colors {
  BLACK = 0,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,	
  MAGENTA = 5,
  BROWN = 6,
  LIGHT_GREY = 7,
  DARK_GREY = 8,
  LIGHT_BLUE = 9,
  LIGHT_GREEN = 10,
  LIGHT_CYAN = 11,
  LIGHT_RED = 12,
  LIGHT_MAGENTA = 13,
  LIGHT_BROWN,
  WHITE,
};

/*
 * Print the given string to the screen
 */
void puts (char *string);

/*
 * Print the given number in the specified base to the screen
 */
void putNumber (int number, int base);

/*
 * Clear the whole screen
 */
void cls (void);

/*
 * Set the text-mode VGA pointer, then clears the screen
 */
void initVideo (void);
