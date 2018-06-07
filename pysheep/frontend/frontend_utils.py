"""
Various helper functions for the SHEEP flask app.
"""

import os
import re
import uuid
import subprocess

from ..common.database import session,BenchmarkMeasurement
from ..common import common_utils
from ..interface import sheep_client


def cleanup_upload_dir(config):
    """
    At the start of a new test, remove all the uploaded circuits, inputs, and parameters
    files from the uploads dir.
    """
    for file_prefix in ["circuit","inputs","param"]:
        cmd = "rm -fr "+config["UPLOAD_FOLDER"]+"/"+file_prefix+"*"
        os.system(cmd)
        os.makedirs(config["UPLOAD_FOLDER"],exist_ok=True)


def upload_files(filedict,upload_folder):
    """
    Upload circuit file to server storage.
    """
    uploaded_filenames = {}
    for k,v in filedict.items():
        uploaded_filename = os.path.join(upload_folder,v.filename)
        v.save(uploaded_filename)
        uploaded_filenames[k] = uploaded_filename
    return uploaded_filenames


def construct_run_cmd(context_name,data,config, eval_strategy="serial", parameter_file=None):
    """
    Build up the list of arguments to be sent to subprocess.Popen in order to run
    the benchmark test.
    """
    circuit_file = data["uploaded_filenames"]["circuit_file"]
    inputs_file = data["uploaded_filenames"]["inputs_file"]
    input_type = data["input_type"]

    # run_cmd is a list of arguments to be passed to subprocess.run()
    run_cmd = [config["EXECUTABLE_DIR"]+"/benchmark"]
    run_cmd.append(circuit_file)
    run_cmd.append(context_name)    
    run_cmd.append(input_type) 
    run_cmd.append(inputs_file)
    run_cmd.append(eval_strategy)
    if parameter_file:
        run_cmd.append(parameter_file)
    return run_cmd


def construct_get_param_cmd(context_name,input_type,config,parameter_file=None):
    """
    Build up the list of arguments to be sent to subprocess.Popen in order to run
    the benchmark test to get the params for chosen context.
    """

    # run_cmd is a list of arguments to be passed to subprocess.run()
    run_cmd = [config["EXECUTABLE_DIR"]+"/benchmark"]
    run_cmd.append("PARAMS")
    run_cmd.append(context_name)    
    run_cmd.append(input_type) 
    if parameter_file:
        run_cmd.append(parameter_file)
    return run_cmd


def parse_param_output(outputstring):
    """
    read the output of benchmark PARAMS <context_name>
    and return a dict {param_name: val , ... }
    """
    params = {}
    for line in outputstring.decode("utf-8").splitlines():
        if line.startswith("Parameter"):
            #### line will be of format "Parameter x = y" - we want x and y
            tokens = line.strip().split()
            params[tokens[1]] = tokens[3]
    print("PARAM_OUTPUT",params)
    return params


def find_param_file(context,config):
    """
    If parameters have been set by hand via the frontend, they will
    be in UPLOAD_FOLDER / params_[context].txt.
    return this path if it exists, or None if it doesn't.
    """
    param_filename = config["UPLOAD_FOLDER"]+"/parameters_"+context+".txt"
    if os.path.exists(param_filename):
        print("Found PARAM FILE ",param_filename)
        return param_filename
    else:
        return None


def run_test(data):
    """
    run the executable and return the results dict.
    """
    results = {} ## will be a dictionary key-ed by context
    contexts_to_run = data["HE_libraries"]
### always run clear context, for comparison, unless we already have 4 contexts
### in which case the outputs page would be too cluttered...
    if len(contexts_to_run) < 4:
        contexts_to_run.append("Clear")
        pass
    for context in contexts_to_run:
        sheep_client.new_job()
        sheep_client.set_input_type(data["input_type"])
        sheep_client.set_context(context)
        sheep_client.set_circuit_filename(data["uploaded_filenames"]["circuit_file"])
        sheep_client.set_inputs(data["input_vals"])
#        sheep_client.set_parameters(data["params"][context])
        sheep_client.set_eval_strategy(data["eval_strategy"][context])
        sheep_client.run_job()

        results[context] = sheep_client.get_results()
    return results
        
