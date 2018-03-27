"""
Run low-level benchmark tests over specified combinations of 
gate, depth, input_type, context.
"""
import subprocess
import os
import re
from scripts.benchmark_utils import run_circuit

if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["HOME"]+"/SHEEP"
else:
    BASE_DIR = os.environ["SHEEP_HOME"]


CIRCUIT_FILE_DIR = BASE_DIR+"/benchmark_inputs/low_level/circuits"
if not os.path.exists(CIRCUIT_FILE_DIR):
    os.system("mkdir "+CIRCUIT_FILE_DIR)

INPUT_FILE_DIR = BASE_DIR+"/benchmark_inputs/low_level/inputs"
if not os.path.exists(INPUT_FILE_DIR):
    os.system("mkdir "+INPUT_FILE_DIR)    



def low_level_benchmarks(gates,types,contexts,max_depth=10):
    """
    call this function with lists of the Gates, input-types, and contexts to be benchmarked.
    this in turn calls run_single_benchmark for each combination.
    """
    for gate in gates:
        for input_type in types:
            bitwidth = get_bitwidth(input_type)
            signed = input_type.startswith("i")
            for depth in range(1,max_depth):
                circuit_file = CIRCUIT_FILE_DIR+"/circuit-"+gate+"-"+str(depth)+".sheep"

### inputs file depends on the gate now - most gates have depth+1 inputs, but SELECT and NEGATE have
### different requirements
                
                inputs_file = INPUT_FILE_DIR+"/inputs-"
                if gate == "SELECT":
                    inputs_file += "select-"
                elif gate == "NEGATE":
                    inputs_file += "1-to-1-"
                else:
                    inputs_file += "2-to-1-"
                inputs_file +=input_type+"-"+str(depth)+".inputs"
### loop over desired contexts
                for context in contexts:
                    print("Doing benchmark for %s %s %i %s" %
                          (context,gate,depth,input_type))
### run the test
                    results = run_circuit(circuit_file,
                                          inputs_file,
                                          input_type,
                                          context,"debug_"+context+".txt")


