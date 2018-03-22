"""
Various helper functions for the SHEEP flask app.
"""

import os
import re
import uuid
import subprocess

def cleanup_upload_dir(config):
    """
    At the start of a new test, remove all the uploaded circuits, inputs, and parameters
    files from the uploads dir.
    """
    for file_prefix in ["circuit","inputs","param"]:
        cmd = "rm "+config["UPLOAD_FOLDER"]+"/"+file_prefix+"*"
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



def check_inputs(input_dict, input_type):
    """ 
    check that the supplied inputs are within the ranges for the specified input type.
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

def construct_run_cmd(context_name,data,config, parameter_file):
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
        time_in_seconds = round(time_in_seconds,3)
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
    return a dict in the format { "processing times (seconds)" : {}, "outputs" : {}, "sizes" : {} }
    """
    results = {}
    processing_times = {}
    test_outputs = {}
##    clear_outputs = []
    outputs = []
    in_results_section = False
    in_processing_times = False
    in_outputs = False
    if debug_filename:
        debugfile=open(debug_filename,"w")
### parse the file, assuming we have processing times then outputs.
    for line in outputstring.decode("utf-8").splitlines():
        if debug_filename:
            debugfile.write(line+"\n")
        if in_results_section:
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
                output_vals = re.findall("[\-\d]+",line)
                if len(output_vals) > 0:
                    outputs.append(output_vals)
                if "END RESULTS" in line:
                    in_results_section = False
            elif "Processing times" in line:
                in_outputs = False
                in_processing_times = True                
        elif "=== RESULTS" in line:
            in_results_section = True
            pass
    results["Processing times (s)"] = processing_times
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
    return params


def find_param_file(context,config):
    """
    If parameters have been set by hand via the frontend, they will
    be in UPLOAD_FOLDER / params_[context].txt.
    return this path if it exists, or None if it doesn't.
    """
    param_filename = config["UPLOAD_FOLDER"]+"/params_"+context+".txt"
    if os.path.exists(param_filename):
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
    for context in data["HE_libraries"]:
        param_file = find_param_file(context,config,param_file)
        run_cmd = construct_run_cmd(context,data,config)
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
    return params
    
def update_params(context,param_dict,appdata,appconfig):
    """
    We have received a dict of params for a given context from the web form.  
    However, we need to get the benchmark executable to calculate params, if e.g. A_predefined_param_set 
    was changed for HElib.  
    So, we write all the params from the form out to a file, run benchmark PARAMS .... , then parse 
    the output, write that to a file, and return it.
    """
    param_filename = os.path.join(appconfig["UPLOAD_FOLDER"],"parameters_"+context+".txt")
    param_file = open(param_filename,"w")
    for k,v in param_dict.items():
        ### ignore the "apply" button:
        if v=="Apply":
            continue
        print("writing "+k+" to "+str(v))
        param_file.write(k+" "+str(v)+"\n")
    param_file.close()
    updated_params = get_params_single_context(context,appdata["input_type"],appconfig,param_filename)
    param_file = open(param_filename,"w")
    for k,v in updated_params.items():
        print("writing updated "+k+" to "+str(v))
        param_file.write(k+" "+str(v)+"\n")
    param_file.close()
    return updated_params
