#ifndef CIRCUIT_TEST_UTIL_HPP
#define CIRCUIT_TEST_UTIL_HPP

template <typename ContextT, typename PTcontainer>
bool eval_encrypted_check_equal(ContextT ctx, Circuit circ,
				PTcontainer in, PTcontainer out_expected)
{
	PTcontainer out(eval_with_encrypted(ctx, circ, in));
	return std::equal(out.begin(), out.end(),
			  out_expected.begin(), out_expected.end());
}

#endif // CIRCUIT_TEST_UTIL_HPP
