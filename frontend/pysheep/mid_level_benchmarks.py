"""
Script to generate circuits and inputs for mid-level benchmarks, in particular:
 * single server PIR, using the recursive pir trick.
 * mean and variance
 * bitonic sort
"""

import os
import subprocess
import random

if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["HOME"]+"/SHEEP"
else:
    BASE_DIR = os.environ["SHEEP_HOME"]

CIRCUIT_DIR = BASE_DIR+"/benchmark_inputs/mid_level/circuits"
EXECUTABLE_DIR = BASE_DIR+"/build/bin"

VAR_COUNT = 0

def new_var():
    global VAR_COUNT
    VAR_COUNT = VAR_COUNT + 1
    return 'w{}'.format(VAR_COUNT - 1)

def generate_pir_circuit(database_size, alphas):
    """
    Generate the circuit.
    """
    filename = CIRCUIT_DIR+"/circuit-pir-"+str(database_size)
    for a in alphas:
        filename += "_"+str(a)
    filename += ".sheep"
    sizes = []
    l = database_size
    for i, v in enumerate(alphas):
        l, x = divmod(l, v)
        sizes.append(l)
        #l = l2
        assert x == 0
    output_length = sizes[len(alphas) - 1]
    outputs = None
    inputs = ['d_0_{}_{}'.format(i, j) for i in range(alphas[0]) for j in range(sizes[0])]
    inputs += ['s_{}_{}'.format(i, j) for i in range(len(alphas)) for j in range(alphas[i])]
    operations = []

    for i in range(len(alphas)):
        # component-wise multiplication
        for j in range(alphas[i]):
            #print i, sizes
            for z in range(sizes[i]):
                op = ('c_{}_{}_{}'.format(i, j, z),
                      'MULTIPLY' if i == 0 else 'MULTIPLY',  # First one should be MULTBYCONST
                      's_{}_{}'.format(i, j), 'd_{}_{}_{}'.format(i, j, z))
                operations.append(op)
        # component-wise sum
        next_database = []
        for j in range(sizes[i]):
            op = ('e_{}_{}'.format(i, j),
                  'ADD',
                  ['c_{}_{}_{}'.format(i, z, j) for z in range(alphas[i])])
            operations.append(op)
            next_database.append('e_{}_{}'.format(i, j))
        # rename
        index = 0
        #print next_database
        if i < len(alphas) - 1:
            for j in range(alphas[i+1]):
                for z in range(sizes[i+1]):
                    #print j, z
                    op = ('d_{}_{}_{}'.format(i + 1, j, z),
                          'ALIAS',
                          next_database[index])
                    index += 1
                    operations.append(op)
        else:
            outputs = next_database


    #for op in operations:
    #    print op

    write_output(operations, inputs, outputs, filename)
    return filename

def binary_add_tree(assignment):
    lhs, op, arg_list = assignment
    if len(arg_list) == 2:
        return ['{} {} {} {}'.format(arg_list[0], arg_list[1], op, lhs)]
    if len(arg_list) == 1:
        return ['{} {} {}'.format(arg_list[0], 'ALIAS', lhs)]
    else:
        arg_list_1 = arg_list[: len(arg_list) / 2]
        arg_list_2 = arg_list[len(arg_list) / 2 :]
        x1 = new_var()
        x2 = new_var()
        l1 = binary_add_tree((x1, op, arg_list_1))
        l2 = binary_add_tree((x2, op, arg_list_2))
        return ['{} {} {} {}'.format(x1, x2, op, lhs)] + l1 + l2



def write_output(operations, inputs, outputs, filename):
    """
    write the circuit to a file.
    """
    outfile = open(filename,"w")
    outfile.write("INPUTS ")
    for i in inputs:
        outfile.write(" "+i)
    outfile.write("\n")
    outfile.write("OUTPUTS ")
    for o in outputs:
        outfile.write(" "+o)
    outfile.write("\n")
    for t in operations:
        lhs = t[0]
        op = t[1]
        assert not op == 'MULTIPLY' or len(t) == 4
        assert not op == 'MULTIPLY' or len(t) == 4 # Should be MULTBYCONST
        assert not op == 'ADD' or (len(t) == 3 and isinstance(t[2], list))
        if op in {'MULTIPLY', 'MULTIPLY'}: # One should be MULTBYCONST
            outfile.write('{} {} {} {}\n'.format(t[2], t[3], op, lhs))
        elif op in ('ALIAS'):
            outfile.write('{} {} {}\n'.format(t[2],  op, lhs))
        else:
            assert op == 'ADD'
            ops = binary_add_tree(t)
            ops.reverse()
            for s in ops:
                outfile.write(s)
                outfile.write('\n')
    outfile.close()


def generate_bitonic_sort_circuit(num_inputs):
    """
    Generate the circuit.
    """
    run_cmd=[]
    circuit_filename = os.path.join(CIRCUIT_DIR,"circuit-bitonic-sort-"+str(num_inputs)+".sheep")
    run_cmd.append(os.path.join(EXECUTABLE_DIR,"bitonic-sorting-circuit"))
    run_cmd.append(str(num_inputs))
    run_cmd.append(circuit_filename)
    p=subprocess.Popen(args=run_cmd,stdout=subprocess.PIPE)
    job_output = p.communicate()[0]
    return circuit_filename

def generate_gaussian_inputs(num_inputs,mean,sigma):
    """
    randomly generate inputs from a gaussian distribution (rounded to integers).
    however, these values should not be too large, to avoid integer overflows..
    """
    values = {}
    for i in range(num_inputs):
        values["x_"+str(i)] = [(int(random.gauss(mean,sigma)))]
    return values


