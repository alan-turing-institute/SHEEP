from vector_ops import vec_mono_op_cond
from interactions import mini_mod
from particles import enc_vec
from functions import reduce_add
import numpy as np


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


class linear_layer(nn_layer):
    def __init__(self, name, weight, inputs, outputs):
        nn_layer.__init__(self, name=name, inputs=inputs,
                          outputs=outputs, weight=weight, layer_type='linear')
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
        self.add(reduce_add(name=self.name + '_reduce_adder_',
                            inputs=_tmp, outputs=self.outputs))


if __name__ == '__main__':
    weight = np.asarray([0, 1, 1, 1])
    inputs = enc_vec(name='nn_input', nb=4)
    outputs = enc_vec(name='nn_outputs', nb=4)
    layer = linear_layer(name='linear', weight=weight,
                         inputs=inputs, outputs=outputs)
    print(layer)
