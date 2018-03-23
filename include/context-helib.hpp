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

template <typename PlaintextT, typename CiphertextT>
class ContextHElib : public Context< PlaintextT , CiphertextT> {

public:

  typedef PlaintextT Plaintext;
  typedef CiphertextT Ciphertext;  

  /// constructors

  ContextHElib(long p,             // plaintext modulus
	       long param_set=0,   // parameter set, from 0 (tiny) to 4 (huge)
	       long bootstrapl=1, // bootstrap or not?
	       long haming_weight=128): // Haming weight of secret key

    m_p(p),
    m_param_set(param_set),
    m_bootstrapl(bootstrapl),
    m_w(haming_weight)
  {

    /// BITWIDTH(bool) is 8, so need to deal with this by hand...
    //// (better to specialize class?)
    if (std::is_same<Plaintext, bool>::value)
      m_bitwidth = 1;
    else
      m_bitwidth = BITWIDTH(Plaintext);

    ////  populate the map that will allow us to set parameters via an input file (or string)
    
    this->m_param_name_map.insert({"A_predefined_param_set", m_param_set});
    this->m_param_name_map.insert({"Haming_weight", m_w});
    this->m_param_name_map.insert({"bootstrap", m_bootstrapl});            
    this->m_param_name_map.insert({"m",m_m});
    this->m_param_name_map.insert({"phi(m)",m_phim});
    this->m_param_name_map.insert({"d",m_d});
    this->m_param_name_map.insert({"m1",m_m1});
    this->m_param_name_map.insert({"m2",m_m2});
    this->m_param_name_map.insert({"m3",m_m3});
    this->m_param_name_map.insert({"g1",m_g1});
    this->m_param_name_map.insert({"g2",m_g2});
    this->m_param_name_map.insert({"g3",m_g3});
    this->m_param_name_map.insert({"ord1",m_ord1});
    this->m_param_name_map.insert({"ord2",m_ord2});
    this->m_param_name_map.insert({"ord3",m_ord3});
    this->m_param_name_map.insert({"c",m_c});
    this->m_param_name_map.insert({"B",m_B});
    this->m_param_name_map.insert({"levels",m_L});

    
    /// configure
    configure();
  }

  
  void configure() {

    m_bootstrap = (bool)m_bootstrapl;
    /// Set all the other parameters.
    
    long mValues[][15] = { 
      // { p, phi(m),   m,   d, m1, m2, m3,    g1,   g2,   g3, ord1,ord2,ord3, B,c}
      {  2,    48,   105, 12,  3, 35,  0,    71,    76,    0,   2,  2,   0, 25, 2},
      {  2 ,  600,  1023, 10, 11, 93,  0,   838,   584,    0,  10,  6,   0, 25, 2},
      {  2,  2304,  4641, 24,  7,  3,221,  3979,  3095, 3760,   6,  2,  -8, 25, 3},
      {  2, 15004, 15709, 22, 23,683,  0,  4099, 13663,    0,  22, 31,   0, 25, 3},
      {  2, 27000, 32767, 15, 31,  7, 151, 11628, 28087,25824, 30,  6, -10, 28, 4}
    };

    long* vals = mValues[m_param_set];
    
    ///    long m = vals[2];

    m_m = vals[2];
    m_phim = vals[1];
    m_d = vals[3];
    m_m1 = vals[4];
    m_m2 = vals[5];
    m_m3 = vals[6];
    m_g1 = vals[7];
    m_g2 = vals[8];
    m_g3 = vals[9];
    m_ord1 = vals[10];
    m_ord2 = vals[11];
    m_ord3 = vals[12];
    m_B = vals[13];
    m_c = vals[14];    

    /*    
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
    */

    NTL::Vec<long> mvec;
    append(mvec, m_m1);
    if (m_m2>1) append(mvec, m_m2);
    if (m_m3>1) append(mvec, m_m3);
    
    std::vector<long> gens;
    gens.push_back(m_g1);
    if (m_g2>1) gens.push_back(m_g2);
    if (m_g3>1) gens.push_back(m_g3);

    std::vector<long> ords;
    ords.push_back(m_ord1);
    if (abs(m_ord2)>1) ords.push_back(m_ord2);
    if (abs(m_ord3)>1) ords.push_back(m_ord3);
    m_B = vals[13];
    m_c = vals[14];

    /// number of levels
    
    if (m_bootstrap) m_L = 30; // that should be enough
    else m_L = 3 + NTL::NumBits(m_bitwidth+2);
    
    /// initialize HElib context
    m_helib_context = new FHEcontext(m_m, m_p, 1, gens, ords);
    m_helib_context->bitsPerLevel = m_B;
    /// modify context, add primes to modulus chain
    buildModChain(*m_helib_context, m_L, m_c,8);

    if (m_bootstrap) {
      m_helib_context->makeBootstrappable(mvec, /*t=*/0,
					  /*flag=*/false, /*cacheType=DCRT*/2);
    }

    // unpack slot encoding
    buildUnpackSlotEncoding(m_unpackSlotEncoding, *(m_helib_context->ea));
    
    /// create secret key structure
    m_secretKey = new FHESecKey(*m_helib_context);
    
    m_publicKey = m_secretKey;  //// points to the same place
  
    /// generate a secret key
    m_secretKey->GenSecKey(m_w);   /// Haming weight of 128
     
    addSome1DMatrices(*m_secretKey);
    addFrbMatrices(*m_secretKey);

    if (m_bootstrap) m_secretKey->genRecryptData();

    
    m_ea = new EncryptedArray(*m_helib_context);
    
    m_nslots = m_ea->size();


};

