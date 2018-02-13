#include "circuit.hpp"

std::ostream& operator<<(std::ostream& stream, const Circuit& c) {
	for (auto assignment : c.get_assignments()) {
		stream << "Assignment:  " << std::endl
		       << "   " << assignment.get_input1().get_name()
		       << " " << assignment.get_input2().get_name()
		       << "  --> " << assignment.get_output().get_name()
		       << std::endl;
	}
	return stream;
}
