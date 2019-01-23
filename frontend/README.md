# The SHEEP frontend

The SHEEP frontend consists of a set of Python utilities, and example notebooks,
to help users interact with the core functionality of SHEEP.  Most of these
interactions can be done via the [sheep_client](https://github.com/alan-turing-institute/SHEEP/tree/master/frontend/pysheep/sheep_client.py), a set of wrapper functions for calls to the [SHEEP API](https://github.com/alan-turing-institute/SHEEP/tree/master/backend/API.md).  These functions are documented [here](https://github.com/alan-turing-institute/SHEEP/blob/master/frontend/CLIENT.md).

By far the easiest way to run both the frontend and backend applications is
via **docker-compose** - from the [base SHEEP directory](https://github.com/alan-turing-institute/SHEEP/tree/master) simply run
```
docker-compose build
docker-compose up
```
and then when you want to shut down, do
```
docker-compose down
```

## Web application

A basic web server, allowing users to upload circuits, set inputs and parameters, evaluate circuits and view the results, is provided via a ***Flask*** app.
As mentioned above, the easiest way to run this is via ***docker-compose***, with which the webpage will be available at ```http:\\localhost:5000```.  It is
also possible to run the app locally, using the following:
```
export SHEEP_HOME=/path/to/SHEEP/frontend
cd /path/to/SHEEP/frontend/webapp
python app.py
```
and again, the webpage will be available at ```http:\\localhost:5000```.
Note that the [sheep server](https://github.com/alan-turing-institute/SHEEP/tree/master/backend/API.md) must also be running on ```localhost:34568``` (this is already taken care of when running with ***docker-compose***).

## Jupyter notebooks

Several notebooks are available [in this directory](https://github.com/alan-turing-institute/SHEEP/tree/master/frontend/notebooks) to demonstrate the functionality of SHEEP and provide starting points for users to explore further.

### Running the notebook server locally

To run the notebooks locally, you should be able to do:
```
export SHEEP_HOME=/path/to/SHEEP/frontend
cd /path/to/SHEEP/frontend/notebooks
jupyter notebook
```
then point your browser to ```localhost:8888```.  The sheep-server must be running and available at ```localhost:34568``` (this is already taken care of when running with ***docker-compose***).

### Running with docker-compose

If running via ***docker-compose*** (recommended) you will also need to put an authentication token string into the URL.  In the console output in the terminal from which you run ```docker-compose up``` there should be something like:

```
sheep-notebook_1  |     Copy/paste this URL into your browser when you connect for the first time,
sheep-notebook_1  |     to login with a token:
sheep-notebook_1  |         http://<SOME_SHORT_HEX_STRING>:8888/?token=<SOME_LONG_HEX_STRING>
```
Copy/paste this URL into your browser, replacing ```<SOME_SHORT_HEX_STRING>``` (which is the id of the docker container) with ```localhost```.

### Currently available notebooks

The basic [Using_SHEEP.ipynb](https://github.com/alan-turing-institute/SHEEP/tree/master/frontend/notebooks/Using_SHEEP.ipynb) demonstrates how to use the python functions defined in [sheep_client.py](https://github.com/alan-turing-institute/SHEEP/tree/master/frontend/pysheep/sheep_client.py) to configure and run a job.

The ability to "pack" several input values into the same ciphertext (also known
as "slots") for SIMD operations is demonstrated in the [Using_SHEEP_with_slots.ipynb](https://github.com/alan-turing-institute/SHEEP/tree/master/frontend/notebooks/Using_SHEEP_with_slots.ipynb).

The [Low-level-benchmarks.ipynb](https://github.com/alan-turing-institute/SHEEP/blob/master/frontend/notebooks/Low-level-benchmarks.ipynb), [Mid-level-benchmarks.ipynb](https://github.com/alan-turing-institute/SHEEP/blob/master/frontend/notebooks/Mid-level-benchmarks.ipynb), and [Parameters_vs_slots_and_serialized_ciphertext_sizes.ipynb](https://github.com/alan-turing-institute/SHEEP/blob/master/frontend/notebooks/Parameters_vs_slots_and_serialized_ciphertext_sizes.ipynb)
demonstrate how one might perform systematic sets of tests comparing different HE libraries with one another.

Finally, the [vector_dot_product.ipynb](https://github.com/alan-turing-institute/SHEEP/blob/master/frontend/notebooks/vector_dot_product.ipynb) and [matrix_vector_multiplication.ipynb](https://github.com/alan-turing-institute/SHEEP/blob/master/frontend/notebooks/matrix_vector_multiplication.ipynb) notebooks give examples of how more complex calculations can be performed using the SIMD/slots functionality and the ***ROTATE*** operation.