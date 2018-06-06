#ifndef SHEEP_SERVER_HPP
#define SHEEP_SERVER_HPP

#include <memory>
#include <fstream>
#include <map>

#include "circuit.hpp"
#include "context-clear.hpp"
#include "context-helib.hpp"
#include "context-tfhe.hpp"
//#include "context-seal.hpp"


using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

static std::vector<std::string> available_contexts = {"HElib_Fp",
						      "HElib_F2",
						      "TFHE",
						      "Clear"};

static std::vector<std::string> available_input_types = {"bool",
							 "uint8_t",
							 "uint16_t",
							 "uint32_t",
							 "int8_t",
							 "int16_t",
							 "int32_t"};


struct SheepJobConfig {
  utility::string_t context;
  utility::string_t input_type;
  utility::string_t circuit_filename;
  EvaluationStrategy eval_strategy;
  Circuit circuit;
  std::vector<std::string> input_names;
  std::vector<int> input_vals;
  std::map<std::string, long&> parameters;

  void setDefaults() {
    eval_strategy = EvaluationStrategy::serial;
  }
  bool isConfigured() {
    return ((context.size() > 0) &&
	    (input_type.size() > 0) &&	    
    	    (circuit.get_inputs().size() > 0) &&
	    (input_vals.size() == circuit.get_inputs().size())); 
  }
};


typedef std::chrono::duration<double, std::micro> DurationT;

struct SheepJobResult {
  std::map<std::string, std::string> outputs;  /// store output values as strings so we don't worry about Plaintext type.
  std::vector<DurationT> timings;  /// store time values as std::chrono::duration 
};



class SheepServer
{
public:
	SheepServer() {}
	SheepServer(utility::string_t url);

	pplx::task<void> open() { return m_listener.open(); }
	pplx::task<void> close() { return m_listener.close(); }

  template <typename PlaintextT>
  BaseContext<PlaintextT>* make_context(std::string context_type);

  template<typename PlaintextT>
  std::vector<PlaintextT> make_plaintext_inputs();

  template <typename PlaintextT>
  void configure_and_run();

  
private:
  /// generic methods - will then dispatch to specific ones based on URL
	void handle_get(http_request message);
	void handle_put(http_request message);
	void handle_post(http_request message);
  /// actual endpoints

        void handle_get_context(http_request message);
        void handle_get_input_type(http_request message);
        void handle_get_inputs(http_request message);
        void handle_get_parameters(http_request message);
        void handle_get_config(http_request message);
        void handle_get_results(http_request message);    
        void handle_get_job(http_request message);
        void handle_post_inputs(http_request message);    
        void handle_post_input_type(http_request message);
        void handle_post_circuit(http_request message);

        void handle_post_context(http_request message);

        void handle_post_job(http_request message);
        void handle_post_run(http_request message);  

        void handle_put_parameters(http_request message);  
        void handle_put_eval_strategy(http_request message);  
  /// listen to http requests
	http_listener m_listener;
  /// structs to store the configuration and results of a test.
        SheepJobConfig m_job_config;
        SheepJobResult m_job_result;
        bool m_job_finished;

};

#endif
