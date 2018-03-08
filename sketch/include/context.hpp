#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <functional>
#include <unordered_map>
#include <list>
#include <chrono>
#include <algorithm>
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "circuit.hpp"

/// Base base class
template <typename PlaintextT>
class BaseContext {
public:
  virtual std::list<PlaintextT>
  eval_with_plaintexts(Circuit, std::list<PlaintextT>, std::vector<std::chrono::duration<double, std::micro> >&) = 0 ; 
};

// Base class - abstract interface to each library
template <typename PlaintextT, typename CiphertextT>
class Context : public BaseContext<PlaintextT> {
	typedef std::chrono::duration<double, std::micro> microsecond;
public:
	typedef PlaintextT Plaintext;
	typedef CiphertextT Ciphertext;

	typedef std::function<microsecond(const std::list<Ciphertext>&, std::list<Ciphertext>&)> CircuitEvaluator;
	
        virtual Ciphertext encrypt(Plaintext) =0;
	virtual Plaintext decrypt(Ciphertext) =0;

	struct GateNotImplemented : public std::runtime_error {
		GateNotImplemented() : std::runtime_error("Gate not implemented.") { };
	};

	virtual Ciphertext Alias(Ciphertext a)             { return a; };
	virtual Ciphertext Identity(Ciphertext)            { throw GateNotImplemented(); };
	virtual Ciphertext Multiply(Ciphertext,Ciphertext) { throw GateNotImplemented(); };
	virtual Ciphertext Maximum(Ciphertext,Ciphertext)  { throw GateNotImplemented(); };
	virtual Ciphertext Add(Ciphertext,Ciphertext)      { throw GateNotImplemented(); };
	virtual Ciphertext Subtract(Ciphertext,Ciphertext) { throw GateNotImplemented(); };
	virtual Ciphertext Negate(Ciphertext)              { throw GateNotImplemented(); };
	// if a > b, returns a Ciphertext representation of 1, and a Ciphertext 0 otherwise.
	virtual Ciphertext Compare(Ciphertext a, Ciphertext b) { throw GateNotImplemented(); };
	// Select(s,a,b) := lsb(s)?a:b
	virtual Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) { throw GateNotImplemented(); };
	virtual Ciphertext AddConstant(Ciphertext, long  ) { throw GateNotImplemented(); };
	virtual Ciphertext MultByConstant(Ciphertext, long  ) { throw GateNotImplemented(); };    

	virtual Ciphertext dispatch(Gate g, std::vector<Ciphertext> inputs) {
		using namespace std::placeholders;
		switch(g) {
		case(Gate::Alias):
			return Alias(inputs.at(0));
			break;

		case(Gate::Identity):
			return Identity(inputs.at(0));
			break;

		case(Gate::Multiply):
			return Multiply(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Maximum):
			return Maximum(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Add):
			return Add(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Subtract):
			return Subtract(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Negate):
			return Negate(inputs.at(0));
			break;

		case(Gate::Compare):
			return Compare(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Select):
			return Select(inputs.at(0), inputs.at(1), inputs.at(2));
			break;
		}
		throw std::runtime_error("Unknown op");
	}

	template <typename InputContainer, typename OutputContainer>
	microsecond eval(const Circuit& circ,
			 const InputContainer& input_vals,
			 OutputContainer& output_vals) {
		std::unordered_map<std::string, Ciphertext> eval_map;
		
		// add Circuit::inputs and inputs into the map
		auto input_vals_it = input_vals.begin();
		auto input_wires_it = circ.get_inputs().begin();
		const auto input_wires_end = circ.get_inputs().end();

		for (; input_vals_it != input_vals.end() || input_wires_it != input_wires_end;
		     ++input_vals_it, ++input_wires_it)
		{
		  eval_map.insert({input_wires_it->get_name(), *input_vals_it});
		}
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
			std::vector<Ciphertext> inputs;
			std::transform(assn.get_inputs().begin(),
				       assn.get_inputs().end(),
				       std::back_inserter(inputs),
				       [&eval_map](Wire w) {
					       // throws out_of_range if not present in the map
					       return eval_map.at(w.get_name());
				       });
			Ciphertext output = dispatch(assn.get_op(), inputs);
			eval_map.insert({assn.get_output().get_name(), output});
		}

		auto end_time = high_res_clock::now();
		microsecond duration = microsecond(end_time - start_time);

		// Look up the required outputs in the eval_map and
		// push them onto output_vals.
		auto output_wires_it = circ.get_outputs().begin();
		auto output_wires_end = circ.get_outputs().end();
		for (; output_wires_it != output_wires_end; ++output_wires_it) {
			output_vals.push_back(eval_map.at(output_wires_it->get_name()));
		}
		std::cout<<"duration here in context::eval is "<<duration.count()<<std::endl;
		return duration;
	}
	
	virtual CircuitEvaluator compile(const Circuit& circ) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		auto run = std::bind(&Context::eval<std::list<Ciphertext>, std::list<Ciphertext> >, this, circ, _1, _2);
		return CircuitEvaluator(run);
	}

        virtual std::list<Plaintext> eval_with_plaintexts(Circuit C,
							  std::list<Plaintext> plaintext_inputs,
							  std::vector<std::chrono::duration<double, std::micro> >& durations) {

	  typedef std::chrono::duration<double, std::micro> microsecond;
	  typedef std::chrono::high_resolution_clock high_res_clock;
	  auto enc_start_time = high_res_clock::now();
	  
	  /// encrypt the inputs
	  std::list<Ciphertext> ciphertext_inputs;
	  for (auto pt_iter = plaintext_inputs.begin(); pt_iter != plaintext_inputs.end(); ++pt_iter) 
	    ciphertext_inputs.push_back(encrypt(*pt_iter));
	  auto enc_end_time = high_res_clock::now();
	  durations.push_back(microsecond(enc_end_time - enc_start_time));
	  
	  //// evaluate the circuit	  
	  std::list<Ciphertext> ciphertext_outputs;
	  microsecond eval_duration = eval(C, ciphertext_inputs, ciphertext_outputs);
	  durations.push_back(eval_duration);

	  //// decrypt the outputs again
	  auto dec_start_time = high_res_clock::now();
	  std::list<Plaintext> plaintext_outputs;
	  for (auto ct_iter = ciphertext_outputs.begin(); ct_iter != ciphertext_outputs.end(); ++ct_iter) 
	    plaintext_outputs.push_back(decrypt(*ct_iter));
	  auto dec_end_time = high_res_clock::now();
	  durations.push_back(microsecond(dec_end_time - dec_start_time));	  
	  return plaintext_outputs;
	}
  
  
        virtual void read_params_from_file(std::string filename) {
	  std::ifstream inputstream(filename);
	  
	  if (inputstream.bad()) {
	    std::cout<<"Empty or non-existent input file"<<std::endl;
	  }
	  
	  /// loop over all lines in the input file 
	  std::string line;
	  while (std::getline(inputstream, line) ) {
	    /// remove comments (lines starting with #) and empty lines
	    int found= line.find_first_not_of(" \t");
	    if( found != std::string::npos) {   
	      if ( line[found] == '#') 
		continue;
	      
	      /// split up by whitespace
	      std::string buffer;
	      std::vector<std::string> tokens;
	      std::stringstream ss(line);
	      while (ss >> buffer) tokens.push_back(buffer);
	      
	      if (tokens.size() == 2) {   /// assume we have param_name param_value
		set_parameter(tokens[0],stol(tokens[1])); 		
	      }	      
	    }    
	  } // end of loop over lines
	}
  
  
  
        virtual void set_parameter(std::string param_name, long param_value) {
	  auto map_iter = m_param_name_map.find(param_name);
	  if ( map_iter == m_param_name_map.end() ) {
	    std::cout<<"Parameter "<<param_name<<" not found."<<std::endl;
	    return;
	  } else {
	    std::cout<<"Setting parameter "<<map_iter->first<<" to "<<param_value<<std::endl;
	    map_iter->second = param_value;
	    return;
	  }
	}

        virtual void print_parameters() {
	  for ( auto map_iter = m_param_name_map.begin(); map_iter != m_param_name_map.end(); ++map_iter) {
	    std::cout<<"Parameter "<<map_iter->first<<" = "<<map_iter->second<<std::endl;
	  }
	}

  
