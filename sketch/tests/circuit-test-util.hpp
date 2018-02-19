#ifndef CIRCUIT_TEST_UTIL_HPP
#define CIRCUIT_TEST_UTIL_HPP

// Should be equivalent to the C++17 statement:
// return std::equal(out.begin(), out.end(),
// out_expected.begin(), out_expected.end());
template <typename T>
bool all_equal(T a, T b)
{
	auto it_a = a.begin();
	auto it_b = b.begin();
	for ( ; it_a != a.end() || it_b != b.end();
	      ++it_a, ++it_b)
	{
		if (*it_a != *it_b) return false;
	}

	return (it_a == a.end() && it_b == b.end());
}

template <typename ContextT, typename PTcontainer>
bool eval_encrypted_check_equal(ContextT ctx, Circuit circ,
				PTcontainer in, PTcontainer out_expected)
{
	PTcontainer out(eval_with_encrypted(ctx, circ, in));
	return all_equal(out, out_expected);
}

#endif // CIRCUIT_TEST_UTIL_HPP
