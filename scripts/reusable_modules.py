from particles import mini_mod
from particles import variables
from particles import mono_assign, bi_assign


class oneb_adder(mini_mod):
    def __init__(self, name, inputs, outputs, nb=None, randomize_temps=1):
        self.randomize_temps = randomize_temps
        mini_mod.__init__(self, name, inputs, outputs)
        assert(len(inputs) == 3), "You need to have 3 inputs"
        self.nb = nb
        if self.nb is not None:
            assert(self.nb == 1)
        self.create()

    def create(self):
        cin = self.inputs[0]
        x = self.inputs[1]
        y = self.inputs[2]
        temp1 = variables(self.name + '_tmp1',
                          randomize_name=self.randomize_temps)
        temp2 = variables(self.name + '_tmp2',
                          randomize_name=self.randomize_temps)
        temp3 = variables(self.name + '_tmp3',
                          randomize_name=self.randomize_temps)
        self.add(bi_assign(ass_type='xor', lhs=[x, y], rhs=temp1))
        self.add(bi_assign(ass_type='xor', lhs=[temp1, cin],
                           rhs=self.outputs[0]))
        self.add(bi_assign(ass_type='and', lhs=[x, y],
                           rhs=temp2))

        self.add(bi_assign(ass_type='and', lhs=[cin, temp1],
                           rhs=temp3))
        self.add(bi_assign(ass_type='or', lhs=[temp3, temp2],
                           rhs=self.outputs[1]))


class nb_adder(mini_mod):
    def __init__(self, name, inputs, outputs, nb, randomize_temps=1):
        '''
        Adds x1 and x2 where the bit length of both is nb
        Inputs will be structured as [cin x1 x2] - size(2*nb)+1
        Outputs - size (b+1)
        '''
        mini_mod.__init__(self, name, inputs, outputs)
        self.randomize_temps = randomize_temps
        self.nb = nb
        assert(self.nb * 2 + 1 == len(inputs)), "You need to have " + \
            str(self.nb + 1) + " inputs not " + str(len(self.inputs))
        self.carry_list = [variables(self.name + '_carry_' + str(i),
                                     randomize_name=self.randomize_temps)
                           for i in range(self.nb + 1)]
        assert(len(self.outputs) ==
               self.nb + 1), "Output bitwidth should be  the length of adder"
        self.create()

    def create(self):
        inp1 = self.inputs[1:self.nb + 1]
        inp2 = self.inputs[self.nb + 1:]
        self.add(mono_assign(ass_type='alias',
                             lhs=[self.inputs[0]],
                             rhs=self.carry_list[0]))
        for bit_id in range(self.nb):
            self.add(oneb_adder(name='2b_adder_' + str(bit_id),
                                inputs=[self.carry_list[bit_id],
                                        inp1[bit_id],
                                        inp2[bit_id]],
                                outputs=[self.outputs[bit_id],
                                         self.carry_list[bit_id + 1]]))
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
