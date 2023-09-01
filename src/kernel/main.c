#include <stdint.h>
#include "stdio.h"
#include "string.h"


void OSStart() {
  char string[] = "Lore ipsum";

  initVideo();
  putNumber(511, 16);
  puts("\n");
  putNumber(255, 2);
  puts("\n");
  putNumber(255, 16);
}
