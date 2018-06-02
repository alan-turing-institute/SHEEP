from particles import variables, enc_vec
from interactions import mono_assign, bi_assign, mini_mod, tri_assign
import copy


class oneb_adder(mini_mod):
    def __init__(self, name, inputs, outputs, nb=None,
                 randomize_temps=1, carry=True):
        self.randomize_temps = randomize_temps
        mini_mod.__init__(self, name, inputs, outputs)
        if carry:
            assert(len(inputs) == 3), "You need to have 3 inputs"
        else:
            assert(len(inputs) == 2), "You need to have 2 inputs"
        self.nb = nb
        if self.nb is not None:
            assert(self.nb == 1)
        self.create(carry)

    def create(self, carry):
        if carry:
            cin = self.inputs[0]

        x = self.inputs[carry]
        y = self.inputs[carry + 1]
        temp1 = variables(self.name + '_tmp1',
                          randomize_name=self.randomize_temps)
        temp2 = variables(self.name + '_tmp2',
                          randomize_name=self.randomize_temps)
        temp3 = variables(self.name + '_tmp3',
                          randomize_name=self.randomize_temps)
        self.add(bi_assign(ass_type='xor', lhs=[x, y], rhs=temp1))

        if carry:
            self.add(bi_assign(ass_type='xor', lhs=[temp1, cin],
                               rhs=self.outputs[0]))
        else:
            self.add(mono_assign(ass_type='alias', lhs=[temp1],
                                 rhs=self.outputs[0]))

        self.add(bi_assign(ass_type='and', lhs=[x, y],
                           rhs=temp2))

        if carry:
            self.add(bi_assign(ass_type='and', lhs=[cin, temp1],
                               rhs=temp3))
            self.add(bi_assign(ass_type='or', lhs=[temp3, temp2],
                               rhs=self.outputs[1]))
        else:
            self.add(mono_assign(ass_type='alias', lhs=[temp2],
                                 rhs=self.outputs[1]))


class nb_adder(mini_mod):
    def __init__(self, name, inputs, outputs, nb, randomize_temps=2,
                 carry_out=True, carry_in=False):
        '''
        Adds x1 and x2 where the bit length of both is nb
        Inputs will be structured as [cin x1 x2] - size(2*nb)+1
        Outputs - size (b+1)
        '''
        mini_mod.__init__(self, name, inputs, outputs)
        self.randomize_temps = randomize_temps
        self.nb = nb
        if not carry_in:
            assert(self.nb * 2 == len(inputs)), "You need to have " + \
                str(2 * self.nb) + " inputs not " + str(len(self.inputs))
        else:
            assert(self.nb * 2 + 1 == len(inputs)), "You need to have " + \
                str(2 * self.nb + 1) + " inputs not " + str(len(self.inputs))

        self.carry_list = [variables(self.name + '_carry_' + str(i),
                                     randomize_name=self.randomize_temps)
                           for i in range(self.nb + 1)]

        # TODO - Fill with zeros
        assert(len(self.outputs) ==
               self.nb + 1), "Output bitwidth should be length of adder: " + \
            str(self.nb + 1) + " not " + str(len(self.outputs))
        self.create(carry_out=carry_out, carry_in=carry_in)

    def create(self, carry_out, carry_in):
        inp1 = self.inputs[0 + int(carry_in):self.nb + int(carry_in)]
        inp2 = self.inputs[self.nb + int(carry_in):]

        if carry_in:
            self.add(mono_assign(ass_type='alias',
                                 lhs=[self.inputs[0]],
                                 rhs=self.carry_list[0]))
            self.carry_list[0] = self.inputs[0]
        for bit_id in range(self.nb):
            if bit_id > 0 or carry_in:
                self.add(oneb_adder(name=self.name + '2b_adder_' + str(bit_id),
                                    inputs=[self.carry_list[bit_id],
                                            inp1[bit_id],
                                            inp2[bit_id]],
                                    outputs=[self.outputs[bit_id],
                                             self.carry_list[bit_id + 1]],
                                    carry=True))
            else:
                self.add(oneb_adder(name=self.name + '2b_adder_' + str(bit_id),
                                    inputs=[inp1[bit_id],
                                            inp2[bit_id]],
                                    outputs=[self.outputs[bit_id],
                                             self.carry_list[bit_id + 1]],
                                    carry=bit_id > 0))
        if carry_out:
            self.add(mono_assign(ass_type='alias',
                                 lhs=[self.carry_list[-1]],
                                 rhs=self.outputs[-1]))


