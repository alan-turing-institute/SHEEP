#ifndef CONTEXT_HELIB_F2_HPP
#define CONTEXT_HELIB_F2_HPP

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


namespace SHEEP {

  ////////////////////////////////////////////////////////////////////////////////////
  ///  ContextHElib_F2 -  use p=2, do everything with arrays of Ciphertext,
  ///  and binary operations for add, multiply, compare etc.
  
template<typename PlaintextT>
class ContextHElib_F2 : public Context< PlaintextT, NTL::Vec<Ctxt> > {

public:

  typedef PlaintextT Plaintext;
  typedef std::vector< Plaintext > PlaintextVec;
  typedef NTL::Vec<Ctxt> Ciphertext;  
  
  ContextHElib_F2(long param_set=1,   // parameter set, from 0 (tiny) to 4 (huge)
		  bool bootstrap=true, // bootstrap or not?
		  long haming_weight=128): // Haming weight of secret key
    
    m_p(2),
    m_param_set(param_set),
    m_bootstrap(bootstrap),
    m_w(haming_weight)
  {

    /// BITWIDTH(bool) is 8, so need to deal with this by hand...
    //// (better to specialize class?)
    if (std::is_same<Plaintext, bool>::value)
      m_bitwidth = 1;
    else
      m_bitwidth = BITWIDTH(Plaintext);

    /// Set all the other parameters.
    
    long mValues[][15] = { 
      // { p, phi(m),   m,   d, m1, m2, m3,    g1,   g2,   g3, ord1,ord2,ord3, B,c}
      {  2,    48,   105, 12,  3, 35,  0,    71,    76,    0,   2,  2,   0, 25, 2},
      {  2 ,  600,  1023, 10, 11, 93,  0,   838,   584,    0,  10,  6,   0, 25, 2},
      {  2,  2304,  4641, 24,  7,  3,221,  3979,  3095, 3760,   6,  2,  -8, 25, 3},
      {  2, 15004, 15709, 22, 23,683,  0,  4099, 13663,    0,  22, 31,   0, 25, 3},
      {  2, 27000, 32767, 15, 31,  7, 151, 11628, 28087,25824, 30,  6, -10, 28, 4}
    };

    std::cout<<" in ContextHElib constructor p0"<<std::endl;
    
    long* vals = mValues[m_param_set];
    
    long m = vals[2];
    
    NTL::Vec<long> mvec;
    append(mvec, vals[4]);
    if (vals[5]>1) append(mvec, vals[5]);
    if (vals[6]>1) append(mvec, vals[6]);

    std::vector<long> gens;
    gens.push_back(vals[7]);
    if (vals[8]>1) gens.push_back(vals[8]);
    if (vals[9]>1) gens.push_back(vals[9]);

    std::vector<long> ords;
    ords.push_back(vals[10]);
    if (abs(vals[11])>1) ords.push_back(vals[11]);
    if (abs(vals[12])>1) ords.push_back(vals[12]);
    
    m_B = vals[13];
    m_c = vals[14];

    /// number of levels
    std::cout<<" in ContextHElib constructor p1"<<std::endl;
    
    if (m_bootstrap) m_L = 30; // that should be enough
    else m_L = 3 + NTL::NumBits(m_bitwidth+2);

    std::cout<<" in ContextHElib constructor p2"<<std::endl;    
    /// initialize HElib context
    m_helib_context = new FHEcontext(m, m_p, 1, gens, ords);
    std::cout<<" in ContextHElib constructor p3"<<std::endl;
    m_helib_context->bitsPerLevel = m_B;
    std::cout<<" in ContextHElib constructor p4"<<std::endl;    
    /// modify context, add primes to modulus chain
    buildModChain(*m_helib_context, m_L, m_c,8);
    std::cout<<" in ContextHElib constructor p5"<<std::endl;
    if (m_bootstrap) {
      m_helib_context->makeBootstrappable(mvec, /*t=*/0,
					  /*flag=*/false, /*cacheType=DCRT*/2);
    }
    std::cout<<" in ContextHElib constructor p6"<<std::endl;
    // unpack slot encoding
    buildUnpackSlotEncoding(m_unpackSlotEncoding, *(m_helib_context->ea));

    std::cout<<" in ContextHElib constructor p7"<<std::endl;    
    /// create secret key structure
    m_secretKey = new FHESecKey(*m_helib_context);
    
    m_publicKey = m_secretKey;  //// points to the same place

    std::cout<<" in ContextHElib constructor p8"<<std::endl;    
    /// generate a secret key
    m_secretKey->GenSecKey(m_w);   /// Haming weight of 128
    std::cout<<" in ContextHElib constructor p9"<<std::endl;
    
    addSome1DMatrices(*m_secretKey);
    addFrbMatrices(*m_secretKey);

    if (m_bootstrap) m_secretKey->genRecryptData();

    
    m_ea = new EncryptedArray(*m_helib_context);

    std::cout<<" in ContextHElib constructor p10"<<std::endl;    
    m_nslots = m_ea->size();

    std::cout<<" in ContextHElib constructor p11 "<<m_nslots<<std::endl;    
    ////  populate the map that will allow us to set parameters via an input file (or string)
    
    m_param_name_map.insert({"param_set", m_param_set});
    m_param_name_map.insert({"Haming_weight", m_w});
    m_param_name_map.insert({"p", m_p});

  /// this is not nice, but for Compare, it helps to know if we are dealing with signed or unsigned inputs
    m_signed_plaintext = (std::is_same<Plaintext, int8_t>::value ||
			  std::is_same<Plaintext, int16_t>::value ||
			  std::is_same<Plaintext, int32_t>::value ||
			  std::is_same<Plaintext, int64_t>::value);
  }  

