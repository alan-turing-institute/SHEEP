"""
A set of scripts, likely to be run via Jupyter notebook, to allow setting input
and parameter values to temporary files, and running the benchmark job.

"""
import requests

import os, uuid
import random
from . import common_utils
from . import sheep_client
from .database import BenchmarkMeasurement, Timing, ParameterSetting, session, upload_benchmark_result

if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.path.join(os.environ["HOME"],"SHEEP","frontend")
else:
    BASE_DIR = os.environ["SHEEP_HOME"]

TMP_INPUTS_DIR = BASE_DIR+"/benchmark_inputs/mid_level/inputs/TMP"
if not os.path.exists(TMP_INPUTS_DIR):
    os.system("mkdir "+TMP_INPUTS_DIR)


TMP_PARAMS_DIR = BASE_DIR+"/benchmark_inputs/params/TMP"
if not os.path.exists(TMP_PARAMS_DIR):
    os.system("mkdir "+TMP_PARAMS_DIR)


def generate_input_vals(inputs, const_inputs, input_type, nslots):
    """
    randomly generate input values conforming to the selected input types
    """
    lower,upper = common_utils.get_min_max(input_type)
    inputs_dict = {}
    for k in inputs:
        inputs_dict[k] = []
        for slot in range(nslots):
            inputs_dict[k].append(random.randint(lower,upper))
    const_inputs_dict = {}
    for k in const_inputs:
        const_inputs_dict[k] = random.randint(allowed_range)
    return inputs_dict, const_inputs_dict


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
    for k,v in param_dict.items():
        params_file.write(k+" "+str(v)+"\n")
    params_file.close()
    return filename



def run_circuit(circuit_file, input_type, context, params, eval_strategy="serial"):
    """
    Run the circuit and retreive the results.
    """
    sheep_client.new_job()
    ## configure the sheep client
    sheep_client.set_context(context)
    sheep_client.set_input_type(input_type)
    sheep_client.set_parameters(params)
    sheep_client.set_eval_strategy(eval_strategy)
    sheep_client.set_circuit(circuit_file)

    ## randomly assign input values
    nslots = sheep_client.get_nslots()["content"]["nslots"]
    inputs = sheep_client.get_inputs()["content"]
    const_inputs = sheep_client.get_const_inputs()["content"]
    input_vals, const_input_vals = generate_input_vals(inputs, const_inputs, input_type, nslots)
    sheep_client.set_inputs(input_vals)
    sheep_client.set_const_inputs(input_vals)

    ## run the job
    sheep_client.run_job()
    ## get the results
    results = sheep_client.get_results()["content"]
    parameters = sheep_client.get_parameters()["content"]
    uploaded_OK = upload_benchmark_result(circuit_file.split("/")[-1],
                                          context,
                                          input_type,
                                          len(inputs),
                                          nslots,
                                          eval_strategy=="parallel",
                                          results,
                                          parameters)
    return uploaded_OK

#def run_circuit(circuit_filepath,inputs_file,input_type,context,eval_strategy="serial",params_file=None,debugfilename=None):
#    """
#    run the circuit and retrieve the results.
#    """
#    run_cmd=[]
#    run_cmd.append(os.path.join(EXECUTABLE_DIR,"benchmark"))
#    run_cmd.append(circuit_filepath)
#    run_cmd.append(context)
#    run_cmd.append(input_type)
#    run_cmd.append(inputs_file)
#    run_cmd.append(eval_strategy)
#    if params_file:
#        run_cmd.append(params_file)
#    p=subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
#    job_output = p.communicate()[0]
#    results = common_utils.parse_test_output(job_output,debugfilename)
#    #### now just add other fields into the "results" dict, to go into the db
#    results["context"] = context
#    input_bitwidth = common_utils.get_bitwidth(input_type)
#    input_signed = input_type.startswith("i")
#    results["input_bitwidth"] = input_bitwidth
#    results["input_signed"] = input_signed
#    if "low_level" in circuit_filepath:
#        gate_name, depth = common_utils.get_gate_name(circuit_filepath)
#        results["gate_name"] = gate_name
#        results["depth"] = depth
#    elif "mid_level" in circuit_filepath:
#        circuit_name, num_inputs = common_utils.get_circuit_name(circuit_filepath)
#        results["circuit_name"] = circuit_name
#        results["num_inputs"] = num_inputs
#
#
#    upload_measurement(results)
#    return results["Outputs"]

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
