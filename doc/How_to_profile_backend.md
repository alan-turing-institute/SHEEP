# How to profile SHEEP backend

### 1. Build Dockerfile_profile image

```
cd SHEEP/backend
docker build -t backend_profile:latest -f Dockerfile_profile .
```

### 2. Run the docker_profile image in the interactive shell mode


```
docker run -it backend_profile /bin/bash
```


### 2.1. (in the interactive shell mode) Add profiling flags in the _/SHEEP/backend/CMakeLists.txt_ and _/SHEEP/backend/tests/CMakeLists.txt_ files

Profiling flags:
```
# Flags for profiling
set (CMAKE_C_FLAGS " -pg ")
set (CMAKE_CXX_FLAGS " -pg ")
```

_/SHEEP/backend/CMakeLists.txt_ should look similar to:
```
............
enable_testing()

include_directories(include)

# Flags for profiling
set (CMAKE_C_FLAGS " -pg ")
set (CMAKE_CXX_FLAGS " -pg ")

find_package(LP)
............
```

_/SHEEP/backend/tests/CMakeLists.txt_ should look similar to:
```
set(CTEST_OUTPUT_ON_FAILURE TRUE)

# Flags for profiling
set (CMAKE_C_FLAGS " -pg ")
set (CMAKE_CXX_FLAGS " -pg ")

# add_executable(test-invalid-circuit

............
```


### 2.2. (in the interactive shell mode) Recompile SHEEP base

```
rm -fr /SHEEP/backend/build; mkdir SHEEP/backend/build;

cd /SHEEP/backend/build; export CC=gcc-7; export CXX=g++-7; cmake ../; make  

```

### 2.3. (in the interactive shell mode) Recompile the libraries that we want to profile

#### HElib

- Add profiler flags to NTL

```
cd /ntl-11.1.0/src

./configure

# Add "-pg" to makefile
CXXFLAGS=-g -O2 -pg

make; make install

```

- Add profiler flags to GMP

```
cd /gmp-6.1.0
./configure
# Add following flag to the Makefile
CXXFLAGS = -pg
```

- Add profiler flags to CMakeList
```
cd /SHEEP/backend/lib/HElib/src

```

Add the "-pg" to the following line in the Makefile file:

```
CFLAGS = -g -O2 -std=c++11 -pthread -DFHE_THREADS -DFHE_BOOT_THREADS -fmax-errors=2 -pg
```
Then recompile the code

```
export CC=gcc-7; export CXX=g++-7; make clean; make;
```

#### TFHE

- In order to get function timings we need to recompile fftw library with the "-pg" flags

```
apt-get -y remove libfftw3-dev
wget http://fftw.org/fftw-3.3.8.tar.gz
tar -xvzf fftw-3.3.8.tar.gz
cd fftw-3.3.8

# Add the following to CMakeList.txt
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pg")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pg")

./configure; make; make install
```

- Adding "-pg" compiler flags to /SHEEP/backend/lib/tfhe/src/CMakeLists.txt
```
# -std=c99 seems to be required in Travis tests for whatever reason
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pg")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pg")

```

#### SEAL

Adding

```
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pg")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pg")
```

to /SEAL_3.0/SEAL/CMakeList.txt file in the following position

```

# Require thread library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Flags for profiling
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pg")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pg")


# Link Threads with seal
target_link_libraries(seal PUBLIC Threads::Threads)

```

#### LP

```
cd libpaillier-0.8
./configure

# Add additional -pg flag to CFLAGS
CFLAGS  = -O3 -Wall -pg  .....

make
make install 
```

### 2.4 (in the interactive shell mode) Visualise profiling results as a

- Execute the code - it will produce gmon.out file which is later used for stats generation.
For example:
```
./test-helib-f2-bool-c7
```

- Run gprof - gmon.out file from 2. step needs to passed as a parameter
For example:
```
gprof test-helib-f2-bool-c7 gmon.out > analysis.txt
```

- To visualise the profiling results as a dot graph [gprof2dot](https://github.com/jrfonseca/gprof2dot):
For example:
```
gprof test-helib-f2-bool-c7 gmon.out | gprof2dot -s | dot -Tsvg -o output.svg
```

### 3. Copy results from docker to host:

```
docker cp <docker-id>:<image path> <local path>
```
