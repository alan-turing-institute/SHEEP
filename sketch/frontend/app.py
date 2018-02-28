"""
Flask app for SHEEP.   Allow user to view table of results, or upload a new circuit file and run a test 
"""

from flask import Flask, render_template, request, redirect, url_for
from werkzeug.utils import secure_filename

import subprocess
import os
import uuid

from forms import CircuitForm, build_inputs_form


EXECUTABLE_DIR = "/Users/nbarlow/SHEEP/sketch/build/bin"
UPLOAD_FOLDER = "/Users/nbarlow/SHEEP/sketch/frontend/uploads"


app = Flask(__name__)
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER
app.config["EXECUTABLE_DIR"] = EXECUTABLE_DIR
#### how do we keep information throughout the lifetime of the app?  Give the app a data dict.
app.data = {}



def check_inputs(input_vals):
    """ 
    check that the supplied inputs are within the ranges for the specified input type.
    """
    return True


def upload_files(filedict):
    """
    Upload circuit file to server storage...  and other files?
    """
    uploaded_filenames = {}
    for k,v in filedict.items():
        uploaded_filename = os.path.join(app.config["UPLOAD_FOLDER"],v.filename)
        v.save(uploaded_filename)
        uploaded_filenames[k] = uploaded_filename
    return uploaded_filenames


def parse_circuit_file(uploaded_filenames):
    """ 
    read the circuit file and check what the names of the inputs are, so
    they can be selected in another form.
    """
    inputs = []
    f=open(uploaded_filenames["circuit_file"])
    for line in f.readlines():
        if line.startswith("INPUTS"):
            inputs += line.strip().split()[1:]
    return inputs


def write_inputs_file(inputs):
    """
    write the input names and values to a file, 
    just because that's easier to pass to the executable.
    """
    inputs_filename = os.path.join(app.config["UPLOAD_FOLDER"],"inputs_"+str(uuid.uuid4())+".inputs")
    f = open(inputs_filename,"w")
    for k,v in inputs.items():
        f.write(k+" "+str(v)+"\n")
    f.close()
    return inputs_filename

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
        print("Will upload files")
        uploaded_filenames = upload_files(request.files)
        inputs = parse_circuit_file(uploaded_filenames)
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
        if check_inputs(input_vals):
            app.data["uploaded_filenames"]["inputs_file"] = write_inputs_file(input_vals)            
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
    circuit_file = app.data["uploaded_filenames"]["circuit_file"]
    print("Found circuit file ",circuit_file)
    inputs_file = app.data["uploaded_filenames"]["inputs_file"]
    print("Found inputs file ",inputs_file)    
    context_name = app.data["HE_library"]
    input_type = app.data["input_type"]
    print("input type",input_type)
    parameter_file = None
    if "parameter_file" in app.data["uploaded_filenames"].keys():
        parameter_file = app.data["uploaded_filenames"]["parameter_file"]
        print("Found parameter file ",parameter_file)    
    # run_cmd is a list of arguments to be passed to subprocess.run()
    run_cmd = [app.config["EXECUTABLE_DIR"]+"/benchmark"]
    run_cmd.append(circuit_file)
    run_cmd.append(context_name)    
    run_cmd.append(input_type) 
    run_cmd.append(inputs_file)    
    if parameter_file:
        run_cmd.append(parameter_file)        
    p = subprocess.run(args=run_cmd)
    return str(p.returncode)



if __name__ == "__main__":
    app.run()
