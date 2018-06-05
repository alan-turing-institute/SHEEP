from vector_ops import vec_mono_op_cond
from interactions import mini_mod, bi_assign
from functools import partial
from tqdm import tqdm
from particles import enc_vec, enc_mat, variables
from functions import reduce_add
from reusable_modules import compare_cp
from op_utils import col2im, im2col
import numpy as np
import math
import multiprocessing as mp
import time


def create_ith_ll(name_, filter_col_, im_col_, out_, map_inp):
    col_idx = map_inp[1]
    temp_lyr = linear_layer(name=name_ + 'ln' + str(col_idx),
                            weight=filter_col_,
                            inputs=im_col_[col_idx],
                            outputs=out_[col_idx], parallel=False)
    map_inp[0].put(temp_lyr)
    return 1


class nn_layer(mini_mod):

    def __init__(self, layer_type, weight, name, inputs, outputs,
                 parallel=False):
        mini_mod.__init__(self, name=name, inputs=inputs,
                          outputs=outputs, parallel=parallel)
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
        if isinstance(self.outputs, enc_vec):
            assert(len(self.outputs) == 1), "Only one bit output"
        else:
            assert(isinstance(self.outputs, variables)), "Only one bit output"
        # self.add(bi_assign(ass_type='or', lhs=[
        #         self.inputs[-1], self.inputs[-2]],
        #    rhs=self.outputs[0]))
        self.add(compare_cp(self.name + 'cpg_',
                            inputs=(self.inputs, self.input_length / 2),
                            outputs=self.outputs))


class linear_layer_1d(nn_layer):
    def __init__(self, name, weight, inputs, outputs):
        nn_layer.__init__(self, name=name, inputs=inputs,
                          outputs=outputs, weight=weight,
                          layer_type='l_1d')
        self.nb = weight.size
        self.create()

    def check_weight_validity(self):
        '''
        Only supports vector ops for now.
        '''
        assert(self.weight.ndim == 1)
        assert(self.weight.size == len(self.inputs)), str(
            self.weight.size) + ' ~ ' + str(len(self.inputs))

    def create(self):
        _tmp = enc_vec(nb=len(self.inputs), name=self.name + '_xnor_tmp_')
        self.add(vec_mono_op_cond(name=self.name,
                                  cond=self.weight,
                                  ass_types=('not', 'alias'),
                                  inputs=self.inputs,
                                  outputs=_tmp))
        depth = int(math.floor(math.log(self.nb, 2)))
        _tmp_out = enc_vec(nb=depth + 1, name=self.name + '_mult_out')

        self.add(reduce_add(name=self.name + '_ra1_',
                            inputs=_tmp, outputs=_tmp_out))
        self.add(sign_fn(name=self.name + 'sgn',
                         inputs=_tmp_out, outputs=self.outputs,
                         input_length=len(self.inputs)))


class linear_layer(nn_layer):
    def __init__(self, name, weight, inputs, outputs, parallel=False):
        nn_layer.__init__(self, name=name, inputs=inputs,
                          outputs=outputs, weight=weight,
                          layer_type='linear', parallel=parallel)
        self.nb = weight[0].size
        self.create()

    def check_weight_validity(self):
        '''
        Only supports vector ops for now.
        '''
        assert(self.weight.ndim == 2)
        if isinstance(self.inputs, enc_mat):
            assert(self.weight.shape[1] == len(self.inputs[0])), str(
                self.weight.shape[1]) + ' ~ ' + str(len(self.inputs[0]))
        else:
            assert(self.weight.shape[1] == len(self.inputs)), str(
                self.weight.shape[1]) + ' ~ ' + str(len(self.inputs))
        assert(self.weight.shape[0] == len(self.outputs)), str(
            self.weight.shape[0]) + " ~ " + str(len(self.outputs))

    def create(self):
        if isinstance(self.inputs, enc_mat):
            rel_inp = self.inputs[0]
        else:
            rel_inp = self.inputs

        for idx in range(len(self.weight)):
            self.add(linear_layer_1d(name=self.name + 'lin_lyr' + str(idx),
                                     weight=self.weight[idx],
                                     inputs=rel_inp,
                                     outputs=self.outputs[idx]))


class conv_layer(nn_layer):
    def __init__(self, name,
                 weight, inputs, outputs,
                 stride=1, padding=0):
        self.padding = padding
        self.stride = stride
        nn_layer.__init__(self, name=name, inputs=inputs, outputs=outputs,
                          weight=weight, layer_type='conv', parallel=False)
        self.nb = reduce(lambda x, y: x * y, self.inputs.size)
        self.create()

    def check_weight_validity(self):
        assert(self.weight.ndim == 4)
        assert(self.padding == 0), "Only supports zero padding"

    def create(self):
        """
        A naive implementation of the forward pass for a convolutional layer.

        The input consists of N data points, each with C channels,
        height H and width W. We convolve each input with F different filters,
        where each filter spans  all C channels and has height HH and width HH.

        Input:
        - x: Input data of shape (N, C, H, W)
        - w: Filter weights of shape (F, C, HH, WW)
        - b: Biases, of shape (F,)
        - conv_param: A dictionary with the following keys:
        - 'stride': The number of pixels between adjacent receptive
        fields in the horizontal and vertical directions.
        - 'pad': The number of pixels that will be used to zero-pad the input.

        Returns a tuple of:
        - out: Output data, of shape (N, F, H', W') where H' and W' are
        H' = 1 + (H + 2 * pad - HH) / stride
        W' = 1 + (W + 2 * pad - WW) / stride
        - cache: (x, w, b, conv_param)
        """

        pad_num = self.padding
        stride = self.stride
        C, H, W = self.inputs.size
        F, C, HH, WW = self.weight.shape
        H_prime = (H + 2 * pad_num - HH) // stride + 1
        W_prime = (W + 2 * pad_num - WW) // stride + 1
        out = enc_mat(name=self.name + 'c_i',
                      size=(H_prime * W_prime, F))

        print("Entering im2col....")
        beg = time.time()
        im_col = im2col(name=self.name + 'im2col',
                        x=self.inputs, hh=HH, ww=WW, stride=stride)
        end = time.time()
        print("Exiting im2col after " + str(end - beg) + str("sec."))
        print()
        print("Adding linear layers")
        beg = time.time()
        filter_col = np.reshape(self.weight, (F, -1))
        cpus = mp.cpu_count()
        p = mp.Pool(cpus)
        queue = mp.Manager().Queue()
        p.map(partial(create_ith_ll,
                      self.name,
                      filter_col,
                      im_col,
                      out),
              [(queue, x) for x in range(im_col.size[0])])
        p.close()
        p.join()
        for col_idx in tqdm(range(im_col.size[0])):
            self.add(queue.get())
        end = time.time()
        print("Exiting im2col after " + str(end - beg) + str("sec."))
        print("Added Linear Layers")
        print()
        print("Entering col2im")
        beg = time.time()
        col2im(self, out, H_prime, W_prime, 1, self.outputs)
        end = time.time()
        print("Exiting im2col after " + str(end - beg) + str("sec."))
        print("Exiting col2im")


if __name__ == '__main__':
    weight = np.asarray([0, 1, 1, 1])
    inputs = enc_vec(name='nn_input', nb=4)
    outputs = enc_vec(name='nn_outputs', nb=4)
    layer = linear_layer_1d(name='linear', weight=weight,
                            inputs=inputs, outputs=outputs)
    print(layer)
