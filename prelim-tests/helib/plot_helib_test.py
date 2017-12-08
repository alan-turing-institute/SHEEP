#!/usr/bin/env python

""" 
Tests for HElib tests 
"""


import unittest
import plot_helib


import numpy as np

class TestHelib(unittest.TestCase):

    def test_read_input(self):
        data=plot_helib.read_into_arrays("testfiles/HElib_noiseTest.csv")
        keylist= list(data.keys())
        same_length = True
        for k in keylist:
            same_length = same_length and len(data[k]) == len(data[keylist[0]])
        self.assertTrue(same_length)


    def test_get_dimensions_horizontal(self):
        l=[1,1,1,1,1,2,2,2,2,2,3,3,3,3,3]
        xvals = np.array(l)
        x_size,y_size = plot_helib.get_scan_dimensions(xvals)
        self.assertTrue(x_size == 5 and y_size == 3)

    def test_get_dimensions_vertical(self):
        l=[1,2,3,4,5,1,2,3,4,5,1,2,3,4,5]
        xvals = np.array(l)
        x_size,y_size = plot_helib.get_scan_dimensions(xvals)
        self.assertTrue(x_size == 5 and y_size == 3)        
        
if __name__ == "__main__":
    unittest.main()
