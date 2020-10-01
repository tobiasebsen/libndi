#include <string.h>

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef long long int64_t;

static uint8_t internal_xortab[] = {
  0x4e, 0x44, 0x49, 0xae, 0x2c, 0x20, 0xa9, 0x32, 0x30, 0x31, 0x37, 0x20,
  0x4e, 0x65, 0x77, 0x54, 0x65, 0x6b, 0x2c, 0x20, 0x50, 0x72, 0x6f, 0x70,
  0x72, 0x69, 0x65, 0x74, 0x79, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x43, 0x6f,
  0x6e, 0x66, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x61, 0x6c, 0x2e, 0x20,
  0x59, 0x6f, 0x75, 0x20, 0x61, 0x72, 0x65, 0x20, 0x69, 0x6e, 0x20, 0x76,
  0x69, 0x6f, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x6f, 0x66, 0x20,
  0x74, 0x68, 0x65, 0x20, 0x4e, 0x44, 0x49, 0xae, 0x20, 0x53, 0x44, 0x4b,
  0x20, 0x6c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x20, 0x61, 0x74, 0x20,
  0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x6e, 0x65, 0x77, 0x2e, 0x74,
  0x6b, 0x2f, 0x6e, 0x64, 0x69, 0x73, 0x64, 0x6b, 0x5f, 0x6c, 0x69, 0x63,
  0x65, 0x6e, 0x73, 0x65, 0x2f, 0x00, 0x00, 0x00
};

void internal_scramble_type1(uint8_t *buf, int len, uint32_t seed) {
	uint64_t seed64 = ((uint64_t)seed << 32) | seed;
	uint64_t seed1 = seed64 ^ 0xb711674bd24f4b24ULL;
	uint64_t seed2 = seed64 ^ 0xb080d84f1fe3bf44ULL;

	if (len > 7) {
		uint64_t *buf64 = (uint64_t*)buf;
		int qwords = len / 8;
		uint64_t tmp1 = seed1;
		for (int i = 0; i < qwords; i++) {
			seed1 = seed2;
			tmp1 ^= (tmp1 << 23);
			tmp1 = ((seed1 >> 9 ^ tmp1) >> 17) ^ tmp1 ^ seed1;
			seed2 = tmp1 ^ buf64[i];
			buf64[i] ^= tmp1 + seed1;
			tmp1 = seed1;
		}

		buf = buf + qwords * 8;
		len -= qwords * 8;
	}

	if (len) {
		uint64_t remainder = 0;
		memcpy(&remainder, buf, len);
		seed1 ^= seed1 << 23;
		seed1 = ((seed2 >> 9 ^ seed1) >> 17) ^ seed1 ^ seed2;
		remainder ^= seed1 + seed2;
		memcpy(buf, &remainder, len);
	}
}

void internal_unscramble_type1(uint8_t *buf, int len, uint32_t seed) {
	uint64_t seed64 = ((uint64_t)seed << 32) | seed;
	uint64_t seed1 = seed64 ^ 0xb711674bd24f4b24ULL;
	uint64_t seed2 = seed64 ^ 0xb080d84f1fe3bf44ULL;

	if (len > 7) {
		uint64_t *buf64 = (uint64_t*)buf;
		int qwords = len / 8;
		uint64_t tmp1 = seed1;
		for (int i = 0; i < qwords; i++) {
			seed1 = seed2;
			tmp1 ^= (tmp1 << 23);
			tmp1 = ((seed1 >> 9 ^ tmp1) >> 17) ^ tmp1 ^ seed1;
			buf64[i] ^= tmp1 + seed1;
			seed2 = tmp1 ^ buf64[i];
			tmp1 = seed1;
		}

		buf = buf + qwords * 8;
		len -= qwords * 8;
	}

	if (len) {
		uint64_t remainder = 0;
		memcpy(&remainder, buf, len);
		seed1 ^= seed1 << 23;
		seed1 = ((seed2 >> 9 ^ seed1) >> 17) ^ seed1 ^ seed2;
		remainder ^= seed1 + seed2;
		memcpy(buf, &remainder, len);
	}
}

void internal_scramble_type2(uint8_t *buf, int len, uint32_t seed) {

	int xor_len = 128;

	if (len < xor_len)
		xor_len = len;

	for (int i = 0; i < xor_len; i++)
		buf[i] ^= internal_xortab[i];

	if (len >= 8) {
		uint64_t *buf64 = (uint64_t*)buf;
		int len8 = len >> 3;
		int64_t tmp;
		for (int i = 0; i < len8; i++) {
			tmp = seed;
			seed = buf64[i] & 0xffffffff;
			buf64[i] = ((tmp * len * -0x61c8864680b583ebLL + 0xc42bd7dee6270f1bLL) ^ buf64[i]) * -0xe217c1e66c88cc3LL + 0x2daa8c593b1b4591LL;
		}
	}
}

void internal_unscramble_type2(uint8_t *buf, int len, uint32_t seed) {
	int xor_len = 128;

	if (len >= 8) {
		uint64_t *buf64 = (uint64_t*)buf;
		int len8 = len >> 3;
		int64_t tmp;
		for (int i = 0; i < len8; i++) {
			tmp = seed;
			seed = buf64[i] & 0xffffffff;
			buf64[i] = ((tmp * len * -0x61c8864680b583ebLL + 0xc42bd7dee6270f1bLL) ^ buf64[i]) * -0xe217c1e66c88cc3LL + 0x2daa8c593b1b4591LL;
		}
	}

	if (len < xor_len)
		xor_len = len;

	for (int i = 0; i < xor_len; i++)
		buf[i] ^= internal_xortab[i];
}
