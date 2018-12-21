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
  std::vector<DurationT> totalDurations;
  std::map<std::string, DurationT> gateDurations;
  std::pair<std::vector<DurationT>, std::map<std::string, DurationT> > durations = std::make_pair(totalDurations,gateDurations);
  ContextHElib_Fp<int8_t> ctx;

  ctx.set_parameter("BaseParamSet",0);
  ctx.configure();
  std::cout<<" num slots "<<ctx.get_num_slots()<<std::endl;
  std::vector<std::vector<ContextHElib_Fp<int8_t>::Plaintext>> pt_input = {
    {15, 10, 100, -80}, {22, -12, 100, -80}};

  std::vector<std::vector<ContextHElib_Fp<int8_t>::Plaintext>> result =
    ctx.eval_with_plaintexts(circ, pt_input, durations);

  std::cout<<" encryption time "<<std::to_string(durations.first[0].count())<<std::endl;
  std::cout<<" decryption time "<<std::to_string(durations.first[2].count())<<std::endl;
  std::cout<<" evaluation time "<<std::to_string(durations.first[1].count())<<std::endl;
  for (auto gate_timing : durations.second) {
    std::cout<<" timing for "<<gate_timing.first<<" is "<<std::to_string(gate_timing.second.count())<<std::endl;
  }

  std::vector<int8_t> exp_values = {37, -2, -56, 96};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " + "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
       assert(result.front()[i] == exp_values[i]);
  }
  std::cout<<" length of output is "<<result[0].size()<<std::endl;
  for (int i=0; i< result[0].size(); i++) {
    std::cout<<" "<<std::to_string(result[0][i]);
  }
  std::cout<<std::endl;
}
