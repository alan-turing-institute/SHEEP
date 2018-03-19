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
class ContextClear : public Context<PlaintextT, std::vector<PlaintextT> > {   //plaintext and ciphertext are the same type
public:
        typedef PlaintextT Plaintext;
        typedef Plaintext CiphertextElement;
        typedef std::vector<CiphertextElement> Ciphertext;
        typedef std::vector<Plaintext> PlaintextVec;

  
	Ciphertext encrypt(PlaintextVec p) {
	  std::cout<<"encrypting plaintext "<<std::to_string(p[0])<<std::endl;
	  return p; // plaintext and ciphertext are the same for this context
	}

	PlaintextVec decrypt(Ciphertext c) {
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
		size_t min_nslots = std::min(a.size(), b.size());
		for (int slot=0; slot < min_nslots; ++slot) {
		  bool sum, carry;
		  std::tie(sum, carry) = HalfAdder(bit(0,a[slot]), bit(0,b[slot]));
		  CiphertextElement result_element;
		  set_bit(0, result_element, sum);
		// Note that the loop starts at ONE, since we have
		// already computed the zeroth bit above
		  for (size_t i = 1; i < BITWIDTH(Plaintext); ++i) {
		    std::tie(sum, carry) = FullAdder(bit(i,a), bit(i,b), carry);
		    set_bit(i, result_element, sum);
		  }
		  result.push_back(result_element);
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
	  Ciphertext result;
	  size_t min_nslots = std::min(a.size(), b.size());
	  for (int slot=0; slot < min_nslots; ++slot) {
	    if (std::is_same<CiphertextElement, bool>::value) {
	      CiphertextElement sum = a[slot] != b[slot];
	      result.push_back(sum);
	      
	    } else {
	      typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextElement>::type >::type uC;
	      
	      uC au = static_cast<uC>(a[slot]);
	      uC bu = static_cast<uC>(b[slot]);
	      CiphertextElement sum = static_cast<CiphertextElement>(au + bu);
	      result.push_back(sum);
	    }
	  }
	  return result;
	}
  
  
	Ciphertext Multiply(Ciphertext a, Ciphertext b) {
	  Ciphertext result;
	  size_t min_nslots = std::min(a.size(), b.size());
	  for (int slot=0; slot < min_nslots; ++slot) {
	    if (std::is_same<CiphertextElement, bool>::value) {
	      CiphertextElement product = a[slot] & b[slot];
	      result.push_back(product);
	      return result;
	    } else {
	      typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextElement>::type >::type uC;
	      uC au = static_cast<uC>(a[slot]);
	      uC bu = static_cast<uC>(b[slot]);
	      CiphertextElement product = static_cast<CiphertextElement>(au * bu);
	      result.push_back(product);
	    }
	  }
	  return result;
	}
  
	Ciphertext Subtract(Ciphertext a, Ciphertext b) {
	  
	  if (std::is_same<CiphertextElement, bool>::value) {
	    return Add(a,b);
	  } else {
	    typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextElement>::type >::type uC;
	    Ciphertext result;
	    size_t min_nslots = std::min(a.size(), b.size());
	    for (int slot=0; slot < min_nslots; ++slot) {
	      uC au = static_cast<uC>(a[slot]);
	      uC bu = static_cast<uC>(b[slot]);
	    CiphertextElement difference =  static_cast<CiphertextElement>(au - bu);
	    result.push_back(difference);
	    }
	    return result;
	  }
	}

  
	Ciphertext Maximum(Ciphertext a, Ciphertext b) {
	  Ciphertext result;
	  size_t min_nslots = std::min(a.size(), b.size());
	  for (int slot=0; slot < min_nslots; ++slot) {
	    result.push_back ((a[slot] >= b[slot])?a[slot]:b[slot]);
	  }
	  return result;
	}
  
	Ciphertext Not(Ciphertext a) {
	  Ciphertext result;
	  for (int slot=0; slot < a.size(); ++slot) {
	    result.push_back(!(a[slot])) ;
	  }
	  return result;
	}
  
	Ciphertext Negate(Ciphertext a) {

	  if (std::is_same<Ciphertext, bool>::value) {
	    return Not(a);
	  } else {
	    Ciphertext result;
	    typedef typename std::make_unsigned<typename CiphertextWrapper<CiphertextElement>::type >::type uC;
	    for (int slot=0; slot < a.size(); ++slot) {
	      uC au = static_cast<uC>(a[slot]);
	      result.push_back(static_cast<CiphertextElement>(-au));
	    }
	    return result;
	  }
	}
  
  
	Ciphertext Compare(Ciphertext a, Ciphertext b) {
	  Ciphertext result;
	  size_t min_nslots = std::min(a.size(), b.size());
	  for (int slot=0; slot < min_nslots; ++slot) {
	    result.push_back(a[slot] > b[slot]);
	  }
	  return result;
	}

	Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
	  Ciphertext result;
	  size_t min_nslots = std::min(a.size(), b.size());
	  min_nslots = std::min(min_nslots, s.size());
	  for (int slot=0; slot < min_nslots; ++slot) {
	    result.push_back((s[slot] % 2)?a[slot]:b[slot]);
	  }
	  return result;
	}
  
};

}  // Leaving Clear namespace
}  // Leaving Sheep namespace
  
#endif // CONTEXT_CLEAR_HPP
