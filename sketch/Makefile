circuit-example: circuit-example.cpp circuit.hpp
	g++ -std=c++14 -Wall -Wextra -Wconversion $< -o $@

context-example: context-example.cpp context-clear.hpp context.hpp circuit.hpp
	g++ -std=c++14 -Wall -Wextra -Wconversion $< -o $@


##### compile against HElib stuff - assume NTL installed in /usr/local/lib and HElib in user's home area.

HELIB_HOME=${HOME}/HElib/src
GMP=-lgmp
NTL=-lntl
LDLIBS = -L/usr/local/lib $(NTL) $(GMP) -lm

helib-example: helib-example.cpp context-helib.o context.hpp circuit.hpp

	g++ -std=c++14 -Wall -Wextra context-helib.o -I$(HELIB_HOME) $(LDLIBS) $< $(HELIB_HOME)/fhe.a -o $@

context-helib.o: context-helib.cpp context-helib.hpp
	g++ -std=c++14 -Wall -Wextra -I$(HELIB_HOME) -c context-helib.cpp


TFHE_INCLUDE=${HOME}/projects/sheep/code/tfhe-include/
TFHE_LIB=${HOME}/projects/sheep/code/tfhe/build/libtfhe/

tfhe-example: tfhe-example.cpp context.hpp circuit.hpp
	g++ -std=c++14 -Wall -Wextra -isystem $(TFHE_INCLUDE) -L$(TFHE_LIB) $< -o $@ -ltfhe-fftw 
