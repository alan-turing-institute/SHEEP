from reusable_modules import oneb_adder
from particles import variables
import os

if "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["SHEEP_HOME"]
else:
    BASE_DIR = os.environ["HOME"] + "/SHEEP"

CIRCUIT_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/circuits/"
INPUTS_DIR_MID = BASE_DIR + "/benchmark_inputs/mid_level/inputs/TMP/"


class enc_vec(object):
    def __init__(self, name, nb=None, lst=None, randomize_name=4,
                 name_list=None):
        self.randomize_name = randomize_name
        self.name = name
        self._lst = lst
        self.nb = nb
        assert (not (self.nb is None and self._lst is None)
                ), "Need to provide something"
        try:
            assert(self._lst is None or isinstance(self._lst, list))
        except Exception:
            raise NotImplementedError
        self.create(name_list)

    def create(self, name_list):
        if name_list is None:
            self._lst = [variables(name=self.name + str(idx),
                                   randomize_name=self.randomize_name)
                         for idx in range(self.nb)]
        else:
            assert(len(name_list) == self.nb), "Name list has differnt length"
            self._lst = [variables(name=name_list[idx])
                         for idx in range(self.nb)]

    def get_input_dict(self, inp_var):
        assert(isinstance(inp_var, list)), "Input should be a list"
        assert(len(inp_var) == self.nb), "length should be the same as vec"
        inp_dict = {}
        for idx in range(self.nb):
            inp_dict[self._lst[idx].name] = inp_var[idx]
        return inp_dict

    def __getitem__(self, item):
        return self._lst[item]

    def __add__(self, next_list):
        assert isinstance(next_list, enc_vec)
        new_name = self.name + '_' + next_list.name
        randomize_name = max(self.randomize_name, next_list.randomize_name)
        new_nb = self.nb + next_list.nb
        new_lst = self._lst + next_list._lst
        new_vec = enc_vec(name=new_name,
                          nb=new_nb,
                          lst=new_lst,
                          randomize_name=randomize_name)
        return new_vec

    def __len__(self):
        return self.nb


class circuit(object):
    def __init__(self, name, circuit, const_inputs=[]):
        self.name = name
        self.circuit = circuit
        self.const_inputs = const_inputs

    def get_inputs(self):
        assert self.circuit is not None
        self.const_inputs = self.const_inputs  # + self.circuit.const_inputs
        self.inputs = self.circuit.inputs
        self.outputs = self.circuit.outputs

    def output_circuit(self, filename):
        with open(filename, 'wb') as outfile:
            outfile.write("INPUTS ")
            for ci in self.const_inputs:
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
