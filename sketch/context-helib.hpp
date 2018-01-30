#ifndef CONTEXT_HELIB_HPP
#define CONTEXT_HELIB_HPP

#include <unordered_map>
#include <chrono>

#include "circuit.hpp"
#include "context.hpp"

#include "FHE.h"
#include "EncryptedArray.h"


class ContextHElib : public Context< bool , Ctxt> {
  ////class ContextHElib : public Context<std::vector<long> , Ctxt> {  

public:
  /// constructor
  ContextHElib(long num_levels, long security);

  // destructor
  ~ContextHElib() {
    /// delete everything we new-ed in the constructor
    if (m_ea != NULL) delete m_ea;

    if (m_secretKey != NULL) delete m_secretKey;

    if (m_helib_context != NULL) delete m_helib_context;
    
  };
  
  Ciphertext encrypt(Plaintext p);

  Plaintext decrypt(Ciphertext c);
	
  Ciphertext And(Ciphertext a, Ciphertext b);

  Ciphertext Or(Ciphertext a, Ciphertext b);

  Ciphertext Xor(Ciphertext a, Ciphertext b);
	
  GateFn get_op(Gate g);



  
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

#endif // CONTEXT_HELIB_HPP
