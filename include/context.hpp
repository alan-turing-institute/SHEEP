#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <functional>
#include <unordered_map>
#include <list>
#include <chrono>
#include <algorithm>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#ifdef HAVE_TBB
#include "tbb/concurrent_unordered_map.h"
#include "tbb/flow_graph.h"
#include "tbb/atomic.h"
#endif // HAVE_TBB

#include "circuit.hpp"

enum class EvaluationStrategy {serial, parallel};

struct GateNotImplemented : public std::runtime_error {
	GateNotImplemented() : std::runtime_error("Gate not implemented.") { };
};

struct TimeoutException : public std::exception {
	std::chrono::duration<double, std::micro> execution_time;
	std::string what_str;
	const char *what() const noexcept
	{
		return what_str.c_str();
	}
	TimeoutException(
		// name of the output we were evaluating at the time
		const std::string& on_output,
		// total wall-clock time when the exception was thrown
		std::chrono::duration<double, std::micro> execution_time_)
		:
		execution_time(execution_time_),
		what_str("Evaluation timed out (wall-clock time: " + std::to_string(execution_time_.count())
			 + " microseconds) while evaluating circuit wire named " + on_output)
	{ }
};

/// Base base class
template <typename PlaintextT>
class BaseContext {
public:
	virtual ~BaseContext() {};

	// Four overloads of eval_with_plaintexts to allow calls with
	// and without providing constant plaintexts, and with and
	// without providing an array in which to store the timing
	// information.

	// overload taking both durations and const_plaintext_inputs
	virtual std::vector<std::vector<PlaintextT>>
	eval_with_plaintexts(const Circuit& C, std::vector<std::vector<PlaintextT>> plaintext_inputs,
	                     std::vector<std::vector<PlaintextT>> const_plaintext_inputs,
	                     std::vector<std::chrono::duration<double, std::micro> >& durations,
	                     EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	                     std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0)) = 0;

	// overload omitting durations only
	virtual std::vector<std::vector<PlaintextT>>
	eval_with_plaintexts(const Circuit& C, std::vector<std::vector<PlaintextT>> plaintext_inputs,
	                     std::vector<std::vector<PlaintextT>> const_plaintext_inputs,
	                     EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	                     std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0)) = 0;

	// overload omitting const_plaintext_inputs only
	virtual std::vector<std::vector<PlaintextT>>
	eval_with_plaintexts(const Circuit& C, std::vector<std::vector<PlaintextT>> plaintext_inputs,
	                     std::vector<std::chrono::duration<double, std::micro> >& durations,
	                     EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	                     std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0)) = 0;
	
	// overload omitting both durations and const_plaintext_inputs
	virtual std::vector<std::vector<PlaintextT>>
	eval_with_plaintexts(const Circuit& C, std::vector<std::vector<PlaintextT>> plaintext_inputs,
	                     EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	                     std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0)) = 0;

	virtual void print_parameters() = 0;
	virtual std::map<std::string, long> get_parameters() = 0;
	virtual void print_sizes() = 0;
	virtual void set_parameter(std::string, long) = 0;
	virtual void configure() = 0;
};

// Base class - abstract interface to each library
template <typename PlaintextT, typename CiphertextT>
class Context : public BaseContext<PlaintextT> {
	typedef std::chrono::duration<double, std::micro> microsecond;
public:
	typedef PlaintextT Plaintext;
	typedef CiphertextT Ciphertext;

	typedef std::function<microsecond(const std::list<Ciphertext>&, std::list<Ciphertext>&)> CircuitEvaluator;
	virtual ~Context() {};
	virtual void       configure()                     { m_configured = true; };
	virtual Ciphertext encrypt(std::vector<Plaintext>) = 0;
	virtual std::vector<Plaintext>  decrypt(Ciphertext) = 0;

