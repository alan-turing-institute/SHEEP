#ifndef CONTEXT_CLEAR_HPP
#define CONTEXT_CLEAR_HPP

#include "circuit.hpp"
#include "context.hpp"
#include "bits.hpp"
#include <type_traits>
#include <cmath>

namespace SHEEP {

template <typename Ciphertext>
struct CiphertextWrapper { typedef Ciphertext type; };

template <>
struct CiphertextWrapper<bool> { typedef int type; };


template<typename PlaintextT>
class ContextClear : public Context<PlaintextT, std::vector<PlaintextT>> {   //plaintext and ciphertext are the same type
public:

  typedef PlaintextT Plaintext;
  typedef std::vector<PlaintextT> Ciphertext;
	typedef PlaintextT CiphertextEl;

  /// constructor
  ContextClear() {
    m_nslots = 100; // reasonable default size
    this->m_public_key_size = 0;
    this->m_private_key_size = 0;
    this->m_ciphertext_size = 0;
    this->m_param_name_map.insert({"num_slots", m_nslots});
  }

	Ciphertext encrypt(std::vector<Plaintext> p) {
	  if (! this->m_configured) this->configure();
	  this->m_ciphertext_size = sizeof(p);
	  return p; // plaintext and ciphertext are the same for this context
	}

	std::vector<Plaintext> decrypt(Ciphertext c) {
		return c; // plaintext and ciphertext are the same for this context
	}

	std::pair<bool,bool> HalfAdder(bool a, bool b) {
		bool sum, carry;
		sum = (a != b);
		carry = (a && b);
		return std::make_pair(sum, carry);
	}

	std::pair<bool,bool> FullAdder(bool a, bool b, bool carry_in) {
		bool sum, carry_out, s1, c1, c2;
		std::tie(s1, c1) = HalfAdder(a, b);
		std::tie(sum, c2) = HalfAdder(s1, carry_in);
		carry_out = c1 || c2;
		return std::make_pair(sum, carry_out);
	}

	Ciphertext RippleCarryAdd(Ciphertext a, Ciphertext b) {
		Ciphertext result;
		CiphertextEl result_el;
		bool sum, carry;

		if (a.size() != b.size()) {
			throw std::runtime_error("Ciphertext a, Ciphertext b - lengths do not match.");
		}

		for (int j = 0; j < a.size(); j++) {

			std::tie(sum, carry) = HalfAdder(bit(0, a[j]), bit(0, b[j]));
			set_bit(0, result_el, sum);

			// Note that the loop starts at ONE, since we have
			// already computed the zeroth bit above
			for (size_t i = 1; i < BITWIDTH(Plaintext); ++i) {
				std::tie(sum, carry) = FullAdder(bit(i, a[j]), bit(i, b[j]), carry);
				set_bit(i, result_el, sum);
			}

			result.push_back(result_el);
		}

		return result;
	}

	// In Add, Multiply, Subtract and Negate, we assume that
	// Ciphertext is either unsigned or a two's complement integer
	// type.  With a signed type, to avoid undefined behaviour,
	// cast to the corresponding unsigned type, perform the
	// operation, then cast back.  To do this with the .

	// Work in the corresponding unsigned type and cast
	// back, so overflow is well-defined.

	Ciphertext Add(Ciphertext a, Ciphertext b) {

		Ciphertext c;

		if (a.size() != b.size()) {
			throw std::runtime_error("Ciphertext a, Ciphertext b - lengths do not match.");
		}

	  if (std::is_same<Ciphertext, std::vector<bool>>::value) {
	    for (int i = 0; i < a.size(); i++) {
				c.push_back(a[i] != b[i]);
			}

	    return c;

	  } else {
			for (int i = 0; i < a.size(); i++) {
				typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextEl>::type >::type uC;

				uC au = static_cast<uC>(a[i]);
				uC bu = static_cast<uC>(b[i]);

				c.push_back(static_cast<CiphertextEl>(au + bu));
			}

	    return c;
	  }
	}

