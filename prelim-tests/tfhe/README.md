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

As of v1.0, it is boostrapped on each gate evaluation.

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
reproduced in `tutorial`.  From within that directory: 

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

### The parameters

`TFheGateBootstrappingParameterSet *new_default_gate_bootstrapping_parameters(int32_t minimum_lambda);`
gives a default parameter set with estimated 128 bits of security (it
simply triggers an error if more security is asked for than can be
provided).

The parameters are represented by the following structs:

```
typedef int32_t Torus32;

struct LweParams {
	const int32_t n;
	const double alpha_min;//le plus petit bruit tq sur
	                       // (the minimal noise)
	const double alpha_max;//le plus gd bruit qui permet le déchiffrement
	                       // (the largest noise that allows decryption)
};

struct TLweParams {
    const int32_t N; ///< a power of 2: degree of the polynomials
    const int32_t k; ///< number of polynomials in the mask
    const double alpha_min; ///< minimal noise s.t. the sample is secure
    const double alpha_max; ///< maximal noise s.t. we can decrypt
    const LweParams extracted_lweparams; ///< lwe params if one extracts
};

struct TGswParams {
    const int32_t l; ///< decomp length
    const int32_t Bgbit;///< log_2(Bg)
    const int32_t Bg;///< decomposition base (must be a power of 2)
    const int32_t halfBg; ///< Bg/2
    const uint32_t maskMod; ///< Bg-1
    const TLweParams *tlwe_params; ///< Params of each row
    const int32_t kpl; ///< number of rows = (k+1)*l
    Torus32 *h; ///< powers of Bgbit
    uint32_t offset; ///< offset = Bg/2 * (2^(32-Bgbit) + 2^(32-2*Bgbit) + ... + 2^(32-l*Bgbit))
};

struct TFheGateBootstrappingParameterSet {
    const int32_t ks_t;
    const int32_t ks_basebit;
    const LweParams *const in_out_params;
    const TGswParams *const tgsw_params;
};
```

Of these, we must set 

| Name         | Member of     | Meaning                   | Default value                          |
| ---------    | ---------     | ---------                 | -------                                |
| `N`          | TLwe          | degree of the polynomials | 1024                                   |
| `k`          | TLwe          | number of polynomials     | 1                                      |
| `n`          | Lwe           | ?                         | 500                                    |
| `l`          | TGsw          | decomp length?            | 2                                      |
| `Bgbit`      | TGsw          | log2(decomp base)         | 10                                     |
| `ks_t`       | Bootstrapping | length of ?               | 8                                      |
| `ks_basebit` | Bootstrapping | ?                         | 2                                      |
| `alpha_min`  | Lwe           | std deviation             | `\frac{\sqrt{2}}{\pi} 2^{-15}`         |
| `alpha_max`  | Lwe           | max std deviation         | `\frac{\sqrt{2}}{4\pi} 2^{-4}`         |
| `bk_stdev`   | Tlwe          | std deviation             | `\frac{\sqrt{2}}{\pi} 9\times 10^{-9}` |


> What are `n`, `alpha` and `q` for the lwe-estimator?

An example of reading the full set of parameters from the command line
and using them for a simple THFE calculation is
`examples/mult_depth.cpp`.

