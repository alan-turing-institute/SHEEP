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
class ContextClear : public Context<PlaintextT, PlaintextT> {   //plaintext and ciphertext are the same type
public:
        typedef PlaintextT Plaintext;
        typedef PlaintextT Ciphertext;  

  /// constructor

        ContextClear() {
	  this->m_public_key_size = 0;
	  this->m_private_key_size = 0;
	  this->m_ciphertext_size = 0;
	  
        }
	Ciphertext encrypt(Plaintext p) {
	  if (! this->m_configured) this->configure();
	  this->m_ciphertext_size = sizeof(p);
	  return p; // plaintext and ciphertext are the same for this context
	}

	Plaintext decrypt(Ciphertext c) {
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
		bool sum, carry;
		std::tie(sum, carry) = HalfAdder(bit(0,a), bit(0,b));
		set_bit(0, result, sum);
		// Note that the loop starts at ONE, since we have
		// already computed the zeroth bit above
		for (size_t i = 1; i < BITWIDTH(Plaintext); ++i) {
			std::tie(sum, carry) = FullAdder(bit(i,a), bit(i,b), carry);
			set_bit(i, result, sum);
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
	  if (std::is_same<Ciphertext, bool>::value) {
	    return a != b;
	  } else {
	    typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
	    
	    uC au = static_cast<uC>(a);
	    uC bu = static_cast<uC>(b);
	    return static_cast<Ciphertext>(au + bu);
	  }
	}

	Ciphertext Multiply(Ciphertext a, Ciphertext b) {
	  if (std::is_same<Ciphertext, bool>::value) {
	    return a & b;
	  } else {
	    typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
	    uC au = static_cast<uC>(a);
	    uC bu = static_cast<uC>(b);
	    return static_cast<Ciphertext>(au * bu);
	  }
	}

	std::pair<bool, bool> MultiplyBit(bool x, bool y, bool sum_in, bool carry_in)
	{
		bool sum_out, carry_out, product_bit;
		product_bit = x & y;
		return FullAdder(sum_in, product_bit, carry_in);
	}
  
  
	Ciphertext Subtract(Ciphertext a, Ciphertext b) {
	  if (std::is_same<Ciphertext, bool>::value) {
	    return Add(a,b);
	  } else {
	    typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
	    
	    uC au = static_cast<uC>(a);
	    uC bu = static_cast<uC>(b);
	    return static_cast<Ciphertext>(au - bu);
	  }
	}

	Ciphertext Maximum(Ciphertext a, Ciphertext b) {
		return (a>=b)?a:b;
	}
  
	Ciphertext Not(Ciphertext a) {
		return !a;
	}
  
	Ciphertext Negate(Ciphertext a) {

	  if (std::is_same<Ciphertext, bool>::value) {
	    return Not(a);
	  } else {
	    typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
	    uC au = static_cast<uC>(a);
	    return static_cast<Ciphertext>(-au);
	  }
	  
	}
  
  
	Ciphertext Compare(Ciphertext a, Ciphertext b) {
		return (a > b);
	}

	Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
		return (s % 2)?a:b;
	}
};


}  // Leaving Sheep namespace
  
#endif // CONTEXT_CLEAR_HPP
