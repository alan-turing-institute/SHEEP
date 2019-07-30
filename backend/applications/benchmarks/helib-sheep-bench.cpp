#include <memory>
#include <chrono>

#include "context-helib.hpp"
#include "circuit-repo.hpp"

#include <unistd.h>

#include <FHE.h>
#include <EncryptedArray.h>

using namespace std;
using namespace std::chrono;
using namespace SHEEP;

typedef ContextHElib_F2<bool>::Plaintext Plaintext;
typedef ContextHElib_F2<bool>::Ciphertext Ciphertext;

void time_helib_f2() {
  high_resolution_clock::time_point t1, t2;
  int duration;

  // Timing encrypt

  // SHEEP

  t1 = high_resolution_clock::now();

  ContextHElib_F2<bool> sheep_helib_f2_context = new ContextHElib_F2<bool>();
  std::vector<Plaintext> pt_orig = {1, 0, 1, 0};
  Ciphertext temp = sheep_helib_f2_context.encrypt(pt_orig);

  t2 = high_resolution_clock::now();

  std::vector<Plaintext> pt_new = sheep_helib_f2_context.decrypt(temp);

  duration = duration_cast<microseconds>(t2 - t1).count();
  std::cout << "Sheep: " << duration << " ms" << std::endl;

  // getting parameters which were used in sheep
  std::map<std::string, long> helib_f2_params_map = sheep_helib_f2_context.get_parameters();



  // // LIB

  // t1 = high_resolution_clock::now();

  // unsigned long helib_f2_p = 2;
  // unsigned long helib_f2_r = 1;

  // NTL::Vec<long> helib_f2_mvec;
  // append(helib_f2_mvec, helib_f2_params_map["m1"]);
  // if (helib_f2_params_map["m2"] > 1) append(helib_f2_mvec, helib_f2_params_map["m2"]);
  // if (helib_f2_params_map["m3"] > 1) append(helib_f2_mvec, helib_f2_params_map["m3"]);

  // std::vector<long> helib_f2_gens;
  // helib_f2_gens.push_back(helib_f2_params_map["g1"]);
  // if (helib_f2_params_map["g2"] > 1) helib_f2_gens.push_back(helib_f2_params_map["g2"]);
  // if (helib_f2_params_map["g3"] > 1) helib_f2_gens.push_back(helib_f2_params_map["g3"]);

  // std::vector<long> helib_f2_ords;
  // helib_f2_ords.push_back(helib_f2_params_map["ord1"]);
  // if (abs(helib_f2_params_map["ord2"]) > 1) helib_f2_ords.push_back(helib_f2_params_map["ord2"]);
  // if (abs(helib_f2_params_map["ord3"]) > 1) helib_f2_ords.push_back(helib_f2_params_map["ord3"]);

  // /// number of levels  (copied from HElib's Test_binaryCompare)
  // if (!this->override_param("Levels")) {
  //   if (m_bootstrap)
  //     m_L = 30;
  //   else
  //     m_L = 3 + NTL::NumBits(m_bitwidth + 2);
  // }

  // FHEcontext helib_f2_context(helib_f2_params_map["m"], helib_f2_p, helib_f2_r, helib_f2_gens, helib_f2_ords);

  // helib_f2_context->bitsPerLevel = m_B;

  // t2 = high_resolution_clock::now();

  // duration = duration_cast<microseconds>(t2 - t1).count();

  // std::cout << "Lib: " << duration << " ms" << std::endl;



  // std::cout << sheep_helib_f2_context.get_parameters() << std::endl;
  // unsigned long p =
  // FHEcontext context(m, p, r);

}

int main(void) {
  //// instantiate the Circuit Repository
  /// CircuitRepo cr;

    time_helib_f2();

}
