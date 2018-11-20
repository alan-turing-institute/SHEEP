"""
Test the sheep_client functions, when there is no sheep server running.
All functions should return a dictionary with status_code 404, and the content
giving some info about what function was being called.
"""

import os
from pysheep.interface import sheep_client

if "SHEEP_HOME" in os.environ.keys():
    SHEEP_HOME = os.environ["SHEEP_HOME"]
else:
    SHEEP_HOME = os.path.join(os.environ["HOME"],"SHEEP")

def check_404_response(r):
    assert(isinstance(r,dict))
    assert("status_code" in r.keys())
    assert("content" in r.keys())
    assert(r["status_code"] == 404)

def test_is_configured():
    """
    check the is_configured function
    """
    r = sheep_client.is_configured()
    check_404_response(r)


def test_get_contexts():
    """
    check the get_available_contexts function
    """
    r=sheep_client.get_available_contexts()
    check_404_response(r)


def test_get_input_types():
    """
    check the get_available_input_types function
    """
    r=sheep_client.get_available_input_types()
    check_404_response(r)


def test_set_context():
    """
    check the set_context function
    """
    r=sheep_client.set_context("dummy")
    check_404_response(r)


def test_set_context_bad_input():
    """
    check the set_context function with a wrong input type
    """
    r=sheep_client.set_context(33)
    assert(r["status_code"]==550)


def test_set_input_type():
    """
    check the set_input_type function
    """
    r=sheep_client.set_input_type("dummy")
    check_404_response(r)


def test_set_input_type_bad_input():
    """
    check the set_input_type function with a wrong input type
    """
    r=sheep_client.set_input_type(33)
    assert(r["status_code"]==550)


def test_get_inputs():
    """
    check the get_inputs function
    """
    r=sheep_client.get_inputs()
    check_404_response(r)


def test_set_inputs():
    """
    check the set_inputs function
    """
    r=sheep_client.set_inputs({"x":6})
    check_404_response(r)


def test_set_circuit_filename():
    """
    check the set_circuit_filename function
    """
    r=sheep_client.set_circuit_filename("x")
    check_404_response(r)


def test_set_circuit_missing():
    """
    check the set_circuit function with a non-existent filename
    - should get status_code 554
    """
    r=sheep_client.set_circuit("x")
    assert(r["status_code"]==554)


def test_set_circuit_not_missing():
    """
    check the set_circuit function with a real filename
    but no server
    - should get status_code 404
    """
    circuit_filename = os.path.join(SHEEP_HOME,"pysheep","pysheep","tests",
                                    "testfiles","simple_add.sheep")
    r=sheep_client.set_circuit(circuit_filename)
    check_404_response(r)


def test_get_parameters():
    """
    check the get_parameters function
    """
    r=sheep_client.get_parameters()
    check_404_response(r)


def test_set_parameters():
    """
    check the set_parameters function
    """
    r=sheep_client.set_parameters({"a":5})
    check_404_response(r)


def test_set_parameters_bad_input():
    """
    check the set_parameters function with wrong input type
    """
    r=sheep_client.set_parameters("a")
    assert(r["status_code"] == 550)


def test_set_eval_strategy():
    """
    check the set_eval_strategy function
    """
    r=sheep_client.set_eval_strategy("serial")
    check_404_response(r)


def test_set_eval_strategy_bad_input_type():
    """
    check the set_eval_strategy function
    """
    r=sheep_client.set_eval_strategy(7)
    assert(r["status_code"] == 550)


def test_set_eval_strategy_bad_input_val():
    """
    check the set_eval_strategy function
    """
    r=sheep_client.set_eval_strategy("ser")
    assert(r["status_code"] == 556)


def test_get_eval_strategy():
    """
    check the get_eval_strategy function
    """
    r=sheep_client.get_eval_strategy()
    check_404_response(r)


def test_new_job():
    """
    check the new_job function
    """
    r=sheep_client.new_job()
    check_404_response(r)


def test_run_job():
    """
    check the run_job function
    """
    r=sheep_client.run_job()
    check_404_response(r)


def test_get_config():
    """
    check the get_config function
    """
    r=sheep_client.get_config()
    check_404_response(r)


def test_get_results():
    """
    check the get_results function
    """
    r=sheep_client.get_results()
    check_404_response(r)