	virtual Ciphertext Alias(Ciphertext a)             { return a; };
	virtual Ciphertext Identity(Ciphertext)            { throw GateNotImplemented(); };
	virtual Ciphertext Multiply(Ciphertext, Ciphertext) { throw GateNotImplemented(); };
	virtual Ciphertext Maximum(Ciphertext, Ciphertext)  { throw GateNotImplemented(); };
	virtual Ciphertext Add(Ciphertext, Ciphertext)      { throw GateNotImplemented(); };
	virtual Ciphertext Subtract(Ciphertext, Ciphertext) { throw GateNotImplemented(); };
	virtual Ciphertext Negate(Ciphertext)              { throw GateNotImplemented(); };
	// if a > b, returns a Ciphertext representation of 1, and a Ciphertext 0 otherwise.
	virtual Ciphertext Compare(Ciphertext a, Ciphertext b) { throw GateNotImplemented(); };
	// Select(s,a,b) := lsb(s)?a:b
	virtual Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) { throw GateNotImplemented(); };
	virtual Ciphertext AddConstant(std::vector<Ciphertext>, long  ) { throw GateNotImplemented(); };
	virtual Ciphertext MultByConstant(std::vector<Ciphertext>, long  ) { throw GateNotImplemented(); };

	virtual Ciphertext dispatch(Gate g,
				    std::vector<Ciphertext> inputs,
				    std::vector<std::vector<Plaintext>> const_inputs)
	{
		using namespace std::placeholders;
		switch (g) {
		case (Gate::Alias):
			return Alias(inputs.at(0));
			break;
		case (Gate::Identity):
			return Identity(inputs.at(0));
			break;
		case (Gate::Multiply):
			return Multiply(inputs.at(0), inputs.at(1));
			break;
		case (Gate::Maximum):
			return Maximum(inputs.at(0), inputs.at(1));
			break;
		case (Gate::Add):
			return Add(inputs.at(0), inputs.at(1));
			break;
		case (Gate::Subtract):
			return Subtract(inputs.at(0), inputs.at(1));
			break;
		case (Gate::Negate):
			return Negate(inputs.at(0));
			break;
		case (Gate::Compare):
			return Compare(inputs.at(0), inputs.at(1));
			break;
		case (Gate::Select):
			return Select(inputs.at(0), inputs.at(1), inputs.at(2));
			break;
		case (Gate::AddConstant):
			return AddConstant( inputs, (const_inputs.at(0)).at(0));
			break;
		case (Gate::MultByConstant):
			return MultByConstant( inputs, (const_inputs.at(0)).at(0));
			break;
		}
		throw std::runtime_error("Unknown op");
	}

	template <typename InputContainer,
		  typename ConstInputContainer = std::vector<Plaintext>,
		  typename OutputContainer>
	microsecond eval(const Circuit& circ,
	                 const InputContainer& input_vals,
	                 OutputContainer& output_vals,
	                 const ConstInputContainer& const_input_vals = ConstInputContainer(),
	                 std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0)) {
		std::unordered_map<std::string, Ciphertext> eval_map;
		std::unordered_map<std::string, std::vector<Plaintext>> const_eval_map;

		// add Circuit::inputs and inputs into the map
		auto input_vals_it = input_vals.begin();
		auto input_wires_it = circ.get_inputs().begin();
		const auto input_wires_end = circ.get_inputs().end();
		for (; input_vals_it != input_vals.end() || input_wires_it != input_wires_end ;
		     ++input_vals_it, ++input_wires_it)
		{
			eval_map.insert({input_wires_it->get_name(), *input_vals_it});
		}

		// add Circuit::const_inputs and const inputs into the map
		auto const_input_vals_it = const_input_vals.begin();
		auto const_input_wires_it = circ.get_const_inputs().begin();
		const auto const_input_wires_end = circ.get_const_inputs().end();
		for (; const_input_vals_it != const_input_vals.end() || const_input_wires_it != const_input_wires_end;
		     ++const_input_vals_it, ++const_input_wires_it)
		{
			const_eval_map.insert({const_input_wires_it->get_name(), *const_input_vals_it});
		}

		// error check: all iterators should be at the end
		if (input_vals_it != input_vals.end()
		    || input_wires_it != input_wires_end
		    || const_input_vals_it != const_input_vals.end()
		    || const_input_wires_it != const_input_wires_end)
		{
			throw std::runtime_error("Number of inputs or const_inputs does not match");
		}

		// This is where the actual evaluation occurs.  For
		// each assignment, look up the input Wires in the
		// map, insert the output wire (of the gate) with the
		// required name into eval_map.

		typedef std::chrono::duration<double, std::micro> microsecond;
		typedef std::chrono::high_resolution_clock high_res_clock;
		auto start_time = high_res_clock::now();

		for (const Assignment assn : circ.get_assignments()) {
			std::vector<Ciphertext> inputs;
			std::vector<std::vector<Plaintext>> const_inputs;
			for (Wire w : assn.get_inputs())
			{
				typename decltype(eval_map)::iterator it;
				if ((it = eval_map.find(w.get_name())) != eval_map.end()) {
					inputs.push_back(it->second);
				} else {
					const_inputs.push_back(const_eval_map.at(w.get_name()));
				}
			}
			Ciphertext output = dispatch(assn.get_op(), inputs, const_inputs);
			eval_map.insert({assn.get_output().get_name(), output});
			microsecond wall_time = high_res_clock::now() - start_time;
			if (timeout.count() > 0.0 && wall_time > timeout)
				throw TimeoutException(assn.get_output().get_name(), wall_time);
		}

		auto end_time = high_res_clock::now();
		microsecond duration = microsecond(end_time - start_time);

		// Look up the required outputs in the eval_map and push them onto output_vals.
		auto output_wires_it = circ.get_outputs().begin();
		auto output_wires_end = circ.get_outputs().end();
		for (; output_wires_it != output_wires_end; ++output_wires_it) {
			output_vals.push_back(eval_map.at(output_wires_it->get_name()));
		}
		return duration;
	}

	template <typename InputContainer,
		  typename ConstInputContainer = std::vector<PlaintextT>,
		  typename OutputContainer>
	microsecond parallel_eval(const Circuit& circ,
				  const InputContainer& input_vals,
				  OutputContainer& output_vals,
				  const ConstInputContainer& const_input_vals = ConstInputContainer(),
				  std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0))
	{
#ifdef HAVE_TBB
		using namespace tbb::flow;

		typedef std::chrono::duration<double, std::micro> microsecond;
		typedef std::chrono::high_resolution_clock high_res_clock;

		tbb::concurrent_unordered_map<std::string, Ciphertext> eval_map;
		tbb::concurrent_unordered_map<std::string, Plaintext> const_eval_map;
		tbb::concurrent_unordered_map<std::string, continue_node<continue_msg> > node_map;

		tbb::flow::graph DAG;

		// insert the inputs into the map
		auto input_vals_it = input_vals.begin();
		auto input_wires_it = circ.get_inputs().begin();
		const auto input_wires_end = circ.get_inputs().end();

		for (; input_vals_it != input_vals.end() || input_wires_it != input_wires_end;
		     ++input_vals_it, ++input_wires_it)
		{
			auto inserter = [=, &eval_map](const continue_msg&) {
				eval_map.insert({input_wires_it->get_name(), *input_vals_it});
			};
			node_map.insert({input_wires_it->get_name(),
						continue_node<continue_msg>(DAG, inserter)});
		}

		auto const_input_vals_it = const_input_vals.begin();
		auto const_input_wires_it = circ.get_const_inputs().begin();
		const auto const_input_wires_end = circ.get_const_inputs().end();
		for (; const_input_vals_it != const_input_vals.end() || const_input_wires_it != const_input_wires_end;
		     ++const_input_vals_it, ++const_input_wires_it)
		{
			auto inserter = [=, &const_eval_map](const continue_msg&) {
				const_eval_map.insert({const_input_wires_it->get_name(), *const_input_vals_it});
			};
			node_map.insert({const_input_wires_it->get_name(),
						continue_node<continue_msg>(DAG, inserter)});
		}

		// error check: both iterators should be at the end
		if (input_vals_it != input_vals.end()
		    || input_wires_it != input_wires_end
		    || const_input_vals_it != const_input_vals.end()
		    || const_input_wires_it != const_input_wires_end)
		{
			throw std::runtime_error("Number of inputs doesn't match");
		}

		// This is where the actual evaluation occurs.  For
		// each assignment, look up the input Wires in the
		// map, insert the output wire (of the gate) with the
		// required name into eval_map.

		decltype((high_res_clock::now())) start_time;

		tbb::atomic<bool> circuit_timed_out = false;
		// the following two variables are only to be written
		// to from a single thread.  This is enforced by a
		// check of circuit_timed_out, which is written to
		// exactly once.
		std::string timeout_gate_name;
		microsecond timeout_wall_time;

		for (const Assignment assn : circ.get_assignments()) {
			auto current_eval = [=,&eval_map,&circuit_timed_out,
					     &timeout_gate_name,&timeout_wall_time,&start_time
				](const continue_msg&) {
				std::vector<Ciphertext> inputs;
				std::vector<Plaintext> const_inputs;

				for (Wire w : assn.get_inputs()) {
					typename decltype(eval_map)::iterator it;
					if ((it = eval_map.find(w.get_name())) != eval_map.end()) {
						inputs.push_back(it->second);
					} else {
						const_inputs.push_back(const_eval_map.at(w.get_name()));
					}
				}
				Ciphertext output = dispatch(assn.get_op(), inputs, const_inputs);
				eval_map.insert({assn.get_output().get_name(), output});

				microsecond wall_time = high_res_clock::now() - start_time;
				if (timeout.count() > 0.0
				    && wall_time > timeout
				    && !circuit_timed_out.fetch_and_store(true))
					// last condition ensures the body of the conditional is
					// only entered from one thread.
				{
					timeout_gate_name = assn.get_output().get_name();
					timeout_wall_time = wall_time;
					tbb::task::self().group()->cancel_group_execution();
				}
			};
			node_map.insert({assn.get_output().get_name(),
						continue_node<continue_msg>(DAG, current_eval)});

			for (const Wire input_wire : assn.get_inputs()) {
				make_edge(node_map.at(input_wire.get_name()),
					  node_map.at(assn.get_output().get_name()));
			}
		}
		start_time = high_res_clock::now();

		// for each input, send a continue message to start
		// the evaluation
		for (auto input_wires_it = circ.get_inputs().begin();
		     input_wires_it != circ.get_inputs().end();
		     ++input_wires_it)
		{
			node_map.at(input_wires_it->get_name()).try_put(continue_msg());
		}

		DAG.wait_for_all();

		if (circuit_timed_out) throw TimeoutException(timeout_gate_name, timeout_wall_time);

		auto end_time = high_res_clock::now();
		microsecond duration = microsecond(end_time - start_time);

		// Look up the required outputs in the eval_map and
		// push them onto output_vals.
		auto output_wires_it = circ.get_outputs().begin();
		auto output_wires_end = circ.get_outputs().end();
		for (; output_wires_it != output_wires_end; ++output_wires_it) {
			output_vals.push_back(eval_map.at(output_wires_it->get_name()));
		}
		return duration;
#else
		throw std::runtime_error("SHEEP was not compiled with TBB, so parallel evaluation is not available.");
#endif // HAVE_TBB
	}

	// virtual CircuitEvaluator compile(const Circuit& circ) {
	// 	using std::placeholders::_1;
	// 	using std::placeholders::_2;
	// 	using std::placeholders::_3;
	// 	auto run = std::bind(&Context::eval<std::list<Ciphertext>, std::list<Ciphertext> >, this, circ, _1, _2,
	// 	                     std::chrono::duration<double, std::micro>(0.0));
	// 	return CircuitEvaluator(run);
	// }

	// overload taking both durations and const_plaintext_inputs
	virtual std::vector<std::vector<PlaintextT>> 
	eval_with_plaintexts(
	  const Circuit& C,
	  std::vector<std::vector<PlaintextT>> plaintext_inputs,
	  std::vector<std::vector<PlaintextT>> const_plaintext_inputs,
	  std::vector<std::chrono::duration<double, std::micro> >& durations,
	  EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	  std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0))
	{
		typedef std::chrono::duration<double, std::micro> microsecond;
		typedef std::chrono::high_resolution_clock high_res_clock;
		auto enc_start_time = high_res_clock::now();

		/// encrypt the inputs
		std::vector<Ciphertext> ciphertext_inputs;
		std::vector<std::vector<Plaintext>> const_inputs;
		for (auto pt : plaintext_inputs) ciphertext_inputs.push_back(encrypt(pt));
		for (auto cpt : const_plaintext_inputs) const_inputs.push_back(cpt);


		auto enc_end_time = high_res_clock::now();
		durations.push_back(microsecond(enc_end_time - enc_start_time));

		//// evaluate the circuit
		std::vector<Ciphertext> ciphertext_outputs;
		microsecond eval_duration;

		switch (eval_strategy) {
		case EvaluationStrategy::serial:
			eval_duration = eval(C, ciphertext_inputs, ciphertext_outputs,
					     const_inputs, timeout);
			break;
		case EvaluationStrategy::parallel:
			eval_duration = parallel_eval(C, ciphertext_inputs, ciphertext_outputs,
						       const_inputs, timeout);
			break;
		default:
			std::runtime_error("eval_with_plaintexts: Unknown evaluation strategy");
		}
		durations.push_back(eval_duration);

		//// decrypt the outputs again
		auto dec_start_time = high_res_clock::now();
		std::vector<std::vector<Plaintext>> plaintext_outputs;
		for (auto ct : ciphertext_outputs) plaintext_outputs.push_back(decrypt(ct));
		auto dec_end_time = high_res_clock::now();
		durations.push_back(microsecond(dec_end_time - dec_start_time));

		return plaintext_outputs;
	}

	// overload omitting const_plaintext_inputs only
	virtual std::vector<std::vector<PlaintextT>> eval_with_plaintexts(
	  const Circuit& C,
	  std::vector<std::vector<PlaintextT>> plaintext_inputs,
	  std::vector<std::chrono::duration<double, std::micro> >& durations,
	  EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	  std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0))
	{
		std::vector<std::vector<PlaintextT>> cptxts_empty;
		return eval_with_plaintexts(C, plaintext_inputs, cptxts_empty,
					    durations, eval_strategy, timeout);
	}

	// overload omitting durations only
	virtual std::vector<std::vector<PlaintextT>>
	eval_with_plaintexts(const Circuit& c, std::vector<std::vector<PlaintextT>> ptxts,
	                     std::vector<std::vector<PlaintextT>> cptxts,
	                     EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	                     std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0))
	{
		std::vector<std::chrono::duration<double, std::micro> > ignored;
		return eval_with_plaintexts(c, ptxts, cptxts, ignored, eval_strategy, timeout);
	}

	// overload omitting both durations and const_plaintext_inputs
	virtual std::vector<std::vector<PlaintextT>>
	eval_with_plaintexts(const Circuit& c, std::vector<std::vector<PlaintextT>> ptxts,
	                     EvaluationStrategy eval_strategy = EvaluationStrategy::serial,
	                     std::chrono::duration<double, std::micro> timeout = std::chrono::duration<double, std::micro>(0.0))
	{
		std::vector<std::chrono::duration<double, std::micro> > ignored;
		std::vector<std::vector<Plaintext>> cptxts_empty;
		return eval_with_plaintexts(c, ptxts, cptxts_empty, ignored, eval_strategy, timeout);
	}

	virtual void read_params_from_file(std::string filename) {
		std::ifstream inputstream(filename);

		if (inputstream.bad()) {
			std::cout << "Empty or non-existent input file" << std::endl;
		}

		/// loop over all lines in the input file
		std::string line;
		while (std::getline(inputstream, line) ) {
			/// remove comments (lines starting with #) and empty lines
			int found = line.find_first_not_of(" \t");
			if ( found != std::string::npos) {
				if ( line[found] == '#')
					continue;

				/// split up by whitespace
				std::string buffer;
				std::vector<std::string> tokens;
				std::stringstream ss(line);
				while (ss >> buffer) tokens.push_back(buffer);

				if (tokens.size() == 2) {   /// assume we have param_name param_value
					set_parameter(tokens[0], stol(tokens[1]));
				}
			}
		} // end of loop over lines

		//// now configure the library
		configure();
	}

	virtual void set_parameter(std::string param_name, long param_value) {
		auto map_iter = m_param_name_map.find(param_name);
		if ( map_iter == m_param_name_map.end() ) {
			std::cout << "Parameter " << param_name << " not found." << std::endl;
			return;
		} else {
			std::cout << "Setting parameter " << map_iter->first << " to " << param_value << std::endl;
			map_iter->second = param_value;
			m_param_overrides.push_back(map_iter->first);
			return;
		}
	}

	/// see if a parameter was explicitly set, in order to avoid accidentally overwriting it
	virtual bool override_param(std::string param_name) {
		return std::find(m_param_overrides.begin(),
		                 m_param_overrides.end(),
		                 param_name)  != m_param_overrides.end();
	}

	virtual std::map<std::string, long> get_parameters() {
		std::map<std::string, long> param_map;
		for (auto map_iter = m_param_name_map.begin();
		     map_iter != m_param_name_map.end();
		     ++map_iter) {
			param_map[map_iter->first] = map_iter->second;
		}
		return param_map;
	};

	virtual void print_parameters() {
		for ( auto map_iter = m_param_name_map.begin(); map_iter != m_param_name_map.end(); ++map_iter) {
			std::cout << "Parameter " << map_iter->first << " = " << map_iter->second << std::endl;
		}
	}

	virtual void print_sizes() {
		std::cout << "size of publicKey: " << m_public_key_size << std::endl;
		std::cout << "size of privateKey: " << m_private_key_size << std::endl;
		std::cout << "size of ciphertext: " << m_ciphertext_size << std::endl;
	}

protected:

	std::map<std::string, long&> m_param_name_map;
	std::vector<std::string> m_param_overrides;
	bool m_configured;

	////  sizes (in bytes) of keys and ciphertext
	int  m_private_key_size;
	int  m_public_key_size;
	int  m_ciphertext_size;
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
		       [&context](typename ContextT::Plaintext pt) {
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
		       [&context](typename ContextT::Ciphertext ct) {
			       return context.decrypt(ct);
		       });
}

#endif // CONTEXT_HPP
