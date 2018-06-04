"""
Script to generate a circuit to sort a list of input values.
The number of inputs to the sorting algorithm must be a power of 2.

The outputs will be the input values ordered from high-to-low.
"""
import os
import random
from frontend.utils import get_min_max
import subprocess

if "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["SHEEP_HOME"]
else:
    BASE_DIR = os.environ["HOME"]+"/SHEEP"    

CIRCUIT_DIR_MID = BASE_DIR+"/benchmark_inputs/mid_level/circuits"
INPUTS_DIR_MID  = BASE_DIR+"/benchmark_inputs/mid_level/inputs/TMP"

EXECUTABLE_DIR = BASE_DIR+"/build/bin"

def generate_inputs(input_type):
    """
    randomly generate inputs 
    """
    values = {}
    minval,maxval = get_min_max(input_type)
    for i in range(num_inputs):
        values["i"+str(i)] = (int(random.randint(minval,maxval)))
    return values
                    


def generate(num_inputs):
    """
    Generate the circuit.
    """
    run_cmd=[]
    circuit_filename = os.path.join(CIRCUIT_DIR_MID,"circuit-bitonic-sort-"+str(num_inputs)+".sheep")
    run_cmd.append(os.path.join(EXECUTABLE_DIR,"bitonic-sorting-circuit"))
    run_cmd.append(str(num_inputs))
    run_cmd.append(circuit_filename)
    p=subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    job_output = p.communicate()[0]
    return circuit_filename


