#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <thread>

#include <complex>
#include <cmath>

using namespace std::complex_literals;


#include "sheep-server.hpp"
#include "protect-eval.hpp"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;
using namespace SHEEP;


typedef std::chrono::duration<double, std::micro>
    Duration;  /// for storing execution times

/// constructor - bind listener to endpoints and create empty structs for config
/// and result
SheepServer::SheepServer(utility::string_t url) : m_listener(url) {
  m_job_config = {};  /// zero all fields
  m_job_config.setDefaults();
  m_job_result = {};
  m_job_finished = false;
  /// bind generic methods to listen for any request - the URL will be parsed
  /// within these functions.
  m_listener.support(methods::GET, std::bind(&SheepServer::handle_get, this,
                                             std::placeholders::_1));
  m_listener.support(methods::PUT, std::bind(&SheepServer::handle_put, this,
                                             std::placeholders::_1));
  m_listener.support(methods::POST, std::bind(&SheepServer::handle_post, this,
                                              std::placeholders::_1));

  //// what contexts are supported?
  m_available_contexts.push_back("Clear");
#ifdef HAVE_HElib
  m_available_contexts.push_back("HElib_F2");
  m_available_contexts.push_back("HElib_Fp");
#endif
#ifdef HAVE_TFHE
  m_available_contexts.push_back("TFHE");
#endif
#ifdef HAVE_SEAL_BFV
  m_available_contexts.push_back("SEAL_BFV");
#endif
#ifdef HAVE_SEAL_CKKS
  m_available_contexts.push_back("SEAL_CKKS");
#endif
#ifdef HAVE_LP
  m_available_contexts.push_back("LP");
#endif

  //// what input_types are supported?
  m_available_input_types = {"bool",   "uint8_t", "uint16_t", "uint32_t",
                             "int8_t", "int16_t", "int32_t", "double", "complex"};
}



template <typename PlaintextT>
std::string SheepServer::convert_to_string(PlaintextT t) {
  return std::to_string(t);
}

template <>
std::string SheepServer::convert_to_string(std::complex<double> t) {
  return std::to_string(t.real()) + " + " + std::to_string(t.imag()) + "i";
}



/// templated functions to interact with the contexts.

template <typename PlaintextT>
BaseContext<PlaintextT> *SheepServer::make_context(std::string context_type) {
  if (context_type == "Clear") {
    return new ContextClear<PlaintextT>();

#ifdef HAVE_HElib
  } else if (context_type == "HElib_F2") {
    return new ContextHElib_F2<PlaintextT>();
  } else if (context_type == "HElib_Fp") {
    return new ContextHElib_Fp<PlaintextT>();
#endif

#ifdef HAVE_TFHE
  } else if (context_type == "TFHE") {
    return new ContextTFHE<PlaintextT>();
#endif

#ifdef HAVE_SEAL_BFV
  } else if (context_type == "SEAL_BFV") {
    return new ContextSealBFV<PlaintextT>();
#endif

#ifdef HAVE_SEAL_CKKS
  } else if ((context_type == "SEAL_CKKS") &&
	     ((std::is_same<PlaintextT, double>::value  ) ||
	      (std::is_same<PlaintextT, std::complex<double>>::value ))) {
    return new ContextSealCKKS<PlaintextT>();
#endif

#ifdef HAVE_LP
  } else if (context_type == "LP") {
    return new ContextLP<PlaintextT>();
#endif

  } else {
    throw std::runtime_error("Unknown context requested");
  }
}

template <typename PlaintextT>
PlaintextT SheepServer::convert_to_plaintext(std::string ptString) {
  return (PlaintextT)(std::stoi(ptString));
}

template <>
double SheepServer::convert_to_plaintext(std::string ptString) {
  return (double)(std::stod(ptString));
}