  // destructor
  virtual ~ContextHElib() {
    /// delete everything we new-ed in the constructor
    if (m_ea != NULL) delete m_ea;
    if (m_secretKey != NULL) delete m_secretKey;
    if (m_helib_context != NULL) delete m_helib_context;
  };

  virtual Ciphertext encrypt(Plaintext pt) = 0;
  virtual Plaintext decrypt(Ciphertext pt) = 0;  

 
  
  
  long get_num_slots() {
    return m_nslots;
  }

  
protected:

  long m_param_set;  // which set of parameters to use (0 to 4).
  
  long m_p;     //  modulus of plaintext

  long m_B;     // number of bits per level
  
  long m_L;     // maximum number of homomorphic levels

  long m_w;     // Hamming weight of secret key

  long m_c;     // number of columns in key-switching matrix

  long m_nslots;  // number of SIMD operations that can be done at a time

  long m_m;

  long m_phim;

  long m_d;

  long m_m1;

  long m_m2;

  long m_m3;

  long m_g1;

  long m_g2;

  long m_g3;

  long m_ord1;

  long m_ord2;

  long m_ord3;
  
  EncryptedArray* m_ea; 

  FHESecKey* m_secretKey;

  const FHEPubKey* m_publicKey;

  FHEcontext* m_helib_context;

  int m_bitwidth;

  std::vector<zzX> m_unpackSlotEncoding;

  bool m_bootstrap;

  long m_bootstrapl;
  
};   //// end of ContextHElib class definition.

  
  ////////////////////////////////////////////////////////////////////////////////////
  ///  ContextHElib_F2 -  use p=2, do everything with arrays of Ciphertext,
  ///  and binary operations for add, multiply, compare etc.
  
template<typename PlaintextT>
class ContextHElib_F2 : public ContextHElib< PlaintextT, NTL::Vec<Ctxt> > {

public:

  typedef PlaintextT Plaintext;
  typedef NTL::Vec<Ctxt> Ciphertext;  
  
  ContextHElib_F2(long param_set=0,   // parameter set, from 0 (tiny) to 4 (huge)
		  bool bootstrap=true, // bootstrap or not?
		  long haming_weight=128) // Haming weight of secret key
    : ContextHElib<Plaintext,Ciphertext>(2,param_set,bootstrap,haming_weight)
  {
    

    /// this is not nice, but for Compare, it helps to know if we are dealing with signed or unsigned inputs
    m_signed_plaintext = (std::is_same<Plaintext, int8_t>::value ||
			  std::is_same<Plaintext, int16_t>::value ||
			  std::is_same<Plaintext, int32_t>::value);

  }


  
  Ciphertext encrypt(Plaintext pt) {
    Ctxt mu(*(this->m_publicKey));  /// use this to fill up the vector when resizing
    Ciphertext  ct;   /// now an NTL::Vec<Ctxt>
    resize(ct,this->m_bitwidth, mu);
    for (int i=0; i < this->m_bitwidth; i++) {
      this->m_publicKey->Encrypt(ct[i], ZZX((pt >>i)&1));
    }
    return ct;  
  }

  Plaintext decrypt(Ciphertext ct) {
    std::vector<long> pt;
    decryptBinaryNums(pt, CtPtrs_VecCt(ct), *(this->m_secretKey), *(this->m_ea));
    long pt_transformed = pt[0];
    return pt_transformed  % int(pow(2,this->m_bitwidth));
    
  }


  Ciphertext Negate(Ciphertext a) {

    if (this->m_bootstrap) {
      for (int i=0; i< this->m_bitwidth; ++i) {
	a[i].modDownToLevel(5);
      }
    }

    
    /// Two's complement negation - negate all bits then add one
    Ciphertext output;
    for (int i=0; i < this->m_bitwidth; ++i) {
      Ctxt abit = a[i];
      //      abit.negate();
      abit.addConstant(to_ZZX(1L));
      output.append(abit);
    }
    if (this->m_bitwidth == 1)  return output;  // for a bool, we are already done..
    /// for integers, need to add 1.
    Ciphertext one_enc = encrypt((Plaintext)1);
    Ciphertext output_final = Add(output,one_enc);
    return output_final;
  }

  
  Ciphertext Compare_unsigned(Ciphertext a, Ciphertext b) {

    
    Ctxt mu(*(this->m_publicKey));
    Ctxt ni(*(this->m_publicKey));    
    Ciphertext cmax, cmin;
    CtPtrs_VecCt wMin(cmin), wMax(cmax);  /// wrappers around output vectors
    compareTwoNumbers(wMax, wMin, mu, ni,
		      CtPtrs_VecCt(a), CtPtrs_VecCt(b),
		      &(this->m_unpackSlotEncoding));
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

    Ctxt sign_bit = b_minus_a[this->m_bitwidth -1];   /// is sign-bit set?  if yes, b
    ///    sign_bit.addConstant(to_ZZX(1L));  //// now n
    output.append(sign_bit);
    return output;
  }

