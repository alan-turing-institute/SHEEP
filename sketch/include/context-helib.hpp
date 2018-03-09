#ifndef CONTEXT_HELIB_HPP
#define CONTEXT_HELIB_HPP

#include <unordered_map>
#include <chrono>
#include <type_traits>
#include "bits.hpp"
#include <NTL/BasicThreadPool.h>
NTL_CLIENT


#include "circuit.hpp"
#include "context.hpp"

#include "FHE.h"
#include "EncryptedArray.h"

#include "intraSlot.h"
#include "binaryArith.h"
#include "binaryCompare.h"


namespace Sheep {
namespace HElib {

template <typename PlaintextT>
class ContextHElib : public Context< PlaintextT , NTL::Vec<Ctxt> > {

public:

  typedef PlaintextT Plaintext;
  ///  typedef Ctxt Ciphertext;
  typedef NTL::Vec<Ctxt> Ciphertext;  

  /// constructors

  ContextHElib(long num_levels=20, long security=80, long p=2, long r=1, long c=3, long w=64, long d=0):
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
      m_bitwidth = 1;
    else
      m_bitwidth = BITWIDTH(Plaintext);
  
    /// start to build
    
    long m = FindM(m_security, m_L, m_c, m_p, m_d, 0, 0);
    
    /// initialize HElib context
    m_helib_context = new FHEcontext(m, m_p, m_r);
    /// modify context, add primes to modulus chain
    buildModChain(*m_helib_context, m_L, m_c);


    // unpack slot encoding
    buildUnpackSlotEncoding(m_unpackSlotEncoding, *(m_helib_context->ea));

    
    /// create secret key structure
    m_secretKey = new FHESecKey(*m_helib_context);
    
    m_publicKey = m_secretKey;  //// points to the same place
  
    ZZX G = m_helib_context->alMod.getFactorsOverZZ()[0];
    
    /// generate a secret key
    m_secretKey->GenSecKey(m_w);
    
    addSome1DMatrices(*m_secretKey);
    addFrbMatrices(*m_secretKey);

    
    m_ea = new EncryptedArray(*m_helib_context, G);
    
    m_nslots = m_ea->size();

    ////  populate the map that will allow us to set parameters via an input file (or string)
    
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
    Ctxt mu(*m_publicKey);  /// use this to fill up the vector when resizing
    Ciphertext  ct;   /// now an NTL::Vec<Ctxt>
    resize(ct,m_bitwidth, mu);
    for (int i=0; i < m_bitwidth; i++) {
      m_publicKey->Encrypt(ct[i], ZZX((pt >>i)&1));
    }
    return ct;
    
    /*
  //// if plaintext is a bool, convert it into a vector of longs, with just the first element as 1 or zero
  std::vector<long> ptvec;
  ptvec.push_back(pt);
  
  ////// fill up nslots with zeros//// 
  for (int i = ptvec.size(); i < m_nslots; i++) ptvec.push_back(0);
  
  Ciphertext ct(*m_publicKey);
  m_ea->encrypt(ct, *m_publicKey, ptvec);
  return ct;
    */ 
  };

  Plaintext decrypt(Ciphertext ct) {
    std::vector<long> pt;

    decryptBinaryNums(pt, CtPtrs_VecCt(ct), *m_secretKey, *m_ea);

    
    ///    m_ea->decrypt(ct[0], *m_secretKey, pt);

    long pt_transformed = pt[0];
    //  if ((pt[0]) > m_p / 2)    //// convention - treat this as a negative number
    //  pt_transformed = pt[0] - m_p;
    return pt_transformed  % int(pow(2,m_bitwidth));
  };


  Ciphertext Compare(Ciphertext a, Ciphertext b) {
    std::cout<<" using HElib's COMPARE "<<std::endl;
    Ctxt mu(*m_publicKey);
    Ctxt ni(*m_publicKey);    
    Ciphertext cmax, cmin;
    CtPtrs_VecCt wMin(cmin), wMax(cmax);  /// wrappers around output vectors
    compareTwoNumbers(wMax, wMin, mu, ni,
		      CtPtrs_VecCt(a), CtPtrs_VecCt(b),
		      &m_unpackSlotEncoding);
    /// mu is now a Ctxt which is the encryption of 1 if a>b and 0 otherwise.
    /// but we need to put it into NTL::Vec<Ctxt> as that is our new "Ciphertext" type.
    Ciphertext output;
    output.append(mu);

    return output;
    
  }
  
  
  
  Ciphertext Add(Ciphertext a, Ciphertext b) {
    std::cout<<"using HElib's ADD "<<std::endl;

    Ciphertext sum;
    CtPtrs_VecCt wsum(sum);
    addTwoNumbers(wsum,CtPtrs_VecCt(a),CtPtrs_VecCt(b),
		  m_bitwidth,
		  &m_unpackSlotEncoding);
    return sum;

  };

  
  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
    std::cout<<"using HElib's MULTIPLY "<<std::endl;

   Ciphertext product;
   CtPtrs_VecCt wprod(product);
    multTwoNumbers(wprod,CtPtrs_VecCt(a),CtPtrs_VecCt(b),
		   false,
		   m_bitwidth,
		   &m_unpackSlotEncoding);
    return product;
    
  };

  /*
  Ciphertext Subtract(Ciphertext a, Ciphertext b) {
    a -= b;  
    return a;   
  };

  Ciphertext Negate(Ciphertext a) {
    if (m_bitwidth == 1)  /// special case for binary
      a.addConstant(to_ZZX(1L));
    else
      a.multByConstant(to_ZZX(-1L));  
    return a;   
  };

  Ciphertext MultByConstant(Ciphertext a, long b) {
    a.multByConstant(to_ZZX(b));
    return a;
  }

  Ciphertext AddConstant(Ciphertext a, long b) {
    a.addConstant(to_ZZX(b));
    return a;
  }

  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
    /// s is 0 or 1
    /// output is s*a + (1-s)*b
    Ciphertext sa = Multiply(s,a);
    Ciphertext one_minus_s = MultByConstant( AddConstant(s,-1L), -1L);
    Ciphertext one_minus_s_times_b = Multiply(one_minus_s, b);
    return Add(sa, one_minus_s_times_b);
  }
  */
  
  long get_num_slots() {
    return m_nslots;
  }

  
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

  std::vector<zzX> m_unpackSlotEncoding;

  
};


  typedef ContextHElib<bool> ContextHElib_bool;
  typedef ContextHElib<uint8_t> ContextHElib_uint8_t;  
  
  
}  // leavin HElib namespace
}  // leaving Sheep namespace
  
#endif // CONTEXT_HELIB_HPP
