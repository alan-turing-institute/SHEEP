#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <functional>

#include "circuit.hpp"

// Base class - abstract interface to each library
template <typename PlaintextT, typename CiphertextT>
class Context {
public:
	typedef PlaintextT Plaintext;
	typedef CiphertextT Ciphertext;

	typedef std::function<Ciphertext(Ciphertext,Ciphertext)> GateFn;
	typedef std::function<double(const std::list<Ciphertext>&, std::list<Ciphertext>&)> CircuitEvaluator;

	virtual Ciphertext encrypt(Plaintext) =0;
	virtual Plaintext decrypt(Ciphertext) =0;
	
	virtual Ciphertext And(Ciphertext,Ciphertext) =0;
	virtual Ciphertext Or(Ciphertext,Ciphertext) =0;
	virtual Ciphertext Xor(Ciphertext,Ciphertext) =0;

	virtual double eval(const Circuit& circ,
			    const std::list<Ciphertext>& inputs,
			    std::list<Ciphertext>& outputs) =0;

	virtual CircuitEvaluator compile(const Circuit& circ) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		auto run = std::bind(&Context::eval, this, circ, _1, _2);
		return CircuitEvaluator(run);
	}
};

#endif // CONTEXT_HPP
