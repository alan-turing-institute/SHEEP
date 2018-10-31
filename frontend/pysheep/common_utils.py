"""
Python functions that are useful for both the notebooks and the frontend.
"""

import re
import os
import uuid

def get_inputs(circuit_file):
    """
    print out the inputs that a circuit file expects.
    """
    circuit = open(circuit_file)
    for line in circuit.readlines():
        if line.startswith("INPUTS"):
            inputs = line.strip().split()[1:]
            return inputs

def write_inputs_file(inputs,upload_folder="/tmp"):
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
    inputs = [(k,v) for k,v in input_dict.items() if not "(C)" in k]
    const_inputs = [(k,v) for k,v in input_dict.items() if "(C)" in k]

    try:
        for k, v in inputs:
            if not isinstance(v,list):
                return False  ## need to be lists
            if not len(v) == len(inputs[0][1]): ## need to all have the same length
                return False
            for val_int in v:
                if int(val_int) < min_allowed or int(val_int) > max_allowed:
                    return False
        for k, v in const_inputs:
            if not isinstance(v,int):  ## const_inputs need to be integers.
                print("Not an integer!")
                return False
            if v < min_allowed or v > max_allowed:
                return False
        return True  # all inputs were ok
    except:
        return False




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



def parse_test_output(outputstring,debug_filename=None):
    """
    Extract values from the stdout output of the "benchmark" executable.
    return a dict in the format { "processing times (seconds)" : {},
                                  "outputs" : {},
                                  "sizes" : {},
                                  "params":{},
                                  "sizes"{} }
    TODO - refactor this so it is less monolithic and less fragile..
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
#### parse processing times
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