template <typename PlaintextT>
std::vector<std::vector<PlaintextT>> SheepServer::make_plaintext_inputs() {
  std::vector<std::vector<PlaintextT>> inputs;

  for (auto input_wire : m_job_config.circuit.get_inputs()) {
    std::string input_name = input_wire.get_name();
    std::vector<PlaintextT> input_vector;
    for (auto input_string : m_job_config.input_vals[input_name]) {
      input_vector.push_back(convert_to_plaintext<PlaintextT>(input_string));
    }
    inputs.push_back(input_vector);
  }

  return inputs;
}


std::vector<long> SheepServer::make_const_plaintext_inputs() {
  std::vector<long> const_inputs;

  for (auto input_wire : m_job_config.circuit.get_const_inputs()) {
    std::string input_name = input_wire.get_name();
    const_inputs.push_back(m_job_config.const_input_vals[input_name]);
  }

  return const_inputs;
}


//// populate the stored m_job_config.parameters map
void SheepServer::get_parameters() {
  if (m_job_config.parameters.size() == 0) {
    /// call a function that will create a context and set
    /// m_job_config.parameters to default values

    if (m_job_config.input_type == "bool")
      update_parameters<bool>(m_job_config.context);
    if (m_job_config.input_type == "uint8_t")
      update_parameters<uint8_t>(m_job_config.context);
    if (m_job_config.input_type == "uint16_t")
      update_parameters<uint16_t>(m_job_config.context);
    if (m_job_config.input_type == "uint32_t")
      update_parameters<uint32_t>(m_job_config.context);
    if (m_job_config.input_type == "int8_t")
      update_parameters<int8_t>(m_job_config.context);
    if (m_job_config.input_type == "int16_t")
      update_parameters<int16_t>(m_job_config.context);
    if (m_job_config.input_type == "int32_t")
      update_parameters<int32_t>(m_job_config.context);
    if (m_job_config.input_type == "double")
      update_parameters<double>(m_job_config.context);
    if (m_job_config.input_type == "complex")
      update_parameters<std::complex<double>>(m_job_config.context);
  }
}

template <typename PlaintextT>
void SheepServer::update_parameters(std::string context_type,
                                    json::value parameters)
{
  BaseContext<PlaintextT> *context = make_context<PlaintextT>(context_type);

  size_t nparams = context->get_parameters().size();

  SharedBuffer<size_t> nslots_shared;
  SharedBuffer<long> new_param_vals_shared(nparams);

  int status = protect_eval(60L, [&](){
      /// first set parameters to current values stored in the server (if any)
      for (auto map_iter : m_job_config.parameters) {
	context->set_parameter(map_iter.first, map_iter.second);
      }
      /// update parameters if specified
      auto params = parameters.as_object();
      for (auto p : params) {
	std::string param_name = p.first;
	long param_value = (long)(p.second.as_integer());
	context->set_parameter(param_name, param_value);
      }
      /// apply the new parameters
      context->configure();
      // update the servers param map.
      int i = 0;
      for (auto& param : context->get_parameters()) {
	// place the parameter value only in the shared buffer
	// (iteration is in sorted order, so implicitly know the key
	// to restore later).
	new_param_vals_shared[i++] = param.second;
      }
      // now find out how many slots this context/parameter-set supports
      // and update the job config
      *nslots_shared = (int)(context->get_num_slots());
    });

  if (status == PE_TIMEOUT) {
    throw std::runtime_error("Timed out when setting parameters");
  } else if (status) {
    throw std::runtime_error("Setting parameters failed");
  }

  // extract various things from shared memory

  // set number of slots
  m_job_config.nslots = *nslots_shared;

  // set parameter values
  int i = 0;
  for (auto& p : context->get_parameters()) {
    m_job_config.parameters[p.first] = new_param_vals_shared[i++];
  }

  // cleanup
  delete context;
}

