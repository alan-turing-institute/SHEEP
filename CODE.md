### Overview

Since SHEEP is intended to work with the current leading HE libraries, which
are mainly written in C++, it was decided at an early stage of development
that SHEEP itself should also be written in C++.
This makes it straightforward to interface with the various libraries, and
also ensures that there would be minimal performance overhead from the SHEEP
infrastructure.

The web application that allows users to run their own tests and view the
results is written in Python, using the ***Flask*** and ***WTForms***
libraries for the web views, the ***python-nvd3*** library for plotting results,
and ***sqlalchemy*** for storing and retrieving results from an SQLite 
database.

### Design

The philosophy behind the SHEEP project is to provide a straightforward way
of representing programs or tasks as ***circuits***, and for these to be
run in one or more ***contexts***, where each context essentially corresponds
to a different HE library.

#### Circuits

Circuits are composed of ***Gates*** representing low-level operations such
as addition, multiplication etc.  Each gate has inputs and outputs, and they
can be combined to give arbitrarily complex circuits.

The full list of Gates currently implemented (in at least one "context") in
SHEEP is:
* Add:  takes two inputs, and sums to give one output.
* Subtract: takes two inputs, subtracts the second from the first to give output.
* Multiply: takes two inputs, output is their product.
* Negate: takes one input and has one output.  For boolean inputs, 0 is
changed to 1 and vice versa.  For signed integer inputs, the output is -1
multiplied by the input.  For unsigned integer inputs, the output is INT_MAX (for the
input bitwidth) minus the input.
* Select: takes three inputs (s,a,b), and returns a if s is 1, and b if it is 0.
* Compare: takes two inputs (a,b) and returns 1 if a>b and 0 otherwise.
* AddConstant: takes one ciphertext input and one plaintext input, and outputs their sum.
* MultByConstant: takes one ciphertext input and one plaintext input, and outputs their product.

Circuits can be read from, and written to, text files, which have the ```.sheep``` file extension.  An example
of the format of these files is as follows:
```
INPUTS a b c
OUTPUTS total
a b ADD out1
out1 c ADD total
```
This circuit will sum three inputs a,b,c to give the output called "total".


#### Circuit inputs

The choice of input type, and the assignment of values to the inputs, are both kept separate
from the circuit description in SHEEP - the same circuit file can be
used to represent any type of input.

At present, the following choices of input type are supported:
* boolean
* int8_t
* uint8_t
* int16_t
* uint16_t
* int32_t
* uint32_t
Preliminary developments on a branch of the project also allow arbitrary-length vectors of the above types to be
represented - this facilitates "SIMD" operations for those HE libraries that support them.



#### Contexts

The interface between the SHEEP code and an external HE library is encapsulated in a ***Context*** class.
These all inherit from a base context class that defines the interface.  All concrete context classes must therefore
implement the following functions:
```
Ciphertext encrypt(Plaintext)
Plaintext decrypt(Ciphertext)
```
as well as all the "Gate" operations, operating on ciphertexts. 
In the ideal situation, the implementation of a Gate within a given context
will just be a library call to the corresponding function in that HE library.
However, in order to provide a uniform interfact for all SHEEP contexts, there
may be some manipulation to the inputs and outputs required.  One example of
this is the TFHE library, which only deals with binary inputs.
Therefore, when implementing an "Addition" gate for integers
in the SHEEP TFHE context, it is necessary to encode the integers as bit arrays
and construct the binary circuit.

The base context class contains methods to:
* Compile an input circuit into a ***CircuitEvaluator*** object, which takes ciphertext inputs and computes the sequence of
assignments in the circuit.
* Go through the entire chain of encryption to circuit-evaluation to decryption, taking in a vector of plaintext inputs and
return a vector of plaintext outputs.
* Read parameters from a file, and print parameter values to stdout.

The contexts that have been implemented to date are:
* ***ContextClear***:  no encryption - used to check outputs of other contexts.
* ***ContextTFHE***:  interface to the ***TFHE*** library.  Integer inputs are encoded into vectors of binary numbers which are encrypted
individually, and operations are implemented as binary circuits.
* ***ContextHElib_F2***: interface to the ***HElib*** library, with plaintext space modulus parameter ***p*** set to 2, such that
integer inputs are encoded into vectors of binary numbers.  Although HElib natively supports SIMD operations with vectors of
input integers, we are currently not exploiting this capability in our main development branch.
* ***ContextHElib_Fp***: interface to the ***HElib*** library, with ***p*** set to a larger (prime) number, such that integer inputs
(provided they are smaller than ***p***) can be operated on directly.
* ***ContextSEAL***: interface to the ***SEAL*** library.  SEAL has a lot of flexibility in how it can be configured in terms
of encoding inputs as binary numbers, or using multiple "slots" for SIMD operations.  For the first tests, we are using
integer input space.

#### Build environment, testing, and deployment

The ***cmake*** build-management system is used to compile the C++ code for SHEEP.  This takes care of finding the
external library and header files for the HE libraries, and conditional statements in the cmake
configuration files ensured that executables are only built when the libraries on which they depend are present.

CMake is also used to run the suite of test programs, which cover all combinations of context, gate, and input-type, and
test edge-cases of input values (e.g. maximum or minimum integer values for a given type).

The ***Docker*** containerization software can be used to run and deploy SHEEP.  The `Dockerfile` in the main SHEEP directory
contains all the instructions necessary to download necessary software, and compile and run SHEEP, based on an Ubuntu Linux
image.   The resulting Docker image can then be shared. or deployed on a web-server, for example as an ***Azure web app*** to
provide a public website.

#### Parallelization

The Intel ***TBB*** (Thread Building Blocks) library can be used in SHEEP to speed up the evaluation of circuits.
At present this has only been fully utilized in ***ContextTFHE***.




