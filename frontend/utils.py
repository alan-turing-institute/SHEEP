"""
Various helper functions for the SHEEP flask app.
"""

import os
import re
import uuid
import subprocess
from frontend.database import session,BenchmarkMeasurement



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

def cleanup_upload_dir(config):
    """
    At the start of a new test, remove all the uploaded circuits, inputs, and parameters
    files from the uploads dir.
    """
    for file_prefix in ["circuit","inputs","param"]:
        cmd = "rm -p "+config["UPLOAD_FOLDER"]+"/"+file_prefix+"*"
        os.system(cmd)
        
    


def get_bitwidth(input_type):
    """
    given a string input type e.g. bool, uint32_t, return the number of bits,
    (i.e. 2 or 32 for the examples above)
    """
    bitwidth_match = re.search("[\d]+",input_type)
    if bitwidth_match:
        bitwidth = int(bitwidth_match.group())
    else:
        bitwidth = 1
    return bitwidth


def get_min_max(input_type):
    """
    minimum and maximum values for a given data type
    Assume we have bools, or signed-or-unsigned 8,16,32,64 bit integers
    """
    min_allowed = 0
    max_allowed = 0
    if input_type == "bool":
        min_allowed = 0
        max_allowed = 1
    else:
        bitwidth = re.search("[\d]+",input_type).group()
        if input_type.startswith("u"):
            min_allowed = 0
            max_allowed = pow(2,int(bitwidth)) -1
        else:
            min_allowed = -1* pow(2,int(bitwidth)-1)
            max_allowed = pow(2,int(bitwidth)-1) -1
    return min_allowed, max_allowed

    
def check_inputs(input_dict, input_type):
    """ 
    check that the supplied inputs are within the ranges for the specified input type.
    """
    min_allowed, max_allowed = get_min_max(input_type)

    for val in input_dict.values():
        if int(val) < min_allowed or int(val) > max_allowed:
            return False
    return True  # all inputs were ok


def upload_files(filedict,upload_folder):
    """
    Upload circuit file to server storage...  and other files?
    """
    uploaded_filenames = {}
    for k,v in filedict.items():
        uploaded_filename = os.path.join(upload_folder,v.filename)
        v.save(uploaded_filename)
        uploaded_filenames[k] = uploaded_filename
    return uploaded_filenames


def parse_circuit_file(circuit_filename):
    """ 
    read the circuit file and check what the names of the inputs are, so
    they can be selected in another form.
    """
    inputs = []
    f=open(circuit_filename)
    for line in f.readlines():
        if line.startswith("INPUTS"):
            inputs += line.strip().split()[1:]
    return inputs


def write_inputs_file(inputs,upload_folder):
    """
    write the input names and values to a file, 
    just because that's easier to pass to the executable.
    """
    inputs_filename = os.path.join(upload_folder,"inputs_"+str(uuid.uuid4())+".inputs")
    f = open(inputs_filename,"w")
    for k,v in inputs.items():
        f.write(k+" "+str(v)+"\n")
    f.close()
    return inputs_filename

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


def cleanup_time_string(t):
    """
    convert from microseconds to seconds, and only output 3 s.f.
    """
    time_in_seconds = float(t)/1e6

    if time_in_seconds < 1:
        time_in_seconds = round(time_in_seconds,5)
    else:
        time_in_seconds = int(time_in_seconds)
    timestring = str(time_in_seconds)
    return timestring

def check_outputs(output_list):
    """
    take a list of outputs - each should be [test_result,clear_result]
    and check that in each case, the test and clear results are equal.
    """

    for output in output_list:
        if output[0] != output[1]:
            return False
    return True


def parse_test_output(outputstring,debug_filename=None):
    """
    Extract values from the stdout output of the "benchmark" executable.
    return a dict in the format { "processing times (seconds)" : {}, "outputs" : {}, "sizes" : {}, "params":{}, "sizes"{} }
    """
    results = {}
    processing_times = {}
    test_outputs = {}
    params = {}
    sizes = {}
    clear_check = {}
    in_results_section = False
    in_processing_times = False
    in_outputs = False
    if debug_filename:
        debugfile=open(debug_filename,"w")
