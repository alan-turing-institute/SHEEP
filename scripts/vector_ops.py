from particles import enc_vec, enc_mat, enc_tensor3
from interactions import mini_mod, mono_assign


class vec_elemwise(mini_mod):
    def __init__(self, name, inputs, outputs):
        mini_mod.__init__(self, name=name, inputs=inputs, outputs=outputs)
        assert(isinstance(inputs, enc_vec))
        assert(isinstance(outputs, enc_vec))
        assert len(self.inputs) == len(self.outputs)
        self.size = len(inputs)

    def operate(self, **kwargs):
        pass


class vec_mono_op(vec_elemwise):
    def __init__(self, ass_type, name, inputs, outputs):
        vec_elemwise.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.ass_type = ass_type
        self.size = len(self.inputs)
        self.operate()

    def operate(self):
        for idx in range(self.size):
            self.add(ass_type=self.ass_type,
                     lhs=[self.inputs[idx]],
                     rhs=self.outputs[idx])


class vec_mono_op_cond(vec_elemwise):
    def __init__(self, cond, ass_types, name, inputs, outputs):
        vec_elemwise.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.ass_types = ass_types
        self.cond = cond
        self.operate()

    def operate(self):
        for idx in range(self.size):
            self.add(mono_assign(ass_type=self.ass_types[self.cond[idx]],
                                 lhs=[self.inputs[idx]],
                                 rhs=self.outputs[idx]))


class mat_elemwise(mini_mod):
    def __init__(self, name, inputs, outputs):
        mini_mod.__init__(self, name=name, inputs=inputs, outputs=outputs)
        assert(isinstance(inputs, enc_mat))
        assert(isinstance(outputs, enc_mat))
        assert len(self.inputs) == len(self.outputs)
        assert(len(self.inputs[0])) == len(self.outputs[0])
        self.size = inputs.size

    def operate(self, **kwargs):
        pass


class mat_mono_op(vec_elemwise):
    def __init__(self, ass_type, name, inputs, outputs):
        vec_elemwise.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.ass_type = ass_type
        self.operate()

    def operate(self):
        for idx in range(self.size[0]):
            for jdx in range(self.size[1]):
                self.add(ass_type=self.ass_type,
                         lhs=[self.inputs[idx][jdx]],
                         rhs=self.outputs[idx][jdx])


class mat_mono_op_cond(vec_elemwise):
    def __init__(self, cond, ass_types, name, inputs, outputs):
        vec_elemwise.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.ass_types = ass_types
        self.cond = cond
        self.operate()

    def operate(self):
        for idx in range(self.size[0]):
            for jdx in range(self.size[1]):
                self.add(
                    mono_assign(ass_type=self.ass_types[self.cond[idx][jdx]],
                                lhs=[self.inputs[idx][jdx]],
                                rhs=self.outputs[idx][jdx]))


class tensor3_elemwise(mini_mod):
    def __init__(self, name, inputs, outputs):
        mini_mod.__init__(self, name=name, inputs=inputs, outputs=outputs)
        assert(isinstance(inputs, enc_tensor3))
        assert(isinstance(outputs, enc_tensor3))
        assert len(self.inputs) == len(self.outputs)
        assert len(self.inputs[0]) == len(self.outputs[0])
        assert len(self.inputs[0][0]) == len(self.outputs[0][0])
        self.size = self.inputs.size

    def operate(self, **kwargs):
        pass


class tensor3_mono_op(vec_elemwise):
    def __init__(self, ass_type, name, inputs, outputs):
        vec_elemwise.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.ass_type = ass_type
        self.operate()

    def operate(self):
        for idx in range(self.size[0]):
            for jdx in range(self.size[1]):
                for kdx in range(self.size[2]):
                    self.add(ass_type=self.ass_type,
                             lhs=[self.inputs[idx][jdx][kdx]],
                             rhs=self.outputs[idx][jdx][kdx])


class tensor3_mono_op_cond(vec_elemwise):
    def __init__(self, cond, ass_types, name, inputs, outputs):
        vec_elemwise.__init__(self, name=name, inputs=inputs, outputs=outputs)
        self.ass_types = ass_types
        self.cond = cond
        self.operate()

    def operate(self):
        for idx in range(len(self.inputs)):
            self.add(mono_assign(ass_type=self.ass_types[self.cond[idx]],
                                 lhs=[self.inputs[idx]],
                                 rhs=self.outputs[idx]))
