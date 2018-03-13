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


namespace Sheep {
namespace HElib {

template <typename PlaintextT, typename CiphertextT>
class ContextHElib : public Context< PlaintextT , CiphertextT> {


		       	     //				     NTL::Vec<Ctxt> > {

public:

  typedef PlaintextT Plaintext;
  typedef CiphertextT Ciphertext;  

  /// constructors

  ContextHElib(long param_set=0,   // parameter set, from 0 (tiny) to 4 (huge)
	       bool bootstrap=true, // bootstrap or not?
	       long haming_weight=128):   // Haming weight of secret key
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

    long* vals = mValues[m_param_set];
    m_p = vals[0];
    
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
    
    if (m_bootstrap) m_L = 30; // that should be enough
    else m_L = 3 + NTL::NumBits(m_bitwidth+2);
    
    /// initialize HElib context
    m_helib_context = new FHEcontext(m, m_p, 1, gens, ords);
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

    ////  populate the map that will allow us to set parameters via an input file (or string)
    
    m_param_name_map.insert({"param_set", m_param_set});
    m_param_name_map.insert({"Haming_weight", m_w});    
  
};

  // destructor
  virtual ~ContextHElib() {
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

  std::map<std::string, long& > m_param_name_map;
  
  EncryptedArray* m_ea; 

  FHESecKey* m_secretKey;

  const FHEPubKey* m_publicKey;

  FHEcontext* m_helib_context;

  int m_bitwidth;

  std::vector<zzX> m_unpackSlotEncoding;

  bool m_bootstrap;
  
};   //// end of ContextHElib class definition.
  
  ////////////////////////////////////////////////////////////////////////////////////
  ///  ContextHElib_F2 -  use p=2, do everything with arrays of Ciphertext,
  ///  and binary operations for add, multiply, compare etc.
  
template<typename PlaintextT>
class ContextHElib_F2 : public ContextHElib< PlaintextT, NTL::Vec<Ctxt> > {

public:

  typedef PlaintextT Plaintext;
  typedef NTL::Vec<Ctxt> Ciphertext;  
  
     
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

  
  Ciphertext Compare(Ciphertext a, Ciphertext b) {
    if (this->m_bootstrap) {
      for (int i=0; i< this->m_bitwidth; ++i) {
	a[i].modDownToLevel(5);
	b[i].modDownToLevel(5);
      }
    }
    
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
  

  
};  /// end of class definition

  ////////////////////////////////////////////////////////////////////////////
  //// ContextHElib_Fp  - use integer plaintext space, e.g. p=65537

  
template<typename PlaintextT>
class ContextHElib_Fp : public ContextHElib< PlaintextT, Ctxt > {

public:

  typedef PlaintextT Plaintext;
  typedef Ctxt Ciphertext;  
  
     
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
    std::cout<<" answer before modulus is "<<std::to_string(pt[0])<<std::endl;
    long pt_transformed = pt[0];
    if ((pt[0]) > this->m_p / 2)    //// convention - treat this as a negative number
      pt_transformed = pt[0] - this->m_p;
    return pt_transformed  % int(pow(2,this->m_bitwidth));
    
  }

  
  
  Ciphertext Add(Ciphertext a, Ciphertext b) {

    a += b;
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
  
  
}  // leavin HElib namespace
}  // leaving Sheep namespace
  
#endif // CONTEXT_HELIB_HPP
