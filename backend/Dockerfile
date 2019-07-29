
FROM turingsheep/sheep_base:v2 as sheep

ADD backend/applications SHEEP/backend/applications
ADD backend/cmake SHEEP/backend/cmake
ADD backend/CMakeLists.txt SHEEP/backend/CMakeLists.txt
ADD backend/include SHEEP/backend/include
ADD backend/src SHEEP/backend/src
ADD backend/tests SHEEP/backend/tests

RUN rm -fr SHEEP/backend/build; mkdir SHEEP/backend/build

RUN cd SHEEP/backend/build; export CC=gcc-7; export CXX=g++-7; cmake ../

RUN cd SHEEP/backend/build; make run-sheep-server

WORKDIR SHEEP/backend/build

EXPOSE 34568

CMD ["bin/run-sheep-server"]
