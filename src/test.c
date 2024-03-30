// print the size of uint32_t* and uint8_t* in bits
#include <stdio.h>
#include <stdint.h>
int main() {
  printf("Size of uint32_t* in bits: %lu\n", sizeof(uint32_t*)*8);
  printf("Size of uint8_t* in bits: %lu\n", sizeof(uint8_t*)*8);
  return 0;
}