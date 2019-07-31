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

  ContextHElib_F2<bool> sheep_helib_f2_context;

  std::vector<Plaintext> pt_orig = {1, 0, 1, 0};
  Ciphertext temp = sheep_helib_f2_context.encrypt(pt_orig);
  std::vector<Plaintext> pt_new = sheep_helib_f2_context.decrypt(temp);

  t2 = high_resolution_clock::now();

  duration = duration_cast<microseconds>(t2 - t1).count();
  std::cout << "Sheep: " << duration << " ms" << std::endl;

  // getting parameters which were used in sheep
  std::map<std::string, long> helib_f2_params_map = sheep_helib_f2_context.get_parameters();

  // LIB

  unsigned long helib_f2_bitwidth;
  if (std::is_same<Plaintext, bool>::value)
    helib_f2_bitwidth = 1;
  else
    helib_f2_bitwidth = BITWIDTH(Plaintext);
    
  unsigned long helib_f2_p = 2;
  unsigned long helib_f2_r = 1;

  NTL::Vec<long> helib_f2_mvec;

  append(helib_f2_mvec, helib_f2_params_map["m1"]);
  if (helib_f2_params_map["m2"] > 1) append(helib_f2_mvec, helib_f2_params_map["m2"]);
  if (helib_f2_params_map["m3"] > 1) append(helib_f2_mvec, helib_f2_params_map["m3"]);

  std::vector<long> helib_f2_gens;
  helib_f2_gens.push_back(helib_f2_params_map["g1"]);
  if (helib_f2_params_map["g2"] > 1) helib_f2_gens.push_back(helib_f2_params_map["g2"]);
  if (helib_f2_params_map["g3"] > 1) helib_f2_gens.push_back(helib_f2_params_map["g3"]);

  std::vector<long> helib_f2_ords;
  helib_f2_ords.push_back(helib_f2_params_map["ord1"]);
  if (abs(helib_f2_params_map["ord2"]) > 1) helib_f2_ords.push_back(helib_f2_params_map["ord2"]);
  if (abs(helib_f2_params_map["ord3"]) > 1) helib_f2_ords.push_back(helib_f2_params_map["ord3"]);

  unsigned long helib_f2_L;
  if (!helib_f2_params_map["Levels"]) {
    if (helib_f2_params_map["Bootstrap"])
      helib_f2_L = 30;
    else
      helib_f2_L = 3 + NTL::NumBits(helib_f2_bitwidth + 2);
  } else 
      helib_f2_L = helib_f2_params_map["Levels"];
  

  unsigned long helib_f2_B = helib_f2_params_map["BitsPerLevel"];
  unsigned long helib_f2_c = helib_f2_params_map["c"];
  unsigned long helib_f2_m = helib_f2_params_map["m"];
  unsigned long helib_f2_w = helib_f2_params_map["HammingWeight"];
  
  t1 = high_resolution_clock::now();

  FHEcontext *helib_f2_context = new FHEcontext(helib_f2_m, helib_f2_p, helib_f2_r, helib_f2_gens, helib_f2_ords);
  helib_f2_context->bitsPerLevel = helib_f2_B;
  buildModChain(*helib_f2_context, helib_f2_L, helib_f2_c, 8);
  
  if (helib_f2_params_map["Bootstrap"]) {
      helib_f2_context->makeBootstrappable(helib_f2_mvec, /*t=*/0,
                                          /*flag=*/false, /*cacheType=DCRT*/ 2);
  }
  
  std::vector<zzX> m_unpackSlotEncoding;

  buildUnpackSlotEncoding(m_unpackSlotEncoding, *(helib_f2_context->ea));
  
  FHESecKey* helib_f2_secretKey = new FHESecKey(*helib_f2_context);

  const FHEPubKey& helib_f2_publicKey = *helib_f2_secretKey; 

  helib_f2_secretKey->GenSecKey(helib_f2_w);

  addSome1DMatrices(*helib_f2_secretKey);
  addFrbMatrices(*helib_f2_secretKey);

  EncryptedArray* helib_f2_ea = new EncryptedArray(*helib_f2_context);

  long helib_f2_nslots = helib_f2_ea->size();

  NTL::Vec<Ctxt> edata;
  resize(edata, helib_f2_bitwidth, Ctxt(*helib_f2_secretKey));
  for (long i=0; i<=helib_f2_bitwidth; i++) {
  }


//   Ctxt ctxt(helib_f2_publicKey);
  
//   vector<ZZX> sliced_pt = vector<ZZX>(helib_f2_nslots);
  
//   for (int j = 0; j < helib_f2_bitwidth; j++) {
//     for (int i = 0; i < pt_orig.size(); i++) {
//         sliced_pt[i] = ZZX((pt_orig[i] >> j) & 1);
//     }
//   }
  
//   helib_f2_ea->encrypt(ctxt, helib_f2_publicKey, sliced_pt);
  
//   std::vector<long> ct_decrypt = std::vector<long>(helib_f2_nslots);
//   std::vector<Plaintext> decrypted(helib_f2_nslots);
  
//   decryptBinaryNums(ct_decrypt, CtPtrs_VecCt(ctxt), *helib_f2_secretKey, *helib_f2_ea);

//   for (int i = 0; i < helib_f2_nslots; i++) {
//     decrypted[i] = ct_decrypt[i] % int(pow(2, helib_f2_bitwidth));
//   }

//   t2 = high_resolution_clock::now();
//   duration = duration_cast<microseconds>(t2 - t1).count();
//   std::cout << "Lib: " << duration << " ms" << std::endl;
  
//   std::cout << "Decrypted Ptxt: " << decrypted << std::endl;
}

int main(void) {
  //// instantiate the Circuit Repository
  /// CircuitRepo cr;

    time_helib_f2();

}
