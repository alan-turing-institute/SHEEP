"""
Various helper functions for the SHEEP flask app.
"""

import os
import re
import uuid
import subprocess


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

def construct_run_cmd(data,config):
    """
    Build up the list of arguments to be sent to subprocess.Popen in order to run
    the benchmark test.
    """
    circuit_file = data["uploaded_filenames"]["circuit_file"]
    inputs_file = data["uploaded_filenames"]["inputs_file"]
    context_name = data["HE_library"]
    input_type = data["input_type"]
    parameter_file = None
    if "parameter_file" in data["uploaded_filenames"].keys():
        parameter_file = data["uploaded_filenames"]["parameter_file"]
    # run_cmd is a list of arguments to be passed to subprocess.run()
    run_cmd = [config["EXECUTABLE_DIR"]+"/benchmark"]
    run_cmd.append(circuit_file)
    run_cmd.append(context_name)    
    run_cmd.append(input_type) 
    run_cmd.append(inputs_file)    
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


def parse_test_output(outputstring):
    """
    Extract values from the stdout output of the "benchmark" executable.
    """
    processing_times = []
    test_outputs = []
    clear_outputs = []
    outputs = []
    in_results_section = False
    in_processing_times = False
    in_outputs = False
### parse the file, assuming we have processing times then outputs.
    for line in outputstring.decode("utf-8").splitlines():
        if in_results_section:
            if in_processing_times:
                num_search = re.search("[\d][\d\.e\+]+",line)
                if num_search:
                    processing_time = num_search.group()
                    processing_time = cleanup_time_string(processing_time)
                    processing_times.append(processing_time)  ## assume we keep the same order - setup, enc, eval, dec
                if "Output values" in line:
                    in_processing_times = False
                    in_outputs = True
            elif in_outputs:
                output_vals = re.findall("[\d]+",line)
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
    return processing_times, outputs
        
def run_test(data,config):
    """
    Run the executable in a subprocess, and capture the stdout output.
    """
    run_cmd = construct_run_cmd(data,config)
    p = subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    output = p.communicate()[0]
    return parse_test_output(output)