template <typename PlaintextT>
bool SheepServer::check_job_outputs(
    std::vector<std::vector<PlaintextT>> test_outputs,
    std::vector<std::vector<PlaintextT>> clear_outputs) {
  if (test_outputs.size() != clear_outputs.size()) return false;

  for (int i = 0; i < test_outputs.size(); i++) {
    for (int j = 0; j < test_outputs[i].size(); j++) {
      if (test_outputs[i][j] != clear_outputs[i][j]) return false;
    }
  }
  return true;
}

template <typename PlaintextT>
int SheepServer::configure_and_serialize(std::vector<int> inputvec) {
  /// Return the size of the serialized ciphertext (string itself is not much use)
  size_t ct_buffer_size = 1;
  SharedBuffer<int> serialized_ct_shared(ct_buffer_size);

  int status = protect_eval(60L, [&](){
      /// convert inputvec to plaintext type
      std::vector<PlaintextT> ptvec;
      for (auto input_val : inputvec) {
	ptvec.push_back((PlaintextT)input_val);
      }
      /// we can  assume we have values for context, input_type

      auto context = make_context<PlaintextT>(m_job_config.context);
      /// set parameters for this context
      for (auto map_iter = m_job_config.parameters.begin();
	   map_iter != m_job_config.parameters.end(); ++map_iter) {
	context->set_parameter(map_iter->first, map_iter->second);

      }
      /// apply the new parameters
      context->configure();
      std::string serialized_ct = context->encrypt_and_serialize(ptvec);
      int sct_size = serialized_ct.size();
      serialized_ct_shared[0] = sct_size;
    });

  if (status == PE_TIMEOUT) {
    throw std::runtime_error("Timed out");
  } else if (status) {
    throw std::runtime_error("Error performing serialization");
  } else {
    return serialized_ct_shared[0];
  }
}


