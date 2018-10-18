FROM tomaslaz/sheep_base as sheep

#COPY build SHEEP/build
ADD cmake SHEEP/cmake
ADD CMakeLists.txt SHEEP/CMakeLists.txt 
ADD docker-compose.yml SHEEP/docker-compose.yml
ADD example SHEEP/example
ADD include SHEEP/include
ADD LICENSE SHEEP/LICENSE
ADD pysheep SHEEP/pysheep
ADD README.md SHEEP/README.md 
ADD src SHEEP/src
ADD tests SHEEP/tests

RUN rm -fr SHEEP/build; mkdir SHEEP/build

RUN cd SHEEP/build; export CC=gcc-7; export CXX=g++-7; cmake ../; 

RUN cd SHEEP/build; make all

WORKDIR SHEEP/build
EXPOSE 34568

CMD ["bin/run-sheep-server"]