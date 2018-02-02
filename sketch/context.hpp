#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <functional>

#include "circuit.hpp"

// Base class - abstract interface to each library
template <typename PlaintextT, typename CiphertextT>
class Context {
	typedef std::chrono::duration<double, std::micro> microsecond;
public:
	typedef PlaintextT Plaintext;
	typedef CiphertextT Ciphertext;

	typedef std::function<Ciphertext(Ciphertext,Ciphertext)> GateFn;
	typedef std::function<microsecond(const std::list<Ciphertext>&, std::list<Ciphertext>&)> CircuitEvaluator;
	
        virtual Ciphertext encrypt(Plaintext) =0;
	virtual Plaintext decrypt(Ciphertext) =0;
	
	virtual Ciphertext And(Ciphertext,Ciphertext) =0;
	virtual Ciphertext Or(Ciphertext,Ciphertext) =0;
	virtual Ciphertext Xor(Ciphertext,Ciphertext) =0;
	
	virtual microsecond eval(const Circuit& circ,
				 const std::list<Ciphertext>& input_vals,
				 std::list<Ciphertext>& output_vals) {
		std::unordered_map<std::string, Ciphertext> eval_map;
		
		// add Circuit::inputs and inputs into the map
		auto input_vals_it = input_vals.begin();
		auto input_wires_it = circ.get_inputs().begin();
		const auto input_wires_end = circ.get_inputs().end();

		std::cout<<"Sizes of inputs "<<input_vals.size()<<" "<<circ.get_inputs().size()<<std::endl;
		
		for (; input_vals_it != input_vals.end() || input_wires_it != input_wires_end;
		     ++input_vals_it, ++input_wires_it)
		{
		  std::cout<<" inserting "<<input_wires_it->get_name()<<std::endl;
		  eval_map.insert({input_wires_it->get_name(), *input_vals_it});
		}
		std::cout<<" checking "<<std::endl;		
		// error check: both iterators should be at the end
		if (input_vals_it != input_vals.end() || input_wires_it != input_wires_end)
			throw std::runtime_error("Number of inputs doesn't match");


		// This is where the actual evaluation occurs.  For
		// each assignment, look up the input Wires in the
		// map, insert the output wire (of the gate) with the
		// required name into eval_map.

		typedef std::chrono::duration<double, std::micro> microsecond;
		typedef std::chrono::high_resolution_clock high_res_clock;
		auto start_time = high_res_clock::now();
		
		for (const Assignment assn : circ.get_assignments()) {
			// throws out_of_range if not present in the map
		  std::cout<<" looking up "<<assn.get_input1().get_name()<<" "<<assn.get_input2().get_name()<<std::endl;
		  Ciphertext input1 = eval_map.at(assn.get_input1().get_name());
		  Ciphertext input2 = eval_map.at(assn.get_input2().get_name());
			auto op = get_op(assn.get_op());
			Ciphertext output = op(input1, input2);
			eval_map.insert({assn.get_output().get_name(), output});
		}
		
		auto end_time = high_res_clock::now();
		microsecond duration = microsecond(end_time - start_time);

		
		// Look up the required outputs in the eval_map and
		// push them onto output_vals.
		auto output_wires_it = circ.get_outputs().begin();
		auto output_wires_end = circ.get_outputs().end();
		std::cout<<" about to look at outputs"<<std::endl;
		for (; output_wires_it != output_wires_end; ++output_wires_it) {
		  std::cout<<" name of output wire "<<output_wires_it->wire.get_name()<<std::endl;		  
		  output_vals.push_back(eval_map.at(output_wires_it->wire.get_name()));
		}
		std::cout<<"done"<<std::endl;
		return duration;
	}

	
	virtual CircuitEvaluator compile(const Circuit& circ) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		auto run = std::bind(&Context::eval, this, circ, _1, _2);
		return CircuitEvaluator(run);
	}

        GateFn get_op(Gate g) {
	  using namespace std::placeholders;
	  switch(g) {
	  case(Gate::And):
	    return GateFn(std::bind(&Context::And, this, _1, _2));
	    break;
	    
	  case(Gate::Or):
	    return GateFn(std::bind(&Context::Or, this, _1, _2));
	    break;
	    
	  case(Gate::Xor):
	    return GateFn(std::bind(&Context::Xor, this, _1, _2));
	    break;
	    
	  }
	  throw std::runtime_error("Unknown op");
	}
};

#endif // CONTEXT_HPP
