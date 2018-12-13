#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-helib.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_F2<bool> ctx;

  std::vector<std::vector<ContextHElib_F2<bool>::Plaintext>> pt_input = {
      {0, 0, 1, 1}, {1, 0, 1, 0}};

  std::vector<std::vector<ContextHElib_F2<bool>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input);

  std::vector<int8_t> exp_values = {0, 1, 1, 0};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " + "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    //	  assert(result.front()[i] == exp_values[i]);
  }

  /*
  /// test two 1s
  std::vector< std::vector<bool> > inputs = {{1,1,0,0}, {1,0,1,0}};
  std::vector< std::vector<bool> > result = ctx.eval_with_plaintexts(circ,
  inputs); std::cout<<"  1 + 1 =
  "<<std::to_string(result.front()[0])<<std::endl; assert(result.front()[0] ==
  0); std::cout<<"  1 + 0 = "<<std::to_string(result.front()[1])<<std::endl;
  assert(result.front()[1] == 1);

  /// test one of each
  inputs = {0, 1};
  result = ctx.eval_with_plaintexts(circ, inputs);
  assert(result.front() == 1);
  std::cout<<" 0 + 1 = "<<std::to_string(result.front())<<std::endl;
  /// test both zeros
  inputs = {0, 0};
  result = ctx.eval_with_plaintexts(circ, inputs);
  std::cout<<" 0 + 0 = "<<std::to_string(result.front())<<std::endl;
  assert(result.front() == 0);
  */
}