	Ciphertext Multiply(Ciphertext a, Ciphertext b) {
		Ciphertext c;

		if (a.size() != b.size()) {
			throw std::runtime_error("Ciphertext a, Ciphertext b - lengths do not match.");
		}

	  if (std::is_same<Ciphertext, std::vector<bool>>::value) {
			for (int i = 0; i < a.size(); i++) {
				c.push_back(a[i] & b[i]);
			}

	    return c;

		} else {
			for (int i = 0; i < a.size(); i++) {
				typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextEl>::type >::type uC;

				uC au = static_cast<uC>(a[i]);
				uC bu = static_cast<uC>(b[i]);
				c.push_back(static_cast<CiphertextEl>(au * bu));
			}

	    return c;
	  }
	}

	std::pair<bool, bool> MultiplyBit(bool x, bool y, bool sum_in, bool carry_in) {
		bool sum_out, carry_out, product_bit;
		product_bit = x & y;
		return FullAdder(sum_in, product_bit, carry_in);
	}

	Ciphertext Subtract(Ciphertext a, Ciphertext b) {
		Ciphertext c;

		if (a.size() != b.size()) {
			throw std::runtime_error("Ciphertext a, Ciphertext b - lengths do not match.");
		}

		if (std::is_same<Ciphertext, std::vector<bool>>::value) {
			for (int i = 0; i < a.size(); i++) {
				c.push_back(a[i] != b[i]);
			}

	    return c;

		} else {
			for (int i = 0; i < a.size(); i++) {
				typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextEl>::type >::type uC;

				uC au = static_cast<uC>(a[i]);
				uC bu = static_cast<uC>(b[i]);

				c.push_back(static_cast<CiphertextEl>(au - bu));
			}

	    return c;
	  }
	}

	Ciphertext Maximum(Ciphertext a, Ciphertext b) {

		Ciphertext c;

		if (a.size() != b.size()) {
			throw std::runtime_error("Ciphertext a, Ciphertext b - lengths do not match.");
		}

		for (int i = 0; i < a.size(); i++) {
			c.push_back((a[i]>=b[i])?a[i]:b[i]);
		}

		return c;
	}

	Ciphertext Not(Ciphertext a) {
		Ciphertext c;

		for (int i = 0; i < a.size(); i++) {
				c.push_back(!a[i]);
		}

		return c;
	}

	Ciphertext Negate(Ciphertext a) {

	  if (std::is_same<Ciphertext, std::vector<bool>>::value) {
	    return Not(a);

	  } else {
			Ciphertext c;

			for (int i = 0; i < a.size(); i++) {
				typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextEl>::type >::type uC;

				uC au = static_cast<uC>(a[i]);

				c.push_back(static_cast<CiphertextEl>(-au));
			}

	    return c;
	  }
	}

	Ciphertext Compare(Ciphertext a, Ciphertext b) {

		Ciphertext c;

		if (a.size() != b.size()) {
			throw std::runtime_error("Ciphertext a, Ciphertext b - lengths do not match.");
		}

		for (int i = 0; i < a.size(); i++) {
			c.push_back(a[i] > b[i]);
		}

		return c;
	}

	Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {

		Ciphertext c;

		if ((s.size() != a.size()) || (s.size() != b.size())) {
			throw std::runtime_error("Ciphertext s, Ciphertext a, Ciphertext b - lengths do not match.");
		}

		for (int i = 0; i < a.size(); i++) {
			c.push_back((s[i] % 2)?a[i]:b[i]);
		}

		return c;
	}

	Ciphertext MultByConstant(Ciphertext a, long b) {

		Ciphertext c;
		for (int i = 0; i < a.size(); i++) {
			c.push_back(b);
		}

		return Multiply(a, c);
	}

	Ciphertext AddConstant(Ciphertext a, long b) {

		Ciphertext c;
		for (int i = 0; i < a.size(); i++) {
			c.push_back(b);
		}

		return Add(a, c);
	}

  virtual long get_num_slots() {
    return m_nslots;
  }


protected:
  long m_nslots;

};

}  // Leaving Sheep namespace

#endif // CONTEXT_CLEAR_HPP