#def run_test(data,config):
#    """
#    Run the executable in a subprocess, and capture the stdout output.
#    return a dict of results {"context_name": {"processing_times" : {},
#                                                 "sizes" : {},
#                                                 "outputs" : {} 
#                                                }, ... 
#    """
#    results = {}
#    contexts_to_run = data["HE_libraries"] 
#### always run clear context, for comparison, unless we already have 4 contexts
#### in which case the outputs page would be too cluttered...
#    if len(contexts_to_run) < 4:
#        contexts_to_run.append("Clear")
#    for context in contexts_to_run:
#        param_file = find_param_file(context,config)
#        if not "eval_strategy" in data.keys():
#            eval_strategy = "serial"
#        else:
#            eval_strategy = data["eval_strategy"]
#        run_cmd = construct_run_cmd(context,data,config,eval_strategy,param_file)
#        p = subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
#        output = p.communicate()[0]
#        debug_filename = config["UPLOAD_FOLDER"]+"/debug_"+context+".txt"
#        results[context] = common_utils.parse_test_output(output,debug_filename)
#        
#    return results
#

def get_params_all_contexts(context_list,input_type):
    """
    Return a dict with the key being context_name, and the vals being 
    dicts of param_name:default_val.
    """
    all_params = {}
    for context in context_list:
        all_params[context] = get_params_single_context(context,input_type)
    return all_params


#def get_params_single_context(context,input_type,config,params_file=None):
#    """
#    Run the benchmark executable to printout params and default values.
#    """
#    run_cmd = construct_get_param_cmd(context,input_type,config,params_file)
#    print("run_cmd is ",run_cmd)
#    p = subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
#    output = p.communicate()[0]
#    params = parse_param_output(output)
#    print("OUTPUT ",output)
#    return params

def get_params_single_context(context, input_type):
    """
    use sheep_client to get parameters for given context and input type
    """
    sheep_client.set_context(context)
    sheep_client.set_input_type(input_type)
    params = sheep_client.get_parameters()
    return params


def update_params(context,param_dict,appdata,appconfig):
    """
    We have received a dict of params for a given context from the web form.  
    However, we need to get the benchmark executable to calculate params, if e.g. A_predefined_param_set 
    was changed for HElib.  
    So, we write all the params from the form out to a file, run benchmark PARAMS .... , then parse 
    the output, write that to a file, and return it.
    """
    old_params = appdata["params"][context]
    print("OLD_PARAMS",old_params)
    param_filename = os.path.join(appconfig["UPLOAD_FOLDER"],"parameters_"+context+".txt")
    print( " param_filename",param_filename)
    param_file = open(param_filename,"w")
    print("param_dict ", param_dict)
    for k,v in param_dict.items():
        ### ignore the "apply" button:
        if v=="Apply":
            continue
        ### treat the evaluation strategy separately
        if k=="eval_strategy":
            appdata["eval_strategy"][context] = v
            continue
        ### only write to file if the new param is different to the old one
        if v != old_params[k]:
            print("Writing %s %s to params file" % (k,v))
            param_file.write(k+" "+str(v)+"\n")
    param_file.close()
    updated_params = get_params_single_context(context,appdata["input_type"])
    param_file = open(param_filename,"w")
    if len(updated_params) == 0:  ### something went wrong, e..g.  bad set of parameters
        ### return the default params (i.e. run get_params_single_context with no params file
        params = get_params_single_context(context, appdata["input_type"])
        return params
    for k,v in updated_params.items():
        param_file.write(k+" "+str(v)+"\n")
    param_file.close()
    return updated_params


def upload_test_result(results,app_data):
    """
    Save data from a user-specified circuit test.
    """
    print("Uploading results to DB")
    for context in app_data["HE_libraries"]:
        result = results[context]
        ### see if it follows naming convention for a low-level benchmark test
        circuit_path = app_data["uploaded_filenames"]["circuit_file"]
        circuit_name, num_inputs = common_utils.get_circuit_name(circuit_path)
        execution_time = result["Processing times (s)"]["circuit_evaluation"]
        is_correct = result["Cleartext check"]["is_correct"]
        sizes = result["Object sizes (bytes)"]                    
        ciphertext_size = sizes["ciphertext"]
        public_key_size = sizes["publicKey"]
        private_key_size = sizes["privateKey"]
        param_dict = result["Parameter values"]    
        cm = BenchmarkMeasurement(
            circuit_name = circuit_name,
##            num_inputs = num_inputs,
            context_name = context,
            input_bitwidth = common_utils.get_bitwidth(app_data["input_type"]),
            input_signed = app_data["input_type"].startswith("i"),
            execution_time = execution_time,
            is_correct = is_correct,
            ciphertext_size = sizes["ciphertext"],
            private_key_size = sizes["privateKey"],
            public_key_size = sizes["publicKey"]
        )

        context_prefix = context.split("_")[0]  ### only have HElib, not HElib_F2 and HElib_Fp
        for k,v in param_dict.items():
            column = context_prefix+"_"+k
            cm.__setattr__(column,v)
        session.add(cm)
        session.commit()
    return
