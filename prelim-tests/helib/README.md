
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