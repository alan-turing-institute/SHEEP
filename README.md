# Sheep

SHEEP is a Homomorphic Encryption Evaluation Platform.
Homomorphic Encryption allows mathematical operations
to be performed on encrypted data, such that the result when
decrypted is what would have been obtained from addition or
multiplication of the plaintexts. The goal of the project is to
provide an accessible platform for testing the features and
performance of the available Homomorphic Encryption libraries.

## Installation

After cloning:
```
git submodule init
git submodule update
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