template <typename PlaintextT>
void SheepServer::configure_and_run(http_request message) {
  if (!m_job_config.isConfigured())
    throw std::runtime_error("Job incompletely configured");

  std::vector<std::vector<PlaintextT>> plaintext_inputs =
    make_plaintext_inputs<PlaintextT>();
  std::vector<long> const_plaintext_inputs =
    make_const_plaintext_inputs();

  // shared memory region for returning the results
  size_t n_gates = m_job_config.circuit.get_assignments().size();
  size_t n_outputs = m_job_config.circuit.get_outputs().size();
  // All the inputs should be the same length, thus we can check only the first element
  size_t slot_cnt = plaintext_inputs[0].size();
  size_t n_plaintexts_out = slot_cnt * n_outputs;
  size_t n_summary_timings = 3;
  size_t n_timings = n_summary_timings + n_gates;

  try {
    SharedBuffer<Duration> timings_shared(n_timings);
    SharedBuffer<PlaintextT> outputs_shared(n_plaintexts_out);

    int status = protect_eval(m_job_config.timeout, [&](){

	/// we can now assume we have values for context, inputs, circuit, etc
	auto context = make_context<PlaintextT>(m_job_config.context);
	/// set parameters for this context
	for (auto map_iter = m_job_config.parameters.begin();
	     map_iter != m_job_config.parameters.end(); ++map_iter) {
	  context->set_parameter(map_iter->first, map_iter->second);
	}
	/// apply the new parameters
	context->configure();

	std::vector<Duration> totalTimings;
	std::map<std::string, Duration> perGateTimings;
	auto timings = std::make_pair(totalTimings, perGateTimings);
	//define the timeout
	std::chrono::duration<double, std::micro> timeout_micro(1000000.0 * m_job_config.timeout);

	std::vector<std::vector<PlaintextT>> output_vals =
	context->eval_with_plaintexts(m_job_config.circuit, plaintext_inputs,
                                      const_plaintext_inputs, timings,
                                      m_job_config.eval_strategy,
				      timeout_micro);

	if (timings.first.size() != n_summary_timings) {
	  // signal an error to the server
	  std::cerr << "Child exiting: more than three timings reported!\n";
	  _exit(1);
	}
	// insert the various things in the shared memory buffer

	for (int i = 0; i < n_summary_timings; i++) {
	  timings_shared[i] = timings.first[i];
	}
	/// now we need to get the assignments in the order returned
	/// by the circuit, so we can match up with the timings from
	/// the per-gate timing map.
	int timing_index = n_summary_timings;
	for (int i=0; i < n_gates; i++) {
	  std::string gate_name = m_job_config.circuit.get_assignments()[i].get_output().get_name();
	  timings_shared[timing_index] = timings.second[gate_name];
	  //	for (auto gate_timing : timings.second) {
	  // timings_shared[timing_index] = gate_timing.second;
	  timing_index++;
	}

	for (int i = 0; i < n_outputs; i++) {
	  for (int j = 0; j < slot_cnt; j++) {
	    outputs_shared[i * slot_cnt + j] = output_vals[i][j];
	  }
	}
	std::cerr << "successful evaluation: exiting...\n";
      });

    if (status == PE_TIMEOUT) {
      // this is an error (eval's own timeout should have stopped it)
      message.reply(status_codes::InternalError, ("Evaluation timed out"));
      m_job_finished = false;

    } else if (status) {
      // other abnormal termination (nonzero return or killed by signal)
      message.reply(status_codes::InternalError,
                    ("Evaluation terminated abnormally"));
      m_job_finished = false;

    } else {
      // child exited normally => evaluation completed
      m_job_finished = true;

      /// We will report the output vals as strings, but we also
      /// make a vector<vector<plaintext> > called test_output_vals
      /// in order to validate against the ClearContext

      std::vector<std::vector<PlaintextT>> test_output_vals;

      for (int i = 0; i < n_outputs; i++) {
        std::vector<PlaintextT> output_val;
        std::vector<std::string> string_val;

        for (int j = 0; j < slot_cnt; j++) {
          output_val.push_back(outputs_shared[i * slot_cnt + j]);

          ///  store outputs values as strings, to avoid ambiguity about type.
          string_val.push_back(convert_to_string<PlaintextT>(output_val[j]));
        }

        test_output_vals.push_back(output_val);

        auto output = std::make_pair(
            m_job_config.circuit.get_outputs()[i].get_name(), string_val);

        m_job_result.outputs.insert(output);
      }

      auto encryption = std::make_pair<std::string, std::string>(
          "encryption", std::to_string(timings_shared[0].count()));
      m_job_result.timings.insert(encryption);

      auto evaluation = std::make_pair<std::string, std::string>(
          "evaluation", std::to_string(timings_shared[1].count()));
      m_job_result.timings.insert(evaluation);

      auto decryption = std::make_pair<std::string, std::string>(
          "decryption", std::to_string(timings_shared[2].count()));
      m_job_result.timings.insert(decryption);

      for (int i=0; i < n_gates; i++) {
	auto gate_time = std::make_pair(
	   m_job_config.circuit.get_assignments()[i].get_output().get_name(),
	   std::to_string(timings_shared[n_summary_timings+i].count()));
	m_job_result.timings.insert(gate_time);
      }

      //// now do the plaintext evaluation
      auto clear_context = make_context<PlaintextT>("Clear");
      clear_context->set_parameter("NumSlots",slot_cnt);

      std::vector<std::vector<PlaintextT>> clear_output_vals =
          clear_context->eval_with_plaintexts(m_job_config.circuit,
					      plaintext_inputs,
					      const_plaintext_inputs);

      // Compare the encrypted and plain results
      bool is_correct =
          check_job_outputs<PlaintextT>(test_output_vals, clear_output_vals);
      m_job_result.is_correct = is_correct;

      message.reply(status_codes::OK);
    }
  } catch (std::bad_alloc& e) {
    message.reply(status_codes::InternalError, ("Could not run evaluation"));
  }
}

