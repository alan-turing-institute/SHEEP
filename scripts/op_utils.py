from interactions import mono_assign
from particles import enc_mat
from tqdm import tqdm
from functools import partial
import multiprocessing as mp


def nested_assign(x, hh, ww, stride, new_w, new_h, c, ij):
    out_mat = ['' for t in range(c * hh * ww)]
    i = ij // new_w
    j = ij // new_h
    for cidx in range(c):
        for a in range(hh):
            for b in range(ww):
                out_mat[cidx * hh * ww + a * ww + b] =\
                    x[cidx][i * stride + a][j * stride + b]
    return out_mat


def im2col(name, x, hh, ww, stride=1):
    """
    Args:
      x: image matrix to be translated into columns, (C,H,W)
      hh: filter height
      ww: filter width
      stride: stride
    Returns:
      col: (new_h*new_w,hh*ww*C) matrix, each column is a
           cube that will convolve with a filter
           new_h = (H-hh) // stride + 1, new_w = (W-ww) // stride + 1
    """

    c, h, w = x.size[0], x.size[1], x.size[2]
    new_h = (h - hh) // stride + 1
    new_w = (w - ww) // stride + 1
    col = enc_mat(name=name + 'i2' + str(c * h * w),
                  size=(new_h * new_w, c * hh * ww))
    old_lst_ = x._lst
    old_name_lst_ = x.name_list
    p = mp.Pool(mp.cpu_count())
    new_lst_ = p.map(partial(nested_assign,
                             old_lst_,
                             hh,
                             ww,
                             stride,
                             new_w,
                             new_h,
                             c),
                     range(new_h * new_w))
    p.close()
    p.join()
    p = mp.Pool(mp.cpu_count())
    new_name_lst_ = p.map(partial(nested_assign,
                                  old_name_lst_,
                                  hh,
                                  ww,
                                  stride,
                                  new_w,
                                  new_h,
                                  c),
                          range(new_h * new_w))
    col.reorder(new_lst_, new_name_lst_)
    p.close()
    p.join()
    return col


def col2im(circuit, mul, h_prime, w_prime, C, tgt_out):
    """
      Args:
      mul: (h_prime*w_prime*w,F) matrix,
           each col should be reshaped to
           C*h_prime*w_prime when C>0, or h_prime*w_prime when C = 0
      h_prime: reshaped filter height
      w_prime: reshaped filter width
      C: reshaped filter channel, if 0, reshape the filter to 2D,
         Otherwise reshape it to 3D
    Returns:
      if C == 0: (F,h_prime,w_prime) matrix
      Otherwise: (F,C,h_prime,w_prime) matrix
    """
    F = mul.size[1]
    for i in range(0, F):
        map(lambda idx: circuit.add(
            mono_assign(ass_type='alias',
                        lhs=[mul[idx][i]],
                        rhs=tgt_out[i][idx
                                       / w_prime][idx % w_prime])),
            range(w_prime * h_prime))
