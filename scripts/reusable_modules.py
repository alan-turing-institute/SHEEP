from particles import variables
from interactions import mono_assign, bi_assign, mini_mod


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
    def __init__(self, name, inputs, outputs, nb, randomize_temps=1,
                 carry_out=True):
        '''
        Adds x1 and x2 where the bit length of both is nb
        Inputs will be structured as [cin x1 x2] - size(2*nb)+1
        Outputs - size (b+1)
        '''
        mini_mod.__init__(self, name, inputs, outputs)
        self.randomize_temps = randomize_temps
        self.nb = nb
        assert(self.nb * 2 == len(inputs)), "You need to have " + \
            str(2 * self.nb) + " inputs not " + str(len(self.inputs))
        self.carry_list = [variables(self.name + '_carry_' + str(i),
                                     randomize_name=self.randomize_temps)
                           for i in range(self.nb + 1)]

        # TODO - Fill with zeros
        assert(len(self.outputs) ==
               self.nb + 1), "Output bitwidth should be length of adder: " + \
            str(self.nb + 1) + " not " + str(len(self.outputs))
        self.create(carry_out=carry_out)

    def create(self, carry_out):
        inp1 = self.inputs[0:self.nb]
        inp2 = self.inputs[self.nb:]
        # self.add(mono_assign(ass_type='alias',
        #                     lhs=[self.inputs[0]],
        #                     rhs=self.carry_list[0]))
        for bit_id in range(self.nb):
            if bit_id > 0:
                self.add(oneb_adder(name='2b_adder_' + str(bit_id),
                                    inputs=[self.carry_list[bit_id],
                                            inp1[bit_id],
                                            inp2[bit_id]],
                                    outputs=[self.outputs[bit_id],
                                             self.carry_list[bit_id + 1]],
                                    carry=bit_id > 0))
            else:
                self.add(oneb_adder(name='2b_adder_' + str(bit_id),
                                    inputs=[inp1[bit_id],
                                            inp2[bit_id]],
                                    outputs=[self.outputs[bit_id],
                                             self.carry_list[bit_id + 1]],
                                    carry=bit_id > 0))
        if carry_out:
            self.add(mono_assign(ass_type='alias',
                                 lhs=[self.carry_list[-1]],
                                 rhs=self.outputs[-1]))


if __name__ == '__main__':
    x = variables('x')
    y = variables('y')
    cin = variables('cin')
    s = variables('sum')
    c = variables('carry')
    b2_adder = oneb_adder('2b_adder', [cin, x, y], [s, c], 1)
    print(b2_adder)
