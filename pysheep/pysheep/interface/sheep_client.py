#!/usr/bin/env python

"""
python functions to communicate with the Sheep server (C++ REST API)
"""

import os
import requests
import json

if "SERVER_URL_BASE" in os.environ.keys():
    BASE_URI = os.environ["SERVER_URL_BASE"]
else:
    BASE_URI = "http://localhost:34568/SheepServer"

def is_configured():
    """
    check if circuit, inputs, input_type, and context have all been set.
    """
    r = requests.get(BASE_URI+"/job/")
    if r.status_code != 200:
        raise RuntimeError("Error getting configuration state", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result

def get_available_contexts():
    """
    get a list of the available contexts.
    """
    r = requests.get(BASE_URI+"/context/")
    if r.status_code != 200:
        raise RuntimeError("Error getting list of contexts", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result["contexts"]


def get_available_input_types():
    """
    get a list of the available input types.
    """
    r = requests.get(BASE_URI+"/input_type/")
    if r.status_code != 200:
        raise RuntimeError("Error getting list of input_types", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result["input_types"]

def set_context(context_name):
    """
    set a context.  First check it is in the list of available ones.
    """
    available_contexts = get_available_contexts()
    if not context_name in available_contexts:
        raise RuntimeError("context {} is not in {}".format(context_name,
                                                            available_contexts))
    r = requests.post(BASE_URI+"/context/",
                      json={"context_name": context_name})
    if r.status_code != 200:
        raise RuntimeError("Error setting context to".format(context_name))
    return r

def set_input_type(input_type):
    """
    set input type, if it is in the list of available types.
    """
    available_types = get_available_input_types()
    if not input_type in available_types:
        raise RuntimeError("input_type {} not in {}".format(input_type,
                                                            available_types))
    r = requests.post(BASE_URI+"/input_type/",
                      json={"input_type": input_type})
    if r.status_code != 200:
        raise RuntimeError("Error setting input_type to {}".format(input_type))
    return r


def get_inputs():
    """
    parse the circuit and get the list of inputs.
    """
    r = requests.get(BASE_URI+"/inputs/")
    if r.status_code != 200:
        raise RuntimeError("Error getting inputs - do we have a circuit?")
    result = json.loads(r.content.decode("utf-8"))
    return result["inputs"]


def set_inputs(input_dict):
    """
    set input values.
    """
    input_names = get_inputs()
    unset_inputs = [i for i in input_names if not i in input_dict.keys()]
    if len(unset_inputs) > 0:
        raise RuntimeError("Inputs {} not set".format(unset_inputs))
    unused_inputs = [i for i in input_dict.keys() if not i in input_names]
    if len(unused_inputs) > 0:
        raise RuntimeError("Inputs {} are not inputs to the circuit".format(unused_inputs))
    r = requests.post(BASE_URI+"/inputs/",
                      json=input_dict)
    if r.status_code != 200:
        raise RuntimeError("Error setting input vals")
    return r


def set_circuit_filename(circuit_filename):
    """
    Specify full path to circuit filename.
    """
    r = requests.post(BASE_URI+"/circuitfile/",
                      json={"circuit_filename": circuit_filename})
    if r.status_code != 200:
        raise RuntimeError("Error setting circuit_Filename to {}".format(circuit_filename))
    return r


def set_circuit(circuit_filename):
    """
    Read the circuit and pass it to the server as a string.
    """
    if not os.path.exists(circuit_filename):
        raise RuntimeError("Circuit file not found")
    r = requests.post(BASE_URI+"/circuit/",
                      json={"circuit": open(circuit_filename).read()})
    if r.status_code != 200:
        raise RuntimeError("Error setting circuit {}".format(circuit_filename))
    return r


def get_parameters():
    """
    Will instantiate a context and query it for its parameters"
    """
    r = requests.get(BASE_URI+"/parameters/")
    if r.status_code != 200:
        raise RuntimeError("Error getting parameters", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result


def set_parameters(param_dict):
    r=requests.put(BASE_URI+"/parameters/",
                   json=param_dict)
    if r.status_code != 200:
        raise RuntimeError("Error setting parameters", r.content)
    return r

def set_eval_strategy(strategy):
    """
    choose between serial and parallel evaluation.
    """
    if not strategy in ["serial","parallel"]:
        raise RuntimeError("Eval strategy must be 'serial' or 'parallel'")
    r=requests.put(BASE_URI+"/eval_strategy/",
                   json={"eval_strategy": strategy})
    if r.status_code != 200:
        raise RuntimeError("Error setting eval strategy",r.content)
    return r


def get_eval_strategy():
    """
    see whether the job is set to run in serial or parallel
    """
    r=requests.get(BASE_URI+"/eval_strategy/")
    if r.status_code != 200:
        raise RuntimeError("Error getting eval strategy",r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result


def new_job():
    """
    reset all the job configuration and results structs on the server.
    """
    response_dict = {}
    try:
        r=requests.post(BASE_URI+"/job/")
        response_dict["status_code"] = r.status_code
        response_dict["message"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["message"] = "Unable to connect to SHEEP server"
    return response_dict


def run_job():
    """
    execute the job, if it is fully configured.
    """
    if not is_configured()["job_configured"]:
        raise RuntimeError("Job not fully configured")
    r= requests.post(BASE_URI+"/run/")
    if r.status_code != 200:
        raise RuntimeError("Error running job", r.content)
    return r


def get_config():
    """
    get a json object with all the configuration - inputs, parameters etc.
    """
    r=requests.get(BASE_URI+"/config/")
    if r.status_code != 200:
        raise RuntimeError("Error getting config", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result


def get_results():
    """
    retrieve results - should be a dictionary containing
    a dictionary of outputs, and a dictionary of timings.
    """
    r=requests.get(BASE_URI+"/results/")
    if r.status_code != 200:
        raise RuntimeError("Error getting results", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result
