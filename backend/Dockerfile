FROM turingsheep/sheep_base:v3 as sheep

ENV LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:/PALISADE/bin/lib:/PALISADE/third-party/lib"
ENV LIBRARY_PATH "${LIBRARY_PATH}:/PALISADE/bin/lib"

ENV CMAKE_LIBRARY_PATH "/usr/local/lib:/PALISADE/bin/lib/:/PALISADE/third-party/lib"
ENV CMAKE_INCLUDE_PATH "/PALISADE/src/pke/lib/:/PALISADE/src/:/PALISADE/src/core/lib/:/PALISADE/third-party/include/"

ENV CPLUS_INCLUDE_PATH "/PALISADE/src/pke/lib/:/PALISADE/src/:/PALISADE/src/core/lib/:/PALISADE/third-party/include/"


ADD backend/applications SHEEP/backend/applications
ADD backend/cmake SHEEP/backend/cmake
ADD backend/CMakeLists.txt SHEEP/backend/CMakeLists.txt
ADD backend/include SHEEP/backend/include
ADD backend/src SHEEP/backend/src
ADD backend/tests SHEEP/backend/tests

RUN rm -fr SHEEP/backend/build; mkdir SHEEP/backend/build

RUN cd SHEEP/backend/build; export CC=gcc-7; export CXX=g++-7; cmake ../

RUN cd SHEEP/backend/build; make VERBOSE=1 run-sheep-server


WORKDIR SHEEP/backend/build

EXPOSE 34568

CMD ["bin/run-sheep-server"]
