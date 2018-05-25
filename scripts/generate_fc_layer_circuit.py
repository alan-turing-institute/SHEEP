"""
Generate circuit for a FC layer
"""
import generate_xnor_circuit
import os
import random


if "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["SHEEP_HOME"]
else:
    BASE_DIR = os.environ["HOME"] + "/SHEEP"

CIRCUIT_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/circuits"
INPUTS_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/inputs/TMP"


def generate_circuit(num_inputs, weight, circuit_name='FC'):
    """
    Generate the circuit.
    """
    inputs = [circuit_name + 'INPUT_' + str(i) for i in range(weight.size)]

    fc_const_inputs, fc_inputs, fc_outputs, fc_assignments =\
        generate_xnor_circuit.generate_circuit(
            num_inputs, weight, inputs=inputs)
    const_inputs = fc_const_inputs
    inputs = fc_inputs
    outputs = fc_outputs
    assignments = fc_assignments
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
    filename = CIRCUIT_DIR_MID + "/circuit-fc_layer-" + \
        str(num_inputs) + ".sheep"
    write_circuit_file(filename, const_inputs, inputs, outputs, assignments)
    return filename
