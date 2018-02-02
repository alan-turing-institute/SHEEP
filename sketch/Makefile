circuit-example: circuit-example.cpp circuit.hpp
	g++ -std=c++14 -Wall -Wextra -Wconversion $< -o $@

clear-context-example: clear-context-example.cpp context-clear.hpp context.hpp circuit.hpp circuit-repo.hpp
	g++ -std=c++14 -Wall -Wextra -Wconversion $< -o $@


##### compile against HElib stuff - assume NTL installed in /usr/local/lib and HElib in user's home area.

HELIB_HOME=${HOME}/HElib/src
GMP=-lgmp
NTL=-lntl
LDLIBS = -L/usr/local/lib $(NTL) $(GMP) -lm

helib-context-example: helib-context-example.cpp context-helib.o context.hpp circuit.hpp circuit-repo.hpp

	g++ -std=c++14 -Wall -Wextra context-helib.o -I$(HELIB_HOME) $(LDLIBS) $< $(HELIB_HOME)/fhe.a -o $@

context-helib.o: context-helib.cpp context-helib.hpp
	g++ -std=c++14 -Wall -Wextra -I$(HELIB_HOME) -c context-helib.cpp
