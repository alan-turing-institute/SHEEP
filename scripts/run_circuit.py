"""
Simple script to write user-specified keys and values into an inputs file
that can be given to run_circuit.py.
"""
import subprocess
import os, uuid
from frontend.utils import parse_test_output


if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["HOME"]+"/SHEEP"
else:
    BASE_DIR = os.environ["SHEEP_HOME"]

TMP_INPUTS_DIR = BASE_DIR+"/benchmark_inputs/mid_level/inputs/TMP"
if not os.path.exists(TMP_INPUTS_DIR):
    os.system("mkdir "+TMP_INPUTS_DIR)
    
EXECUTABLE_DIR = BASE_DIR+"/build/bin"


def get_inputs(circuit_file):
    """ 
    print out the inputs that a circuit file expects.
    """
    circuit = open(circuit_file)
    for line in circuit.readlines():
        if line.startswith("INPUTS"):
            inputs = line.strip().split()[1:]
            return inputs


def write_inputs_file(value_dict):
    """
    write k,v pairs into a file.  Randomly generate the filename
    and return to the user.
    """
    filename = TMP_INPUTS_DIR+"/inputs-"+str(uuid.uuid4())+".txt"
    inputs_file = open(filename,"w")
    for k,v in value_dict.items():
        inputs_file.write(k+" "+str(v)+"\n")
    inputs_file.close()
    return filename
                       

def run_circuit(circuit_file,inputs_file,input_type,context,num_inputs=None,debugfilename=None):
    """
    run the circuit and retrieve the results.
    """
    run_cmd=[]
    run_cmd.append(os.path.join(EXECUTABLE_DIR,"benchmark"))
    run_cmd.append(circuit_file)
    run_cmd.append(context)
    run_cmd.append(input_type)
    run_cmd.append(inputs_file)    
    p=subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    job_output = p.communicate()[0]
    results = parse_test_output(job_output,debugfilename)
    return results["Outputs"]
