#ifndef CONTEXT_HELIB_HPP
#define CONTEXT_HELIB_HPP

#include <unordered_map>
#include <chrono>
#include <type_traits>
#include "bits.hpp"

#include "circuit.hpp"
#include "context.hpp"

#include "FHE.h"
#include "EncryptedArray.h"

namespace Sheep {
namespace HElib {

template <typename PlaintextT>
class ContextHElib : public Context< PlaintextT , Ctxt> {

public:

  typedef PlaintextT Plaintext;
  typedef Ctxt Ciphertext;

  /// constructors

  ContextHElib(long num_levels=20, long security=80, long p=65537, long r=1, long c=3, long w=64, long d=0):
    m_L(num_levels),
    m_security(security),
    m_p(p),   // modulus of plaintext
    m_r(r),   // plaintext space 
    m_c(c),   // num columns in key-switching matrix
    m_w(w),  // Hamming weight of secret key
    m_d(d)   // unknown
  {

    /// BITWIDTH(bool) is 8, so need to deal with this by hand...
    //// (better to specialize class?)
    
    if (std::is_same<Plaintext, bool>::value)
      m_bitwidth = 2;
    else
      m_bitwidth = BITWIDTH(bool);
  
    /// start to build
    
    long m = FindM(m_security, m_L, m_c, m_p, m_d, 0, 0);
    
    /// initialize HElib context
    m_helib_context = new FHEcontext(m, m_p, m_r);
    /// modify context, add primes to modulus chain
    buildModChain(*m_helib_context, m_L, m_c);
    /// create secret key structure
    m_secretKey = new FHESecKey(*m_helib_context);
    
    m_publicKey = m_secretKey;  //// points to the same place
  
    ZZX G = m_helib_context->alMod.getFactorsOverZZ()[0];
    
    /// generate a secret key
    m_secretKey->GenSecKey(m_w);
    
    addSome1DMatrices(*m_secretKey);
    
    m_ea = new EncryptedArray(*m_helib_context, G);
    
    m_nslots = m_ea->size();

    m_param_name_map.insert({"security", m_security});
    m_param_name_map.insert({"L", m_L});
    m_param_name_map.insert({"p", m_p});    
    m_param_name_map.insert({"r", m_r});
    m_param_name_map.insert({"c", m_c});
    m_param_name_map.insert({"w", m_w});
    m_param_name_map.insert({"d", m_d});            
  
};

  // destructor
  ~ContextHElib() {
    /// delete everything we new-ed in the constructor
    if (m_ea != NULL) delete m_ea;

    if (m_secretKey != NULL) delete m_secretKey;

    if (m_helib_context != NULL) delete m_helib_context;
    
  };

  void read_params_from_file(std::string filename) {
    std::ifstream inputstream(filename);

    if (inputstream.bad()) {
      std::cout<<"Empty or non-existent input file"<<std::endl;
    }
    
    /// loop over all lines in the input file 
    std::string line;
    while (std::getline(inputstream, line) ) {
      /// remove comments (lines starting with #) and empty lines
      int found= line.find_first_not_of(" \t");
      if( found != std::string::npos) {   
	if ( line[found] == '#') 
	  continue;
	
	/// split up by whitespace
	std::string buffer;
	std::vector<std::string> tokens;
	std::stringstream ss(line);
	while (ss >> buffer) tokens.push_back(buffer);
	
	if (tokens.size() == 2) {   /// assume we have param_name param_value
	  set_parameter(tokens[0],stol(tokens[1])); 
	  
	}
	
      }    
    } // end of loop over lines
  }

  

  void set_parameter(std::string param_name, long param_value) {
    auto map_iter = m_param_name_map.find(param_name);
    if ( map_iter == m_param_name_map.end() ) {
      std::cout<<"Parameter "<<param_name<<" not found."<<std::endl;
      return;
    } else {
      std::cout<<"Setting parameter "<<map_iter->first<<" to "<<param_value<<std::endl;
      map_iter->second = param_value;
      return;
    }
  }

  void print_parameters() {
    for ( auto map_iter = m_param_name_map.begin(); map_iter != m_param_name_map.end(); ++map_iter) {
      std::cout<<"Parameter "<<map_iter->first<<" = "<<map_iter->second<<std::endl;
    }
  }
  
  Ciphertext encrypt(Plaintext pt) {
  //// if plaintext is a bool, convert it into a vector of longs, with just the first element as 1 or zero
  std::vector<long> ptvec;
  ptvec.push_back(pt);
  
  ////// fill up nslots with zeros//// 
  for (int i = ptvec.size(); i < m_nslots; i++) ptvec.push_back(0);
  
  Ciphertext ct(*m_publicKey);
  m_ea->encrypt(ct, *m_publicKey, ptvec);
  return ct; 
  };

  Plaintext decrypt(Ciphertext ct) {
    std::vector<long> pt;
    m_ea->decrypt(ct, *m_secretKey, pt);
    return pt[0] % m_bitwidth;
  };
	
  Ciphertext Add(Ciphertext a, Ciphertext b) {
    std::cout<<"using HElib's ADD "<<std::endl;
    a += b;
    return a;   
  };

  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
    std::cout<<"using HElib's MULTIPLY "<<std::endl;
    a *= b;
    return a;   
  };

  Ciphertext Subtract(Ciphertext a, Ciphertext b) {
    a -= b;  
    return a;   
  };
	

  
private:

  long m_p;     //  modulus of plaintext

  long m_d;     // this is likely to be zero

  long m_L;     // maximum number of homomorphic levels

  long m_r;     // defines plaintext space as A_{p^r} 

  long m_security;  // chosen security level for the encryption

  long m_w;     // Hamming weight of secret key

  long m_c;     // number of columns in key-switching matrix

  long m_nslots;  // number of SIMD operations that can be done at a time

  std::map<std::string, long& > m_param_name_map;
  
  EncryptedArray* m_ea; 

  FHESecKey* m_secretKey;

  const FHEPubKey* m_publicKey;

  FHEcontext* m_helib_context;

  int m_bitwidth;
};


  typedef ContextHElib<bool> ContextHElib_bool;
  typedef ContextHElib<uint8_t> ContextHElib_uint8_t;  
  
  
}  // leavin HElib namespace
}  // leaving Sheep namespace
  
#endif // CONTEXT_HELIB_HPP
