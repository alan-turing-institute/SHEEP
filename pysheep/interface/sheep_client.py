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
