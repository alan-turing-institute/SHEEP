"""
Script to element wise multiply :
a binary weight (cleartext) with a binary input (ciphertext).
The number of input vals needs to be passed to the circuit as a "CONST_INPUT"
The circuit then calculates

The output will be a vector y_i = w_i XNOR x_i
"""
import os
import random


if "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["SHEEP_HOME"]
else:
    BASE_DIR = os.environ["HOME"] + "/SHEEP"

CIRCUIT_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/circuits"
INPUTS_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/inputs/TMP"


def generate_inputs(num_inputs, mean, sigma):
    """
    randomly generate inputs from a gaussian
    distribution (rounded to integers).
    however, these values should not be too large, to avoid integer overflows..
    """
    values = {}
    values["N"] = num_inputs
    for i in range(num_inputs):
        values["x_" + str(i)] = (int(random.gauss(mean, sigma)))
    return values


def generate_circuit(num_inputs, weight):
    """
    Generate the circuit.
    """
    const_inputs = []
    inputs = ['INPUT_' + str(i) for i in range(weight.size)]
    outputs = ['OUTPUT_' + str(i) for i in range(weight.size)]
    assignments = []

    for i in range(num_inputs):
        if weight[i]:
            assignments.append(inputs[i] + ' ALIAS ' + outputs[i])
        else:
            assignments.append(inputs[i] + ' NEGATE ' + outputs[i])

    return const_inputs, inputs, outputs, assignments


def write_inputs_file(filename, input_vals):
    """
    write input values to a file.
    """
    outfile = open(filename, "w")
    outfile.write("N " + str(len(input_vals)) + "\n")
    for i in range(len(input_vals)):
        outfile.write("INPUT_" + str(i) + " " + str(input_vals[i]) + "\n")
        pass
    outfile.close()


def write_circuit_file(filename, const_inputs, inputs, outputs, assignments):
    """
    write the circuit to a file.
    """
    outfile = open(filename, "w")
    outfile.write("INPUTS ")
    for ci in const_inputs:
        outfile.write(" " + ci)
    for i in inputs:
        outfile.write(" " + i)
    outfile.write("\n")
    outfile.write("OUTPUTS ")
    for o in outputs:
        outfile.write(" " + o)
    outfile.write("\n")
    for a in assignments:
        outfile.write(a + "\n")
    outfile.close()


def generate(num_inputs, weight):
    """
    To be called by e.g. jupyter notebook -
    generate a circuit and put it in a standard location.
    """
    const_inputs, inputs, outputs, assignments = generate_circuit(
        num_inputs, weight)
    filename = CIRCUIT_DIR_MID + "/circuit-xnor_multiply-" + \
        str(num_inputs) + ".sheep"
    write_circuit_file(filename, const_inputs, inputs, outputs, assignments)
    return filename


if __name__ == "__main__":
    for num_inputs in range(4, 20):
        const_inputs, inputs, outputs, assignments = generate_circuit(
            num_inputs)
        write_circuit_file(CIRCUIT_DIR_MID + "/circuit-xnor_multiply-" +
                           str(num_inputs) + ".sheep", const_inputs,
                           inputs, outputs, assignments)
        input_vals = generate_inputs(num_inputs)
        write_inputs_file(INPUTS_DIR_MID + "/inputs-xnor_multiply-" +
                          str(num_inputs) + ".inputs", input_vals)