void SheepServer::handle_get(http_request message) {
  auto path = message.relative_uri().path();
  if (path == "context/")
    return handle_get_context(message);
  else if (path == "circuit/")
    return handle_get_circuit(message);
  else if (path == "parameters/")
    return handle_get_parameters(message);
  else if (path == "input_type/")
    return handle_get_input_type(message);
  else if (path == "inputs/")
    return handle_get_inputs(message);
  else if (path == "const_inputs/")
    return handle_get_const_inputs(message);
  else if (path == "job/")
    return handle_get_job(message);
  else if (path == "config/")
    return handle_get_config(message);
  else if (path == "slots/")
    return handle_get_slots(message);
  else if (path == "results/")
    return handle_get_results(message);
  else if (path == "eval_strategy/")
    return handle_get_eval_strategy(message);
  else
    message.reply(status_codes::InternalError, ("Unrecognized request"));
};

void SheepServer::handle_post(http_request message) {
  auto path = message.relative_uri().path();
  if (path == "context/")
    return handle_post_context(message);
  else if (path == "input_type/")
    return handle_post_input_type(message);
  else if (path == "inputs/")
    return handle_post_inputs(message);
  else if (path == "const_inputs/")
    return handle_post_const_inputs(message);
  else if (path == "serialized_ct/")
    return handle_post_serialized_ciphertext(message);
  else if (path == "serialized_ct_size/")
    return handle_post_serialized_ciphertext(message);
  else if (path == "job/")
    return handle_post_job(message);
  else if (path == "circuitfile/")
    return handle_post_circuitfile(message);
  else if (path == "circuit/")
    return handle_post_circuit(message);
  else if (path == "run/")
    return handle_post_run(message);
  else
    message.reply(status_codes::InternalError, ("Unrecognized request"));
};

void SheepServer::handle_put(http_request message) {
  auto path = message.relative_uri().path();
  if (path == "parameters/")
    return handle_put_parameters(message);
  else if (path == "eval_strategy/")
    return handle_put_eval_strategy(message);
  else if (path == "timeout/")
    return handle_put_timeout(message);
  message.reply(status_codes::OK);
};

void SheepServer::handle_post_run(http_request message) {
  /// get a context, configure it with the stored
  /// parameters, and run it.

  if (m_job_config.input_type == "bool")
    configure_and_run<bool>(message);
  else if (m_job_config.input_type == "uint8_t")
    configure_and_run<uint8_t>(message);
  else if (m_job_config.input_type == "uint16_t")
    configure_and_run<uint16_t>(message);
  else if (m_job_config.input_type == "uint32_t")
    configure_and_run<uint32_t>(message);
  else if (m_job_config.input_type == "int8_t")
    configure_and_run<int8_t>(message);
  else if (m_job_config.input_type == "int16_t")
    configure_and_run<int16_t>(message);
  else if (m_job_config.input_type == "int32_t")
    configure_and_run<int32_t>(message);
  else if (m_job_config.input_type == "double")
    configure_and_run<double>(message);
  else if (m_job_config.input_type == "complex")
    configure_and_run<std::complex<double> >(message);
  else
    message.reply(status_codes::InternalError, ("Unknown input type"));
}

void SheepServer::handle_post_circuit(http_request message) {
  ///
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value val = jvalue.get();
      auto circuit = val["circuit"].as_string();
      std::stringstream circuit_stream((std::string)circuit);
      /// create the circuit
      Circuit C;
      try {
	circuit_stream >> C;
	m_job_config.circuit = C;
      } catch (const std::exception& e) {
	std::cerr<<"Caught exception when reading circuit"<<std::endl;
      };
    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unrecognized circuit request"));
    }
  });
  /// did we set the circuit OK?
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  bool circuit_OK = m_job_config.circuit.get_inputs().size() > 0;
  if (circuit_OK) message.reply(status_codes::OK);
  else message.reply(status_codes::InternalError,
		     ("Bad circuit - undefined or multiply defined inputs"));

}

