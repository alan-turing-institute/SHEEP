# Sheep

## Installation

After cloning:
```
mkdir build
cd build
cmake ..
make all
```
Currently, this builds the examples in `build/bin`.

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
