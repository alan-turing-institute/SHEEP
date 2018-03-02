"""
Automate generation of circuit files and input files to run benchmarks on.
"""

import subprocess
import random
import os

EXECUTABLE_DIR = "/Users/nbarlow/SHEEP/sketch/build/bin"

OUTPUT_DIR = "/Users/nbarlow/SHEEP/sketch/benchmark_files"

def rnd_num_in_range(input_type):
    """ 
    generate a random number of the appropriate type
    """
    if input_type == "bool":
        return random.randint(0,1)



if __name__ == "__main__":

    for depth in range(1,10):
        for gate in ["ADD","SUBTRACT","MULTIPLY"]:
            output_circuit_filename = os.path.join(OUTPUT_DIR,"test-circuit-"+gate+"-"+str(depth)+".sheep")

            run_cmd = []
            run_cmd.append(os.path.join(EXECUTABLE_DIR,"simple-circuit-maker"))
            run_cmd.append(gate)
            run_cmd.append(str(depth))
            run_cmd.append(output_circuit_filename)
## now generate inputs of different sizes
        input_types = ["bool"]
        for bitwidth in ["8","16","32"]:
            input_types.append("uint"+bitwidth)
            input_types.append("int"+bitwidth)            
        for input_type in input_types:
            output_inputs_filename = os.path.join(OUTPUT_DIR,"test-inputs-"+input_type+"-"+str(depth)+".inputs")
            output_inputs_file = open(output_inputs_filename,"w")
            for i in range(depth+1):
                output_inputs_file.write(str(rnd_num_in_range(input_type))+" ")
            output_inputs_file.write("\n")
            output_inputs_file.close()
                
            
