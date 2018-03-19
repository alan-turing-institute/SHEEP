"""
test that we can check the chosen input values comply with the bitwidths.
"""

from utils import check_inputs


def test_bool_true():
    """ 
    test the check_inputs function in utils.py
    """
    input_vals = {"a": 0, "b": 1}
    assert(check_inputs(input_vals,"bool") == True)

def test_bool_false():
    """ 
    test the check_inputs function in utils.py
    """
    input_vals = {"a": -1, "b": 1}
    assert(check_inputs(input_vals,"bool") == False)


def test_uint8_true():
    """ 
    test the check_inputs function in utils.py
    """
    input_vals = {"a": 0, "b": 255}
    assert(check_inputs(input_vals,"uint8_t") == True)

def test_uint8_negative():
    """ 
    test the check_inputs function in utils.py
    """
    input_vals = {"a": -1, "b": 1}
    assert(check_inputs(input_vals,"uint8_t") == False)

def test_uint8_too_big():
    """ 
    test the check_inputs function in utils.py
    """
    input_vals = {"a": 256, "b": 1}
    assert(check_inputs(input_vals,"uint8_t") == False)


