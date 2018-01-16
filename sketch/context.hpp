#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <functional>

#include "circuit.hpp"

typedef std::function<double(const std::list<bool>&, std::list<bool>&)> CircuitEvaluator;

// Base class - abstract interface to each library
class Context {
public:
	virtual bool And(bool,bool) =0;
	virtual bool Or(bool,bool) =0;
	virtual bool Xor(bool,bool) =0;

	virtual double eval(const Circuit& circ,
			    const std::list<bool>& inputs,
			    std::list<bool>& outputs) =0;

	virtual CircuitEvaluator compile(const Circuit& circ)
	{
		using std::placeholders::_1;
		using std::placeholders::_2;
		auto run = std::bind(&Context::eval, this, circ, _1, _2);
		return CircuitEvaluator(run);
	}
};

#endif // CONTEXT_HPP
