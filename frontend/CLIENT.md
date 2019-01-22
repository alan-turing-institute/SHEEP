# The SHEEP client

The sheep server API is documented [here](https://github.com/alan-turing-institute/SHEEP/blob/master/backend/API.md).  However, for convenience, a python module
is provided here containing functions that wrap the API endpoints.

The module is [here](https://github.com/alan-turing-institute/SHEEP/blob/master/frontend/pysheep/sheep_client.py) and the list of functions is as follows:

* ```new_job()``` - reset everything ready for a new circuit evaluation.
* ```get_available_contexts()``` - returns a list of "contexts" (i.e. interfaces to HE libraries) that the SHEEP backend has been linked against.
* ```set_context(<str:context>)``` - set the context
* ```get_available_input_types()``` - returns a list of types (e.g. "int8_t") that can be selected for input values.
* ```set_input_type(<str:input_type>)``` - set the input type.
* ```set_circuit_filename(<str:path/to/circuit_filename>)``` - set the circuit file to be evaluated, using the location ***relative to the server***.
* ```set_circuit(<str:/path/to/circuit_filename>)``` - set the circuit file to be evaluated, if it is in a location accessible to the client (the client will open the file and pass its contents to the server as a string).  This is likely to be more useful than ```set_circuit_filename``` if the backend is running in a docker container or a remote host.
*  ```set_circuit_text(<str:circuit>)``` - set the circuit, passing a string e.g. ```INPUTS a b\n OUTPUTS c\n a b ADD c\n```.
* ```get_inputs()``` - once the circuit is set, this function will return the names of its inputs.
* ```get_circuit()``` - return the currently chosen circuit as a string.
* ```set_inputs({<str:name>:[<int:val>,...],...})``` - set values for the inputs.  The lists of values for each input must be the same length, no longer than *nslots*, and the values themselves must be within the range dictated by the chosen *input_type*.
* ```get_const_inputs()``` - once the circuit is set, this will return the names of any const (i.e. not-to-be-encrypted) inputs.
* ```set_const_inputs({<str:name>:<int:val,...})``` - set values for the inputs.  Unlike the "normal" (i.e. to-be-encrypted) inputs, each const_input only takes a single value rather than a list, and it is not constrained to be within the range defined by *input_type* (it will be treated as a *long int* within the SHEEP code).
* ```get_parameters()``` - get the current set of parameters for the chosen context.
* ```set_parameters({<str:param_name>:<int_param_val>,...})``` - update one or more of the parameters.  Note that setting one parameter may affect the values of other parameters (e.g. changing "BaseParamSet" in HElib will alter the values of all other parameters).
* ```get_nslots()``` - get the number of "slots" (i.e. maximum number of values for each input, to be used in SIMD operations) with the current choice of contextx and parameters.
* ```get_eval_strategy()``` - get the currently configured evaluation strategy ("serial" or "parallel" - default is "serial").
* ```set_eval_strategy(<str:strategy>) - set the evaluation strategy to either "serial" or "parallel".
* ```set_timeout(<int:timeout>)``` - set the timeout for circuit evaluation in seconds.
* ```encrypt_and_serialize([<int:val>,...])``` - encrypt the given input, and return the size of the ciphertext (in bytes).
* ```is_configured()``` - returns True if all configuration options necessary to run the evaluation have been set (circuit, context, input_type, inputs,...) or False otherwise.
* ```get_config()``` - return the full current configuration of the job (circuit, context, parameters, input_type, ...).
* ```run_job()``` - execute the evaluation.
* ```get_results()``` - return the output values, timings, and parameter settings in a dictionary.
