# Sheep

[![Build Status](https://travis-ci.com/alan-turing-institute/SHEEP.svg?token=SEnDtSV7m3nSdUzTdE4a&branch=master)](https://travis-ci.com/alan-turing-institute/SHEEP)

SHEEP is a Homomorphic Encryption Evaluation Platform.
Homomorphic Encryption allows mathematical operations
to be performed on encrypted data, such that the result when
decrypted is what would have been obtained from addition or
multiplication of the plaintexts. The goal of the project is to
provide an accessible platform for testing the features and
performance of the available Homomorphic Encryption libraries.

## Installing the SHEEP package

After cloning from this git repo:
```
mkdir build
cd build
cmake ..
make all
```
Currently, this builds the examples in `build/bin` and tests in `build/testbin`.

It may be necessary to specify the installation location of TFHE, HElib or SEAL, if these are in a non-standard place.
Replace `cmake ..` in the above with
```
CMAKE_LIBRARY_PATH=... CMAKE_INCLUDE_PATH=... cmake ..
```
where the '...' are replaced by a colon-separated list of the relevant paths.

Run the tests with:
```
make test
```

## Adding another HE library

 1. Download and build the package you would like to use, following the instructions that the authors will hopefully provide.
 2. Checkout the `master` branch of SHEEP, and then create a branch for working on your new package:
 ```
 git checkout -b "add-MyNewHElibrary"
 ```
 3. In the directory `SHEEP/sketch/cmake/Modules`  write a `Find<package>.cmake` file, following the same pattern as the files
 that are already there, e.g. `FindHElib.cmake`.
   * For the first line (`find_path(...)`) specify the name of any header file in your package.  CMake will use this to check it
  has the correct ***include*** directory.
   * For the second line (`find_library(..`) specify the name of any compiled lib in your package.  Again, CMake will use this
  to find the directory containing all the libraries it might need.
 4. In the directory `SHEEP/sketch/include/` make a file defining your context, called `context-<package>.hpp` following the pattern
   of one of the existing contexts.
   * The class you create should inherit from `Context` and should be templated on the Plaintext type.  e.g.

```
  template <typename PlaintextT>
  class ContextMyNewHE
  	: public Context< PlaintextT, My_New_HE_Ciphertext_Type >
  {   ... class definition in here ...
  ```
  * You should do this in a namespace `Sheep::YourNewContext`, again following the examples of HElib and TFHE.
  * Implement the encrypt/decrypt functions, and all the gates you can, using calls to your HE library (see `context.hpp` for the
  full list of gates - any that you don't implement will throw a `GateNotImplemented` exception if you try to then use them).
 5. In the directory `SHEEP/sketch/cmake/example/` create a file `mynewlib-context-example.cpp` with a `main()` function to
    test your new context, again following the examples of `tfhe-context-example.cpp` or `helib-context-example.cpp`.
 6. In `example/CMakeLists.txt` add the block to create your new executable.  Ideally, place it inside an `if (MyLIB_FOUND)` block,
 as is done for TFHE and HElib, where "MyLIB_FOUND" is the variable defined by your cmake script from step 2).
 7. From `SHEEP/sketch/build` run `cmake ../` then `make all`.  It should build your new executable, which you can test!
 8. Once you have the basic functionality, add some tests to the `SHEEP/sketch/tests/` directory, testing out the different gates and inputs.
   * You will need to add corresponding blocks to `SHEEP/sketch/tests/CMakeLists.txt` to get the executables build and tests defined.
   * To run all tests, you can run `make test` from the `build` directory, or if you just want to run a specific one, the executables
   are in `build/testbin`.
 9. To add your new library/context to the list that can be run via the frontend, you need to:
   * Edit the `example/benchmark.cpp` file, `#include`-ing your context, and adding another `if` option in the `make_context` function.
   * Edit `frontend/forms.py`, adding another tuple `("<name>","<display_name>")` to the `HE_library` list in the `CircuitForm` definition.  Note that `"<name>"` here must match exactly the name you used in the `if` statement in the `make_context` function in `benchmark.cpp`.
   * To run the frontend app, ensure you have all the necessary python packages installed (see frontend/README.md), then, from the `frontend` directory, do `python app.py` then point your browser to `localhost:5000`.

