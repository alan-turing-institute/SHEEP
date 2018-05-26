from particles import mini_mod
from particles import variables
from particles import mono_assign, bi_assign


class nb_adder(mini_mod):
    def __init__(self, name, inputs, outputs, nb, randomize_temps=6):
        self.randomize_temps = randomize_temps
        mini_mod.__init__(self, name, inputs, outputs)
        assert(len(inputs) == 3), "You need to have 3 inputs"
        self.nb = nb
        self.create()

    def create(self):
        x = self.inputs[0]
        y = self.inputs[1]
        cin = self.inputs[2]
        temp1 = variables(self.name + '_tmp1',
                          randomize_name=self.randomize_temps)
        temp2 = variables(self.name + '_tmp2',
                          randomize_name=self.randomize_temps)
        add1 = bi_assign(ass_type='xor', lhs=[x, y], rhs=temp1)
        self.add(add1)
        add2 = bi_assign(ass_type='xor', lhs=[temp1, cin],
                         rhs=self.outputs[0])
        self.add(add2)
        ab = bi_assign(ass_type='and', lhs=[x, y],
                       rhs=temp2)
        self.add(ab)
        temp3 = variables(self.name + '_tmp3',
                          randomize_name=self.randomize_temps)
        cindaxb = bi_assign(ass_type='and', lhs=[cin, temp1],
                            rhs=temp3)
        self.add(cindaxb)
        final = bi_assign(ass_type='or', lhs=[temp3, temp2],
                          rhs=self.outputs[1])
        self.add(final)


if __name__ == '__main__':
    x = variables('x')
    y = variables('y')
    cin = variables('cin')
    s = variables('sum')
    c = variables('carry')
    b2_adder = nb_adder('2b_adder', [x, y, cin], [s, c], 2)
    print(b2_adder)
