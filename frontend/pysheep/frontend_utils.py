"""
Various helper functions for the SHEEP flask app.
"""

import os
import re
import uuid
import subprocess

from .database import upload_benchmark_result
from . import common_utils
from . import sheep_client
#import pysheep.common_utils
#import pysheep.sheep_client

def convert_input_vals_list(input_val_dict):
  """
  Converts input val dictionary with string values to a dictionary of list integers

  """

  output_dict = {}

  for k, v in input_val_dict.items():
    ## if its a const input we don't want it to be a list
    if "(C)" in k:
      output_dict[k] = int(v)
    else:
      ## split on commas
      output_dict[k] = [int(v_value) for v_value in v.split(",")]
  return output_dict

def cleanup_upload_dir(config):
    """
    At the start of a new test, remove all the uploaded circuits, inputs, and parameters
    files from the uploads dir.
    """
    for file_prefix in ["circuit","inputs","param"]:
        cmd = "rm -fr "+config["UPLOAD_FOLDER"]+"/"+file_prefix+"*"
        os.system(cmd)
        os.makedirs(config["UPLOAD_FOLDER"],exist_ok=True)


def upload_files(filedict,upload_folder):
    """
    Upload circuit file to server storage.
    """
    uploaded_filenames = {}
    for k,v in filedict.items():
        uploaded_filename = os.path.join(upload_folder,v.filename)
        v.save(uploaded_filename)
        uploaded_filenames[k] = uploaded_filename
    return uploaded_filenames


def set_eval_strategy(data, strategy="serial"):
    """
    for each context, set eval_strategy to 'serial' (can override later)
    """
    es_dict = {}
    for context in data["HE_libraries"]:
        es_dict[context] = strategy    # default is serial
    return es_dict


def run_test(data):
    """
    run the executable and return the results dict.
    """
    results = {} ## will be a dictionary key-ed by context
    contexts_to_run = data["HE_libraries"]

    for context in contexts_to_run:
        sheep_client.new_job()
        sheep_client.set_input_type(data["input_type"])
        sheep_client.set_context(context)
        sheep_client.set_circuit(data["uploaded_filenames"]["circuit_file"])
        ## configure the context
        sheep_client.set_parameters(data["params"][context])
        sheep_client.set_eval_strategy(data["eval_strategy"][context])
        ## set input values
        ct_inputs, pt_inputs = {}, {}
        for k,v in data["input_vals"].items():
            if "(C)" in k:
                pt_inputs[k.split()[0]] = v
            else:
                ct_inputs[k] = v
        inputs_set = sheep_client.set_inputs(ct_inputs)
        if not inputs_set["status_code"] == 200:
          return inputs_set
        const_inputs_set = sheep_client.set_const_inputs(pt_inputs)
        if not const_inputs_set["status_code"] == 200:
          return const_inputs_set
        run_request = sheep_client.run_job()
        if run_request["status_code"] != 200:
            return run_request
        results_request = sheep_client.get_results()
        if results_request["status_code"] != 200:
            return results_request
        results[context] = results_request["content"]
        params_request = get_params_and_slots_single_context(context, data["input_type"])
        if params_request["status_code"] != 200:
            return params_request
        results[context]['parameter values'] = params_request["content"]["params"]

    return {"status_code": 200, "content": results}


def get_params_all_contexts(context_list,input_type):
    """
    Return a dict with the key being context_name, and the vals being
    dicts of param_name:default_val.
    """
    all_params = {"params":{}, "slots":{}}
    for context in context_list:
      param_request = get_params_and_slots_single_context(context,input_type)
      if not param_request["status_code"] == 200:
        return param_request
      all_params["params"][context] = param_request["content"]["params"]
      all_params["slots"][context] = param_request["content"]["slots"]
    return {"status_code": 200, "content": all_params}


