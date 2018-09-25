#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
    using namespace SHEEP;

    //// instantiate the Circuit Repository
    CircuitRepo cr;

    // circ contains an Add gate with two inputs and one output
    Circuit circ = cr.create_circuit(Gate::Add, 1);
    std::cout << circ;
    std::vector<DurationT> durations;

    // The type of the wires in circ are unsigned 8-bit numbers
    ContextClear<uint8_t> ctx;

    // test small postitive numbers
    // All inputs must be of teh same length
    std::vector<std::vector<uint8_t>> inputs = {{41}, {40}};
    std::vector<std::vector<uint8_t>> result = ctx.eval_with_plaintexts(circ, inputs, durations);

    // Output is a vector of vectors of length the length of the inputs
    std::cout << " {41}, {40} = " << std::to_string(result[0][0]) << std::endl;
    assert(result.front()[0] == 81);

    /// test result going out of range positive
    // inputs = {{200}, {127}};
    // result = ctx.eval_with_plaintexts(circ, inputs, durations);
    // std::cout<<" {200}, {127} = "<<std::to_string(result.front())<<std::endl;
    // assert(result.front() == 71);

}
