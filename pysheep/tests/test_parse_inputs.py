"""
test that we can extract the labels of input wires from a circuit file.
"""

from utils import parse_circuit_file

def test_parse_inputs():
    """ 
    test the parse_circuit_file function in utils.py
    """
    
    circuit_filename = "tests/testfiles/simple_add.sheep"
    inputs = parse_circuit_file(circuit_filename)
    assert(len(inputs) == 2)
    assert(inputs[0] == "a")
    assert(inputs[1] == "b")

