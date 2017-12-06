#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>
#include <fstream>
#include <sstream>
#include <sys/time.h>

///// Example use of HElib, largely taken from http://tommd.github.io/posts/HELib-Intro.html
///// to build, follow instructions to build fhe.a  library in HElib source directory, then
///// make Example_x
///// ./Example_x

/////  the value numVectors can be changed to perform different numbers of additions and multiplications
////   in order to see when the noise starts to lead to incorrect results.


////   results are output to a csv file output_HElibtests.csv

namespace HE_operation {
  enum operation {addition, multiplication};
}

inline bool file_exists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}



int checkResult(vector<long> decryptedVec,vector<vector<long> > plaintextVectors,int index, int modulus, HE_operation::operation op) {
  int numCorrectSlots = 0;
  for (int i = 0; i< decryptedVec.size(); i++) {
    /// what did we get?
    long heresult = decryptedVec[i];
    /// what should we have got?
    long ptresult = plaintextVectors[0][i];
    for (int j = 1; j<= index; j++) {
      if (op == HE_operation::addition) 
	ptresult += plaintextVectors[j][i];
      else
	ptresult *= plaintextVectors[j][i];	
    }
    ptresult = ptresult % modulus;
    // cout<<"results "<<heresult<<" "<<ptresult<<endl;
    if (ptresult == heresult) numCorrectSlots++;
  }
  return numCorrectSlots;
}






int main(int argc,char **argv)
{
  /** Generate a new key (or read one in), and encrypt secret data set. */

  long m=0, p=65539, r=1;   // native plaintext space, computations will be modulo p
  long L = 16;         // Levels
  long c=3;            // Columns in key switching matrix
  long w=64;           // Hamming weight of secret key
  long d=0;
  long security = 128;
  ZZX G;
  m = FindM(security,L,c,p,d,0,0);
  int numVectors = 30;   ///// how many additions or multiplications to do
  cout<<"m is "<<m<<endl;

  ofstream outputfile;
  string filename = "output_HElibtests.csv";
  if (! file_exists(filename)) {
    outputfile.open(filename,ios::out);
    outputfile << "L,r,c,w,security,m,p,nslots,nops,correct_add,correct_mult"<<endl;
  } else {
    outputfile.open(filename,ios::app);      
  }
  /// because p=2, homomorphic addition is XOR and multiplication is AND.
  /// to use 8 bit ints, can set p to something like 257 (8-bit ints)
  ////  NOTE - p=256 doesn't work - assertion failed in PAlgebra.cpp

  ///////////// Build a private key ////////////////////
  
  FHEcontext context(m,p,r);
  //initialize context
  buildModChain(context, L, c);
  // modify context, add primes to modulus chain
  FHESecKey secretKey(context);
  // construct a secret key structure
  const FHEPubKey& publicKey = secretKey; // note - "upcast" - FHESecKey is a subclass of FHEPubKey

  //cout<<" At this point the Hamming weight of secret key is "<<publicKey.getSKeyWeight()<<endl;
  
  G = context.alMod.getFactorsOverZZ()[0];

  secretKey.GenSecKey(w); // generate a secret key with Hamming weight w

  addSome1DMatrices(secretKey);
  cout<<" Generated key "<<endl;
  cout<<" The Hamming weight of secret key is "<<publicKey.getSKeyWeight()<<endl;
    
  EncryptedArray ea(context, G);
  // construct an Encrypted array object ea that is associated with the given context
  // and the polynomial G
  long nslots = ea.size();
  cout<<"number of slots in the encrypted array is "<<nslots<<endl;

  ////  create a vector of plaintext vectors (each with nslots elements)

  vector<vector<long> > vectorOfPlaintextVectors;

  ////  corresponding vector of ciphertext vectors (each with nslots elements)

  vector<Ctxt> vectorOfCiphertextVectors;
  


  /// fill the plaintext vectors with some simple sequences
  for (int i=0; i < numVectors; i++) {
    vector<long> v;
    for (int j = 0; j < nslots; j++) {
      v.push_back(j*(i+1));
    }

    vectorOfPlaintextVectors.push_back(v);
    ////  encrypt 
    Ctxt ct(publicKey);
    ea.encrypt(ct,publicKey,v);
    vectorOfCiphertextVectors.push_back(ct);
  }
    
  

  //////////////////////////////////////////////////////////////////////////
  //////////// public (untrusted) system - do some homomorphic operations

  vector<Ctxt> ctSums;

  vector<Ctxt> ctProds;


  
  /// "packing" means each element in a ciphertext vector has the sum or multiply operation applied.
  for (int i=0; i < numVectors; i++) {
    ctSums.push_back(vectorOfCiphertextVectors[0]);
    ctProds.push_back(vectorOfCiphertextVectors[0]);
    for (int j = 1; j <=i ; j++) {
      ctSums[i] += vectorOfCiphertextVectors[j];
      ctProds[i] *= vectorOfCiphertextVectors[j];
    }
  }

  
  /////////////////////////////////////////////////////////////////////////
  ///// now decrypt back on our private (trusted) system


  ///  cout<<" All computations are modulo "<<p<<endl;

  bool isOK = true;

  int numCorrectSlotsAdd = 0;
  int numCorrectSlotsMult = 0;  
  
  for (int i=0; i< numVectors; i++) {
  
    vector<long> resSum;
    ea.decrypt(ctSums[i], secretKey,resSum);
    numCorrectSlotsAdd = checkResult(resSum,vectorOfPlaintextVectors,i,p,HE_operation::addition);
    cout<<"Sum vector "<<i<<" is correct? "<<numCorrectSlotsAdd<<endl;

    vector<long> resMult;
    ea.decrypt(ctProds[i], secretKey,resMult);
    numCorrectSlotsMult = checkResult(resMult,vectorOfPlaintextVectors,i,p,HE_operation::multiplication);
    cout<<"Product vector "<<i<<" is correct? "<<numCorrectSlotsMult<<endl;    

    outputfile<<L<<","<<r<<","<<c<<","<<w<<","<<security<<","<<m<<","<<p<<","<<nslots<<","<<i+1<<","<<numCorrectSlotsAdd<<","<<numCorrectSlotsMult<<endl;

  }
  outputfile.close();
  return 0;
    
      
}




