"""
Run over all input files in a directory, and store outputs in database.
"""
import subprocess
import os
import re

EXECUTABLE_DIR = "/Users/nbarlow/SHEEP/sketch/build/bin"

INPUT_FILE_DIR = "/Users/nbarlow/SHEEP/sketch/benchmark_inputs"

DEBUG_FILE_DIR = "/Users/nbarlow/SHEEP/sketch/debug"

from frontend.database import BenchmarkMeasurement,session
from frontend.utils import parse_test_output, check_outputs, get_bitwidth

def insert_measurement(context,
                       bitwidth,
                       gate,
                       depth,
                       nslots,
                       execution_time,
                       is_correct,
                       num_threads=1,
                       parameters="Default"):
    m = BenchmarkMeasurement(context_name=context,
                             input_bitwidth=bitwidth,
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
                         input_type):
    run_cmd=[]
    run_cmd.append(os.path.join(EXECUTABLE_DIR,"benchmark"))
    run_cmd.append(os.path.join(INPUT_FILE_DIR,input_circuit))
    run_cmd.append(context)
    run_cmd.append(input_type)
    run_cmd.append(os.path.join(INPUT_FILE_DIR,input_vals_file))    
    p=subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    job_output = p.communicate()[0]
### write job's stdout to a file for debugging
    debug_output_name = context+"_"+input_circuit.split(".")[0]+"_"+\
                        input_vals_file.split(".")[0]
    outfile = open(os.path.join(DEBUG_FILE_DIR,debug_output_name),"w")
    outfile.write(job_output.decode("utf-8"))
    outfile.close()
### parse the file, return the outputs
    processing_times, outputs = parse_test_output(job_output)
    is_correct = check_outputs(outputs)
    eval_time = processing_times[2]
    return eval_time, is_correct

def run_many_benchmarks(gates,types,contexts,max_depth=9):
    for gate in gates:
        for input_type in types:
            bitwidth = get_bitwidth(input_type)
            for depth in range(1,max_depth):
                circuit_file = "circuit-"+gate+"-"+str(depth)+".sheep"
                inputs_file = "inputs-"+input_type+"-"+str(depth)+".inputs"
                for context in contexts:
                    print("Doing benchmark for %s %s %i %s" %
                          (context,gate,depth,input_type))
                    eval_time, is_correct = run_single_benchmark(
                        circuit_file,
                        inputs_file,
                        context,
                        input_type)
                

                    insert_measurement(
                        context,
                        bitwidth,
                        gate,
                        depth,
                        1,
                        eval_time,
                        is_correct)
                
                    
            


if __name__ == "__main__":
    db = connect_to_db(DB_FILENAME)
    cursor = db.cursor()
