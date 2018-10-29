"""
Automate generation of circuit files and input files to run benchmarks on.
"""

import subprocess
import random
import os
import re

if "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["SHEEP_HOME"]
else:
    BASE_DIR = os.path.join(os.environ["HOME"]+",SHEEP","frontend")    


EXECUTABLE_DIR = BASE_DIR+"/build/bin"

CIRCUIT_DIR_LOW = BASE_DIR+"/benchmark_inputs/low_level/circuits"
INPUTS_DIR_LOW  = BASE_DIR+"/benchmark_inputs/low_level/inputs"

def rnd_num_in_range(input_type):
    """ 
    generate a random number of the appropriate type
    """
    if input_type == "bool":
        return random.randint(0,1)
    else:
        bitwidth = re.search("[\d]+",input_type).group()
        if input_type.startswith("u"):
            return random.randint(0,pow(2,int(bitwidth))-1)
        else:
            return random.randint(-1*pow(2,int(bitwidth)-1),
                                  pow(2,int(bitwidth)-1)-1)            
        

def generate_2_to_1_inputs(input_type, depth):
    """
    generate randomly-generated inputs for depth-level circuits where
    each gate takes two inputs and gives one output.  For depth N, we need 
    N+1 input values.
    """
    output_inputs_filename = os.path.join(INPUTS_DIR_LOW,"inputs-2-to-1-"+input_type+"-"+str(depth)+".inputs")
    output_inputs_file = open(output_inputs_filename,"w")
    for i in range(depth+1):
        output_inputs_file.write("input_"+str(i)+" "+str(rnd_num_in_range(input_type))+"\n")
        
    output_inputs_file.close()

def generate_1_to_1_inputs(input_type, depth):
    """
    generate randomly-generated inputs for depth-level circuits where
    each gate takes one inputs and gives one output.  
    We only need one input value
    """
    output_inputs_filename = os.path.join(INPUTS_DIR_LOW,"inputs-1-to-1-"+input_type+"-"+str(depth)+".inputs")
    output_inputs_file = open(output_inputs_filename,"w")

    output_inputs_file.write("input_0 "+str(rnd_num_in_range(input_type))+"\n")
        
    output_inputs_file.close()

    
def generate_select_inputs(input_type, depth):
    """
    generate randomly-generated inputs for depth-level circuits where
    each gate takes two input values and a select value (bool).  
    For depth N, we need N+1 input values.
    Current format is input_i
    """
    output_inputs_filename = os.path.join(INPUTS_DIR_LOW,"inputs-select-"+input_type+"-"+str(depth)+".inputs")
    output_inputs_file = open(output_inputs_filename,"w")
    for i in range(depth+1):
        output_inputs_file.write("input_"+str(i)+" "+str(rnd_num_in_range(input_type))+"\n")
        if i < depth:
            output_inputs_file.write("select_"+str(i)+" "+str(rnd_num_in_range(input_type))+"\n")            
        
    output_inputs_file.close()
    
    

        
def generate_simple_circuit(gate,max_depth):
    """
    generate sets of circuits that contain sequences of the same gate,
    using the simple-circuit-maker executable,
    which in turn uses circuit-repo.hpp
    """
    for depth in range(max_depth):
        output_circuit_filename = os.path.join(CIRCUIT_DIR_LOW,"circuit-"+gate+"-"+str(depth)+".sheep")

        run_cmd = []
        run_cmd.append(os.path.join(EXECUTABLE_DIR,"simple-circuit-maker"))
        run_cmd.append(gate)
        run_cmd.append(str(depth))
        run_cmd.append(output_circuit_filename)
        p = subprocess.Popen(args=run_cmd, stdout=subprocess.PIPE)


def generate_inputs(depth):
    ## get list of input sizes sizes
    input_types = ["bool"]
    for bitwidth in ["8","16","32"]:
        input_types.append("uint"+bitwidth+"_t")
        input_types.append("int"+bitwidth+"_t")
    for input_type in input_types:
        generate_2_to_1_inputs(input_type,depth)
        generate_1_to_1_inputs(input_type,depth)
        generate_select_inputs(input_type,depth)
        
        
if __name__ == "__main__":

    for depth in range(1,10):
        for gate in ["ADD","SUBTRACT","MULTIPLY","SELECT","COMPARE","NEGATE"]:
            generate_simple_circuit(gate,depth)
## now generate inputs of different sizes
        generate_inputs(depth)

                
            
