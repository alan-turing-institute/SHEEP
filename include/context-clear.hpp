#ifndef CONTEXT_CLEAR_HPP
#define CONTEXT_CLEAR_HPP

#include "circuit.hpp"
#include "context.hpp"
#include "bits.hpp"
#include <type_traits>
#include <cmath>

namespace Sheep {
namespace Clear {

template <typename Ciphertext>
struct CiphertextWrapper { typedef Ciphertext type; };
  
template <>
struct CiphertextWrapper<bool> { typedef int type; };
  

template<typename PlaintextT>
class ContextClear : public Context<PlaintextT, PlaintextT> {   //plaintext and ciphertext are the same type
public:
        typedef PlaintextT Plaintext;
        typedef PlaintextT Ciphertext;  

  
	Ciphertext encrypt(Plaintext p) {
	  std::cout<<"encrypting plaintext "<<std::to_string(p)<<std::endl;
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

	Ciphertext MultiplyBinary(Ciphertext a, Ciphertext b) {
	 	Ciphertext result;
		std::vector<std::array<bool, BITWIDTH(Plaintext)+1> > carry(BITWIDTH(Plaintext)+1);
		
		bool sum_bit, carry_bit;

		result = 0;

		// initialize the first level of carry to zero
		for (int i = 0; i < BITWIDTH(Plaintext)+1; i++)
			carry[0][i] = 0;

		for (int i = 0; i < BITWIDTH(Plaintext); i++) {
			for (int j = 0; j <= i; j++) {
				std::tie(sum_bit, carry_bit) = MultiplyBit(bit(i-j, a), bit(j,b), bit(i,result), carry[j][i]);
				set_bit(i, result, sum_bit);
				carry[j+1][i+1] = carry_bit;
			}
		}
	 	return result;
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

}  // Leaving Clear namespace
}  // Leaving Sheep namespace
  
#endif // CONTEXT_CLEAR_HPP