void SheepServer::handle_post_circuitfile(http_request message) {
  /// set circuit filename to use
  bool found_circuit = false;
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value val = jvalue.get();
      auto circuit_filename = val["circuit_filename"].as_string();
      m_job_config.circuit_filename = circuit_filename;
      /// check that the file exists.
      std::ifstream circuit_file(std::string(m_job_config.circuit_filename));
      if (circuit_file.good()) {
        /// create the circuit
        Circuit C;
        circuit_file >> C;
        m_job_config.circuit = C;
      }
    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unrecognized circuit_filename request"));
    }
  });
  message.reply(status_codes::OK);
}

void SheepServer::handle_get_inputs(http_request message) {

  json::value result = json::value::object();
  json::value inputs = json::value::array();

  int index = 0;
  for (auto input : m_job_config.circuit.get_inputs()) {
    m_job_config.input_names.insert(input.get_name());
    inputs[index] = json::value::string(input.get_name());
    index++;
  }
  result["inputs"] = inputs;

  message.reply(status_codes::OK, result);
}

void SheepServer::handle_get_const_inputs(http_request message) {
  /// check again that the circuit exists.
  //  if (! circuit_file.good())
  //   message.reply(status_codes::InternalError,("Circuit file not found"));
  json::value result = json::value::object();
  json::value const_inputs = json::value::array();

  int index = 0;
  for (auto const_input : m_job_config.circuit.get_const_inputs()) {
    m_job_config.const_input_names.insert(const_input.get_name());
    const_inputs[index] = json::value::string(const_input.get_name());
    index++;
  }
  result["const_inputs"] = const_inputs;

  message.reply(status_codes::OK, result);
}

void SheepServer::handle_post_inputs(http_request message) {
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {

      json::value input_dict = jvalue.get();
      for (auto input_name : m_job_config.input_names) {

        std::vector<std::string> input_vals;
        for (auto input : input_dict[input_name].as_array()) {
	  std::string input_val = input.as_string();
          input_vals.push_back(input_val);
        }
        m_job_config.input_vals[input_name] = input_vals;
      }
    } catch (json::json_exception) {
      message.reply(status_codes::InternalError, ("Unrecognized inputs"));
    }
  });

  message.reply(status_codes::OK);
}

void SheepServer::handle_post_const_inputs(http_request message) {
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value input_dict = jvalue.get();

      for (auto input_name : m_job_config.const_input_names) {
        int input_val = input_dict[input_name].as_integer();
	m_job_config.const_input_vals[input_name] = input_val;
      }
    } catch (json::json_exception) {
      message.reply(status_codes::InternalError, ("Unrecognized inputs"));
    }
  });

  message.reply(status_codes::OK);
}


void SheepServer::handle_post_serialized_ciphertext(http_request message) {

  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      try {
	json::value input_dict = jvalue.get();
	auto input_vals = input_dict["inputs"].as_array();
	std::vector<int> plaintext_inputs;
	for (auto input_val : input_vals) {
	  plaintext_inputs.push_back(input_val.as_integer());
	}
	int sct;

	if (m_job_config.input_type == "bool")
	  sct = configure_and_serialize<bool>(plaintext_inputs);
	else if (m_job_config.input_type == "uint8_t")
	  sct = configure_and_serialize<uint8_t>(plaintext_inputs);
	else if (m_job_config.input_type == "uint16_t")
	  sct = configure_and_serialize<uint16_t>(plaintext_inputs);
	else if (m_job_config.input_type == "uint32_t")
	  sct = configure_and_serialize<uint32_t>(plaintext_inputs);
	else if (m_job_config.input_type == "int8_t")
	  sct = configure_and_serialize<int8_t>(plaintext_inputs);
	else if (m_job_config.input_type == "int16_t")
	  sct = configure_and_serialize<int16_t>(plaintext_inputs);
	else if (m_job_config.input_type == "int32_t")
	  sct = configure_and_serialize<int32_t>(plaintext_inputs);
	else if (m_job_config.input_type == "double")
	  sct = configure_and_serialize<double>(plaintext_inputs);
	else if (m_job_config.input_type == "complex")
	  sct = configure_and_serialize<std::complex<double> >(plaintext_inputs);
	else
	  message.reply(status_codes::InternalError, ("Unknown input type"));
	json::value result = json::value::object();
	result["size"] = json::value::number((int64_t)(sct));

	message.reply(status_codes::OK, result);

      } catch (json::json_exception&) {
	message.reply(status_codes::InternalError, ("Unrecognized inputs"));
      } catch (std::bad_alloc&) {
	message.reply(status_codes::InternalError, ("Could not allocate shared memory"));
      } catch (std::runtime_error& e) {
	message.reply(status_codes::InternalError, (e.what()));
      }
    });
}


