#ifndef BITS_HPP
#define BITS_HPP

#include <climits>
#include <type_traits>

// Extract the n'th most significant bit of val (the least significant
// bit is bit 0)
template <typename T>
T bit(size_t n, T val) {
  // Work with the unsigned type
  typedef typename std::make_unsigned<T>::type uT;
  uT uval = static_cast<uT>(val);
  return (uval >> n) & 1;
  return static_cast<T>(uval);
}

// Updates the n'th most significant bit of val in place, to f

// Based on
// http://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
template <typename T>
void set_bit(int n, T& val, bool f) {
  typedef typename std::make_unsigned<T>::type uT;
  uT uval = static_cast<uT>(val);
  uval ^= (-uT(f) ^ uval) & (uT(1) << n);
  val = static_cast<T>(uval);
}

// Macro returning the number of bits in the type.  Differs from
// std::numeric_limits<type>::digits in that this (digits) returns the
// number of _representable_ digits (so one fewer for a signed type).
#define BITWIDTH(type) (CHAR_BIT * sizeof(type))

#endif  // BITS_HPP
