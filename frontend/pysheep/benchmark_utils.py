"""
A set of scripts, likely to be run via Jupyter notebook, to allow setting input
and parameter values to temporary files, and running the benchmark job.

"""
import requests

import os, uuid
import random
from . import common_utils
from . import sheep_client
from .database import BenchmarkMeasurement, Timing, ParameterSetting, session, upload_benchmark_result

if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.path.join(os.environ["HOME"],"SHEEP","frontend")
else:
    BASE_DIR = os.environ["SHEEP_HOME"]

TMP_INPUTS_DIR = BASE_DIR+"/benchmark_inputs/mid_level/inputs/TMP"
if not os.path.exists(TMP_INPUTS_DIR):
    os.system("mkdir "+TMP_INPUTS_DIR)


TMP_PARAMS_DIR = BASE_DIR+"/benchmark_inputs/params/TMP"
if not os.path.exists(TMP_PARAMS_DIR):
    os.system("mkdir "+TMP_PARAMS_DIR)


def generate_input_vals(inputs, const_inputs, input_type, nslots):
    """
    randomly generate input values conforming to the selected input types
    """
    lower,upper = common_utils.get_min_max(input_type)
    inputs_dict = {}
    for k in inputs:
        inputs_dict[k] = []
        for slot in range(nslots):
            inputs_dict[k].append(random.randint(lower,upper))
    const_inputs_dict = {}
    for k in const_inputs:
        const_inputs_dict[k] = random.randint(allowed_range)
    return inputs_dict, const_inputs_dict



def run_circuit(circuit_file, input_type, context, params, eval_strategy="serial"):
    """
    Run the circuit and retreive the results.
    """
    sheep_client.new_job()
    ## configure the sheep client
    sheep_client.set_context(context)
    sheep_client.set_input_type(input_type)
    sheep_client.set_parameters(params)
    sheep_client.set_eval_strategy(eval_strategy)
    sheep_client.set_circuit(circuit_file)

    ## randomly assign input values
    nslots = sheep_client.get_nslots()["content"]["nslots"]
    inputs = sheep_client.get_inputs()["content"]
    const_inputs = sheep_client.get_const_inputs()["content"]
    input_vals, const_input_vals = generate_input_vals(inputs, const_inputs, input_type, nslots)
    sheep_client.set_inputs(input_vals)
    sheep_client.set_const_inputs(input_vals)

    ## run the job
    sheep_client.run_job()
    ## get the results
    results = sheep_client.get_results()["content"]
    parameters = sheep_client.get_parameters()["content"]
    uploaded_OK = upload_benchmark_result(circuit_file.split("/")[-1],
                                          context,
                                          input_type,
                                          len(inputs),
                                          nslots,
                                          eval_strategy=="parallel",
                                          results,
                                          parameters)
    return uploaded_OK





def params_for_level(context,level):
    """
    set parameters for a given context for a given level
    """
    if context == "HElib_Fp":
        param_dict = {"Levels": level+2}

    elif context == "SEAL":
        param_dict = {
            1: {"N": 2048},
            2: {"N": 4096},
            3: {"N": 4096},
            4: {"N": 8192},
            5: {"N": 8192},
            6: {"N": 16384},
            7: {"N": 16384},
            8: {"N": 16384},
            9: {"N": 32768},
        }
    else:
        param_dict = {}
    return param_dict
