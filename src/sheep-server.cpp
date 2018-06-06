#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <random>
#include <sys/time.h>

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "sheep-server.hpp"


using namespace std;
using namespace web; 
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;
using namespace SHEEP;

SheepServer::SheepServer(utility::string_t url) : m_listener(url)//, m_context(0)
{
    m_job_config = {};  /// zero all fields
    m_job_config.setDefaults();
    m_job_result = {};
    m_job_finished = false;
    /// generic methods to listen for any request - the URL will be parsed within these functions.
    m_listener.support(methods::GET, std::bind(&SheepServer::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&SheepServer::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&SheepServer::handle_post, this, std::placeholders::_1));
   
}

template <typename PlaintextT>
BaseContext<PlaintextT>*
SheepServer::make_context(std::string context_type) {
  if (context_type == "HElib_F2") {
    return  new ContextHElib_F2<PlaintextT>();    
  } else if (context_type == "HElib_Fp") {
    return new ContextHElib_Fp<PlaintextT>();    
  } else if (context_type == "TFHE") {
    return  new ContextTFHE<PlaintextT>();
    //	} else if (context_type == "SEAL") {
    //    return  new ContextSeal<PlaintextT>();
  } else if (context_type == "Clear") {
    return new ContextClear<PlaintextT>();
  } else {
    throw std::runtime_error("Unknown context requested");
  }  
}

template<typename PlaintextT>
std::vector<PlaintextT>
SheepServer::make_plaintext_inputs() {
  std::vector<PlaintextT> inputs;
  for (auto input : m_job_config.input_vals) {
    inputs.push_back( (PlaintextT)(input) );
  }
  return inputs;
}


template <typename PlaintextT>
void
SheepServer::configure_and_run() {
  if (! m_job_config.isConfigured()) throw std::runtime_error("Job incompletely configured");
  /// we can now assume we have values for context, inputs, circuit, etc
  auto context = make_context<PlaintextT>(m_job_config.context);
  /// set parameters for this context
  for ( auto map_iter = m_job_config.parameters.begin(); map_iter != m_job_config.parameters.end(); ++map_iter) {
    context->set_parameter(map_iter->first, map_iter->second);
  }
  std::vector<PlaintextT> plaintext_inputs = make_plaintext_inputs<PlaintextT>();
  std::vector<PlaintextT> output_vals = context->eval_with_plaintexts(m_job_config.circuit,
  								      plaintext_inputs,
  								      m_job_result.timings,
  								      m_job_config.eval_strategy);
  cout<<" finished running job !! "<<endl;
  m_job_finished = true;
  ///  std::vector<std::string> output_
  for (int i=0; i < output_vals.size(); ++i ) {    
    std::cout<<" output_vals "<<std::to_string(output_vals[0])<<std::endl;
    m_job_result.outputs.insert(std::make_pair<const std::string,std::string>(m_job_config.circuit.get_outputs()[i].get_name(),
    									   std::to_string(output_vals[i])));
  }
}

void SheepServer::handle_get(http_request message)
{
  auto path = message.relative_uri().path();
  if (path == "context/") return handle_get_context(message);  
  else if (path == "parameters/") return handle_get_parameters(message);
  else if (path == "input_type/") return handle_get_input_type(message);
  else if (path == "inputs/") return handle_get_inputs(message);  
  else if (path == "job/") return handle_get_job(message);
  else if (path == "config/") return handle_get_config(message);
  else if (path == "results/") return handle_get_results(message);       
  message.reply(status_codes::InternalError,("Unrecognized request"));
};

void SheepServer::handle_post(http_request message)
{
  cout<<" in handle post " <<endl;
  auto path = message.relative_uri().path();
  cout<<" path is "<<path<<endl;
  if (path == "context/") return handle_post_context(message);  
  else if (path == "input_type/") return handle_post_input_type(message);
  else if (path == "inputs/") return handle_post_inputs(message);  
  else if (path == "job/") return handle_post_job(message);
  else if (path == "circuit/") return handle_post_circuit(message);
  else if (path == "run/") return handle_post_run(message);
  message.reply(status_codes::InternalError,("Unrecognized request"));  
};


void SheepServer::handle_put(http_request message)
{
  auto path = message.relative_uri().path();
  if (path == "parameters/") return handle_put_parameters(message);
  else if (path == "eval_strategy/") return handle_put_eval_strategy(message); 
  message.reply(status_codes::OK);
};

/////////////////////////////////////////////////////////////////////

