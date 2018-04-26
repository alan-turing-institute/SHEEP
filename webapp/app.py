"""
Flask app for SHEEP.   Allow user to view table of results, or upload a new circuit file and run a test 
"""

from flask import Flask, render_template, request, redirect, url_for
from werkzeug.utils import secure_filename
import subprocess
import os
import sys

if "SHEEP_HOME" in os.environ.keys():
    SHEEP_HOME = os.environ["SHEEP_HOME"]
else:
    SHEEP_HOME = os.environ["HOME"]+"/SHEEP"
    
EXECUTABLE_DIR = os.path.join(SHEEP_HOME, "build", "bin")
UPLOAD_FOLDER = os.path.join(SHEEP_HOME, "webapp", "uploads")
sys.path.append(SHEEP_HOME)

from pysheep.frontend.forms import CircuitForm, ResultsForm, PlotsForm, build_inputs_form, build_param_form
from pysheep.frontend import frontend_utils, plotting

from pysheep.common import common_utils, database

app = Flask(__name__)
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER
app.config["EXECUTABLE_DIR"] = EXECUTABLE_DIR
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
    Get the user to upload a circuit file and parameter, and select input type, and 
    which HE libs to use.
    """
    ###
    ## first cleanup the files created by previous tests.
    frontend_utils.cleanup_upload_dir(app.config)
    ## create the form to choose circuit file, input_type, and which HE libraries to test
    cform = CircuitForm(request.form)
    if request.method == "POST":
        uploaded_filenames = frontend_utils.upload_files(request.files, app.config["UPLOAD_FOLDER"])
        inputs = common_utils.get_inputs(uploaded_filenames["circuit_file"])
        app.data["inputs"] = inputs
        app.data["input_type"] = cform.input_type.data
        app.data["HE_libraries"] = cform.HE_library.data
        app.data["uploaded_filenames"] = uploaded_filenames
        app.data["params"] = frontend_utils.get_params_all_contexts(app.data["HE_libraries"],
                                                                    app.data["input_type"],
                                                                    app.config)
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
            app.data["uploaded_filenames"]["inputs_file"] = \
                    frontend_utils.write_inputs_file(input_vals,
                                                     app.config["UPLOAD_FOLDER"])
            return redirect(url_for("execute_test"))
    return render_template("enter_input_vals.html",
                           form=iform,
                           circuit=circuit_text)


@app.route("/view_results_plots",methods=["POST","GET"])
def results_plots():
    """
    plots, using nvd3 (i.e. D3.js with a Python wrapper)
    """
    pform = PlotsForm(request.form)
    if request.method == "POST":
        inputs = pform.data
        filename = plotting.generate_plots(inputs)
        return render_template(filename)
    return render_template("result_plots_query.html",form=pform)


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
    results = frontend_utils.run_test(app.data,app.config)   
    if request.method == "POST":  ### upload button was pressed
        frontend_utils.upload_test_result(results,app.data)
        return render_template("uploaded_ok.html")
    return render_template("test_results.html",results = results)


if __name__ == "__main__":
    app.run(host='0.0.0.0')
