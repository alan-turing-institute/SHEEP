
#include "context-helib.hpp"

#include <unordered_map>
#include <chrono>

#include "circuit.hpp"


#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>

/// Constructor

ContextHElib::ContextHElib(long num_levels, long security) {
  m_L = num_levels;
  m_security = security;
  m_r = 1;   // plaintext space 
  m_p = 2;   // modulus of plaintext
  m_c = 3;   // num columns in key-switching matrix
  m_w = 64;  // Hamming weight of secret key
  m_d = 0;   // unknown

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


ContextHElib::Ciphertext
ContextHElib::encrypt(ContextHElib::Plaintext pt) {
  //// if plaintext is a bool, convert it into a vector of longs, with just the first element as 1 or zero
  std::vector<long> ptvec;
  ptvec.push_back(int(pt));
  
  ////// fill up nslots with zeros//// 
  for (int i = ptvec.size(); i < m_nslots; i++) ptvec.push_back(0);
  
  ContextHElib::Ciphertext ct(*m_publicKey);
  m_ea->encrypt(ct, *m_publicKey, ptvec);
  return ct; 
}




ContextHElib::Plaintext
ContextHElib::decrypt(ContextHElib::Ciphertext ct) {
  std::vector<long> pt;
  m_ea->decrypt(ct, *m_secretKey, pt);
  return bool(pt[0]);
}

ContextHElib::Ciphertext
ContextHElib::And(ContextHElib::Ciphertext a, ContextHElib::Ciphertext b) {
  std::cout<<"using HElib's AND "<<std::endl;
  a *= b;
  return a;   //// as long as p=2
}

ContextHElib::Ciphertext
ContextHElib::Or(ContextHElib::Ciphertext a, ContextHElib::Ciphertext b) {
  /// Do XOR( XOR(a,b), AND(a,b) )
  ContextHElib::Ciphertext a_copy = a;

  a += b;  /// XOR(a,b)
  a_copy *= b;  /// AND(a,b)

  a += a_copy;  /// XOR (XOR(a,b), AND(a,b))
  
  return a;   
}

ContextHElib::Ciphertext
ContextHElib::Xor(ContextHElib::Ciphertext a, ContextHElib::Ciphertext b) {
  std::cout<<"using HElib's XOR "<<std::endl;
  a += b;
  return a;   //// as long as p=2
}
	
ContextHElib::GateFn
ContextHElib::get_op(Gate g) {
  using namespace std::placeholders;
		switch(g) {
		case(Gate::And):
			return GateFn(std::bind(&ContextHElib::And, this, _1, _2));
			break;

		case(Gate::Or):
			return GateFn(std::bind(&ContextHElib::Or, this, _1, _2));
			break;

		case(Gate::Xor):
			return GateFn(std::bind(&ContextHElib::Xor, this, _1, _2));
			break;

		}
		throw std::runtime_error("Unknown op");
}