void SheepServer::handle_post_run(http_request message) {
  
  /// get a context, configure it with the stored
  /// parameters, and run it.
  cout<<"Will run job!"<<endl;
  if (m_job_config.input_type == "bool") configure_and_run<bool>();
  else if (m_job_config.input_type == "uint8_t") configure_and_run<uint8_t>();
  else if (m_job_config.input_type == "uint16_t") configure_and_run<uint16_t>();
  else if (m_job_config.input_type == "uint32_t") configure_and_run<uint32_t>();
  else if (m_job_config.input_type == "uint8_t") configure_and_run<int8_t>();
  else if (m_job_config.input_type == "uint16_t") configure_and_run<int16_t>();
  else if (m_job_config.input_type == "uint32_t") configure_and_run<int32_t>();  
  message.reply(status_codes::OK);
}



void SheepServer::handle_post_circuit(http_request message) {
  /// set circuit filename to use
  cout<<" in handle post circuit" <<endl;
  bool found_circuit = false;
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      try {
	json::value val = jvalue.get();
	auto circuit_filename = val["circuit_filename"].as_string();
	std::cout<<" circuit is "<<circuit_filename<<std::endl;
	m_job_config.circuit_filename = circuit_filename;
         /// check that the file exists.
        std::ifstream circuit_file(std::string(m_job_config.circuit_filename));
        cout<<" is file good? "<<circuit_file.good()<<endl;
        if (circuit_file.good()) {
	  /// create the circuit
	  Circuit C;
	  circuit_file >> C;
	  cout << C;
	  m_job_config.circuit = C;
	}
      } catch(json::json_exception) {
	  message.reply(status_codes::InternalError,("Unrecognized circuit_filename request"));
      }      
    });
  message.reply(status_codes::OK);
}

void SheepServer::handle_get_inputs(http_request message) {
  
  /// check again that the circuitfile exists.
  std::ifstream circuit_file(std::string(m_job_config.circuit_filename));
  cout<<" is file good? "<<circuit_file.good()<<endl;
  if (! circuit_file.good())
    message.reply(status_codes::InternalError,("Circuit file not found"));
  json::value result = json::value::object();
  json::value inputs = json::value::array();
  int index = 0;
  for (auto input : m_job_config.circuit.get_inputs() ) {
    m_job_config.input_names.insert(input.get_name());
    inputs[index] = json::value::string(input.get_name());
    index++;
  }
  result["inputs"] = inputs;
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_post_inputs(http_request message) {
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      cout<<"in post inputs"<<endl;
      try {
	json::value input_dict = jvalue.get();
	//	auto input_dict = val["input_dict"].as_object();
	for (auto input_name : m_job_config.input_names) {
	  cout<<" looking for "<<input_name<<endl;
	  int input_val = input_dict[input_name].as_integer();
	  cout<<" input val is "<<input_val<<endl;
	  m_job_config.input_vals.push_back(input_val);
	}
      } catch(json::json_exception) {
	  message.reply(status_codes::InternalError,("Unrecognized inputs"));
      }      
    });
  message.reply(status_codes::OK);
}


void SheepServer::handle_get_context(http_request message) {
  /// list of available contexts?
  json::value result = json::value::object();
  json::value context_list = json::value::array();
  int index = 0;
  for (auto contextIter = available_contexts.begin();
       contextIter != available_contexts.end();
       ++contextIter) {
    context_list[index] = json::value::string(*contextIter);
    index++;
  }
  result["contexts"] = context_list;
  
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_get_input_type(http_request message) {
  /// list of available contexts?
  json::value result = json::value::object();
  json::value type_list = json::value::array();
  int index = 0;
  for (auto typeIter = available_input_types.begin();
       typeIter != available_input_types.end();
       ++typeIter) {
    type_list[index] = json::value::string(*typeIter);
    index++;
  }
  result["input_types"] = type_list;  
  message.reply(status_codes::OK, result);
}


void SheepServer::handle_post_context(http_request message) {
  /// set which context to use
  cout<<" in handle post context" <<endl;
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      try {
	json::value val = jvalue.get();
	auto context = val["context_name"].as_string();
	std::cout<<" context is "<<context<<std::endl;
	if (context != m_job_config.context) {
	  m_job_config.context = context;
	  m_job_config.parameters.clear();
	}
      } catch(json::json_exception) {
	  message.reply(status_codes::InternalError,("Unrecognized context request"));
      }      
    });
  message.reply(status_codes::OK);
}

void SheepServer::handle_post_input_type(http_request message) {
  /// set which input_type to use
  cout<<" in handle post input type" <<endl;
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      try {
	json::value val = jvalue.get();
	auto input_type = val["input_type"].as_string();
	std::cout<<" input_type is "<<input_type<<std::endl;
	m_job_config.input_type = input_type;
      } catch(json::json_exception) {
	  message.reply(status_codes::InternalError,("Unrecognized context request"));
      }      
    });
  message.reply(status_codes::OK);
}

