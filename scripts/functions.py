from particles import variables, enc_vec
from reusable_modules import nb_adder
from interactions import mini_mod, mono_assign
import math


class reduce_add(mini_mod):
    def __init__(self, name, inp_bits, out_bits):
        '''
        Inputs :
        ================================
        inp_bits : Input Vector , type  (enc_vec)
        func : Function for reduce
               Must take two n bit enc_vec and output a k bit enc_vec
        out_bits : Output Vector
        ==================================
        Output a circuit for reduce function as a tree
        '''
        mini_mod.__init__(self, name=name, inputs=inp_bits, outputs=out_bits)
        self.nb = len(self.inputs)
        self.create()

    def create(self):
        inp_vec = self.inputs
        out = self.count_arr(lvl=int(math.log(len(inp_vec)) / math.log(2)))
        for idx in range(len(self.outputs)):
            self.add(mono_assign(ass_type='alias',
                                 lhs=[out[idx]],
                                 rhs=self.outputs[idx]))

    def count_arr(self, lvl):
        inp_len = len(self.inputs)
        lvl_idx = 1
        inp_vec = self.inputs
        while(lvl_idx <= lvl):
            temp_out = enc_vec(name=self.name + '_ra_' + str(lvl_idx),
                               nb=inp_len * (lvl_idx + 1) / (2 * lvl_idx),
                               randomize_name=4)
            for part_id in range(inp_len / (2 * lvl_idx)):
                self.sum_part(
                    inp_vec[part_id * (2 * lvl_idx):
                            (part_id + 1) * (2 * lvl_idx)],
                    temp_out[part_id * (lvl_idx + 1):
                             (part_id + 1) * (lvl_idx + 1)],
                    part_id=part_id)
            inp_vec = temp_out
            inp_len = len(inp_vec)
            lvl_idx += 1

        return inp_vec

    def sum_part(self, inp_vec, out_vec, part_id):
        '''
        Calculates the binary encoding
        of the number of bits in the input vector.
        uses out_vec to output the results.
        '''
        inp_len = len(inp_vec)
        assert(inp_len % 2 == 0), "Only power of 2 accepted as length of input"
        self.add(nb_adder(name=out_vec.name + '_adder_' + str(part_id),
                          inputs=inp_vec,
                          outputs=out_vec,
                          nb=len(inp_vec) / 2))


if __name__ == '__main__':
    inp = enc_vec(name='in', nb=4)
    out = enc_vec(name='out', nb=3)
    ra = reduce_add('adder', inp, out)
    print(ra)
