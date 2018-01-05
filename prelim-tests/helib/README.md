
## About HElib  (from the HElib README).

HElib impolements the BGV scheme, Smart-Vercauteren packing techniques, and
the Gentry-Halevi-Smart optimizations.  Also includes bootstrapping.
It is written in C++, and the product of compilation is a library fhe.a
that can be included in other programs.
Also in the package are some example Test executables.


## installing HElib

git clone git@github.com:shaih/HElib.git

Needs GMP and NTL.  GMP may already be installed.  If not:
(following instructions copied from INSTALL.txt in HElib repo):

1. Download GMP from http://www.gmplib.org
2. uncompress and cd into the directory gmp-XXX
3. On the command line:
      ./configure
      make
      sudo make install
4. This should install GMP into /usr/local

(I got latest version as of 4/12/17 which is gmp-6.1.2)

Then, to install NTL (instructions also copied from HElib's INSTALL.txt):

1. Download NTL from http://www.shoup.net/ntl/download.html
2. uncompress and cd into the directory ntl-XXX/src
3. On the command line:
      ./configure NTL_GMP_LIP=on
      make
      sudo make install
4. This should install NTL into /usr/local

(I got latest version as of 4/12/17 which is ntl-10.5.0)


Then cd to HElib/src and do:
make
make check

This will build the library fhe.a  which can then be included in your
program.

====================================================

Usage:

There are several example programs in the src/ directory called Test_<something>.cpp
which can be compiled with
make Test_<something>_x
to give an executable called Test_<something_x

Similarly, you can write your own code in <myprog>.cpp then do
make <myprog>_x
to build the <myprog>_x executable which uses the fhe.a library.


### "Tutorial" (found by googling):
http://tommd.github.io/posts/HELib-Intro.html
This mentions that NTL, on which HElib is built, is not thread-safe, so the
potential parallelism from the packing is not fully realised.

### "Example_x" (adapted from tutorial above)
Example.cpp program in this directory (copy to HElib/src and do `make Example_x` ) to explore how nslots (the number of elements in the plaintext/ciphertext
vectors (i.e. the "packing")) varies with different parameters, and how
different numbers of additions and multiplications can give rise to "noise".

One of the parameters is "p" - the modulo for the decrypted text.  This is 2
in the original tutorial, meaning that the output is binary, and addition is
XOR, multiplication is AND etc.
It can be changed to give integer results, but ***p needs to be a prime***,
otherwise an assertion fails when calculating "m".

### Further examples and explanation
https://pwnhome.wordpress.com/2013/05/03/guide-to-helib-1/
https://pwnhome.wordpress.com/2013/05/29/guide-to-helib-2/
https://pwnhome.wordpress.com/2013/06/19/guide-to-helib-3/

### Parameters

A nice intro to Helib and its parameters can be found [here](http://people.csail.mit.edu/shaih/pubs/he-library.pdf).

| Name         | Meaning       | Default value             |
| ---------    | ---------     | ---------                 |
| `m`          | Parameter for chosing mth cyclotomic polynomial $\Phi(X)$             | ?                         |
| `\Phi(m)`    | mth cyclotomic polynomial, i.e. irreducible polynomial with integer coefficients that divides $x^n-1$ but not $x^k-1$, for all $k < n$     | ?                         |
| Plaintext space   | usually $A_2 = A/2A$ (binary polynomials modulo $\Phi(m)$)               | ?                         |
| `q`          | Modulus to cancel out noise in decryption. This value evolves as homomorphic operations are applied, and hence we have $q_i$ for each level i             | ?                         |
| Ciphertext space   | $A_q = A/2A$ (binary polynomials modulo $\Phi(m)$)               | ?                         |
| `p`          |  Parameter defining plaintext space as $A_p$ (as opposed to $A_2$ above)            | ?                |
| `r`          |  Parameter defining plaintext space as $A_{p^r}$ (as opposed to $A_p$ above)             | 1, and in this case all slots are isomorphic to $F_{p^d}$, with d defined in the next entry of this table                |
| `l`          |  Number of slots. This corresponds to the number of irreducible factors of $\Phi(m) mod 2$, which all have degree $d=\phi(m)/l$.              | ?                         |
| `L`          |  The maximum number of homomorphic levels. The L moduli q_1, ..., q_l above are defined as as a product of i carefully chosen primes          | ?                |

#### DoubleCRT

A polinomial in the ciphertext space A_q is represented in Helib using the double CRT representation.
See section 2.1 in the document linked above [here](http://people.csail.mit.edu/shaih/pubs/he-library.pdf).


### Observations

* ciphertext size (as obtained from sizeof() on a single element of a CTxt) is
always 122 bytes. 