void SheepServer::handle_put_eval_strategy(http_request message) {
  /// set which eval_strategy to use
  cout<<" in handle put eval_strategy" <<endl;
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      try {
	json::value val = jvalue.get();
	auto eval_strategy = val["eval_strategy"].as_string();
	std::cout<<" eval_strategy is "<<eval_strategy<<std::endl;
	if (eval_strategy == "parallel") 
	  m_job_config.eval_strategy = EvaluationStrategy::parallel;
	else
	  m_job_config.eval_strategy = EvaluationStrategy::serial;	  
	
      } catch(json::json_exception) {
	  message.reply(status_codes::InternalError,("Unable to set evaluation strategy"));
      }      
    });
  message.reply(status_codes::OK);
}



void SheepServer::handle_get_job(http_request message) {
  /// is the sheep job fully configured?
  bool configured = m_job_config.isConfigured();
  cout<<" in handle_get_job"<<endl;
  json::value result = json::value::object();
  result["job_configured"] = json::value::boolean(configured);
  message.reply(status_codes::OK, result);
}


void SheepServer::handle_post_job(http_request message) {
  // reset the job config and job result structs, we have a new job.
  cout<<"resetting config and results"<<endl;
  m_job_config = {};
  m_job_config.setDefaults();
  m_job_result = {};
  m_job_finished = false;
  message.reply(status_codes::OK);
}


void SheepServer::handle_get_parameters(http_request message) {
  
  /// if the job_config has some parameters set, then return them.
  /// Otherwise, create a new context, and get the default parameters.
  /// return a dict of parameters
  std::map<std::string, long&> param_map;
  if (m_job_config.parameters.size() > 0) 
    param_map = m_job_config.parameters;
  else {
    /// create a context, and get default parameters
    if (m_job_config.input_type == "bool") {
      auto context = make_context<bool>(m_job_config.context);
      param_map = context->get_parameters();
      delete context;
    } else if (m_job_config.input_type == "uint8_t") {
      auto context = make_context<uint8_t>(m_job_config.context);
      param_map = context->get_parameters();
    } else if (m_job_config.input_type == "uint16_t") {
      auto context = make_context<uint16_t>(m_job_config.context);
      param_map = context->get_parameters();
    } else if (m_job_config.input_type == "uint32_t") {
      auto context = make_context<uint32_t>(m_job_config.context);
      param_map = context->get_parameters();
    } else if (m_job_config.input_type == "int8_t") {
      auto context = make_context<int8_t>(m_job_config.context);
      param_map = context->get_parameters();
    } else if (m_job_config.input_type == "int16_t") {
      auto context = make_context<int16_t>(m_job_config.context);
      param_map = context->get_parameters();
    } else if (m_job_config.input_type == "int32_t") {
      auto context = make_context<int32_t>(m_job_config.context);
      param_map = context->get_parameters();
    }
    m_job_config.parameters = param_map;
  }
  json::value result = json::value::object();
  json::value param_list = json::value::array();
  int index = 0;
  for ( auto map_iter = param_map.begin(); map_iter != param_map.end(); ++map_iter) {
    json::value param = json::value::object();
    param[map_iter->first] = json::value::number((int64_t)map_iter->second);
    param_list[index] = param;
    index++;
  }
  
  result["parameters"] = param_list;
  
  message.reply(status_codes::OK, result);  
}


void SheepServer::handle_get_config(http_request message) {

  json::value result = json::value::object();
  //  json::value config_list = json::value::array();
  //int index = 0;
  json::value circuit_filename = json::value::object();
  result["circuit_filename"] = json::value::string(m_job_config.circuit_filename);
  
  /*  for ( auto map_iter = param_map.begin(); map_iter != param_map.end(); ++map_iter) {
    json::value param = json::value::object();
    param[map_iter->first] = json::value::number((int64_t)map_iter->second);
    param_list[index] = param;
    index++;
  }
*/
/// result["parameters"] = param_list;
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_put_parameters(http_request message) {
  /// put parameter name:value into job_config
  message.reply(status_codes::OK);
}

void SheepServer::handle_get_results(http_request message) {

  cout<<"in get_results"<<endl;
  //  if (! m_job_finished) message.reply(status_codes::InternalError,("Job not yet finished"));
  // return;
  cout<<"creating the results json"<<endl;
  json::value result = json::value::object();
  json::value outputs = json::value::object();
  for ( auto map_iter = m_job_result.outputs.begin(); map_iter != m_job_result.outputs.end(); ++map_iter) {
    outputs[map_iter->first] = json::value::string(map_iter->second);
  }
  result["outputs"] = outputs;
  message.reply(status_codes::OK, result);
}



