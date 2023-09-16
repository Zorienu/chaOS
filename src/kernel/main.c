#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "mem.h"
#include "mmu.h"
#include "mem.h"

void OSStart() {
  initVideo();
  kmalloc(1);
  // for (int i = 0; i < 100; i ++) {
  //   setBackgroundColor(i % 16);
  //   putNumber(i, 10);
  //   puts("\n");
  // }
}
