#ifndef SHEEP_SERVER_HPP
#define SHEEP_SERVER_HPP

#define _TURN_OFF_PLATFORM_STRING

/// stl includes
#include <fstream>
#include <map>
#include <memory>
#include <set>

/// cpp rest api includes
#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_listener.h"
#include "cpprest/json.h"
#include "cpprest/uri.h"

/// SHEEP includes
#include "circuit.hpp"
#include "context-clear.hpp"
#ifdef HAVE_HElib
#include "context-helib.hpp"
#endif
#ifdef HAVE_TFHE
#include "context-tfhe.hpp"
#endif
#ifdef HAVE_SEAL_BFV
#include "context-seal-bfv.hpp"
#include "context-seal-ckks.hpp"
#endif
#ifdef HAVE_LP
#include "context-lp.hpp"
#endif
#ifdef HAVE_PALISADE
#include "context-palisade.hpp"
#endif

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

struct SheepJobConfig {
  utility::string_t context;
  utility::string_t input_type;
  utility::string_t circuit_filename;
  EvaluationStrategy eval_strategy;
  Circuit circuit;
  std::set<std::string> input_names;
  std::set<std::string> const_input_names;
  std::map<std::string, std::vector<std::string>> input_vals;
  std::map<std::string, long> const_input_vals;
  std::map<std::string, long> parameters;
  int nslots;
  int timeout;
  void setDefaults() { eval_strategy = EvaluationStrategy::serial; timeout = 10; }
  bool isConfigured() {
    return ((context.size() > 0) && (input_type.size() > 0) &&
            (circuit.get_inputs().size() > 0) &&
            (input_vals.size() == circuit.get_inputs().size() &&
             const_input_vals.size() == circuit.get_const_inputs().size()));
  }
  json::value as_json() {
    json::value config = json::value::object();
    json::value j_context = json::value::string(context);
    config["context"] = j_context;
    json::value j_circuit_file = json::value::string(circuit_filename);
    config["circuit_filename"] = j_circuit_file;
    json::value j_input_type = json::value::string(input_type);
    config["input_type"] = j_input_type;
    std::string eval_strat_string;
    if (eval_strategy == EvaluationStrategy::serial)
      eval_strat_string = "serial";
    else
      eval_strat_string = "parallel";
    json::value j_eval_strategy = json::value::string(eval_strat_string);
    config["eval_strategy"] = j_eval_strategy;
    json::value params = json::value::object();
    for (auto map_iter : parameters) {
      params[map_iter.first] = json::value::number((int64_t)map_iter.second);
    }
    config["parameters"] = params;
    json::value j_nslots = json::value::number(nslots);
    config["nslots"] = j_nslots;
    return config;
  }
};

struct SheepJobResult {
  std::map<std::string, std::vector<std::string>>
      outputs;  /// store output values as strings so we don't worry about
                /// Plaintext type.
  std::map<std::string, std::string>
      timings;      /// store time values in microseconds as strings.
  bool is_correct;  /// all outputs compared with clear-context results

  json::value as_json() {
    json::value result = json::value::object();

    json::value j_outputs = json::value::object();
    for (auto map_iter = outputs.begin(); map_iter != outputs.end();
         ++map_iter) {
      json::value outputs = json::value::array();

      int idx = 0, out_val_cnt = 0;
      std::string result = "";
      for (auto out_val : map_iter->second) {
        if (out_val_cnt > 0) {
          result = result + ",";
        }

        for (int i = 0; i < out_val.length(); i++) {
          result = result + out_val[i];
        }

        out_val_cnt += 1;
      }
      outputs[idx] = json::value::string(result);

      j_outputs[map_iter->first] = outputs;
    }
    result["outputs"] = j_outputs;

    json::value j_timings = json::value::object();
    for (auto map_iter = timings.begin(); map_iter != timings.end();
         ++map_iter) {
      j_timings[map_iter->first] = json::value::string(map_iter->second);
    }
    result["timings"] = j_timings;

    json::value clear_check = json::value::object();
    clear_check["is_correct"] = json::value::boolean(is_correct);
    result["cleartext check"] = clear_check;
    return result;
  }
};

class SheepServer {
 public:
  SheepServer(){};
  SheepServer(utility::string_t url);

  pplx::task<void> open() { return m_listener.open(); }
  pplx::task<void> close() { return m_listener.close(); }

  template <typename PlaintextT>
  BaseContext<PlaintextT>* make_context(std::string context_type);

  template <typename PlaintextT>
  PlaintextT convert_to_plaintext(std::string);

  template <typename PlaintextT>
  std::vector<std::vector<PlaintextT>> make_plaintext_inputs();

  std::vector<long> make_const_plaintext_inputs();

  template<typename PlaintextT>
  std::string convert_to_string(PlaintextT t);

  template <typename PlaintextT>
  void configure_and_run(http_request message);

  template <typename PlaintextT>
  int configure_and_serialize(std::vector<int> pt);

  void get_parameters();

  template <typename PlaintextT>
  void update_parameters(std::string context,
                         json::value params = json::value::object());

  template <typename PlaintextT>
  bool check_job_outputs(std::vector<std::vector<PlaintextT>> test_outputs,
                         std::vector<std::vector<PlaintextT>> clear_outputs);

 private:
  /// generic methods - will then dispatch to specific ones based on URL
  void handle_get(http_request message);
  void handle_put(http_request message);
  void handle_post(http_request message);
  /// actual endpoints
  void handle_get_context(http_request message);
  void handle_get_circuit(http_request message);
  void handle_get_input_type(http_request message);
  void handle_get_inputs(http_request message);
  void handle_get_const_inputs(http_request message);
  void handle_get_parameters(http_request message);
  void handle_get_slots(http_request message);
  void handle_get_eval_strategy(http_request message);
  void handle_get_config(http_request message);
  void handle_get_results(http_request message);
  void handle_get_job(http_request message);

  void handle_post_inputs(http_request message);
  void handle_post_const_inputs(http_request message);
  void handle_post_serialized_ciphertext(http_request message);

  void handle_post_input_type(http_request message);
  void handle_post_circuit(http_request message);
  void handle_post_circuitfile(http_request message);
  void handle_post_context(http_request message);
  void handle_post_job(http_request message);
  void handle_post_configure(http_request message);
  void handle_post_run(http_request message);

  void handle_put_parameters(http_request message);
  void handle_put_eval_strategy(http_request message);
  void handle_put_timeout(http_request message);

  /// listen to http requests
  http_listener m_listener;
  /// structs to store the configuration and results of a test.
  SheepJobConfig m_job_config;
  SheepJobResult m_job_result;
  bool m_job_finished;
  std::vector<std::string> m_available_contexts;
  std::vector<std::string> m_available_input_types;
};

#endif
