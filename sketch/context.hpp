#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <functional>

#include "circuit.hpp"

typedef std::function<bool(bool,bool)> CircuitEvaluator;

// Base class - abstract interface to each library
class Context {
public:
	virtual bool And(bool,bool) =0;

	virtual bool eval(const Circuit& circ,
			  const std::list<bool>& inputs,
			  double& t) =0;
	
	// virtual CircuitEvaluator compile(const Circuit& circ) {
	// 	using std::placeholders::_1;
	// 	using std::placeholders::_2;
	// 	return CircuitEvaluator(std::bind(eval, circ, _1, _2));
	// }	
};

#endif // CONTEXT_HPP