class nb_adder_xy(mini_mod):
    '''
    Adds two number of different bit lengths
    '''

    def __init__(self, name, inputs, outputs, nb, randomize_temps=2,
                 carry_out=True):
        '''
        Adds x1 and x2 where the bit length of both is nb
        Inputs will be structured as [cin x1 x2] - size(2*nb)+1
        Outputs - size (b+1)
        '''
        self.x = inputs[0]
        self.y = inputs[1]
        mini_mod.__init__(self, name, self.x + self.y, outputs)

        self.len1 = len(self.x)
        self.len2 = len(self.y)

        if self.len1 < self.len2:
            tmp = copy.deepcopy(self.x)
            self.x = copy.deepcopy(self.y)
            self.y = tmp

        self.randomize_temps = randomize_temps
        self.nb = nb
        assert(self.nb == self.len1), "You need to have " + \
            str(self.nb) + " inputs not " + str(len(self.x))

        self.carry_list = [variables(self.name + '_carry_' + str(i),
                                     randomize_name=self.randomize_temps)
                           for i in range(self.nb + 1)]

        assert(len(self.outputs) ==
               self.nb + 1), "Output bitwidth should be length of adder: " + \
            str(self.nb + 1) + " not " + str(len(self.outputs))
        self.create(carry_out=carry_out)

    def create(self, carry_out):
        inp1 = self.x
        inp2 = self.y

        for bit_id in range(self.nb):
            if bit_id > 0 and bit_id < self.len2:
                self.add(oneb_adder(name=self.name + '2b_adder_' + str(bit_id),
                                    inputs=[self.carry_list[bit_id],
                                            inp1[bit_id],
                                            inp2[bit_id]],
                                    outputs=[self.outputs[bit_id],
                                             self.carry_list[bit_id + 1]],
                                    carry=True))
            elif bit_id == 0:
                self.add(oneb_adder(name=self.name + '2b_adder_' + str(bit_id),
                                    inputs=[inp1[bit_id],
                                            inp2[bit_id]],
                                    outputs=[self.outputs[bit_id],
                                             self.carry_list[bit_id + 1]],
                                    carry=bit_id > 0))
            elif bit_id >= self.len2:
                self.add(oneb_adder(name=self.name + '2b_adder_' + str(bit_id),
                                    inputs=[self.carry_list[bit_id],
                                            inp1[bit_id]],
                                    outputs=[self.outputs[bit_id],
                                             self.carry_list[bit_id + 1]],
                                    carry=False))
        if carry_out:
            self.add(mono_assign(ass_type='alias',
                                 lhs=[self.carry_list[-1]],
                                 rhs=self.outputs[-1]))


class compare_cp(mini_mod):
    def __init__(self, name, inputs, outputs):
        mini_mod.__init__(self, inputs=inputs[0], outputs=outputs, name=name)
        self.plaintext = [x for x in bin(inputs[1])[::-1][:-2]]
        self.create()

    def create(self):
        if isinstance(self.outputs, enc_vec):
            rel_out = self.outputs[0]
        else:
            rel_out = self.outputs
        temp1 = enc_vec(name=self.name + 'out_bit', nb=len(self.inputs))
        self.add(bi_assign(ass_type='and', lhs=[self.inputs[0],
                                                self.zero],
                           rhs=temp1[0]))
        temp2 = enc_vec(name=self.name + 'flag_bit', nb=len(self.inputs))
        for idx in range(len(self.inputs)):
            if idx < len(self.plaintext) and int(self.plaintext[idx]) == 1:
                self.add(tri_assign(ass_type='mux',
                                    lhs=[self.inputs[idx],
                                         temp1[idx],
                                         self.zero],
                                    rhs=temp2[idx]))
            else:
                self.add(tri_assign(ass_type='mux',
                                    lhs=[self.inputs[idx],
                                         self.one,
                                         temp1[idx]],
                                    rhs=temp2[idx]))
            if idx < len(self.inputs) - 1:
                self.add(mono_assign(ass_type='alias',
                                     lhs=[temp2[idx]], rhs=temp1[idx + 1]))
            else:
                self.add(mono_assign(ass_type='alias',
                                     lhs=[temp2[idx]], rhs=rel_out))


if __name__ == '__main__':
    x = variables('x')
    y = variables('y')
    cin = variables('cin')
    s = variables('sum')
    c = variables('carry')
    b2_adder = oneb_adder('2b_adder', [cin, x, y], [s, c], 1)
    print(b2_adder)
