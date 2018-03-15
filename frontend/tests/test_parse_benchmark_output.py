"""
test that we can extract the numbers we want from the output of a 
benchmark test.
"""

from utils import parse_test_output

def test_parse_output():
    """ 
    test the parse_test_output function in utils.py
    """
    
    ### open the file in bytes mode to mimic what subprocess gets from stdout
    output_file = open("tests/testfiles/benchmark_stdout.txt","rb")
    output_string = output_file.read()
    processing_times, outputs = parse_test_output(output_string)
    assert(len(processing_times) == 4)
    assert(len(outputs) == 2)
    assert(len(outputs[0]) == 2)
    assert(len(outputs[1]) == 2)    
    assert(outputs[0][0] == '75')
    assert(outputs[0][1] == '75')
    assert(outputs[1][0] == '-215')
    assert(outputs[1][1] == '-215')        
