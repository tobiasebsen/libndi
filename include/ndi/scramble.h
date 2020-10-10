#ifndef NDI_SCRAMBLE_H
#define NDI_SCRAMBLE_H

#include <stdint.h>

void ndi_scramble_type1(uint8_t *buf, int len, uint32_t seed);
void ndi_unscramble_type1(uint8_t *buf, int len, uint32_t seed);
void ndi_unscramble_type2(uint8_t *buf, int len, uint32_t seed);

#endif // NDI_SCRAMBLE_H