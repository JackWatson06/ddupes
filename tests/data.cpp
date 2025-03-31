
#include "../src/lib.h"

hash uniqueTestHash(uint8_t const first_byte = 255) {
  return new uint8_t[MD5_DIGEST_LENGTH]{first_byte, 255, 255, 255, 255, 255,
                                        255,        255, 255, 255, 255, 255,
                                        255,        255, 255, 255};
}