
#include "context-helib.hpp"

#include <unordered_map>
#include <chrono>

#include "circuit.hpp"


#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>

/// Constructor

using namespace Sheep::HElib;

template< typename PlaintextT>
ContextHElib<PlaintextT>::ContextHElib(long num_levels, long security, long p, long r, long c, long w, long d) :
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

  std::cout<<" Number of slots is "<<m_nslots<<std::endl;
  
  
}

template< typename PlaintextT>
typename ContextHElib<PlaintextT>::Ciphertext
ContextHElib<PlaintextT>::encrypt( PlaintextT pt) {
  //// if plaintext is a bool, convert it into a vector of longs, with just the first element as 1 or zero
  std::vector<long> ptvec;
  ptvec.push_back(pt);
  
  ////// fill up nslots with zeros//// 
  for (int i = ptvec.size(); i < m_nslots; i++) ptvec.push_back(0);
  
  ContextHElib<PlaintextT>::Ciphertext ct(*m_publicKey);
  m_ea->encrypt(ct, *m_publicKey, ptvec);
  return ct; 
}


template< typename PlaintextT>
PlaintextT
ContextHElib<PlaintextT>::decrypt(ContextHElib::Ciphertext ct) {
  std::vector<long> pt;
  m_ea->decrypt(ct, *m_secretKey, pt);
  return pt[0];
}

template< typename PlaintextT>
typename ContextHElib<PlaintextT>::Ciphertext
ContextHElib<PlaintextT>::Add(ContextHElib::Ciphertext a, ContextHElib::Ciphertext b) {
  std::cout<<"using HElib's ADD "<<std::endl;
  a += b;
  return a;   
}

template< typename PlaintextT>
typename ContextHElib<PlaintextT>::Ciphertext
ContextHElib<PlaintextT>::Subtract(ContextHElib::Ciphertext a, ContextHElib::Ciphertext b) {
  a -= b;  
  return a;   
}

template< typename PlaintextT>
typename ContextHElib<PlaintextT>::Ciphertext
ContextHElib<PlaintextT>::Multiply(ContextHElib::Ciphertext a, ContextHElib::Ciphertext b) {
  std::cout<<"using HElib's MULTIPLY "<<std::endl;
  a *= b;
  return a;   
}

void DummyFunction() {
  ContextHElib<uint8_t> tempObj(20,80);
}
