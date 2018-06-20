# SHEEP is a Homomorphic Encryption Evaluation Platform

## Summary

The overall aim of the SHEEP project is to provide a platform for practitioners to evaluate the state-of-the-art of (fully) homomorphic encryption technology in the context of their concrete application. SHEEP enables this evaluation to be done across libraries implementing different HE schemes (with in some cases incomparable security).

<figure>
<img src="https://i.imgur.com/M9KPfZi.png"/>
<figcaption> Components of the SHEEP platform. These include an interpreter for the SHEEP language, an abstraction layer to incorporate HE libraries via their corresponding wrappers, and a reporting component. The user can either provide a program written in the high-level SHEEP language, or run a set of predefined benchmarks. Different aspects of the results can be visualized through a web interface.</figcaption>
</figure>


<!-- ![Components of SHEEP. These include an interpreter for the SHEEP language, an abstraction layer to incorporate HE libraries via their corresponding wrappers, and a reporting component. The user can either provide a program written in the high-level SHEEP language, or run a set of predefined benchmarks. Different aspects of the results can be visualized through a web interface.](https://i.imgur.com/M9KPfZi.png) -->


To achieve this goal, the SHEEP platform includes an **library agnostic language** so that computations can be described generically. One can think of it as an *assembly language*.

To incorporate a library into the platform, all that needs to be done is write a wrapper for it, implementing the operations in the **SHEEP instruction set**. These corresponds to common operations in a HE scheme such as encryption, decryption, homomorphic addition, multiplication, comparison, etc.

Besides allowing the user to execute their own programs via either a web interface or programatically, the library incorporates a set of **native benchmarks**. These are classified in three categories according to their complexity, ranging from low-level microbenchmarks of the basic homomorphic arithmetic operations, to data analysis tasks.

Either on a native benchmark or a user-defined input, the platform evaluates the corresponding program on all the HE libraries that support the instructions of the program. The result is checked for correctness, and metrics such as running time (encryption, decryption, and evaluation) and ciphertext size are reported to a web interface and stored in a database. The web interface also allows convenient plotting of the results for easy visualisation.

The platform hence enables two main use cases:
1. It allows users to evaluate a concrete custom computation across libraries (and thus encryption schemes) by simply describing it in a high-level language. This allows to set up a easily reproducible baseline when trying to optimize a given primitive based on HE, e.g. Private Information Retrieval, secure inner product, etc. For example, addition of two numbers in SHEEP looks like this:
```
INPUTS a b res
OUTPUTS res
a b ADD res
```
2. The set of predefined benchmarks allows performance snapshots of the state-of-the-art of (fully) homomorphic encryption implementations to be continually produced, as new schemes are invented and their implementations are updated and optimized.

In summary, the general problem addressed by SHEEP is that of getting a satisfying answer, from the practitioner's point of view, to the following questions:

> "What is the current state of the implementations of homomorphic encryption?"

> "What homomorphic encryption scheme (and implementation) should I use in my application, given some functional and non-functional, e.g. security, latency or bandwidth constraints?"

In its current state, the SHEEP platform does not provide a complete solution to these problems, as we believe this requires a community effort. This is why SHEEP's code is open source.


## Installation - docker

Assuming you have docker installed, clone this repo and then do
```
git submodule init
git submodule update
```
Then (assuming you have Docker):
```
docker-compose build
```
This will build three docker images: one for the "server", one for a web-based frontend, and one for some Jupyter notebooks containing examples
of SHEEP usage.

To run the web frontend, do:
```
docker-compose run -p 5000:5000 sheep-frontend
```
If you point your browser to `0.0.0.0:5000` you should get the SHEEP frontend.

To run the notebooks, do:
```
docker-compose run -p 8888:8888 sheep-notebook
```
Now if you paste the URL given in the terminal for "sheep-notebook_1"
into your browser, but with `localhost` instead of the hex string immediately after the "http://", you can access the Jupyter notebook examples.



## Installation - local

After cloning:
```
git submodule update --init --recursive
```
This will checkout the HElib and TFHE submodules in the lib/ directory - build TFHE as follows:
```
cd lib/tfhe
mkdir build
cd build
cmake ../src -DENABLE_TESTS=on -DENABLE_FFTW=on -DCMAKE_BUILD_TYPE=optim -DENABLE_NAYUKI_PORTABLE=off -DENABLE_SPQLIOS_AVX=off -DENABLE_SPQLIOS_FMA=off -DENABLE_NAYUKI_AVX=off
make
sudo make install
```
Then build HElib as follows:
```
cd ../../HElib/src
make
mkdir build
cd build
cmake ..
make all
```
And finally build SHEEP:
```
cd ../../../
mkdir build
cd build
cmake ../
make all
```
This builds the examples in `build/bin`.

Run the tests with:
```
make test
```



## Circuit language and input files

The inputs to SHEEP are ***circuit*** files, which have the extension `.sheep` and the following format:
```
INPUTS <circuit_input_0> ...
OUTPUTS <circuit_output_0> ...
<gate_input_0> ...  <GATE> <gate_output>
...
```
