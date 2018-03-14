# Sheep

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

 * Download and build the package you would like to use.
 * In the directory `SHEEP/sketch/cmake/Modules`  write a `Find<package>.cmake` file, following the same pattern as the files
 that are already there, e.g. `FindHElib.cmake`.
   * For the first line (`find_path(...)`) specify the name of any header file in your package.  CMake will use this to check it
  has the correct ***include*** directory.
   * For the second line (`find_library(..`) specify the name of any compiled lib in your package.  Again, CMake will use this
  to find the directory containing all the libraries it might need.
 * In the directory `SHEEP/sketch/include/` make a file defining your context, called `context-<package>.hpp` following the pattern
   of one of the existing contexts.
   * The class you create should inherit from `Context` and should be templated on the Plaintext type.  e.g.

```
  template <typename PlaintextT>
  class ContextMyNewHE
  	: public Context< PlaintextT, My_New_HE_Ciphertext_Type >
  {   ... class definition in here ...
  ```