void SheepServer::handle_get_eval_strategy(http_request message) {
  /// get the evaluation strategy
  json::value result = json::value::object();
  if (m_job_config.eval_strategy == EvaluationStrategy::parallel) {
    result["eval_strategy"] = json::value::string("parallel");
  } else {
    result["eval_strategy"] = json::value::string("serial");
  }
  message.reply(status_codes::OK, result);
}


void SheepServer::handle_get_context(http_request message) {
  /// list of available contexts?
  json::value result = json::value::object();
  json::value context_list = json::value::array();
  int index = 0;
  for (auto contextIter = m_available_contexts.begin();
       contextIter != m_available_contexts.end(); ++contextIter) {
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
  for (auto typeIter = m_available_input_types.begin();
       typeIter != m_available_input_types.end(); ++typeIter) {
    type_list[index] = json::value::string(*typeIter);
    index++;
  }
  result["input_types"] = type_list;
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_post_context(http_request message) {
  /// set which context to use.  If it has changed, reset the list of
  /// parameters.
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value val = jvalue.get();
      auto context = val["context_name"].as_string();
      if (context != m_job_config.context) {
        m_job_config.context = context;
        m_job_config.parameters.clear();
      }
    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unrecognized context request"));
    }
  });
  message.reply(status_codes::OK);
}

void SheepServer::handle_post_input_type(http_request message) {
  /// set which input_type to use
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value val = jvalue.get();
      auto input_type = val["input_type"].as_string();
      m_job_config.input_type = input_type;
    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unrecognized context request"));
    }
  });
  message.reply(status_codes::OK);
}

void SheepServer::handle_put_eval_strategy(http_request message) {
  /// set which eval_strategy to use
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value val = jvalue.get();
      auto eval_strategy = val["eval_strategy"].as_string();
      if (eval_strategy == "parallel")
        m_job_config.eval_strategy = EvaluationStrategy::parallel;
      else
        m_job_config.eval_strategy = EvaluationStrategy::serial;

    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unable to set evaluation strategy"));
    }
  });
  message.reply(status_codes::OK);
}

void SheepServer::handle_put_timeout(http_request message) {
  /// set which eval_strategy to use
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value val = jvalue.get();
      auto timeout = val["timeout"].as_integer();

      m_job_config.timeout = timeout;

    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unable to set timeout"));
    }
  });
  message.reply(status_codes::OK);
}


