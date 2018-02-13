# Sheep

## Installation

After cloning:
```
mkdir build
cd build
cmake ..
make all
```

It may be necessary to specify the installation location of TFHE, HElib or SEAL, if these are in a non-standard place, as
```
CMAKE_LIBRARY_PATH=... CMAKE_INCLUDE_PATH=... cmake ..
```
with the '...' replaced by a colon-separated list of the relevant paths.

Run the tests with:
```
make test
```