def generate_variance_circuit(num_inputs):
    """
    Generate the circuit.
    """
    const_inputs = ["N"]
    inputs = []
    outputs = ["Nxbar", "varianceN3"]  ### (mean * N) and (variance * N^3)
    assignments = []
    last_output = ""
    filename = CIRCUIT_DIR+"/circuit-variance-"+str(num_inputs)+".sheep"
    ## first, add all the inputs, to get the sum N.xbar
    for i in range(num_inputs):
        inputs.append("x_"+str(i))
        if i == 0:
            continue
        elif i == 1:
            assignments.append("x_0 x_1 ADD y_0")
        else:
            assignments.append("y_"+str(i-2)+" x_"+str(i)+" ADD y_"+str(i-1))
            pass
        pass
    last_output = assignments[-1].split()[-1]
    assignments.append(last_output+" ALIAS Nxbar")

    ## now multiply each of the inputs by N and subtract from Nxbar
    for i in range(num_inputs):
        assignments.append("x_"+str(i)+" N  MULTBYCONST Nx_"+str(i))
        assignments.append("Nxbar Nx_"+str(i)+" SUBTRACT v_"+str(i))
    ## now square each of these outputs by multiplying with an ALIAS of themself
        assignments.append("v_"+str(i)+" ALIAS vv_"+str(i))
        assignments.append("v_"+str(i)+" vv_"+str(i)+" MULTIPLY s_"+str(i))
    ## finally sum these squares
    for i in range(1,num_inputs):
        if i == 1:
            assignments.append("s_0 s_1 ADD ss_0")
        else:
            assignments.append("ss_"+str(i-2)+" s_"+str(i)+" ADD ss_"+str(i-1))
            pass
        pass
    last_output = assignments[-1].split()[-1]
    assignments.append(last_output+" ALIAS varianceN3")
    outfile = open(filename,"w")
    outfile.write("CONST_INPUTS ")
    for ci in const_inputs:
        outfile.write(" "+ci)
    outfile.write("\nINPUTS ")
    for i in inputs:
        outfile.write(" "+i)
    outfile.write("\n")
    outfile.write("OUTPUTS ")
    for o in outputs:
        outfile.write(" "+o)
    outfile.write("\n")
    for a in assignments:
        outfile.write(a+"\n")
    outfile.close()
    return filename



def generate_vector_dot_product_circuit(input_0, input_1):
    """
    generate a circuit to do input_0.input_1 assuming that the
    HE context has enough "slots" to hold the vectors.
    """
    if len(input_0) != len(input_1):
        raise RuntimeError("input_0 and input_1 must be the same length")
    circuit_str = "INPUTS input_0 input_1\nCONST_INPUTS rotate_1\n"
    circuit_str += "OUTPUTS output prod_s1 prod_s2 prod_s3\n"
    circuit_str += "input_0 input_1 MULTIPLY prod_r0\n"
    for i in range(len(input_0)-1):
        circuit_str += "prod_r{} rotate_1 ROTATE prod_r{}\n".format(i,i+1)
        if i==0:
            circuit_str += "prod_r0 prod_r1 ADD prod_s1\n"
        else:
            circuit_str += "prod_s{} prod_r{} ADD prod_s{}\n".format(i,i+1,i+1)
    circuit_str += "prod_s{} ALIAS output\n".format(i+1)
    return circuit_str


def rotate_vec(input_vec, n):
    """
    rotate an input vec by n places, wrapping around.
    """
    output_vec = []
    for i in range(len(input_vec)):
        output_vec.append(input_vec[(i-n)%len(input_vec)])
    return output_vec


def generate_matrix_vector_mult(input_matrix, input_vec):
    """
    Takes a matrix expressed as a list of lists (each inner list being a row)
    and a vector expressed as a list, and uses the GAZELLE (arXiv:1801.05507)
    trick for expressing the matrix.
    Will return, a circuit (string), a dictionary of input values, and a dictionary
    of const_input_values (though the latter is trivial).
    """
    for row in input_matrix:
        if len(row) != len(input_vec):
            print("Width of matrix must be the same as length of vector")
            return
    const_input_vals = {"rotate_minus1": -1}
    circuit_str = "OUTPUTS output_vec\n"
    circuit_str += "CONST_INPUTS rotate_minus1\n"
    circuit_str += "INPUTS input_vec "
    input_vals = {}
    input_vals["input_vec"] = input_vec
    for i in range(len(input_matrix)):
        for j in range(len(input_vec)):
            index = rotate_vec(list(range(len(input_vec))),i)[j]
            if not "mstrip_{}".format(index) in input_vals.keys():
                input_vals["mstrip_{}".format(index)] = []
                circuit_str += "mstrip_{} ".format(index)
            input_vals["mstrip_{}".format(index)].append(input_matrix[i][j])
            pass
        pass
    circuit_str+= "\n"
    ## now start doing the assigments - multiply each diagonal strip by vec
    ## and rotate vec by 1.
    circuit_str += "input_vec ALIAS vec_r0\n"
    circuit_str += "mstrip_0 vec_r0 MULTIPLY prod_0\n"
    for i in range(1,len(input_vec)):
        circuit_str += "vec_r{} rotate_minus1 ROTATE vec_r{}\n".format(i-1,i)
        circuit_str += "mstrip_{} vec_r{}  MULTIPLY prod_{}\n".format(i,i,i)
    ## we now have prod_0 up to prod_(n-1) - just need to sum them
    circuit_str += "prod_0 prod_1 ADD sum_0\n"
    for i in range(1, len(input_vec)-1):
        circuit_str += "sum_{} prod_{} ADD sum_{}\n".format(i-1,i+1,i)
    ## rename the final output
    circuit_str += "sum_{} ALIAS output_vec\n".format(len(input_vec)-2)

    ## return the circuit, the input_val dict and the const_input_val dict
    return circuit_str, input_vals, const_input_vals
