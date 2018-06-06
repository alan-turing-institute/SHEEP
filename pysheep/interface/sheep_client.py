#!/usr/bin/env python

"""
python functions to communicate with the Sheep server (C++ REST API)
"""

import requests
import json

BASE_URI = "http://localhost:34568/SheepServer"

def get_available_contexts():
    r = requests.get(BASE_URI+"/context/")
    if r.status_code != 200:
        raise RuntimeError("Error getting list of contexts", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result


def get_available_input_types():
    r = requests.get(BASE_URI+"/input_type/")
    if r.status_code != 200:
        raise RuntimeError("Error getting list of input_types", r.content)
    result = json.loads(r.content.decode("utf-8"))
    return result

def set_context(context_name):
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


def get_parameters():
    r = requests.get(BASE_URI+"/parameters/")
    if r.status_code != 200:
        raise RuntimeError("Error getting parameters")
    result = json.loads(r.content.decode("utf-8"))
    return result


def set_parameters(param_dict):
    r=requests.put(BASE_URI+"/parameters/",
                   json=param_dict)
