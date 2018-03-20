FROM ubuntu:16.04

### get pip git etc


RUN apt-get update
RUN apt-get -y install git
RUN apt-get -y install python-pip
RUN apt-get -y install cmake
RUN apt-get -y install wget


###### get fftw3 (needed for TFHE)

RUN apt-get install -y libfftw3-dev

###### get gmp (needed for HElib)
RUN apt-get -y install m4
RUN wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz
RUN tar -xvf gmp-6.1.2.tar.xz
RUN cd gmp-6.1.2; ./configure; make; make install

###### get ntl (needed for HElib)

RUN wget http://www.shoup.net/ntl/ntl-10.5.0.tar.gz
RUN tar -xvzf ntl-10.5.0.tar.gz
RUN cd ntl-10.5.0/src; ./configure NTL_GMP_LIP=on; make; make install

###### install python packages for the frontend
RUN pip install flask
RUN pip install wtforms
RUN pip install pytest
RUN pip install sqlalchemy
RUN pip install python-nvd3


## build SEAL

RUN wget https://download.microsoft.com/download/B/3/7/B3720F6B-4F4A-4B54-9C6C-751EF194CBE7/SEAL_v2.3.0-4_Linux.tar.gz
RUN tar xf SEAL_v2.3.0-4_Linux.tar.gz
RUN cd SEAL/SEAL; export CC=gcc; export CXX=g++; ./configure; sed -i.bak 's/-march=native//g' Makefile Makefile.in; make; make install;


#############################################################################################################
#####  Everything before this can be run to give a base docker image, to avoid having to recompile everything.
#####  From this point on, things might change as SHEEP code evolves.

ADD . SHEEP

RUN cd SHEEP; git submodule init; git submodule update 


## build TFHE

RUN cd SHEEP/lib/tfhe; git submodule init
RUN cd SHEEP/lib/tfhe; git submodule update
RUN rm -fr SHEEP/lib/tfhe/build
RUN mkdir -p SHEEP/lib/tfhe/build
RUN cd SHEEP/lib/tfhe/build; cmake ../src -DENABLE_TESTS=on -DENABLE_FFTW=on -DCMAKE_BUILD_TYPE=optim -DENABLE_NAYUKI_PORTABLE=off -DENABLE_SPQLIOS_AVX=off -DENABLE_SPQLIOS_FMA=off -DENABLE_NAYUKI_AVX=off
RUN cd SHEEP/lib/tfhe/build; make; make install;

## build HElib

RUN cd SHEEP/lib/HElib/src ; make clean; make;


#### now build SHEEP

RUN rm -fr SHEEP/build
RUN mkdir SHEEP/build
RUN cd SHEEP/build; cmake ../ ; make all 


### run the flask app

WORKDIR SHEEP/frontend
###
EXPOSE 5000
ENV FLASK_APP app.py
ENV SHEEP_HOME /SHEEP
###
CMD ["python","app.py"]