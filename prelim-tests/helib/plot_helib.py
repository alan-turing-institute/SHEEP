#!/usr/bin/env python

"""  
plot number of slots in HElib as a function of different parameter choices
"""


import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import argparse
from pylab import *


def read_into_arrays(input_filename):
    infile = open(input_filename).readlines()
    columns = infile[0].strip().split(",")
    array_dict = {}
    for i in range(len(columns)):
        array_dict[i] = {}
        array_dict[i]["title"] = columns[i]
        array_dict[i]["values"] = []        
        pass
    for line in infile[1:]:
        vals = line.strip().split(",")
        for i in range(len(vals)):
            array_dict[i]["values"].append(int(vals[i]))
            pass
        pass
    output_dict = {}
    for v in array_dict.values():
        output_dict[v["title"]] = np.array(v["values"])
    return output_dict


def apply_cut(cut_var,cut_val,input_dict):
    output_dict = {}
    for k in input_dict.keys():
        output_dict[k] = []
        pass
    for i in range(len(input_dict[cut_var])):
        if str(input_dict[cut_var][i]) == str(cut_val):
            for k in input_dict.keys():
                output_dict[k].append(input_dict[k][i])
                pass
            pass
        pass
### turn lists back into numpy arrays
    for k in input_dict.keys():
        output_dict[k] = np.array(output_dict[k])
        pass
    return output_dict


def get_scan_dimensions(x_vals):
    """ assume we have a 'rectangular' scan, with one var being held
constant at a time while the other is scanned over, i.e. either
(x0,y0),(x1,y0),(x2,y0),...,(x0,y1),(x1,y1),...
or
(x0,y0),(x0,y1),(x0,y2),...,(x1,y0),(x1,y1),...
"""
    x_size = 0
    y_size = 0
    i=0
    while x_vals[i] == x_vals[0]:
        i+=1
    if i > 1:  ## vertical scan
        x_size = i

    else:
        for i in range(1,len(x_vals)):
            if x_vals[i] == x_vals[0]:
                break
        x_size = i
    y_size = len(x_vals) // x_size
    return x_size,y_size
    
def convert_to_2d_arrays(x_vals,y_vals,z_vals):
    ## assuming we have a series of scans where one var is held constant,
    x_size,y_size = get_scan_dimensions(x_vals)
    
    x_arrays = []
    y_arrays = []
    z_arrays = []
    for i in range(y_size):
        x_arrays.append(x_vals[x_size*i:x_size*(i+1)])
        y_arrays.append(y_vals[x_size*i:x_size*(i+1)])
        z_arrays.append(z_vals[x_size*i:x_size*(i+1)])
        pass
    x = np.array(x_arrays)
    y = np.array(y_arrays)
    z = np.array(z_arrays)
    return x,y,z


def plot_nslots_map(data,x_var,y_var):
    x_vals = data[x_var]
    y_vals = data[y_var]
    p_vals = data["nslots"]
    x,y,p = convert_to_2d_arrays(x_vals,y_vals,p_vals)
    fig = plt.figure()
    ax=fig.add_subplot(1,1,1)
    pp = plt.pcolor(x,y,p,cmap="rainbow")
    ax.set_xscale("log")
    plt.title("Number of slots")
    plt.xlabel(x_var)
    plt.ylabel(y_var)
    plt.colorbar()

def plot_num_correct(data,cut_vars=None,cut_vals=None):
    if cut_vars or cut_vals:
        if not len(cut_vars) == len(cut_vals):
            raise ValueError("Cut values and vars different length!")
        pass
    
    for i in range(len(cut_vars)):
        data = apply_cut(cut_vars[i],cut_vals[i],data)
        pass
    fig = plt.figure()
    ax = fig.add_subplot(1,1,1)
    p_add, = ax.plot(data["nops"],data["correct_add"],"o")
    p_add.set_label("Addition")
    p_mult, = ax.plot(data["nops"],data["correct_mult"],"^")
    p_mult.set_label("Multiplication")
    ax.legend(loc=7)
    plt.xlabel("Number of additions or multiplications")
    plt.ylabel("Number of slots correctly calculated")
    plt.title("L=16,c=3,w=64,d=0,security=90,p=65539")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="plot HElib test results")
    parser.add_argument("--input_filename",help="input filename",required=True)
    parser.add_argument("--plot",help="thing to plot (noise, nslots)",default="noise")
    args=parser.parse_args()
    print("Loading in file %s" % args.input_filename)
    data = read_into_arrays(args.input_filename)
    if args.plot == "noise":
        plot_num_correct(data,["p"],[65539])
    elif args.plot == "nslots":
        plot_nslots_map(data,"p","security")
    plt.show()

        
