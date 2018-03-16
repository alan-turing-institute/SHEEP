"""
Automate generation of circuit files and input files to run benchmarks on.
"""

import subprocess
import random
import os
import re

EXECUTABLE_DIR = os.env["HOME"]+"/SHEEP/build/bin"

OUTPUT_DIR = os.env["HOME"]+"/SHEEP/benchmark_inputs"

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
        


if __name__ == "__main__":

    for depth in range(1,10):
        for gate in ["ADD","SUBTRACT","MULTIPLY"]:
            output_circuit_filename = os.path.join(OUTPUT_DIR,"circuit-"+gate+"-"+str(depth)+".sheep")

            run_cmd = []
            run_cmd.append(os.path.join(EXECUTABLE_DIR,"simple-circuit-maker"))
            run_cmd.append(gate)
            run_cmd.append(str(depth))
            run_cmd.append(output_circuit_filename)
            p = subprocess.Popen(args=run_cmd, stdout=subprocess.PIPE)
## now generate inputs of different sizes
        input_types = ["bool"]
        for bitwidth in ["8","16","32"]:
            input_types.append("uint"+bitwidth+"_t")
            input_types.append("int"+bitwidth+"_t")            
        for input_type in input_types:
            output_inputs_filename = os.path.join(OUTPUT_DIR,"inputs-"+input_type+"-"+str(depth)+".inputs")
            output_inputs_file = open(output_inputs_filename,"w")
            for i in range(depth+1):
                output_inputs_file.write("input_"+str(i)+" "+str(rnd_num_in_range(input_type))+"\n")

            output_inputs_file.close()
                
            