  // destructor
  virtual ~ContextHElib_F2() {
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

  long get_num_slots() {
    return m_nslots;
  }

 Ciphertext encrypt(PlaintextVec pt) {
    Ctxt mu(*m_publicKey);  /// use this to fill up the vector when resizing
    Ciphertext  ct;   /// now an NTL::Vec<Ctxt>
    resize(ct,m_bitwidth, mu);

    std::cout<<" encrypting - first two elements of pt are "<<std::to_string(pt[0])<<" "<<std::to_string(pt[1])<<std::endl;
    for (int i=0; i < m_bitwidth; i++) {
      std::vector<long> pt_long;      
      for (int slot=0; slot < pt.size(); slot++){
	pt_long.push_back((pt[slot] >> i)&1);
      /// fill up the slots with zeros
	std::cout<<"bit "<<i<<" slot "<<slot<<" is "<<std::to_string(pt_long.back())<<std::endl;
      }
      for (int slot = pt.size(); slot < m_nslots; slot++) {
	pt_long.push_back(0L);
      ////      this->m_publicKey->Encrypt(ct[i], ZZX((pt >>i)&1));

      }
	//this->m_publicKey->Encrypt(ct[i], pt_long);
      std::cout<<" encrypting ciphertext "<<i<<std::endl;
      Ctxt ct_element(*m_publicKey);
      m_ea->encrypt(ct_element, *m_publicKey, pt_long);
      ct[i] = ct_element;
      
    }
    std::cout<<" encrypted all ciphertexts "<<ct.length()<<std::endl;
    return ct;  
  }

  PlaintextVec decrypt(Ciphertext ct) {
    std::cout<<" decrypting "<<std::to_string(ct.length())<<std::endl;
    PlaintextVec pt;
    std::vector<long> pt_long;
    decryptBinaryNums(pt_long, CtPtrs_VecCt(ct), *m_secretKey, *m_ea);
    for (int i=0; i<pt_long.size(); i++) {
      std::cout<<" decrypting slot "<<i<<" "<<std::to_string(pt_long[i])<<std::endl;
      long pt_transformed = pt_long[i];
      pt.push_back((Plaintext)(pt_transformed  % int(pow(2,m_bitwidth))));
    }
    return pt;
    
  }


  Ciphertext Negate(Ciphertext a) {

    if (m_bootstrap) {
      for (int i=0; i< m_bitwidth; ++i) {
	a[i].modDownToLevel(5);
      }
    }

    
    /// Two's complement negation - negate all bits then add one
    Ciphertext output;
    for (int i=0; i < m_bitwidth; ++i) {
      Ctxt abit = a[i];
      //      abit.negate();
      abit.addConstant(to_ZZX(1L));
      output.append(abit);
    }
    if (m_bitwidth == 1)  return output;  // for a bool, we are already done..
    /// for integers, need to add 1.
    PlaintextVec ones;
    for (int slot=0; slot< m_nslots; slot++) {
      ones.push_back((Plaintext)1);
    }
    Ciphertext one_enc = encrypt(ones);
    Ciphertext output_final = Add(output,one_enc);
    return output_final;
  }

  
  Ciphertext Compare_unsigned(Ciphertext a, Ciphertext b) {

    
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

  Ciphertext Compare_signed(Ciphertext a, Ciphertext b) {
    //// subtract a-b and look at sign-bit
    Ciphertext b_minus_a = Subtract(b,a);
    Ciphertext output;

    Ctxt sign_bit = b_minus_a[m_bitwidth -1];   /// is sign-bit set?  if yes, b
    ///    sign_bit.addConstant(to_ZZX(1L));  //// now n
    output.append(sign_bit);
    return output;
  }

  Ciphertext Compare(Ciphertext a, Ciphertext b) {
    if (m_bootstrap) {
      for (int i=0; i< m_bitwidth; ++i) {
	a[i].modDownToLevel(5);
	b[i].modDownToLevel(5);
      }
    }
    if (m_signed_plaintext) return Compare_signed(a,b);
    else return Compare_unsigned(a,b);
  }
  
  
  Ciphertext Subtract(Ciphertext a, Ciphertext b) {

    if (m_bitwidth == 1) return Add(a,b);  //// for bools, add and subtract are the same
    
    Ciphertext output;
    Ciphertext b_neg = Negate(b);
    CtPtrs_VecCt wout(output);
    addTwoNumbers(wout,CtPtrs_VecCt(a),CtPtrs_VecCt(b_neg),
		  m_bitwidth,
		  &m_unpackSlotEncoding);
    return output;
  }

  
  Ciphertext Add(Ciphertext a, Ciphertext b) {

    Ciphertext sum;
    CtPtrs_VecCt wsum(sum);
    addTwoNumbers(wsum,CtPtrs_VecCt(a),CtPtrs_VecCt(b),
		  m_bitwidth,
		  &m_unpackSlotEncoding);
    return sum;
  }
  

  
  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
    
    Ciphertext product;
    CtPtrs_VecCt wprod(product);
    multTwoNumbers(wprod,CtPtrs_VecCt(a),CtPtrs_VecCt(b),
		   false,
		   m_bitwidth,
		   &m_unpackSlotEncoding);
    return product;
  }

  
  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {

    if (m_bootstrap) {
      for (int i=0; i< m_bitwidth; ++i) {
	a[i].modDownToLevel(5);
	b[i].modDownToLevel(5);
      }
    }
    /// s is 0 or 1
    /// for each bit of a,b,output, do output = s*a + (1-s)*b
    Ciphertext output;

    for (int i=0; i < m_bitwidth; ++i) {
      Ctxt sbit = s[0];
      Ctxt abit = a[i];
      Ctxt bbit = b[i];
      abit *= sbit;
      sbit.addConstant(to_ZZX(-1L));
      sbit.multByConstant(to_ZZX(-1L));
      sbit *= bbit;
      abit += sbit;
      output.append(abit);
    }
    return output;
  }

  
private:

  long m_param_set;  // which set of parameters to use (0 to 4).
  
  long m_p;     //  modulus of plaintext

  long m_B;     // number of bits per level
  
  long m_L;     // maximum number of homomorphic levels

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

  bool m_bootstrap;

  bool m_signed_plaintext;
  
};   //// end of ContextHElib_F2 class definition.


}  // leaving Sheep namespace
  
#endif // CONTEXT_HELIB_F2_HPP