def get_params_and_slots_single_context(context, input_type):
    """
    Do params and slots together, to avoid having to re-instantiate the context (can be time-consuming)
    Use sheep_client to get parameters for given context and input type
    We get back from sheep_client a dict with keys "status_code" and "content"
    Check this is OK, then get the slots.
    """
    sheep_client.set_context(context)
    sheep_client.set_input_type(input_type)
    params_request = sheep_client.get_parameters()
    if not params_request["status_code"] == 200:
      return params_request
    params = params_request["content"]
    slots_request = sheep_client.get_nslots()
    if not slots_request["status_code"] == 200:
      return slots_request
    slots = slots_request["content"]
    return {"status_code": 200, "content": {"params":params,
                                            "slots": slots} }


def update_params(context,param_dict,appdata,appconfig):
    """
    We have received a dict of params for a given context from the web form.
    However, we need to get the benchmark executable to calculate params, if e.g. A_predefined_param_set
    was changed for HElib.
    So, we first get the default params, then see if any of the values in the form are different
    """
    # first "set" an empty set of parameters
    default_param_request = sheep_client.set_parameters({})
    if default_param_request["status_code"] != 200:
        return param_update_request
    # now retrieve the parameters
    default_param_request = get_params_and_slots_single_context(context,appdata["input_type"])
    if default_param_request["status_code"] != 200:
        return param_update_request
    default_params = default_param_request["content"]["params"]
    # now compare to the parameters in the form.
    params_to_update = {}
    eval_strat = "serial"
    for k,v in param_dict.items():
        ### ignore the "apply" button:
        if v=="Apply":
            continue
        ### treat the evaluation strategy separately
        if k=="eval_strategy":
            eval_strat = v
            appdata["eval_strategy"][context] = v
            continue
        ### only modify if the new param is different to the old one
        if str(v) != str(default_params[k]):
            params_to_update[k] = int(v)
    param_update_request = sheep_client.set_parameters(params_to_update)
    if param_update_request["status_code"] != 200:
        return param_update_request
    updated_params_request = get_params_and_slots_single_context(context,appdata["input_type"])
    if updated_params_request["status_code"] != 200:
      return updated_params_request
    ## insert the eval_strat into the content to be returned
    updated_params_request["content"]["eval_strat"] = eval_strat
    return updated_params_request


def upload_test_result(results,app_data):
    """
    Save data from a user-specified circuit test.
    """
    print("Uploading results to DB")
    for context in app_data["HE_libraries"]:
        result = results[context]
        ### see if it follows naming convention for a low-level benchmark test
        circuit_path = app_data["uploaded_filenames"]["circuit_file"]
        circuit_name = circuit_path.split("/")[-1]
        input_type = app_data["input_type"]
        num_inputs = len(app_data["inputs"])
        input_signed = app_data["input_type"].startswith("i")
        num_slots = app_data["slots"][context]
        tbb_enabled = (app.data["eval_strategy"][context] == "parallel")
        upload_benchmark_result(circuit_name, context, input_type, num_inputs, num_slots, tbb_enabled,
                                result, results["parameter values"])

#        execution_time = result["timings"]["evaluation"]
#        is_correct = result["cleartext check"]["is_correct"]
#        param_dict = result["parameter values"]
#        cm = BenchmarkMeasurement(
#            circuit_name = circuit_name,
#            context_name = context,
#            input_bitwidth = common_utils.get_bitwidth(app_data["input_type"]),
#            input_signed = app_data["input_type"].startswith("i"),
#            execution_time = execution_time,
#            is_correct = is_correct,
##            ciphertext_size = sizes["ciphertext"],
##            private_key_size = sizes["privateKey"],
##            public_key_size = sizes["publicKey"]
#        )
#
#        context_prefix = context.split("_")[0]  ### only have HElib, not HElib_F2 and HElib_Fp
#        for k,v in param_dict.items():
#            column = context_prefix+"_"+k
#            cm.__setattr__(column,v)
#        session.add(cm)
#        session.commit()
    return
