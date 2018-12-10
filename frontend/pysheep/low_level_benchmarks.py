"""
Run low-level benchmark tests over specified combinations of
gate, depth, input_type, context.
"""
import subprocess
import os
import re

from .benchmark_utils import (run_circuit, params_for_level)

from .common_utils import get_bitwidth


if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["HOME"]+"/SHEEP"
else:
    BASE_DIR = os.environ["SHEEP_HOME"]


CIRCUIT_FILE_DIR = BASE_DIR+"/benchmark_inputs/low_level/circuits"
if not os.path.exists(CIRCUIT_FILE_DIR):
    print("Circuit directory not found - is SHEEP_HOME environment variable set?")


def get_gate_name_from_circuit_name(circuit_name):
    """
    Assuming circuits for low-level benchmarks follow naming convention circuit-XXX-N.sheep
    """
    gatename_search = re.search("circuit-([A-Z]+)-[\d]",circuit_name)
    if gatename_search:
        return gatename_search.groups()[0]
    return "unknown"


def run_all(gates, types, contexts, max_depth=10, params_dict={}):
    """
    call this function with lists of the Gates, input-types, and contexts to be benchmarked.
    this in turn calls run_single_benchmark for each combination.
    Params dict is a dictionary of dictionaries, with the top-level key being the context
    """
    uploaded_OK = True
    for gate in gates:
        for input_type in types:
            bitwidth = get_bitwidth(input_type)
            signed = input_type.startswith("i")
            for depth in range(1,max_depth+1):
                circuit_file = CIRCUIT_FILE_DIR+"/circuit-"+gate+"-"+str(depth)+".sheep"
                ## loop over desired contexts
                for context in contexts:
                    ## if we don't have user-specified parameters for this context,
                    ## create an empty dict to pass to run_circuit (will use default params)
                    if not context in params_dict.keys():
                        params_dict[context] = {}
                    print("Doing benchmark for %s %s %i %s" %
                          (context,gate,depth,input_type))
                    ## run the test
                    uploaded_OK &= run_circuit(circuit_file,
                                               input_type,
                                               context,
                                               params_dict[context],
                                               "serial")
                    pass
                pass
            pass
        pass
    return uploaded_OK

def scan_1(contexts=["HElib_Fp","SEAL"]):
    """
    one level of each gate, levels set to 1, 4, 8
    """
    uploaded_OK = True
    for gate in ["ADD","SUBTRACT","MULTIPLY","SELECT","NEGATE"]:
        for input_type in ["bool","uint8_t","int8_t","uint16_t","int16_t","uint32_t","int32_t"]:
            for d in [1,4,8]:
                print("Using context %i" % len(contexts))
                for context in contexts:
                    params = params_for_level(context,d)
                    circuit_file = CIRCUIT_FILE_DIR+"/circuit-"+gate+"-1.sheep"
                    print("Doing benchmark for %s %s %s %i" %
                          (context,gate,input_type,d))
                    uploaded_OK &= run_circuit(circuit_file,
                                              input_type,
                                              context,
                                              params,
                                              "serial")
                    pass ## end of loop over contexts
                pass ## end of loop over depths
            pass ## end of loop over input_types
        pass ## end of loop over gates
    return uploaded_OK


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
