"""
Script to generate a circuit to sort a list of input values.
The number of inputs to the sorting algorithm must be a power of 2.

The outputs will be the input values ordered from high-to-low.
"""
import os
import random


if "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["SHEEP_HOME"]
else:
    BASE_DIR = os.environ["HOME"]+"/SHEEP"    

CIRCUIT_DIR_MID = BASE_DIR+"/benchmark_inputs/mid_level/circuits"
INPUTS_DIR_MID  = BASE_DIR+"/benchmark_inputs/mid_level/inputs/TMP"


def generate_inputs(num_inputs,minval,maxval):
    """
    randomly generate inputs 
    """
    values = {}
    for i in range(num_inputs):
        values["x_"+str(i)] = (int(random.randint(minval,maxval)))
    return values
                    


def generate_circuit(num_inputs):
    """
    Generate the circuit.
    """
    pass

def write_circuit_file(filename, inputs,outputs,assignments):
    """
    write the circuit to a file.
    """
    outfile = open(filename,"w")
    outfile.write("INPUTS ")
    for i in inputs:
        outfile.write(" "+i)
    outfile.write("\n")
    outfile.write("OUTPUTS ")
    for o in outputs:
        outfile.write(" "+o)
    outfile.write("\n")
    for a in assignments:
        outfile.write(a+"\n")
    outfile.close()



def generate(num_inputs):
    """ 
    To be called by e.g. jupyter notebook - generate a circuit and put it in a standard location.
    """
    inputs, outputs, assignments = generate_circuit(num_inputs)
    filename = CIRCUIT_DIR_MID+"/circuit-bitonic-sort-"+str(num_inputs)+".sheep"
    write_circuit_file(filename,inputs,outputs,assignments)    
    return filename


if __name__ == "__main__":
    for num_inputs in range(4,20):
        inputs, outputs, assignments = generate_circuit(num_inputs)
        write_circuit_file(CIRCUIT_DIR_MID+"/circuit-bitonic-sort-"+str(num_inputs)+".sheep",inputs,outputs,assignments)
        input_vals = generate_inputs(num_inputs)
        write_inputs_file(INPUTS_DIR_MID+"/inputs-bitonic-sort-"+str(num_inputs)+".inputs",input_vals)
