FROM ubuntu:16.04

### get pip git etc


RUN apt-get update
RUN apt-get -y install git
RUN apt-get -y install python-pip
RUN apt-get -y install cmake


ADD . /SHEEP



RUN cd /SHEEP; git submodule init; git submodule update 


## build TFHE
RUN apt-get install -y libfftw3-dev
RUN cd /SHEEP/lib/tfhe; git submodule init
RUN cd /SHEEP/lib/tfhe; git submodule update
RUN mkdir /SHEEP/lib/tfhe/build
RUN cd /SHEEP/lib/tfhe/build; cmake ../src -DENABLE_TESTS=on -DENABLE_FFTW=on -DCMAKE_BUILD_TYPE=optim -DENABLE_NAYUKI_PORTABLE=off -DENABLE_SPQLIOS_AVX=off -DENABLE_SPQLIOS_FMA=off -DENABLE_NAYUKI_AVX=off
RUN cd /SHEEP/lib/tfhe/build; make; make install;

## build HElib
RUN apt-get -y install wget
RUN apt-get -y install m4
RUN wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz
RUN tar -xvf gmp-6.1.2.tar.xz
RUN cd gmp-6.1.2; ./configure; make; make install

RUN wget http://www.shoup.net/ntl/ntl-10.5.0.tar.gz
RUN tar -xvzf ntl-10.5.0.tar.gz
RUN cd ntl-10.5.0/src; ./configure NTL_GMP_LIP=on; make; make install
RUN cd /SHEEP/lib/HElib/src ; make

### now build SHEEP

RUN rm -fr /SHEEP/build
RUN mkdir /SHEEP/build
RUN cd /SHEEP/build; cmake ../ ; make all 

### install python packages for the frontend

RUN pip install flask
RUN pip install wtforms
RUN pip install pytest
RUN pip install sqlalchemy
RUN pip install python-nvd3

WORKDIR /SHEEP/frontend

EXPOSE 5000
ENV FLASK_APP app.py

CMD ["python","app.py"]