protected:

        std::map<std::string, long& > m_param_name_map;

};

template <typename ContextT,
	  typename PlaintextCIterator,
	  typename CiphertextIterator>
void encrypt(ContextT& context,
	     PlaintextCIterator plaintext_begin,
	     PlaintextCIterator plaintext_end,
	     CiphertextIterator ciphertext_begin)
{
	std::transform(plaintext_begin, plaintext_end, ciphertext_begin,
		       [&context](typename ContextT::Plaintext pt){
			       return context.encrypt(pt);
		       });
}

template <typename ContextT,
	  typename CiphertextCIterator,
	  typename PlaintextIterator>
void decrypt(ContextT& context,
	     CiphertextCIterator ciphertext_begin,
	     CiphertextCIterator ciphertext_end,
	     PlaintextIterator plaintext_begin)
{
	std::transform(ciphertext_begin, ciphertext_end, plaintext_begin,
		       [&context](typename ContextT::Ciphertext ct){
			       return context.decrypt(ct);
		       });
}

template <typename ContextT,
	  typename PlaintextContainer>
PlaintextContainer eval_with_plaintexts(ContextT context,
					Circuit circ,
					PlaintextContainer plaintexts_in)
{
	typedef std::list<typename ContextT::Ciphertext> CiphertextContainer;
	CiphertextContainer ciphertexts_in, ciphertexts_out;
	encrypt(context, plaintexts_in.begin(), plaintexts_in.end(),
		std::back_inserter(ciphertexts_in));

	context.eval(circ, ciphertexts_in, ciphertexts_out);

	PlaintextContainer plaintexts_out;
	decrypt(context, ciphertexts_out.begin(), ciphertexts_out.end(),
		std::back_inserter(plaintexts_out));

	return plaintexts_out;
}


#endif // CONTEXT_HPP
