# The Sheep-server API

## Introduction

One way to interact with SHEEP is via a REST API.  The ```run-sheep-server```
executable built here from (https://github.com/alan-turing-institute/SHEEP/blob/master/backend/applications/server/run-sheep-server.cpp)[this].

The server makes use of the (https://github.com/Microsoft/cpprestsdk)[cpprestsdk] framework from Microsoft.

By default, the server uses port *34568*, i.e. if running locally, the endpoints
can be reached at ```http://localhost:34568/<endpoint>/```

The response to all requests is a json object in the format
```
{"status_code": <int:status_code>,
 "content": <response body> }
```
where ```status_code``` will be 200 for success, or 500 for error.


## Available endpoints

### ```GET``` methods

* ```/context/```: returns the current context (string).
* ```/circuit/```: returns the current circuit as a string.
* ```/input_type/```: returns the currently selected input type (e.g. "int8_t") as a string.
* ```/parameters/```: returns a json object ```{<str:param_name>:<int:param_val>}```.