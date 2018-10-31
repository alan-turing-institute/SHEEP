#include <iostream>
#include <cassert>
#include <cstdint>
#include "bits.hpp"

int main(void) {
	uint8_t a(0);
	int8_t b(0);

	for (int i=0; i<8; i++) {
		assert(bit(i, a) == 0);
		assert(bit(i, b) == 0);
	}

	set_bit(7, a, 1);
	assert(bit(7, a) == 1);
	assert(a == 128);
	
	// int8_t, if it is supported, is required to represent
	// negative numbers using two's complement
	set_bit(7, b, 1);
	assert(b == -128);

	for (int i=0; i<8; i++) {
		set_bit(i, a, 1);
		set_bit(i, b, 1);
	}

	assert(a == 255);
	assert(b == -1);

	set_bit(7, b, 0);
	assert(b == 127);
}