void SheepServer::handle_get_job(http_request message) {
  /// is the sheep job fully configured?

  bool configured = m_job_config.isConfigured();
  json::value result = json::value::object();
  result["job_configured"] = json::value::boolean(configured);
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_post_job(http_request message) {
  // reset the job config and job result structs, we have a new job.
  m_job_config = {};
  m_job_config.setDefaults();
  m_job_result = {};
  m_job_finished = false;
  message.reply(status_codes::OK);
}

void SheepServer::handle_get_parameters(http_request message) {
  /// if the job_config has some parameters set, then return them.
  /// Otherwise, create a new context, and get the default parameters.
  /// return a dict of parameters.
  /// input_type needs to be set already otherwise we can't instantiate context.
  if ((m_job_config.input_type.size() == 0) ||
      (m_job_config.context.size() == 0)) {
    message.reply(
        status_codes::InternalError,
        ("Need to set input_type and context before getting parameters"));
    return;
  }
  /// call the function to populate m_job_config.parameters
  get_parameters();
  /// build a json object out of it.
  std::map<std::string, long> param_map = m_job_config.parameters;
  json::value result = json::value::object();
  for (auto map_iter = param_map.begin(); map_iter != param_map.end();
       ++map_iter) {
    result[map_iter->first] = json::value::number((int64_t)map_iter->second);
  }

  message.reply(status_codes::OK, result);
}

void SheepServer::handle_get_slots(http_request message) {
  /// if the job_config has some parameters set, then return them.
  /// Otherwise, create a new context, and get the default parameters.
  /// return a dict of parameters.
  /// input_type needs to be set already otherwise we can't instantiate context.
  if ((m_job_config.input_type.size() == 0) ||
      (m_job_config.context.size() == 0)) {
    message.reply(
        status_codes::InternalError,
        ("Need to set input_type and context before getting num slots"));
    return;
  }
  /// call the get_parameters() function to populate m_job_config.nslots
  get_parameters();
  /// build a json object out of it.
  json::value result = json::value::object();
  result["nslots"] = json::value::number(m_job_config.nslots);
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_get_config(http_request message) {
  /// if we haven't already got the parameters, do this now.
  if (m_job_config.parameters.size() == 0) get_parameters();
  json::value result = m_job_config.as_json();
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_get_circuit(http_request message) {
  /// if we haven't already got the parameters, do this now.
  if (m_job_config.circuit.get_inputs().size() <= 0) {
    message.reply(
        status_codes::InternalError,
        ("Circuit not set"));
    return;
  }
  std::stringstream circuit_stream;
  circuit_stream << m_job_config.circuit;
  std::string circuit_string = circuit_stream.str();
  json::value result = json::value::object();
  result["circuit"] = json::value::string(circuit_string);
  message.reply(status_codes::OK, result);

}



void SheepServer::handle_put_parameters(http_request message) {
  /// put parameter name:value into job_config
  if ((m_job_config.input_type.size() == 0) ||
      (m_job_config.context.size() == 0)) {
    message.reply(
        status_codes::InternalError,
        ("Need to specify input_type and context before setting parameters"));
    return;
  }
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
    try {
      json::value params = jvalue.get();
      /// reset our existing map
      m_job_config.parameters.clear();

      if (m_job_config.input_type == "bool")
        update_parameters<bool>(m_job_config.context, params);
      else if (m_job_config.input_type == "uint8_t")
        update_parameters<uint8_t>(m_job_config.context, params);
      else if (m_job_config.input_type == "uint16_t")
        update_parameters<uint16_t>(m_job_config.context, params);
      else if (m_job_config.input_type == "uint32_t")
        update_parameters<uint32_t>(m_job_config.context, params);
      else if (m_job_config.input_type == "int8_t")
        update_parameters<int8_t>(m_job_config.context, params);
      else if (m_job_config.input_type == "int16_t")
        update_parameters<int16_t>(m_job_config.context, params);
      else if (m_job_config.input_type == "int32_t")
        update_parameters<int32_t>(m_job_config.context, params);
      else if (m_job_config.input_type == "double")
        update_parameters<double>(m_job_config.context, params);
      else if (m_job_config.input_type == "complex")
        update_parameters<std::complex<double> >(m_job_config.context, params);
      else
        message.reply(status_codes::InternalError,
                      ("Unknown input type when updating parameters"));

      message.reply(status_codes::OK);

    } catch (json::json_exception&) {
      message.reply(status_codes::InternalError,
                    ("Unable to set evaluation strategy"));
    } catch(std::runtime_error& e) {
      message.reply(status_codes::InternalError, e.what());
    }
  });
}

void SheepServer::handle_get_results(http_request message) {
  json::value result = m_job_result.as_json();

  message.reply(status_codes::OK, result);
}
