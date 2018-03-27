"""
A set of scripts, likely to be run via Jupyter notebook, to allow setting input 
and parameter values to temporary files, and running the benchmark job.

"""
import subprocess
import os, uuid
from frontend.utils import parse_test_output, get_bitwidth
from frontend.database import BenchmarkMeasurement, session
import re

if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["HOME"]+"/SHEEP"
else:
    BASE_DIR = os.environ["SHEEP_HOME"]

TMP_INPUTS_DIR = BASE_DIR+"/benchmark_inputs/mid_level/inputs/TMP"
if not os.path.exists(TMP_INPUTS_DIR):
    os.system("mkdir "+TMP_INPUTS_DIR)
    

TMP_PARAMS_DIR = BASE_DIR+"/benchmark_inputs/params/TMP"
if not os.path.exists(TMP_PARAMS_DIR):
    os.system("mkdir "+TMP_PARAMS_DIR)

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
    filename = TMP_INPUTS_DIR+"/inputs-"+str(uuid.uuid4())+".inputs"
    inputs_file = open(filename,"w")
    for k,v in value_dict.items():
        inputs_file.write(k+" "+str(v)+"\n")
    inputs_file.close()
    return filename

def write_params_file(param_dict):
    """
    write k,v pairs into a file.  Randomly generate the filename
    and return to the user.
    """
    filename = TMP_PARAMS_DIR+"/params-"+str(uuid.uuid4())+".params"
    params_file = open(filename,"w")
    for k,v in value_dict.items():
        inputs_file.write(k+" "+str(v)+"\n")
    params_file.close()
    return filename

def get_circuit_name(circuit_filename):
    """
    parse the circuit filename, and, assuming it follows a naming convention,
    return the name of the circuit and the number of inputs.
    If it doesn't follow the convention, just return the filename and 0.
    """
    filename_without_path = circuit_filename.split("/")[-1]
    match = re.search("circuit-([-_\w]+)-([\d]+).sheep",filename_without_path)
    if match:
        return match.groups()[0], int(match.groups()[1])
### some circuits (e.g. PIR) follow a different convention:
    match = re.search("circuit-([-_\w]+).sheep",filename_without_path)
    if match:
        return match.groups()[0], 0
### if we got to here, just return the filename
    return filename_without_path, 0


def get_gate_name(circuit_filename):
    """
    parse the circuit filename, and, assuming it follows a naming convention,
    return the name of the gate and the depth.
    If it doesn't follow the convention, just return None,None
    """
    filename_without_path = circuit_filename.split("/")[-1]
    match = re.search("circuit-([-_\w]+)-([\d]+).sheep",filename_without_path)
    if match:
        return match.groups()[0], int(match.groups()[1])
### Didn't find matching names - return None
    return None, None

    
def run_circuit(circuit_filepath,inputs_file,input_type,context,params_file=None,debugfilename=None):
    """
    run the circuit and retrieve the results.
    """
    run_cmd=[]
    run_cmd.append(os.path.join(EXECUTABLE_DIR,"benchmark"))
    run_cmd.append(circuit_filepath)
    run_cmd.append(context)
    run_cmd.append(input_type)
    run_cmd.append(inputs_file)
    if params_file:
        run_cmd.append(params_file)
    p=subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    job_output = p.communicate()[0]
    results = parse_test_output(job_output,debugfilename)
    #### now just add other fields into the "results" dict, to go into the db
    results["context"] = context    
    input_bitwidth = get_bitwidth(input_type)
    input_signed = input_type.startswith("i")
    results["input_bitwidth"] = input_bitwidth
    results["input_signed"] = input_signed
    if "low_level" in circuit_filepath:
        gate_name, depth = get_gate_name(circuit_filepath)
        results["gate_name"] = gate_name
        results["depth"] = depth
    elif "mid_level" in circuit_filepath:
        circuit_name, num_inputs = get_circuit_name(circuit_filepath)
        results["circuit_name"] = circuit_name
        results["num_inputs"] = num_inputs


    upload_measurement(results)
    return results["Outputs"]

def upload_measurement(results):
    """
    insert a single benchmark run into the database.
    """
    execution_time = results["Processing times (s)"]["circuit_evaluation"]
    is_correct = results["Cleartext check"]["is_correct"]
    sizes = results["Object sizes (bytes)"]                    
    ciphertext_size = sizes["ciphertext"]
    public_key_size = sizes["publicKey"]
    private_key_size = sizes["privateKey"]
    param_dict = results["Parameter values"]
    
    m = BenchmarkMeasurement(context_name=results["context"],
                             input_bitwidth=results["input_bitwidth"],
                             input_signed=results["input_signed"],
                             execution_time=execution_time,
                             is_correct=is_correct,
                             ciphertext_size=ciphertext_size,
                             public_key_size=public_key_size,
                             private_key_size=private_key_size)
### the next two will be filled for low-level benchmarks
    if "gate_name" in results.keys():
        m.gate_name=results["gate_name"]
    if "depth" in results.keys():
        m.depth = results["depth"]
### these two will be filled for mid-level benchmarks         
    if "circuit_name" in results.keys():
        m.circuit_name = results["circuit_name"]
    if "num_inputs" in results.keys():
        m.num_inputs = results["num_inputs"]
### the following two are unlikely to be filled for now..
    if "num_slots" in results.keys():
        m.num_slots = results["num_slots"]
    if "tbb_enabled" in results.keys():
        m.tbb_enabled = results["tbb_enabled"]
### now fill the parameter columns for the chosen context

    context_prefix = results["context"].split("_")[0]  ### only have HElib, not HElib_F2 and HElib_Fp
    for k,v in param_dict.items():
        column = context_prefix+"_"+k
        m.__setattr__(column,v)
### commit to the DB        
    session.add(m)
    session.commit()
