# The SHEEP backend

The SHEEP backend is the core codebase implementing the functionality of SHEEP.
It is written in C++, and can be built using ***cmake*** following the instructions [here](https://github.com/alan-turing-institute/SHEEP/README.md), but it is recommended to use the provided Dockerfile and run the backend in a container.

The backend consists of three main components:

## Circuit

In SHEEP, calculations are represented as ***circuits***.  The circuits are composed of "gates" representing arithmetic operations such as "Add" and "Multiply", each of which have "inputs" and "outputs".
The header file defining our circuit class is [here](https://github.com/alan-turing-institute/SHEEP/backend/include/circuit.hpp).

## Contexts

The interface between external HE libraries and the evaluation of a calculation (represented as a circuit) is via SHEEP ***contexts***.  The [base context](https://github.com/alan-turing-institute/SHEEP/backend/include/context.hpp)
class contains the logic that steps through a circuit and dispatches calls to
the functions that evaluate the gates.  These functions are implemented in the
HE-library-specific contexts (e.g. [this one](https://github.com/alan-turing-institute/SHEEP/backend/include/context-seal.hpp) for the SEAL library), and are
essentially wrappers for function calls to the corresponding HE library.

## Server

Perhaps the easiest way to interact with SHEEP is via REST API.
This is documented [here](https://github.com/alan-turing-institute/SHEEP/backend/API.md).