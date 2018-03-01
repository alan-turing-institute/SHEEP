"""
Flask app for SHEEP.   Allow user to view table of results, or upload a new circuit file and run a test 
"""

from flask import Flask, render_template, request, redirect, url_for
from werkzeug.utils import secure_filename

import subprocess


from forms import CircuitForm, build_inputs_form
import utils

EXECUTABLE_DIR = "/Users/nbarlow/SHEEP/sketch/build/bin"
UPLOAD_FOLDER = "/Users/nbarlow/SHEEP/sketch/frontend/uploads"


app = Flask(__name__)
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER
app.config["EXECUTABLE_DIR"] = EXECUTABLE_DIR
#### how do we keep information throughout the lifetime of the app?  Give the app a data dict.
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
    which HE lib to use.
    """
    cform = CircuitForm(request.form)
    if request.method == "POST":
        uploaded_filenames = utils.upload_files(request.files, app.config["UPLOAD_FOLDER"])
        inputs = utils.parse_circuit_file(uploaded_filenames["circuit_file"])
        app.data["inputs"] = inputs
        app.data["input_type"] = cform.input_type.data
        app.data["HE_library"] = cform.HE_library.data
        app.data["uploaded_filenames"] = uploaded_filenames
        return redirect(url_for("enter_input_vals"))
    else:
        result = None
    return render_template("new_test.html", form=cform)
        

@app.route("/enter_input_vals",methods=["POST","GET"])
def enter_input_vals():
    """
    based on the input identifiers parsed from the circuit file, prompt the 
    user for the input values.
    """
    iform = build_inputs_form(app.data["inputs"])(request.form)
    if request.method == "POST" and iform.validate():
        print("Reading input values")
        input_vals = iform.data
        if utils.check_inputs(input_vals, app.data["input_type"]):
            app.data["uploaded_filenames"]["inputs_file"] = utils.write_inputs_file(input_vals,
                                                                                    app.config["UPLOAD_FOLDER"])            
            return redirect(url_for("execute_test"))
    return render_template("enter_input_vals.html",form=iform)

@app.route("/view_results")
def results_table():
    """
    results table.  Could get too complicated for a simple 2D table, may need some selectable
    options e.g. how many cores in parallel?
    """
    return render_template("results_table.html")


@app.route("/execute_test",methods=["POST","GET"])
def execute_test():
    """
    actually run the executable, passing it all the filenames, options etc as arguments.
    """
    proc_time, outputs = utils.run_test(app.data,app.config)   
    return render_template("test_results.html",proc_time=proc_time,outputs=outputs,context_name=app.data["HE_library"])


if __name__ == "__main__":
    app.run()