### parse the file, assuming we have processing times then outputs.
    for line in outputstring.decode("utf-8").splitlines():
        if debug_filename:
            debugfile.write(line+"\n")
#### read lines where parameters are printed out
        param_search = re.search("Parameter ([\S]+) = ([\d]+)",line)
        if param_search:
            params[param_search.groups()[0]] = param_search.groups()[1]
#### read lines where sizes of keys or ciphertexts are printed out
        size_search = re.search("size of ([\S]+):[\s]+([\d]+)",line)
        if size_search:
            sizes[size_search.groups()[0]] = size_search.groups()[1]
        if in_results_section:
#### parse the check against clear context:
            if line.startswith("Cleartext check"):
                clear_check["is_correct"] = "passed OK" in line

            if in_processing_times:
                num_search = re.search("([\w]+)\:[\s]+([\d][\d\.e\+]+)",line)
                if num_search:
                    label = num_search.groups()[0]
                    processing_time = num_search.groups()[1]
                    processing_time = cleanup_time_string(processing_time) ## and convert to seconds
                    processing_times[label] = processing_time
                if "Output values" in line:
                    in_processing_times = False
                    in_outputs = True
            elif in_outputs:
                output_search = re.search("([\w]+)\:[\s]+([-\d]+)",line)
                if output_search:
                    label = output_search.groups()[0]
                    val = output_search.groups()[1]
                    test_outputs[label] = val
                if "END RESULTS" in line:
                    in_results_section = False
            elif "Processing times" in line:
                in_outputs = False
                in_processing_times = True                
        elif "=== RESULTS" in line:
            in_results_section = True
            pass
    results["Processing times (s)"] = processing_times
    results["Outputs"] = test_outputs
    results["Object sizes (bytes)"] = sizes
    results["Parameter values"] = params
    results["Cleartext check"] = clear_check
    if debug_filename:
        debugfile.close()
    return results

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


def run_test(data,config):
    """
    Run the executable in a subprocess, and capture the stdout output.
    return a dict of results {"context_name": {"processing_times" : {},
                                                 "sizes" : {},
                                                 "outputs" : {} 
                                                }, ... 
    """
    results = {}
    contexts_to_run = data["HE_libraries"] 
### always run clear context, for comparison, unless we already have 4 contexts
### in which case the outputs page would be too cluttered...
    if len(contexts_to_run) < 4:
        contexts_to_run.append("Clear")
    for context in contexts_to_run:
        param_file = find_param_file(context,config)
        if not "eval_strategy" in data.keys():
            eval_strategy = "serial"
        else:
            eval_strategy = data["eval_strategy"]
        run_cmd = construct_run_cmd(context,data,config,eval_strategy,param_file)
        p = subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
        output = p.communicate()[0]
        debug_filename = config["UPLOAD_FOLDER"]+"/debug_"+context+".txt"
        results[context] = parse_test_output(output,debug_filename)
        
    return results


def get_params_all_contexts(context_list,input_type,config):
    """
    Return a dict with the key being context_name, and the vals being 
    dicts of param_name:default_val.
    """
    all_params = {}
    for context in context_list:
        all_params[context] = get_params_single_context(context,input_type,config)
    return all_params

def get_params_single_context(context,input_type,config,params_file=None):
    """
    Run the benchmark executable to printout params and default values.
    """
    run_cmd = construct_get_param_cmd(context,input_type,config,params_file)
    print("run_cmd is ",run_cmd)
    p = subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    output = p.communicate()[0]
    params = parse_param_output(output)
    print("OUTPUT ",output)
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
        ### only write to file if the new param is different to the old one
        if v != old_params[k]:
            print("Writing %s %s to params file" % (k,v))
            param_file.write(k+" "+str(v)+"\n")
    param_file.close()
    updated_params = get_params_single_context(context,appdata["input_type"],appconfig,param_filename)
    param_file = open(param_filename,"w")
    if len(updated_params) == 0:  ### something went wrong, e..g.  bad set of parameters
        ### return the default params (i.e. run get_params_single_context with no params file
        params = get_params_single_context(context, appdata["input_type"],appconfig)
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
        circuit_name, num_inputs = get_circuit_name(circuit_path)
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
            input_bitwidth = get_bitwidth(app_data["input_type"]),
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
