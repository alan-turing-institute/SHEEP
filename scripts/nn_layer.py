from vector_ops import vec_mono_op_cond
from interactions import mini_mod, bi_assign
from particles import enc_vec
from functions import reduce_add
from reusable_modules import compare_cp
import numpy as np
import math


class nn_layer(mini_mod):

    def __init__(self, layer_type, weight, name, inputs, outputs):
        mini_mod.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.weight = weight
        isinstance(self.weight, np.ndarray)
        self.check_weight_validity()

    def check_weight_validity(self):
        pass

    def create(self):
        pass


class sign_fn(nn_layer):
    def __init__(self, name,  inputs, outputs, input_length, weight=None):
        nn_layer.__init__(self, name=name, inputs=inputs, outputs=outputs,
                          weight=None, layer_type=name + '_sign_')
        self.input_length = input_length
        self.create()

    def check_weight_validity(self):
        '''
        It is not parameteried for now
        But can include bias here later
        '''
        assert self.weight is None, "Sign functions is not parameterized"

    def create(self):
        assert(len(self.outputs) == 1), "Only one bit output"
        # self.add(bi_assign(ass_type='or', lhs=[
        #         self.inputs[-1], self.inputs[-2]],
        #    rhs=self.outputs[0]))
        self.add(compare_cp(self.name + 'compare_gate_',
                            inputs=(self.inputs, self.input_length / 2),
                            outputs=self.outputs))


class linear_layer(nn_layer):
    def __init__(self, name, weight, inputs, outputs):
        nn_layer.__init__(self, name=name, inputs=inputs,
                          outputs=outputs, weight=weight, layer_type='linear')
        self.nb = weight.size
        self.create()

    def check_weight_validity(self):
        '''
        Only supports vector ops for now.
        '''
        assert(self.weight.ndim == 1)
        assert(self.weight.size == len(self.inputs))

    def create(self):
        _tmp = enc_vec(nb=len(self.inputs), name=self.name + '_xnor_tmp_')
        self.add(vec_mono_op_cond(name=self.name,
                                  cond=self.weight,
                                  ass_types=('not', 'alias'),
                                  inputs=self.inputs,
                                  outputs=_tmp))
        depth = int(math.floor(math.log(self.nb, 2)))
        _tmp_out = enc_vec(nb=depth + 1, name=self.name + '_mult_out')
        self.add(reduce_add(name=self.name + '_reduce_adder_',
                            inputs=_tmp, outputs=_tmp_out))
        self.add(sign_fn(name=self.name + 'sign_fn',
                         inputs=_tmp_out, outputs=self.outputs,
                         input_length=len(self.inputs)))


if __name__ == '__main__':
    weight = np.asarray([0, 1, 1, 1])
    inputs = enc_vec(name='nn_input', nb=4)
    outputs = enc_vec(name='nn_outputs', nb=4)
    layer = linear_layer(name='linear', weight=weight,
                         inputs=inputs, outputs=outputs)
    print(layer)
