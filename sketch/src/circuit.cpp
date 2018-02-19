#include "circuit.hpp"

std::ostream& operator<<(std::ostream& stream, const Circuit& c) {
	for (auto assignment : c.get_assignments()) {
		stream << assignment.get_output().get_name() << " <- Op";
		for (auto input : assignment.get_inputs()) {
			stream << " " << input.get_name();
		}
		stream << "\n";
	}
	return stream;
}
