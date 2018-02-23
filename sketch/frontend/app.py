"""
Flask app for SHEEP.  
"""

from flask import Flask, render_template

import subprocess

app = Flask(__name__)

@app.route("/")
def homepage():
    return render_template("index.html")


@app.route("/new_test")
def new_test():
    contexts = ["Clear","HElib","TFHE"]
    types = ["bool","int8_t","uint8_t"]
    return render_template("new_test.html",contexts=contexts,types=types)

@app.route("/view_results")
def results_table():
    return render_template("results_table.html")


@app.route("/execute_test")
def execute_test():
    return


if __name__ == "__main__":
    app.run()
