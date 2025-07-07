#include <stdbool.h>
#include <stddef.h>
#include "pico/rand.h"


bool mg_random(void *buf, size_t len) {
  bool success = false;
  unsigned char *p = (unsigned char *) buf;
  while (len--) *p++ = (unsigned char) (get_rand_32() & 255);
  success = true;

  return success;
}