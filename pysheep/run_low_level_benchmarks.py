#!/usr/bin/env python

""" 
script to run the low level benchmarks, basically
just calling the functions in pysheep/benchmarks/low_level_benchmarks.py
"""
import argparse
from benchmarks import low_level_benchmarks

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="run low-level benchmarks")
    parser.add_argument("--scan",help="benchmark to run",required=False)
    parser.add_argument("--contexts",help="contexts to run",required=False,
                        default=["HElib_Fp","HElib_F2","TFHE"])
    parser.add_argument("--gates",help="gates to run",required=False,
                        default=["ADD","MULTIPLY","SUBTRACT","NEGATE"])
    parser.add_argument("--depth",help="depth",required=False,default=10)    
    args=parser.parse_args()
    if args.scan == "scan1":
        low_level_benchmarks.scan_1()
    elif args.scan == "scan2":
        low_level_benchmarks.scan_2()
    elif args.scan == "scan3":
        low_level_benchmarks.scan_3()
    else: # run them all
        low_level_benchmarks.run_all(args.gates,
                                     args.types,
                                     args.contexts,
                                     args.depth)
