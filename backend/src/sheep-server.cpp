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

#include "sheep-server.hpp"

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
#ifdef HAVE_SEAL
  m_available_contexts.push_back("SEAL");
#endif

#ifdef HAVE_LP
  m_available_contexts.push_back("LP");
#endif

  //// what input_types are supported?
  m_available_input_types = {"bool",   "uint8_t", "uint16_t", "uint32_t",
                             "int8_t", "int16_t", "int32_t"};
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

#ifdef HAVE_SEAL
  } else if (context_type == "SEAL") {
    return new ContextSeal<PlaintextT>();
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
std::vector<std::vector<PlaintextT>> SheepServer::make_plaintext_inputs() {
  std::vector<std::vector<PlaintextT>> inputs;

  // TODO: input from json

  for (auto input : m_job_config.input_vals) {
    std::vector<PlaintextT> input_vector(begin(input), end(input));

    inputs.push_back(input_vector);
  }

  return inputs;
}


template <typename PlaintextT>
std::vector<PlaintextT> SheepServer::make_const_plaintext_inputs() {
  std::vector<PlaintextT> const_inputs;

  for (auto input : m_job_config.const_input_vals) {
    const_inputs.push_back(input);
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
  }
}

template <typename PlaintextT>
void SheepServer::update_parameters(std::string context_type,
                                    json::value parameters) {

  BaseContext<PlaintextT> *context = make_context<PlaintextT>(context_type);

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
  m_job_config.parameters = context->get_parameters();
  // now find out how many slots this context/parameter-set supports
  // and update the job config
  m_job_config.nslots = (int)(context->get_num_slots());
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
std::string SheepServer::configure_and_serialize(std::vector<int> inputvec) {
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
  std::string serialized_ct = context->encrypt_and_serialize(ptvec);
  return serialized_ct;
}


template <typename PlaintextT>
void SheepServer::configure_and_run(http_request message) {
  if (!m_job_config.isConfigured())
    throw std::runtime_error("Job incompletely configured");
  /// we can now assume we have values for context, inputs, circuit, etc
  auto context = make_context<PlaintextT>(m_job_config.context);
  /// set parameters for this context
  for (auto map_iter = m_job_config.parameters.begin();
       map_iter != m_job_config.parameters.end(); ++map_iter) {
    context->set_parameter(map_iter->first, map_iter->second);
  }

  std::vector<std::vector<PlaintextT>> plaintext_inputs =
      make_plaintext_inputs<PlaintextT>();
  std::vector<PlaintextT> const_plaintext_inputs =
      make_const_plaintext_inputs<PlaintextT>();

  // shared memory region for returning the results
  size_t n_outputs = m_job_config.circuit.get_outputs().size();
  size_t n_timings = 3;

  Duration *timings_shared = (Duration *)mmap(
      NULL, n_timings * sizeof(Duration), PROT_READ | PROT_WRITE,
      MAP_ANONYMOUS | MAP_SHARED, 0, 0);

  if (timings_shared == MAP_FAILED) {
    message.reply(status_codes::InternalError, ("Could not run evaluation"));
    return;
  }

  // All the inputs should be the same length, thus we can check only the first
  // element
  int slot_cnt = plaintext_inputs[0].size();

  PlaintextT *outputs_shared = (PlaintextT *)mmap(
      NULL, slot_cnt * (n_outputs * sizeof(PlaintextT)), PROT_READ | PROT_WRITE,
      MAP_ANONYMOUS | MAP_SHARED, 0, 0);

  if (outputs_shared == MAP_FAILED) {
    message.reply(status_codes::InternalError, ("Could not run evaluation"));
    munmap(timings_shared, n_timings);
    return;
  }

  pid_t child_pid = fork();

  if (child_pid == -1) {
    // fork did not succeed
    message.reply(status_codes::InternalError, ("Could not run evaluation"));
    m_job_finished = false;

  } else if (!child_pid) {
    // child process: perform the evaluation
    std::vector<Duration> timings;
    std::vector<std::vector<PlaintextT>> output_vals =
        context->eval_with_plaintexts(m_job_config.circuit, plaintext_inputs,
                                      const_plaintext_inputs, timings,
                                      m_job_config.eval_strategy);

    if (timings.size() != 3) {
      // signal an error to the server
      std::cerr << "Child exiting: more than three timings reported!\n";
      _exit(1);
    }

    // insert the various things in the shared memory buffer
    for (int i = 0; i < 3; i++) {
      timings_shared[i] = timings[i];
    }

    for (int i = 0; i < n_outputs; i++) {
      for (int j = 0; j < slot_cnt; j++) {
        outputs_shared[i * slot_cnt + j] = output_vals[i][j];
      }
    }

    // if we get here, evaluation finished successfully: child can exit
    std::cerr << "successful evaluation: exiting...\n";
    _exit(0);

  } else {
    // parent process: wait for child or kill after timeout

    // timeout hardcoded as 10 s for now
    // POSIX: can assume this is an integer type

    time_t timeout_us = 60000000L;

    // go to sleep for the length of the timeout and a grace period
    struct timespec req, rem;
    req.tv_nsec = (timeout_us % 1000000) * 1000;
    // allow one second grace (since the timeout above refers to the evaluation
    // only
    req.tv_sec = (timeout_us / 1000000) + 1;

    nanosleep(&req, &rem);

    // on waking up, is the child still alive?
    int status;
    if (!waitpid(child_pid, &status, WNOHANG)) {
      // this is an error (eval's own timeout should have stopped it)
      kill(child_pid, SIGKILL);
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

      std::vector<std::vector<PlaintextT>> output_vals;

      for (int i = 0; i < n_outputs; i++) {
        std::vector<PlaintextT> output_val;
        std::vector<std::string> string_val;

        for (int j = 0; j < slot_cnt; j++) {
          output_val.push_back(outputs_shared[i * slot_cnt + j]);

          ///  store outputs values as strings, to avoid ambiguity about type.
          string_val.push_back(std::to_string(output_val[j]));
        }

        output_vals.push_back(output_val);

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

      //// now do the plaintext evaluation
      auto clear_context = make_context<PlaintextT>("Clear");
      clear_context->set_parameter("NumSlots",slot_cnt);
      std::vector<Duration> timings_clear;

      std::vector<std::vector<PlaintextT>> clear_output_vals =
          clear_context->eval_with_plaintexts(
              m_job_config.circuit, plaintext_inputs, const_plaintext_inputs,
              timings_clear);

      // Compare the encrypted and plain results
      bool is_correct =
          check_job_outputs<PlaintextT>(output_vals, clear_output_vals);
      m_job_result.is_correct = is_correct;

      message.reply(status_codes::OK);
    }
  }

  // clean up shared memory buffers
  munmap(timings_shared, n_timings * sizeof(Duration));
  munmap(outputs_shared, n_outputs * sizeof(PlaintextT));
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
      circuit_stream >> C;
      m_job_config.circuit = C;

    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unrecognized circuit request"));
    }
  });
  message.reply(status_codes::OK);
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
        std::vector<int> input_vals;

        for (auto input : input_dict[input_name].as_array()) {
          int input_val = input.as_integer();
          input_vals.push_back(input_val);
        }

        m_job_config.input_vals.push_back(input_vals);
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
        m_job_config.const_input_vals.push_back(input_val);
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
      std::string sct;
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
	else
	  message.reply(status_codes::InternalError, ("Unknown input type"));
      std::cout<<"size of ciphertext string is "<<sct.size()<<std::endl;
      json::value result = json::value::object();
      result["ciphertext"] = json::value::string(sct);
      result["size"] = json::value::number((int64_t)(sct.size()));

      message.reply(status_codes::OK, result);

    } catch (json::json_exception) {
      message.reply(status_codes::InternalError, ("Unrecognized inputs"));
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
      else
        message.reply(status_codes::InternalError,
                      ("Unknown input type when updating parameters"));

      message.reply(status_codes::OK);

    } catch (json::json_exception) {
      message.reply(status_codes::InternalError,
                    ("Unable to set evaluation strategy"));
    }
  });
}

void SheepServer::handle_get_results(http_request message) {
  json::value result = m_job_result.as_json();

  message.reply(status_codes::OK, result);
}
