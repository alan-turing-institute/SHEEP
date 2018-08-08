"""
Flask app for SHEEP.   Allow user to view table of results, or upload a new circuit file and run a test
"""

from flask import Flask, render_template, request, redirect, url_for
from werkzeug.utils import secure_filename
import subprocess
import os
import sys
import time

import config

app = Flask(__name__)
app.config.from_object(config.SheepConfig)
sys.path.append(app.config["SHEEP_HOME"])

from pysheep.frontend.forms import CircuitForm, ResultsForm, PlotsForm, \
    build_inputs_form, build_param_form
from pysheep.frontend import frontend_utils
from pysheep.interface import sheep_client
from pysheep.common import common_utils, database


# how do we keep information throughout the lifetime of the app?  Give the app a data dict.
app.data = {}


@app.route("/")
def homepage():
    """
    basic homepage - options to view results table or run a new test.
    """
    return render_template("index.html")


@app.route("/new_test",methods=["POST","GET"])
def new_test():
    """
    Get the user to upload a circuit file and parameter,
    and select input type, and
    which HE libs to use.
    """
    ### reset the app's data dict
    app.data = {}
    ###
    ## first cleanup the files created by previous tests.
    frontend_utils.cleanup_upload_dir(app.config)
    # get available choices of input_type and context
    response = sheep_client.new_job()
    if response["status_code"] != 200:
        return redirect(url_for("sheep_error",
                                status = response["status_code"],
                                message = response["content"]))
    input_type_request = sheep_client.get_available_input_types()
    if input_type_request["status_code"] != 200:
        return redirect(url_for("sheep_error",
                                status = input_type_request["status_code"],
                                message = input_type_request["content"]))
    input_types = input_type_request["content"]
    context_request = sheep_client.get_available_contexts()
    if context_request["status_code"] != 200:
        return redirect(url_for("sheep_error",
                                status = context_request["status_code"],
                                message = context_request["content"]))
    libraries = context_request["content"]
    ## create the form to choose circuit file, input_type, and which HE libraries to test
    cform = CircuitForm(request.form)#, libraries, input_types)
    cform.input_type.choices=[(t,t) for t in input_types]
    cform.HE_library.choices=[(l,l) for l in libraries]
    if request.method == "POST":
        print("REQUEST FILES", request.files)
        uploaded_filenames = frontend_utils.upload_files(request.files, app.config["UPLOAD_FOLDER"])
        r = sheep_client.set_circuit(uploaded_filenames["circuit_file"])
        if r["status_code"] != 200:
            return redirect(url_for("sheep_error",
                                    status = r["status_code"],
                                    message = r["content"]))
        time.sleep(0.1)
        inputs_request = sheep_client.get_inputs()
        if inputs_request["status_code"] != 200:
            return redirect(url_for("sheep_error",
                                    status = inputs_request["status_code"],
                                    message = inputs_request["content"]))
        inputs = inputs_request["content"]
        app.data["inputs"] = inputs
        app.data["input_type"] = cform.input_type.data
        app.data["HE_libraries"] = cform.HE_library.data
        app.data["uploaded_filenames"] = uploaded_filenames
        app.data["eval_strategy"] = frontend_utils.set_default_eval_strategy(app.data)
        param_request = frontend_utils.get_params_all_contexts(app.data["HE_libraries"],
                                                               app.data["input_type"])
        if param_request["status_code"] != 200:
            return redirect(url_for("sheep_error",
                                    status = param_request["status_code"],
                                    message = param_request["content"]))
        app.data["params"] = param_request["content"]
        return redirect(url_for("enter_parameters"))
    else:
        result = None
    return render_template("new_test.html", form=cform)


@app.route("/enter_parameters",methods=["POST","GET"])
def enter_parameters():
    """
    query the selected contexts for their configurable parameters
    and default values.
    """

    params = app.data["params"]

    pforms = {}
    for context in params.keys():
        pform = build_param_form(params[context])(request.form)
        pforms[context] = pform
    if request.method == "POST":
        for context in pforms.keys():
            if context in request.form.keys():
                params = frontend_utils.update_params(context,request.form,
                                             app.data,app.config)
                app.data["params"][context] = params
                return redirect(url_for("enter_parameters"))

        param_sets = {}
        for k,v in pforms.items():
            param_sets[k] = v.data
            pass
        if request.form["next"] == "Next":
            return redirect(url_for("enter_input_vals"))
    return render_template("enter_parameters.html",forms=pforms)


@app.route("/enter_input_vals",methods=["POST","GET"])
def enter_input_vals():
    """
    based on the input identifiers parsed from the circuit file, prompt the
    user for the input values.
    """
    iform = build_inputs_form(app.data["inputs"])(request.form)
    circuit_text = open(app.data["uploaded_filenames"]["circuit_file"]).readlines()
    if request.method == "POST" and iform.validate():
        input_vals = iform.data
        if common_utils.check_inputs(input_vals, app.data["input_type"]):
            app.data["input_vals"] = input_vals
            return redirect(url_for("execute_test"))
    return render_template("enter_input_vals.html",
                           form=iform,
                           circuit=circuit_text)


@app.route("/execute_test",methods=["POST","GET"])
def execute_test():
    """
    actually run the executable, passing it all the filenames, options etc as arguments.
    Get back a dict of results {"context_name": {"processing_times" : {},
                                                 "sizes" : {},
                                                 "outputs" : {}
                                                }, ...
                                }
    """
    run_request = frontend_utils.run_test(app.data)
    if run_request["status_code"] != 200:
        return redirect(url_for("sheep_error",
                                status = run_request["status_code"],
                                message = run_request["content"]))
    results = run_request["content"]
    if request.method == "POST":  ### upload button was pressed
        frontend_utils.upload_test_result(results,app.data)
        return render_template("uploaded_ok.html")
    return render_template("test_results.html",results = results)


@app.route("/error/<status>/<message>",methods=["GET"])
def sheep_error(status, message):
    """
    Display an error message (hopefully descriptive enough to be useful).
    """
    return render_template("sheep_error.html",
                           errordict={"code": status,
                                      "message": message})



if __name__ == "__main__":
    app.run(host='0.0.0.0')
