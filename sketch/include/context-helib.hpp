#ifndef CONTEXT_HELIB_HPP
#define CONTEXT_HELIB_HPP

#include <unordered_map>
#include <chrono>

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

  ContextHElib(long num_levels, long security, long p=65537, long r=1, long c=3, long w=64, long d=0):
    m_L(num_levels),
    m_security(security),
    m_p(p),   // modulus of plaintext
    m_r(r),   // plaintext space 
    m_c(c),   // num columns in key-switching matrix
    m_w(w),  // Hamming weight of secret key
    m_d(d)   // unknown
  {
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
    
  
};

  // destructor
  ~ContextHElib() {
    /// delete everything we new-ed in the constructor
    if (m_ea != NULL) delete m_ea;

    if (m_secretKey != NULL) delete m_secretKey;

    if (m_helib_context != NULL) delete m_helib_context;
    
  };
  
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
    return pt[0];
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

  EncryptedArray* m_ea; 

  FHESecKey* m_secretKey;

  const FHEPubKey* m_publicKey;

  FHEcontext* m_helib_context;
  
};


  typedef ContextHElib<bool> ContextHElib_bool;
  typedef ContextHElib<uint8_t> ContextHElib_uint8_t;  
  
  
}  // leavin HElib namespace
}  // leaving Sheep namespace
  
#endif // CONTEXT_HELIB_HPP
