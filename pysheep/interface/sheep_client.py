#!/usr/bin/env python

"""
python functions to communicate with the Sheep server (C++ REST API)
"""

import requests
import json

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
    return result


def get_available_input_types():
    """
    get a list of the available input types.
    """
    r = requests.get(BASE_URI+"/input_type/")
    if r.status_code != 200:
        raise RuntimeError("Error getting list of input_types", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result

def set_context(context_name):
    """
    set a context.  First check it is in the list of available ones.
    """
    available_contexts = get_available_contexts()["contexts"]
    if not context_name in available_contexts:
        raise RuntimeError("context {} is not known".format(context_name))
    r = requests.post(BASE_URI+"/context/",
                      json={"context_name": context_name})
    if r.status_code != 200:
        raise RuntimeError("Error setting context to".format(context_name))
    return r

def set_input_type(input_type):
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
    return result

def set_inputs(input_dict):
    """
    set input values.
    """
    input_names = get_inputs()["inputs"]
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
    r = requests.post(BASE_URI+"/circuit/",
                      json={"circuit_filename": circuit_filename})
    if r.status_code != 200:
        raise RuntimeError("Error setting circuit_Filename to {}".format(circuit_filename))
    return r


def get_parameters():
    r = requests.get(BASE_URI+"/parameters/")
    if r.status_code != 200:
        raise RuntimeError("Error getting parameters")
    result = json.loads(r.content.decode("utf-8"))
    return result


def set_parameters(param_dict):
    r=requests.put(BASE_URI+"/parameters/",
                   json=param_dict)
    if r.status_code != 200:
        raise RuntimeError("Error setting parameters")
    return r

def set_eval_strategy(strategy):
    r=requests.put(BASE_URI+"/eval_strategy/",
                   json={"eval_strategy": strategy})
    if r.status_code != 200:
        raise RuntimeError("Error setting eval strategy")
    return r
    
def new_job():
    r=requests.post(BASE_URI+"/job/")
    if r.status_code != 200:
        raise RuntimeError("Error resetting job")
    return r


def run_job():
    if not is_configured()["job_configured"]:
        raise RuntimeError("Job not fully configured")
    r= requests.post(BASE_URI+"/run/")
    if r.status_code != 200:
        raise RuntimeError("Error running job")
    return r


def get_config():
    r=requests.get(BASE_URI+"/config/")
    if r.status_code != 200:
        raise RuntimeError("Error getting config")
    result = json.loads(r.decode("utf-8"))
    return result


def get_results():
    r=requests.get(BASE_URI+"/results/")
    if r.status_code != 200:
        raise RuntimeError("Error getting results")
    result = json.loads(r.content.decode("utf-8"))
    return result
