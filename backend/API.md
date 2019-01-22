# The Sheep-server API

## Introduction

One way to interact with SHEEP is via a REST API.  The ```applications/run-sheep-server```
executable built here from [this source code](https://github.com/alan-turing-institute/SHEEP/tree/master/backend/applications/server/run-sheep-server.cpp).

The server makes use of the [cpprestsdk](https://github.com/Microsoft/cpprestsdk) framework from Microsoft.

By default, the server uses port *34568*, i.e. if running locally, the endpoints
can be reached at ```http://localhost:34568/SheepServer/<endpoint>/```

The response to all requests is a json object in the format
```
{"status_code": <int:status_code>,
 "content": <response body> }
```
where ```status_code``` will be 200 for success, or 500 for error, and the "content" will be a string or JSON object as detailed below.

Users can hit these API endpoints using the tool of their choice,
but for convenience, a python module (using the ***requests*** library) providing functions that wrap these calls is provided [here](https://github.com/alan-turing-institute/SHEEP/tree/master/frontend/pysheep/sheep_client.py) and documented [here](https://github.com/alan-turing-institute/SHEEP/tree/master/frontend/CLIENT.md).


## Available endpoints

### ```GET``` methods

* ```/context/```: returns the list of available contexts.
* ```/input_type/```: returns list of available input types.
* ```/circuit/```: returns the current circuit as a string.
* ```/parameters/```: returns a JSON object containing the parameter settings for the current context ```{<str:param_name>:<int:param_val>, ...}```.
* ```/eval_strategy/```: returns the curently selected eval strategy as a string (either "serial" or "parallel").
```/inputs/```: returns a JSON object containing the names of the input wires for the current circute ```[<input_1_name>, <input_2_name>, ...]```
```/const_inputs/```: same as "inputs/", but for const (i.e. unencrypted) inputs to the circuit.
* ```/slots/``` returns JSON object ```{"nslots": <int:num_slots>}``` with the
number of "slots" for SIMD operations that each input can take.  This is determined by the choice of parameters for a given context.
* ```/job/```: returns a JSON object ```{"job_configured": <bool:is_configured>}``` where "is_configured" is ***true*** if the server has a circuit, context, input_type, eval_strategy, and all input and const_input values specified, and ***false*** otherwise.
```/config/```: returns a JSON object with the full configuration ```{"circuit_filename":<str:filename>,"context":<str:context>, "input_type": <str:input_type>, "eval_strategy":<str:eval_strategy>, "nslots": <int:nslots>,"parameters":<param_dict>}``` where the parameters dictionary is the same as that returned from the ```parameters``` endpoint.
* ```/results/```: once a job has run, this returns the results as a JSON object: ```{<str:output_name>:<str:output_val>,...}```.  Note that although the "output_val" is a string, it will in general represent a list of integers.


### ```PUT``` methods

* ```/parameters/```, payload=```{<str:param_name>:<str:param_value>,...}``` -
set one or more parameters.  Note that in some cases, setting a parameter can affect the values of other parameters (e.g. choosing "BaseParamSet" in HElib).
* ```/eval_strategy/```, payload=```{"eval_strategy":<str:strat>}``` - set the evaluation strategy to either "serial" (the default - serial evaluation) or "parallel" (use Intel TBB to parallelise the evaluation of the circuit).
* ```/timeout/```, payload=```{"timeout": <int:timeout>}``` - set the timeout for circuit evaluation in seconds.

### ```POST``` methods

* ```/job/```, no payload.  Resets all configuration on the server to be ready for a new job.
* ```/circuitfile/```, payload=```{"circuit_filename": <str:local_path_to_circuit_file>}```.  Load a circuit via specifying the filename.  Note that this will generally only be possible when running the sheep server on the local machine (not on a remote server or in a docker container).
* ```/circuit/```, payload=```{"circuit": <str:circuit>}```.  Load a circuit as a string.  If running the sheep server somewhere other than the local machine (e.g. in a docker container), this is the best option - the client can open and read a local circuit file and post the contents as a string.
* ```/context/```, payload=```{"context_name":<str:context_name>}``` - set the context by name, e.g. "SEAL".  The list of available contexts can be obtained via a GET request to ```/context/``` as described above.
* ```/input_type/```, payload=```{"input_type":<str:input_type>}``` - set the input_type to e.g. "int8_t".  The list of available input types can be obtained via a GET request to ```/input_type/``` as described above.
* ```/inputs/```, payload=```{<str:input_name_1: [<int:val_1_0>,...], ...}``` i.e. for every input to the circuit (the names of which can be obtained via a GET request to ```/inputs/```) a list of values (no longer than ***nslots***) is provided.  All lists must be the same length, and if this length is less than ***nslots***, remaining slots will be filled up by repeating the sequence of input values.
* ```/const_inputs/```, payload=```{<str:const_input_name_1: <int:const_val_1>,...}```, i.e. one value for each of the const (not-to-be-encrypted) inputs to the circuit.
* ```/serialized_ct/```, payload =```{"inputs": [<int:val_1>,...]}```, return value=```{"size":<int:ct_size>}```, i.e. given a vector (of length no greater than ***nslots***) of input values, this returns thesize in bytes of the corresponding ciphertext.
* ```/run/```, no payload.  If circuit, context, input_type, and all parameters and inputs are specified, run the circuit evaluation.