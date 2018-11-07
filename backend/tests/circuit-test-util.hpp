#ifndef CIRCUIT_TEST_UTIL_HPP
#define CIRCUIT_TEST_UTIL_HPP

#include "all_equal.hpp"

template <typename ContextT, typename PTcontainer>
bool eval_encrypted_check_equal(ContextT ctx, Circuit circ, PTcontainer in,
                                PTcontainer out_expected) {
  PTcontainer out(ctx.eval_with_plaintexts(circ, in));

  return all_equal(out, out_expected);
}

#endif  // CIRCUIT_TEST_UTIL_HPP
