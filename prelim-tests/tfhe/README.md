## TFHE

https://github.com/tfhe/tfhe

https://tfhe.github.io/tfhe/

From the website

> TFHE is an open-source library for fully homomorphic encryption,
> distributed under the terms of the Apache 2.0 license.

> It also implements a dedicated Fast Fourier Transformation for the
> anticyclic ring R[X]/(X^N + 1), and uses AVX assembly vectorization
> instructions. The default parameter set achieves a 110-bit
> cryptographic security, based on ideal lattice assumptions.

### Installation

```
git clone https://github.com/tfhe/tfhe.git
cd tfhe
git submodule init
git submodule update
mkdir build
cd build
cmake ../src -DENABLE_TESTS=on -DENABLE_FFTW=on -DCMAKE_BUILD_TYPE=debug 
make
make test
```

Can use additional flags to cmake if there is trouble building:

```
-DENABLE_SPQLIOS_AVX=off -DENABLE_SPQLIOS_FMA=off -DENABLE_NAYUKI_AVX=off
```

ots note: I used the macports 'fftw-3' package (version 3.3.5), with
clang 5.0 (also macports).  The resulting library is portable across
compilers.

### Running the example

The [tutorial example](https://tfhe.github.io/tfhe/coding.html) is
reproduced in tutorial.  From within that directory: 

```
# directory containing the TFHE header files, under a subdirectory tfhe
export TFHE_INCLUDE_PATH= 
# directory containing the TFHE libraries
export TFHE_LIBRARY_PATH= 

C_INCLUDE_PATH=$TFHE_INCLUDE_PATH LIBRARY_PATH=$TFHE_LIBRARY_PATH make
```

Running it (from the same directory):
```
% export LD_LIBRARY_PATH=$TFHE_LIBRARY_PATH:$LD_LIBRARY_PATH
% ./alice # produces secret.key, cloud.key and cloud.data
Hi there! Today, I will ask the cloud what is the minimum between 2017 and 42
% ./cloud # produces answer.data
% ./verif
And the result is: 42
I hope you remember what was the question!
```
