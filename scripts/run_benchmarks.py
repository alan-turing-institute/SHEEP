"""
Run over all input files in a directory, and store outputs in database.
"""
import subprocess
import os
import re

EXECUTABLE_DIR = os.environ["HOME"]+"/SHEEP/build/bin"

INPUT_FILE_DIR = os.environ["HOME"]+"/SHEEP/benchmark_inputs"
if not os.path.exists(INPUT_FILE_DIR):
    os.system("mkdir "+INPUT_FILE_DIR)

DEBUG_FILE_DIR = os.environ["HOME"]+"/SHEEP/debug"
if not os.path.exists(DEBUG_FILE_DIR):
    os.system("mkdir "+DEBUG_FILE_DIR)

from frontend.database import BenchmarkMeasurement,session
from frontend.utils import parse_test_output, check_outputs, get_bitwidth

def insert_measurement(context,
                       bitwidth,
                       signed,
                       gate,
                       depth,
                       nslots,
                       execution_time,
                       is_correct,
                       num_threads=1,
                       parameters="Default"):
    """
    insert a single benchmark run into the database.
    """
    
    m = BenchmarkMeasurement(context_name=context,
                             input_bitwidth=bitwidth,
                             input_signed=signed,
                             gate_name=gate,
                             depth=depth,
                             num_slots=nslots,
                             num_threads=num_threads,
                             parameters=parameters,
                             execution_time=execution_time,
                             is_correct=is_correct)
    session.add(m)
    session.commit()


def run_single_benchmark(input_circuit,
                         input_vals_file,
                         context,
                         input_type,
                         debug=False):
    """
    Call the executable "benchmark" with command-line args for a specific run,
    and parse the stdout output to get the processing time, and whether the 
    test outputs equal the ClearContext outputs.
    """
    run_cmd=[]
    run_cmd.append(os.path.join(EXECUTABLE_DIR,"benchmark"))
    run_cmd.append(os.path.join(INPUT_FILE_DIR,input_circuit))
    run_cmd.append(context)
    run_cmd.append(input_type)
    run_cmd.append(os.path.join(INPUT_FILE_DIR,input_vals_file))    
    p=subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    job_output = p.communicate()[0]
### write job's stdout to a file for debugging
    if debug:
        debug_output_name = context+"_"+input_circuit.split(".")[0]+"_"+\
                            input_vals_file.split(".")[0]
        outfile = open(os.path.join(DEBUG_FILE_DIR,debug_output_name),"w")
        outfile.write(job_output.decode("utf-8"))
        outfile.close()
### parse the file, return the outputs

    results = parse_test_output(job_output)
    processing_times = results["Processing times (s)"]
    outputs = results["Outputs"] 
    is_correct = check_outputs(outputs)
    eval_time = processing_times[2]
    return eval_time, is_correct



def run_many_benchmarks(gates,types,contexts,max_depth=9):
    """
    call this function with lists of the Gates, input-types, and contexts to be benchmarked.
    this in turn calls run_single_benchmark for each combination.
    """
    for gate in gates:
        for input_type in types:
            bitwidth = get_bitwidth(input_type)
            signed = input_type.startswith("i")
            for depth in range(1,max_depth):
                circuit_file = "circuit-"+gate+"-"+str(depth)+".sheep"

### inputs file depends on the gate now - most gates have depth+1 inputs, but SELECT and NEGATE have
### different requirements
                
                inputs_file = "inputs-"
                if gate == "SELECT":
                    inputs_file += "select-"
                elif gate == "NEGATE":
                    inputs_file += "1-to-1-"
                else:
                    inputs_file += "2-to-1-"
                inputs_file +=input_type+"-"+str(depth)+".inputs"
### loop over desired contexts
                for context in contexts:
                    print("Doing benchmark for %s %s %i %s" %
                          (context,gate,depth,input_type))
### run the test
                    eval_time, is_correct = run_single_benchmark(
                        circuit_file,
                        inputs_file,
                        context,
                        input_type)
### insert the measurement into the database                    
                    insert_measurement(
                        context,
                        bitwidth,
                        signed,
                        gate,
                        depth,
                        1,
                        eval_time,
                        is_correct)
                
                    
            


if __name__ == "__main__":
    db = connect_to_db(DB_FILENAME)
    cursor = db.cursor()
