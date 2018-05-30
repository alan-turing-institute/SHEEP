from reusable_modules import oneb_adder
from particles import variables
import os

if "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["SHEEP_HOME"]
else:
    BASE_DIR = os.environ["HOME"] + "/SHEEP"

CIRCUIT_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/circuits/"
INPUTS_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/inputs/TMP/"


class circuit(object):
    def __init__(self, name, circuit,
                 const_inputs=[]):
        self.name = name

        self.circuit = circuit
        self.const_inputs = const_inputs
        self.flag_inputs = ['TRUE', 'FALSE']

    def get_inputs(self):
        assert self.circuit is not None
        self.const_inputs = self.const_inputs  # + self.circuit.const_inputs
        self.inputs = self.circuit.inputs
        self.outputs = self.circuit.outputs

    def output_circuit(self, filename):
        with open(filename, 'wb') as outfile:
            outfile.write("CONST_INPUTS")
            outfile.write("\n")
            outfile.write("INPUTS ")
            for ci in self.flag_inputs:
                outfile.write(" " + ci)
            for i in self.inputs:
                outfile.write(" " + i)
            outfile.write("\n")
            outfile.write("OUTPUTS ")
            for o in self.outputs:
                outfile.write(" " + o)
            outfile.write("\n")
            outfile.write(str(self.circuit))
            outfile.write("\n")

    def write_file(self, filename=None):
        self.get_inputs()
        if filename is None:
            self.write_default_file()
        else:
            self.write_spec_file(filename)

    def write_default_file(self):
        filename = CIRCUIT_DIR_MID + self.name + ".sheep"
        self.output_circuit(filename=filename)

    def write_spec_file(self, filename):
        self.output_circuit(filename=filename)


if __name__ == '__main__':
    x = variables('x')
    y = variables('y')
    cin = variables('cin')
    s = variables('sum')
    c = variables('carry')
    b2_adder = oneb_adder('2b_adder', [x, y, cin], [s, c], 1)
    circuit = circuit('2 Bit Adder', b2_adder)
    circuit.write_file(filename='./test.sheep')
