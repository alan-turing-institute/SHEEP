from particles import enc_vec
from interactions import mini_mod


class vec_elemwise(mini_mod):
    def __init__(self, name, inputs, outputs):
        mini_mod.__init__(self, name=name, inputs=inputs, outputs=outputs)
        assert(isinstance(inputs, enc_vec))
        assert(isinstance(outputs, enc_vec))
        assert len(self.inputs) == len(self.outputs)

    def operate(self, **kwargs):
        pass


class vec_mono_op(vec_elemwise):
    def __init__(self, ass_type, name, inputs, outputs):
        vec_elemwise.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.ass_type = ass_type
        self.operate()

    def operate(self):
        for idx in range(self.inputs):
            self.add(ass_type=self.ass_type,
                     lhs=[self.inputs[idx]],
                     rhs=self.outputs[idx])
