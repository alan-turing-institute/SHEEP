FROM ubuntu:16.04 as sheep_base

### get wget git etc


RUN apt-get update; apt-get -y install git
RUN apt-get update; apt-get -y install wget

### get gcc-7 (gcc >=6 needed for SEAL).

RUN apt-get -y install software-properties-common
RUN add-apt-repository -y  ppa:ubuntu-toolchain-r/test
RUN apt-get update; apt-get -y install gcc-7 g++-7
RUN apt-get update; apt-get -y install build-essential

###### get fftw3 (needed for TFHE)

RUN apt-get install -y libfftw3-dev

###### build cmake from source (to get a new enough version for SEAL)

RUN wget https://cmake.org/files/v3.11/cmake-3.11.4.tar.gz
RUN tar -xvzf cmake-3.11.4.tar.gz
RUN cd cmake-3.11.4; export CC=gcc-7; export CXX=g++-7; ./bootstrap; make -j4; make install

####### install intel-tbb for parallelisation
RUN apt-get -y install libtbb-dev

###### get gmp (needed for HElib)
RUN apt-get -y install m4
RUN wget https://gmplib.org/download/gmp/gmp-6.1.0.tar.xz
RUN tar -xvf gmp-6.1.0.tar.xz
RUN cd gmp-6.1.0; export CC=gcc-7; export CXX=g++-7; ./configure; make; make install

###### get ntl (needed for HElib)

RUN wget http://www.shoup.net/ntl/ntl-11.1.0.tar.gz
RUN tar -xvzf ntl-11.1.0.tar.gz
RUN cd ntl-11.1.0/src; export CC=gcc-7; export CXX=g++-7; ./configure NTL_GMP_LIP=on NTL_EXCEPTIONS=on; make; make install

###### get cpprestsdk (for the REST API)
RUN apt-get update
RUN apt-get -y install libssl-dev
RUN apt-get -y install libboost-all-dev
RUN git clone https://github.com/Microsoft/cpprestsdk.git casablanca
RUN cd casablanca/Release; mkdir build.debug; cd build.debug; export CC=gcc-7; export CXX=g++-7; cmake .. -DCMAKE_BUILD_TYPE=Debug; make install


###### build SEAL

RUN wget https://download.microsoft.com/download/B/3/7/B3720F6B-4F4A-4B54-9C6C-751EF194CBE7/SEAL_2.3.1.tar.gz
RUN tar xf SEAL_2.3.1.tar.gz
RUN cd SEAL_2.3.1/SEAL; mkdir build; cd build; export CC=gcc-7; export CXX=g++-7; cmake ..; make; make install;

###### get and build libpaillier
RUN wget http://hms.isi.jhu.edu/acsc/libpaillier/libpaillier-0.8.tar.gz
RUN tar -xvzf libpaillier-0.8.tar.gz
RUN cd libpaillier-0.8 ; ./configure; make; make install


#############################################################################################################
#####  Everything before this can be run to give a base docker image, to avoid having to recompile everything.
#####  From this point on, things might change as SHEEP code evolves.

FROM sheep_base as sheep_dev

ADD . SHEEP

RUN cd SHEEP; git submodule init; git submodule update

## build libpaillier
##RUN cd SHEEP/lib/libpaillier-0.8; git submodule init
##RUN cd SHEEP/lib/libpaillier-0.8; git submodule update
##RUN cd SHEEP/lib/libpaillier-0.8; for f in `ls *.cpp`; do sed 's/<paillier.h>/"paillier.h"/g' $f > ${f}.tmp; mv ${f}.tmp $f; done;
##RUN cd SHEEP/lib/libpaillier-0.8; make


## build TFHE

RUN cd SHEEP/lib/tfhe; git submodule init
RUN cd SHEEP/lib/tfhe; git submodule update
RUN rm -fr SHEEP/lib/tfhe/build
RUN mkdir -p SHEEP/lib/tfhe/build
RUN cd SHEEP/lib/tfhe/build; export CC=gcc-7; export CXX=g++-7; cmake ../src -DENABLE_TESTS=on -DENABLE_FFTW=on -DCMAKE_BUILD_TYPE=optim -DENABLE_NAYUKI_PORTABLE=off -DENABLE_SPQLIOS_AVX=off -DENABLE_SPQLIOS_FMA=off -DENABLE_NAYUKI_AVX=off
RUN cd SHEEP/lib/tfhe/build; make; make install;

## build HElib

RUN cd SHEEP/lib/HElib/src ; export CC=gcc-7; export CXX=g++-7; make clean; make;


FROM sheep_dev as sheep

#### now build SHEEP

RUN rm -fr SHEEP/build
RUN mkdir SHEEP/build
RUN cd SHEEP/build; export CC=gcc-7; export CXX=g++-7; cmake ../ ; make run-sheep-server


WORKDIR SHEEP/build
EXPOSE 34568
CMD ["bin/run-sheep-server"]
