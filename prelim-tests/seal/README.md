## SEAL

https://www.microsoft.com/en-us/research/project/simple-encrypted-arithmetic-library/

From the website:

> The Simple Encrypted Arithmetic Library is an easy-to-use homomorphic
> encryption library, developed by researchers in the Cryptography
> Research Group at Microsoft Research. SEAL is written in C++, and
> contains .NET wrappers for the public API. It has no external
> dependencies, so it is easy to compile in many different
> environments. SEAL uses the Microsoft Research License Agreement, and
> is free for research use.

### Installation

It was reasonable straightforward to set up on MacOS, with gcc
installed, although '-march=native' doesn't work for me.

```
wget https://download.microsoft.com/download/B/3/7/B3720F6B-4F4A-4B54-9C6C-751EF194CBE7/SEAL_v2.3.0-4_Linux.tar.gz
tar xf SEAL_v2.3.0-4_Linux.tar.gz
cd SEAL
pushd SEAL
CC=gcc CXX=g++ ./configure
sed -i.bak 's/-march=native//g' Makefile Makefile.in
make
popd
```

produces `bin/libseal.a`.

```
pushd SEALExamples
sed -i.bak 's/-march=native//g' Makefile
make
popd
```

produces `bin/sealexamples`, which allows several examples to be run
interactively.

The examples in `SEAL/SEALExamples/main.cpp`---for example, of the
meaning and usage of the parameters---are well commented and are worth
studying.
