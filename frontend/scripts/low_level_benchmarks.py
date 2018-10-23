"""
Run low-level benchmark tests over specified combinations of 
gate, depth, input_type, context.
"""
import subprocess
import os
import re
from scripts.benchmark_utils import run_circuit, write_params_file, write_inputs_file

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

DEBUG_FILE_DIR = BASE_DIR+"/benchmark_inputs/low_level/debug"
if not os.path.exists(DEBUG_FILE_DIR):
    os.system("mkdir "+DEBUG_FILE_DIR)    

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
                                          context,
                                          "serial",
                                          params,
                                          "debug_"+context+".txt")


def params_for_level(context,level):
    """
    set parameters for a given context for a given level
    """
    if context == "HElib_Fp":
        param_dict = {"Levels": level+2}
        param_file = write_params_file(param_dict)
        return param_file
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
        param_file = write_params_file(param_dict[level])
        return param_file
    else:
        return None
    
def scan_1(contexts=["HElib_Fp","SEAL"]):
    """
    one level of each gate, levels set to 1, 4, 8
    """
    for gate in ["ADD","SUBTRACT","MULTIPLY","SELECT","NEGATE"]:
        for input_type in ["bool","uint8_t","int8_t","uint16_t","int16_t","uint32_t","int32_t"]:
            for d in [1,4,8]:
                for context in contexts:
                    param_file = params_for_level(context,d)
                    circuit_file = CIRCUIT_FILE_DIR+"/circuit-"+gate+"-1.sheep"
### inputs file depends on the gate now - most gates have depth+1 inputs, but SELECT and NEGATE have
### different requirements                
                    inputs_file = INPUT_FILE_DIR+"/inputs-"
                    if gate == "SELECT":
                        inputs_file += "select-"
                    elif gate == "NEGATE":
                        inputs_file += "1-to-1-"
                    else:
                        inputs_file += "2-to-1-"
                    inputs_file +=input_type+"-1.inputs"
                    print("Doing benchmark for %s %s %s %i" %
                          (context,gate,input_type,d))
                    results = run_circuit(circuit_file,
                                          inputs_file,
                                          input_type,
                                          context,
                                          "serial",
                                          param_file,
                                          DEBUG_FILE_DIR+"/debug_"+gate+str(d)+input_type+".txt"
                                          )


def scan_2(contexts=["HElib_Fp","SEAL"]):
    """
    set levels set to 4, see where we go wrong
    """
    for gate in ["ADD","SUBTRACT","MULTIPLY","SELECT","NEGATE"]:
        for input_type in ["bool","uint8_t","int8_t","uint16_t","int16_t","uint32_t","int32_t"]:
            for d in range(1,9):
                for context in contexts:
                    param_file = params_for_level(context,4)
                    circuit_file = CIRCUIT_FILE_DIR+"/circuit-"+gate+"-"+str(d)+".sheep"
### inputs file depends on the gate now - most gates have depth+1 inputs, but SELECT and NEGATE have
### different requirements                
                    inputs_file = INPUT_FILE_DIR+"/inputs-"
                    if gate == "SELECT":
                        inputs_file += "select-"
                    elif gate == "NEGATE":
                        inputs_file += "1-to-1-"
                    else:
                        inputs_file += "2-to-1-"
                    inputs_file +=input_type+"-"+str(d)+".inputs"
                    print("Doing benchmark for %s %s %s %i" %
                          (context,gate,input_type,d))
                    results = run_circuit(circuit_file,
                                          inputs_file,
                                          input_type,
                                          context,
                                          "serial",
                                          param_file,
                                          DEBUG_FILE_DIR+"/debug_"+gate+str(d)+input_type+".txt"
                                          )    


def scan_3(contexts=["TFHE","HElib_F2","HElib_Fp","SEAL"]):
    """
    all tests, with the levels set appropriately
    """
    for gate in ["ADD","SUBTRACT","MULTIPLY","SELECT","NEGATE"]:
        for input_type in ["bool","uint8_t","uint16_t","uint32_t"]:
            for d in range(1,9):
                for context in contexts:
                    param_file = params_for_level(context,d)
                    circuit_file = CIRCUIT_FILE_DIR+"/circuit-"+gate+"-"+str(d)+".sheep"
### inputs file depends on the gate now - most gates have depth+1 inputs, but SELECT and NEGATE have
### different requirements                
                    inputs_file = INPUT_FILE_DIR+"/inputs-"
                    if gate == "SELECT":
                        inputs_file += "select-"
                    elif gate == "NEGATE":
                        inputs_file += "1-to-1-"
                    else:
                        inputs_file += "2-to-1-"
                    inputs_file +=input_type+"-"+str(d)+".inputs"
                    print("Doing benchmark for %s %s %s %i" %
                          (context,gate,input_type,d))
                    results = run_circuit(circuit_file,
                                          inputs_file,
                                          input_type,
                                          context,
                                          "serial",
                                          param_file,
                                          DEBUG_FILE_DIR+"/debug_"+gate+str(d)+input_type+".txt"
                                          )    