  Ciphertext Compare(Ciphertext a, Ciphertext b) {
    if (this->m_bootstrap) {
      for (int i=0; i< this->m_bitwidth; ++i) {
	a[i].modDownToLevel(5);
	b[i].modDownToLevel(5);
      }
    }
    if (this->m_signed_plaintext) return Compare_signed(a,b);
    else return Compare_unsigned(a,b);
  }
  
  
  Ciphertext Subtract(Ciphertext a, Ciphertext b) {

    if (this->m_bitwidth == 1) return Add(a,b);  //// for bools, add and subtract are the same
    
    Ciphertext output;
    Ciphertext b_neg = Negate(b);
    CtPtrs_VecCt wout(output);
    addTwoNumbers(wout,CtPtrs_VecCt(a),CtPtrs_VecCt(b_neg),
		  this->m_bitwidth,
		  &(this->m_unpackSlotEncoding));
    return output;
  }

  
  Ciphertext Add(Ciphertext a, Ciphertext b) {

    Ciphertext sum;
    CtPtrs_VecCt wsum(sum);
    addTwoNumbers(wsum,CtPtrs_VecCt(a),CtPtrs_VecCt(b),
		  this->m_bitwidth,
		  &(this->m_unpackSlotEncoding));
    return sum;
  }
  

  
  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
    
    Ciphertext product;
    CtPtrs_VecCt wprod(product);
    multTwoNumbers(wprod,CtPtrs_VecCt(a),CtPtrs_VecCt(b),
		   false,
		   this->m_bitwidth,
		   &(this->m_unpackSlotEncoding));
    return product;
  }

  
  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {

    if (this->m_bootstrap) {
      for (int i=0; i< this->m_bitwidth; ++i) {
	a[i].modDownToLevel(5);
	b[i].modDownToLevel(5);
      }
    }
    /// s is 0 or 1
    /// for each bit of a,b,output, do output = s*a + (1-s)*b
    Ciphertext output;

    for (int i=0; i < this->m_bitwidth; ++i) {
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

  bool m_signed_plaintext;

  
};  /// end of class definition

  ////////////////////////////////////////////////////////////////////////////
  //// ContextHElib_Fp  - use integer plaintext space, e.g. p=65537

  
template<typename PlaintextT>
class ContextHElib_Fp : public ContextHElib< PlaintextT, Ctxt > {

public:

  typedef PlaintextT Plaintext;
  typedef Ctxt Ciphertext;  
  
  ContextHElib_Fp(long p=65537,      // plaintext modulus
		  long param_set=0,   // parameter set, from 0 (tiny) to 4 (huge)
		  bool bootstrap=false, // bootstrap or not?
		  long haming_weight=128) // Haming weight of secret key
    : ContextHElib<Plaintext,Ciphertext>(p,param_set,bootstrap,haming_weight)
  {
    this->m_param_name_map.insert({"p", this->m_p});     
  }

  
  Ciphertext encrypt(Plaintext pt) {

//// if plaintext is a bool, convert it into a vector of longs, with just the first element as 1 or zero
    std::vector<long> ptvec;
    ptvec.push_back(pt);
    
    ////// fill up nslots with zeros//// 
    for (int i = ptvec.size(); i < this->m_nslots; i++) ptvec.push_back(0);
    
    Ciphertext ct(*(this->m_publicKey));
    this->m_ea->encrypt(ct, *(this->m_publicKey), ptvec);
    return ct; 
   
  }

  Plaintext decrypt(Ciphertext ct) {
    
    std::vector<long> pt;
    this->m_ea->decrypt(ct, *(this->m_secretKey), pt);
    long pt_transformed = pt[0];
    if ((pt[0]) > this->m_p / 2)    //// convention - treat this as a negative number
      pt_transformed = pt[0] - this->m_p;
    return pt_transformed  % int(pow(2,this->m_bitwidth));
    
  }

  
  
  Ciphertext Add(Ciphertext a, Ciphertext b) {

    a += b;
    return a;

  }

  Ciphertext Subtract(Ciphertext a, Ciphertext b) {

    a -= b;
    return a;

  }
  
  
  Ciphertext Multiply(Ciphertext a, Ciphertext b) {

    a *= b;
    return a;
    
  }

  Ciphertext Negate(Ciphertext a) {

    if (this->m_bitwidth == 1)  /// special case for binary
      a.addConstant(to_ZZX(1L));
    else
      a.multByConstant(to_ZZX(-1L));  
    return a;   

  }

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
  
};  /// end of class definition
  
  
}  // leaving Sheep namespace
  
#endif // CONTEXT_HELIB_HPP
