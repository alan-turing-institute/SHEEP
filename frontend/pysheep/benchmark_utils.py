"""
A set of scripts, likely to be run via Jupyter notebook, to allow setting input
and parameter values to temporary files, and running the benchmark job.

"""
import requests

import os, uuid
import random
import re

from . import common_utils
from . import sheep_client
from .database import BenchmarkMeasurement, Timing, ParameterSetting, session, upload_benchmark_result


def check_result(func,**kwargs):
    """
    wrap calls to sheep-server API, check return code, and return response content
    """
    response = func(**kwargs)
    if response["status_code"] != 200:
        print(response["content"])
        raise RuntimeError("Error in call to sheep-server")
    return response["content"]

def generate_input_vals(inputs, const_inputs, input_type, nslots):
    """
    randomly generate input values conforming to the selected input types
    """
    lower,upper = common_utils.get_min_max(input_type)
    lower = max(lower, -1*pow(2,15))
    upper = min(upper, pow(2,15))
    inputs_dict = {}
    for k in inputs:
        inputs_dict[k] = []
        for slot in range(nslots):
            inputs_dict[k].append(random.randint(lower,upper))
    const_inputs_dict = {}
    for k in const_inputs:
        const_inputs_dict[k] = random.randint(allowed_range)
    return inputs_dict, const_inputs_dict



def run_circuit(circuit_file, input_type, context, params, eval_strategy="serial", scan_id=None):
    """
    Run the circuit and retreive the results.
    scan_id is an optional argument that can help with retrieving a set of results from the database.
    """
    check_result(sheep_client.new_job)
    ## configure the sheep client
    check_result(sheep_client.set_context,context_name=context)
    check_result(sheep_client.set_input_type,input_type=input_type)
## TEMP COMMENT OUT FOR NOW    check_result(sheep_client.set_parameters,param_dict=params)
    check_result(sheep_client.set_eval_strategy,strategy=eval_strategy)
    check_result(sheep_client.set_circuit,circuit_filename=circuit_file)

    ## randomly assign input values
    r = check_result(sheep_client.get_nslots)
    nslots = min(r["nslots"],100)
#    nslots = r["nslots"]
    inputs = check_result(sheep_client.get_inputs)
    const_inputs = check_result(sheep_client.get_const_inputs)
    input_vals, const_input_vals = generate_input_vals(inputs, const_inputs, input_type, nslots)
    check_result(sheep_client.set_inputs,input_dict=input_vals)
    check_result(sheep_client.set_const_inputs, input_dict=const_input_vals)

    ## run the job
    check_result(sheep_client.run_job)
    ## get the results
    results = check_result(sheep_client.get_results)
    parameters = check_result(sheep_client.get_parameters)
    uploaded_OK = upload_benchmark_result(circuit_file.split("/")[-1],
                                          context,
                                          input_type,
                                          len(inputs),
                                          nslots,
                                          eval_strategy=="parallel",
                                          results,
                                          parameters,
                                          scan_id)
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
        return param_dict[level]
    else:
        param_dict = {}
    return param_dict

def levels_for_params(context, param_dict):
    """
    inverse of the above function - given a set of parameters,
    look up what we expect the safe multiplicative depth to be.
    """
    if context == "HElib_Fp":
        return param_dict["Levels"]
    elif context == "SEAL":
        if param_dict["N"] == 2048:
            return 1
        elif param_dict["N"] == 4096:
            return 3
        elif param_dict["N"] == 8192:
            return 5
        elif param_dict["N"] == 16384:
            return 8
        elif param_dict["N"] == 32768:
            return 9
        else:
            raise RuntimeError("Unrecognized value of N parameter")
    else:
        print("Levels not known for this context")
        return 0


def timing_per_gate_type(timings, circuit):
    """
    The output of running a benchmark job will be a dict containing timings for
    every named gate.  Or, if we are querying the database, it will be a list of Timing rows.
    Either way, we want to know what type of gate each of these was, so
    we parse the circuit to get the mapping.
    """
    gate_map = {}
    lines = circuit.split("\n")
    gate_rex = re.compile("([\w]+[\s]+){1,3}([A-Z]+)[\s]+([\w]+)[\s]*$")
    for line in lines:
        if line.startswith("INPUTS") or line.startswith("CONST_INPUTS") \
           or line.startswith("OUTPUTS") or line.startswith("#"):
            continue
        gate_match = gate_rex.search(line)
        if not gate_match:
            continue
        gate_type, gate_name = gate_match.groups()[1:]
        gate_map[gate_name] = gate_type
    ## now go through the results_dict and sum the timings
    output_dict = {}
    if isinstance(timings, dict):
        for k,v in timings.items():
            if k=="evaluation" or k=="encryption" or k=="decryption":
                continue
            gate_type = gate_map[k]
            if not gate_type in output_dict.keys():
                output_dict[gate_type] = 0.
                output_dict[gate_type] += float(v)
    elif isinstance(timings, list):
        for row in timings:
            if row.timing_name=="evaluation" or row.timing_name=="encryption" \
               or row.timing_name=="decryption":
                continue
            gate_name = row.timing_name
            if not gate_type in output_dict.keys():
                output_dict[gate_type] = 0.
                output_dict[gate_type] += float(row.timing_value)
    return output_dict
