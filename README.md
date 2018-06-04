# Sheep

SHEEP is a Homomorphic Encryption Evaluation Platform.
Homomorphic Encryption allows mathematical operations
to be performed on encrypted data, such that the result when
decrypted is what would have been obtained from addition or
multiplication of the plaintexts. The goal of the project is to
provide an accessible platform for testing the features and
performance of the available Homomorphic Encryption libraries.

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

## Installation - docker

As above, clone this repo and then do
```
git submodule init
git submodule update
```
Then (assuming you have Docker):
```
docker build -t sheep ./
docker run -p 5000:5000 sheep
```
If you point your browser to `0.0.0.0:5000` you should get the SHEEP frontend.

## Circuit language and input files

The inputs to SHEEP are ***circuit*** files, which have the extension `.sheep` and the following format:
```
INPUTS <circuit_input_0> ...
OUTPUTS <circuit_output_0> ...
<gate_input_0> ...  <GATE> <gate_output>
...
```
