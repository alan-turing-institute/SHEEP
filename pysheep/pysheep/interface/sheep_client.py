#!/usr/bin/env python

"""
python functions to communicate with the Sheep server (C++ REST API)
All functions should return a json structure
{"status_code": <int>, "content": x }
where x can be a string (if there is an error) or a json structure.
In all cases, should catch ConnectionError in case the server is down,
so that this can be communicated to the frontend.
"""

import os
import requests
import json

from ..common import database
from ..common import common_utils

if "SERVER_URL_BASE" in os.environ.keys():
    BASE_URI = os.environ["SERVER_URL_BASE"]
else:
    BASE_URI = "http://localhost:34568/SheepServer"

def is_configured():
    """
    check if circuit, inputs, input_type, and context have all been set.
    """
    response_dict = {}
    try:
        r = requests.get(BASE_URI+"/job/")
        response_dict["status_code"] = r.status_code
        response_dict["content"] = json.loads(r.content.decode("utf-8"))
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to check configuration"
    return response_dict


def get_available_contexts():
    """
    get a list of the available contexts.
    """
    response_dict = {}
    try:
        r = requests.get(BASE_URI+"/context/")
        response_dict["status_code"] = r.status_code
        result = json.loads(r.content.decode("utf-8"))
        if r.status_code == 200:
            response_dict["content"] = result["contexts"]
        else:
            response_dict["content"] = result
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get context list"
    return response_dict


def get_available_input_types():
    """
    get a list of the available input types.
    """
    response_dict = {}
    try:
        r = requests.get(BASE_URI+"/input_type/")
        response_dict["status_code"] = r.status_code
        result = json.loads(r.content.decode("utf-8"))
        if r.status_code == 200:
            response_dict["content"] = result["input_types"]
        else:
            response_dict["content"] = result
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get list of input types"
    return response_dict


def set_context(context_name):
    """
    set a context.  First check it is in the list of available ones.
    """
    if not isinstance(context_name, str):
        return {"status_code": 500, "content": "incorrect input type for set_context"}
    context_request = get_available_contexts()
    if context_request["status_code"] != 200:
        return context_request
    response_dict = {}
    available_contexts = context_request["content"]
    if not context_name in available_contexts:
        response_dict["status_code"] = 500
        response_dict["content"] = "context {} is not in {}".format(context_name,
                                                                    available_contexts)
        return response_dict
    try:
        r = requests.post(BASE_URI+"/context/",
                          json={"context_name": context_name})
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set context"
    return response_dict


def set_input_type(input_type):
    """
    set input type, if it is in the list of available types.
    """
    if not isinstance(input_type, str):
        return {"status_code": 550, "content": "incorrect input type for set_input_type"}
    type_request = get_available_input_types()
    if type_request["status_code"] != 200:
        return type_request
    response_dict = {}
    available_types = type_request["content"]
    if not input_type in available_types:
        response_dict["status_code"] = 500
        response_dict["content"] = "input_type {} not in {}".format(input_type,
                                                                    available_types)
        return response_dict
    try:
        r = requests.post(BASE_URI+"/input_type/",
                          json={"input_type": input_type})
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set input type"
    return response_dict


def get_inputs():
    """
    parse the circuit and get the list of inputs.
    """
    response_dict = {}
    try:
        r = requests.get(BASE_URI+"/inputs/")
        response_dict["status_code"] = r.status_code
        result = json.loads(r.content.decode("utf-8"))
        if r.status_code == 200:
            response_dict["content"] = result["inputs"]
        else:
            response_dict["content"] = "Error getting inputs - do we have a circuit?"
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get inputs"
    return response_dict


def get_const_inputs():
    """
    parse the circuit and get the list of inputs.
    """
    response_dict = {}
    try:
        r = requests.get(BASE_URI+"/const_inputs/")
        response_dict["status_code"] = r.status_code
        result = json.loads(r.content.decode("utf-8"))
        if r.status_code == 200:
            response_dict["content"] = result["const_inputs"]
        else:
            response_dict["content"] = "Error getting inputs - do we have a circuit?"
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get inputs"
    return response_dict


def set_inputs(input_dict):
    """
    set input values.
    """
    if not isinstance(input_dict, dict):
        return {"status_code": 550, "content": "incorrect input type for set_inputs"}
    input_request = get_inputs()
    if input_request["status_code"] != 200:
        return input_request
    input_names = input_request["content"]
    response_dict = {}
    unset_inputs = [i for i in input_names if not i in input_dict.keys()]
    if len(unset_inputs) > 0:
        response_dict["status_code"] = 500
        response_dict["content"] = "Inputs {} not set".format(unset_inputs)
        return response_dict
    unused_inputs = [i for i in input_dict.keys() if not i in input_names]
    if len(unused_inputs) > 0:
        response_dict["status_code"] = 500
        response_dict["content"] = "Inputs {} are not inputs to the circuit".format(unused_inputs)
        return response_dict
    try:
        r = requests.post(BASE_URI+"/inputs/",
                          json=input_dict)
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set inputs"
    return response_dict

def set_const_inputs(input_dict):
    """
    set input values.
    """
    if not isinstance(input_dict, dict):
        return {"status_code": 550, "content": "incorrect input type for set_inputs"}
    input_request = get_const_inputs()
    if input_request["status_code"] != 200:
        return input_request
    input_names = input_request["content"]
    response_dict = {}
    unset_inputs = [i for i in input_names if not i in input_dict.keys()]
    if len(unset_inputs) > 0:
        response_dict["status_code"] = 500
        response_dict["content"] = "Inputs {} not set".format(unset_inputs)
        return response_dict
    unused_inputs = [i for i in input_dict.keys() if not i in input_names]
    if len(unused_inputs) > 0:
        response_dict["status_code"] = 500
        response_dict["content"] = "Inputs {} are not inputs to the circuit".format(unused_inputs)
        return response_dict
    try:
        r = requests.post(BASE_URI+"/const_inputs/",
                          json=input_dict)
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set inputs"
    return response_dict


