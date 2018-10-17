FROM tomaslaz/sheep_base as sheep

#COPY build SHEEP/build
COPY cmake SHEEP/cmake
COPY CMakeLists.txt SHEEP/CMakeLists.txt 
COPY docker-compose.yml SHEEP/docker-compose.yml
COPY example SHEEP/example
COPY include SHEEP/include
COPY LICENSE SHEEP/LICENSE
COPY pysheep SHEEP/pysheep
COPY README.md SHEEP/README.md 
COPY src SHEEP/src
COPY tests SHEEP/tests

RUN rm -fr SHEEP/build
RUN mkdir SHEEP/build
RUN cd SHEEP/build; export CC=gcc-7; export CXX=g++-7; cmake ../ ; make run-sheep-server

WORKDIR SHEEP/build
EXPOSE 34568
CMD ["bin/run-sheep-server"]