def set_circuit_filename(circuit_filename):
    """
    Specify full path to circuit filename.
    """
    if not isinstance(circuit_filename, str):
        return {"status_code": 550, "content": "incorrect input type for set_circuit_filename"}
    response_dict = {}
    try:
        r = requests.post(BASE_URI+"/circuitfile/",
                          json={"circuit_filename": circuit_filename})
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set circuit filename"
    return response_dict


def set_circuit(circuit_filename):
    """
    Read the circuit and pass it to the server as a string.
    """
    if not isinstance(circuit_filename, str):
        return {"status_code": 550, "content": "incorrect input type for set_circuit"}
    response_dict = {}
    if not os.path.exists(circuit_filename):
        response_dict["status_code"] = 500
        response_dict["content"] = "Circuit file not found"
        return response_dict
    try:
        r = requests.post(BASE_URI+"/circuit/",
                          json={"circuit": open(circuit_filename).read()})
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set circuit"
    return response_dict


def get_parameters():
    """
    Will instantiate a context and query it for its parameters"
    """
    response_dict = {}
    try:
        r = requests.get(BASE_URI+"/parameters/")
        response_dict["status_code"] =  r.status_code
        response_dict["content"] = json.loads(r.content.decode("utf-8"))
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get parameters"
    return response_dict


def set_parameters(param_dict):
    """
    Set the parameters from a dict { param_name: param_val}
    """
    if not isinstance(param_dict, dict):
        return {"status_code": 550, "content": "incorrect input type for set_parameters"}
    response_dict = {}
    try:
        r=requests.put(BASE_URI+"/parameters/",
                       json=param_dict)
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set parameters"
    return response_dict


def set_eval_strategy(strategy):
    """
    choose between serial and parallel evaluation.
    """
    if not isinstance(strategy, str):
        return {"status_code": 550, "content": "incorrect input type for set_strategy"}
    response_dict = {}
    if not strategy in ["serial","parallel"]:
        response_dict["status_code"] = 500
        response_dict["content"] = "Eval strategy must be 'serial' or 'parallel'"
        return response_dict
    try:
        r=requests.put(BASE_URI+"/eval_strategy/",
                       json={"eval_strategy": strategy})
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to set eval strategy"
    return response_dict


def get_eval_strategy():
    """
    see whether the job is set to run in serial or parallel
    """
    response_dict = {}
    try:
        r=requests.get(BASE_URI+"/eval_strategy/")
        response_dict["status_code"] = r.status_code
        response_dict["content"] = json.loads(r.content.decode("utf-8"))
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get eval strategy"
    return response_dict


def new_job():
    """
    reset all the job configuration and results structs on the server.
    """
    response_dict = {}
    try:
        r=requests.post(BASE_URI+"/job/")
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server for new job"
    return response_dict


def run_job():
    """
    execute the job, if it is fully configured.
    """
    response_dict = {}
    config_request = is_configured()
    if config_request["status_code"] != 200:
        return config_request
    if not config_request["content"]["job_configured"]:
        response_dict["status_code"] = 500
        response_dict["content"] = "Job not fully configured"
        return response_dict
    try:
        r = requests.post(BASE_URI+"/run/")
        response_dict["status_code"] = r.status_code
        response_dict["content"] = r.content.decode("utf-8")
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to run job"
    return response_dict


def get_config():
    """
    get a json object with all the configuration - inputs, parameters etc.
    """
    response_dict = {}
    try:
        r = requests.get(BASE_URI+"/config/")
        response_dict["status_code"] = r.status_code
        response_dict["content"] = json.loads(r.content.decode("utf-8"))
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get config"
    return response_dict


def get_results():
    """
    retrieve results - should be a dictionary containing
    a dictionary of outputs, and a dictionary of timings.
    """
    response_dict = {}
    try:
        r=requests.get(BASE_URI+"/results/")
        response_dict["status_code"] = r.status_code
        response_dict["content"] = json.loads(r.content.decode("utf-8"))
    except(requests.exceptions.ConnectionError):
        response_dict["status_code"] = 404
        response_dict["content"] = "Unable to connect to SHEEP server to get results"
    return response_dict


def upload_results(circuit_name):
    """
    upload test result and some configuration to db
    """
    results_dict = {}
    results_dict['circuit_name'] = circuit_name
    try:
        ## first get the "results"
        r=requests.get(BASE_URI+"/results/")
        rj = json.loads(r.content.decode("utf-8"))
        results_dict["is_correct"] = rj["cleartext check"]["is_correct"]
        results_dict["execution_time"] = rj["timings"]["evaluation"]
        ## now get the configuration
        c = requests.get(BASE_URI+"/config/")
        cj = json.loads(c.content.decode("utf-8"))
        input_type = cj['input_type']
        results_dict['input_bitwidth'] = common_utils.get_bitwidth(input_type)
        results_dict['input_signed'] = input_type.startswith("i")
        results_dict['context_name'] = cj['context']

        uploaded_ok = database.upload_benchmark_result(results_dict)
        if uploaded_ok:
            return {"status_code": 200, "content": "uploaded OK"}
        else:
            return {"status_code": 500, "content": "Error uploading results"}

    except(requests.exceptions.ConnectionError):
        return {"status_code": 404,
                "content": "Unable to connect to SHEEP server to get results"}
