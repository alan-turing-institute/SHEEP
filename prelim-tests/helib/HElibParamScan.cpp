#include "FHE.h"
#include "EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <bitset>
#include <map>
#include <algorithm>


///// Example use of HElib, largely taken from http://tommd.github.io/posts/HELib-Intro.html
///// to build, follow instructions to build fhe.a  library in HElib source directory, then
///// make HElibParamScan_x
///// ./HElibParamScan_x

/////  the value numVectors can be changed to perform different numbers of additions and multiplications
////   in order to see when the noise starts to lead to incorrect results.


////   usage ./HElibParamScan --scanVars  <comma-separated-list> --scanType <noise|ctsize>

////   results are output to a csv file (name depends on type of scan


namespace scanType {
  enum type {noise, ciphertextsize, nslots};
}


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

/////////////////////////////////////////////////////////////////////////

/// setup a scan - define one or two variables to be the "scan variables"

map<string, vector<long int> > setupScan(std::vector<string> varsToScan) {
  map<string, vector<long int> > vars;


  /// p - modulus - must be prime!
  std::vector<long int> pvec;  
  if ( find(varsToScan.begin(), varsToScan.end(),"p" ) == varsToScan.end()) {
    pvec.push_back(2);
  } else {
    pvec.push_back(2);    
    pvec.push_back(3);
    pvec.push_back(7);
    pvec.push_back(13);
    pvec.push_back(31);
    pvec.push_back(61);
    pvec.push_back(127);
    pvec.push_back(251);
    pvec.push_back(509);
  }
  vars["p"] = pvec;  

  /// r - native plaintext space
  std::vector<long int> rvec;
  if ( find(varsToScan.begin(), varsToScan.end(),"r" ) == varsToScan.end()) {
    rvec.push_back(1);
  } else {
    for (long int i=0; i < 10; i++) rvec.push_back(i);
  }
  vars["r"] = rvec;

  /// L - levels
  std::vector<long int> Lvec;
  if ( find(varsToScan.begin(), varsToScan.end(),"L" ) == varsToScan.end()) {
    Lvec.push_back(16);
  } else {
    for (long int i=6; i < 20; i+=1) Lvec.push_back(i);
  }
  vars["L"] = Lvec;

  /// c - Columns in key-switching matrix
  std::vector<long int> cvec;
  cvec.push_back(3);
  vars["c"] = cvec;

  /// w - Hamming weight of secret key
  std::vector<long int> wvec;
  wvec.push_back(64);
  vars["w"] = wvec;

  /// d - don't know what this does - set to zero?
  std::vector<long int> dvec;
  dvec.push_back(0);
  vars["d"] = dvec;

  /// security - bits of security
  std::vector<long int> secvec;
  if ( find(varsToScan.begin(), varsToScan.end(),"sec" ) == varsToScan.end()) {
    secvec.push_back(90);
  } else {
    for (long int i=80; i <= 160; i +=20 ) secvec.push_back(i);
  }
  vars["sec"] = secvec;
  int totalNumVals = 1;
  totalNumVals = totalNumVals * vars["sec"].size();
  totalNumVals = totalNumVals * vars["L"].size();
  totalNumVals = totalNumVals * vars["p"].size();
  totalNumVals = totalNumVals * vars["r"].size();
  totalNumVals = totalNumVals * vars["c"].size();
  totalNumVals = totalNumVals * vars["d"].size();
  totalNumVals = totalNumVals * vars["w"].size();
  cout<<"Setup map of parameber vectors - total number of scan points is "<<totalNumVals<<endl;
  return vars;
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



int doScan(map<string,vector<long int> > scanConfig, int scanToDo, int numOperations=1) {

  /** open an output file and write headings if not already there */
  string filename = "";
  if (scanToDo == scanType::noise) filename = "helib_noiseScan.csv";
  else if (scanToDo == scanType::ciphertextsize) filename = "helib_ciphertextsizeScan.csv";
  else if (scanToDo == scanType::nslots) filename = "helib_nslotsScan.csv";  

  ofstream outputfile;

  if (! file_exists(filename)) {
    outputfile.open(filename,ios::out);
    //// write the field headings
    if (scanToDo == scanType::noise)
      outputfile << "L,r,c,w,security,m,p,nslots,ctsize,nops,correct_add,correct_mult"<<endl;
    else if (scanToDo == scanType::ciphertextsize)
      outputfile << "L,r,c,w,security,m,p,nslots,ctsize"<<endl;
    else if (scanToDo == scanType::nslots)
      outputfile << "L,r,c,w,security,m,p,nslots"<<endl;
  } else {
    outputfile.open(filename,ios::app);      
  }
  
  ///// iterate over all the vectors in the scan config, i.e. a multiply nested loop.
  for (std::vector<long int>::iterator Liter = scanConfig["L"].begin(); Liter != scanConfig["L"].end(); Liter++) {
    long L = *Liter;
    for (std::vector<long int>::iterator riter = scanConfig["r"].begin(); riter != scanConfig["r"].end(); riter++) {
      long r = *riter;
      for (std::vector<long int>::iterator citer = scanConfig["c"].begin(); citer != scanConfig["c"].end(); citer++) {
	long c = *citer;
	for (std::vector<long int>::iterator witer = scanConfig["w"].begin(); witer != scanConfig["w"].end(); witer++) {
	  long w = *witer;
	  for (std::vector<long int>::iterator seciter = scanConfig["sec"].begin(); seciter != scanConfig["sec"].end(); seciter++) {
	    long security = *seciter;
	    for (std::vector<long int>::iterator piter = scanConfig["p"].begin(); piter != scanConfig["p"].end(); piter++) {
	      long p = *piter;
	      for (std::vector<long int>::iterator diter = scanConfig["d"].begin(); diter != scanConfig["d"].end(); diter++) {
		long d = *diter;

		cout<<"Current scan point: "<<security<<" "<<L<<" "<<c<<" "<<p<<" "<<d<<endl;
  /** Generate a new key (or read one in), and encrypt secret data set. */

		  ZZX G;
		  long m = FindM(security,L,c,p,d,0,0);

  ///////////// Build a private key ////////////////////
  
		  FHEcontext context(m,p,r);
		  //initialize context
		  buildModChain(context, L, c);
		  // modify context, add primes to modulus chain
		  FHESecKey secretKey(context);
		  // construct a secret key structure
		  const FHEPubKey& publicKey = secretKey; // note - "upcast" - FHESecKey is a subclass of FHEPubKey

		  G = context.alMod.getFactorsOverZZ()[0];

		  secretKey.GenSecKey(w); // generate a secret key with Hamming weight w

		  addSome1DMatrices(secretKey);
		  cout<<" Generated key "<<endl;
    
		  EncryptedArray ea(context, G);
		  // construct an Encrypted array object ea that is associated with the given context
		  // and the polynomial G
		  long nslots = ea.size();
		  if (scanToDo == scanType::nslots) {
		    outputfile<<L<<","<<r<<","<<c<<","<<w<<","<<security<<","<<m<<","<<p<<","<<nslots<<endl;
		    cout<<"number of slots in the encrypted array is "<<nslots<<endl;
		    //// we don't care about encrypting for this scan - just go onto the next scan point.
		    continue;

		  }
		  ///  OK, if we got to here, we will want to encrypt at least once..
		  ////  create a vector of plaintext vectors (each with nslots elements)
		  ///
		  vector<vector<long> > vectorOfPlaintextVectors;

		  ////  corresponding vector of ciphertext vectors (each with nslots elements)

		  vector<Ctxt> vectorOfCiphertextVectors;

		  
		  /// fill the plaintext vectors with some simple sequences
		  for (int i=0; i < numOperations; i++) {
		    vector<long> v;
		    for (int j = 0; j < nslots; j++) {
		      v.push_back(j*(i+1));
		    }
		    
		    vectorOfPlaintextVectors.push_back(v);
		    ////  encrypt 
		    Ctxt ct(publicKey);
		    ea.encrypt(ct,publicKey,v);

		    int ciphertextSize = (int)(sizeof(ct));
		    cout<<"Size of ciphertext with "<<nslots<<" slots is "<<ciphertextSize<<endl;
		    vectorOfCiphertextVectors.push_back(ct);
		    
		    //// if we just care about the ciphertext size, write out to a file and continue
		    if (scanToDo == scanType::ciphertextsize) {
		      outputfile<<L<<","<<r<<","<<c<<","<<w<<","<<security<<","<<m<<","<<p<<","<<nslots<<","<<ciphertextSize<<endl;
		      ///don't need to go any further, just go onto the next scan point
		      continue;
		    }

		  }

		  ////  If we got to here, we really want to look at doing stuff with the ciphertext		  
		  //////////////////////////////////////////////////////////////////////////
		  //////////// public (untrusted) system - do some homomorphic operations
		  
		  vector<Ctxt> ctSums;
		  vector<Ctxt> ctProds;
		  
		  
		  /// "packing" means each element in a ciphertext vector has the sum or multiply operation applied.
		  for (int i=0; i < numOperations; i++) {
		    ctSums.push_back(vectorOfCiphertextVectors[0]);
		    ctProds.push_back(vectorOfCiphertextVectors[0]);
		    for (int j = 1; j <=i ; j++) {
		      ctSums[i] += vectorOfCiphertextVectors[j];
		      ctProds[i] *= vectorOfCiphertextVectors[j];
		    }
		  }

  
  /////////////////////////////////////////////////////////////////////////
  ///// now decrypt back on our private (trusted) system
		  
		  bool isOK = true;
		  
		  int numCorrectSlotsAdd = 0;
		  int numCorrectSlotsMult = 0;
		  
		  int numGoodOpsAdd = numOperations;
		  int numGoodOpsMult = numOperations;
		  
		  for (int i=0; i< numOperations; i++) {
		    
		    //  vector<long> resSum;
		    // ea.decrypt(ctSums[i], secretKey,resSum);
		    //numCorrectSlotsAdd = checkResult(resSum,vectorOfPlaintextVectors,i,p,HE_operation::addition);
		    //cout<<"Sum vector "<<i<<" is correct? "<<numCorrectSlotsAdd<<endl;
		    //if ((numCorrectSlotsAdd < nslots) && (numGoodOpsAdd == numOperations)) numGoodOpsAdd = i;
		    vector<long> resMult;
		    ea.decrypt(ctProds[i], secretKey,resMult);
		    numCorrectSlotsMult = checkResult(resMult,vectorOfPlaintextVectors,i,p,HE_operation::multiplication);
		    cout<<"Product vector "<<i<<" is correct? "<<numCorrectSlotsMult<<endl;
		    if ((numCorrectSlotsMult < nslots) && (numGoodOpsMult == numOperations)) numGoodOpsMult = i;		    
		    //	    if (scanToDo == scanType::noise)
		    //   outputfile<<L<<","<<r<<","<<c<<","<<w<<","<<security<<","<<m<<","<<p<<","<<nslots<<","<<sizeof(ctProds[i])<<","<<i+1<<","<<numCorrectSlotsAdd<<","<<numCorrectSlotsMult<<endl;
		    if (numGoodOpsMult < numOperations) break;
		  }
		  if (scanToDo == scanType::noise)
		    outputfile<<L<<","<<r<<","<<c<<","<<w<<","<<security<<","<<m<<","<<p<<","<<nslots<<","<<sizeof(ctProds[0])<<","<<numOperations<<","<<numGoodOpsAdd<<","<<numGoodOpsMult<<endl;
	      }
	    }
	  }
	}
      }
    }
  }
  outputfile.close();
  return 0;      
}

  
int main(int argc,char **argv) {
  
  int numOperations = 1;
  bool lookForType = false;
  bool lookForVars = false;
  bool lookForNumOps = false;  
  int scanToDo = -1;
  std::vector<string> varsToScan;

  for (int i=0; i < argc; i++) {
    if (lookForType) {
      if (strncmp(argv[i],"noise",5) == 0) {
	scanToDo = scanType::noise;
      } else if (strncmp(argv[i],"ciphertextsize",14) == 0) {
	scanToDo = scanType::ciphertextsize;
      } else if (strncmp(argv[i],"nslots",6) == 0) {
	scanToDo = scanType::nslots;
      }
      lookForType = false;
    } else if (lookForVars) {
      string allScanVars = argv[i];
      stringstream varList(allScanVars);
      while (varList.good()) {
	string subst;
	getline ( varList, subst, ',');
	varsToScan.push_back( subst );
      }
      lookForVars = false;
    } else if (lookForNumOps) {
      numOperations = atoi(argv[i]);
      lookForNumOps = false;
    }
    if (strncmp(argv[i],"--scanType",10) ==0) {
      lookForType = true;
    } else if (strncmp(argv[i],"--scanVars",10) ==0) {
      lookForVars = true;
    } else if (strncmp(argv[i],"--numOps",8) ==0) {
      lookForNumOps = true;
    } 
    
  }
  
  map<string, vector<long int> > scanConfig = setupScan(varsToScan);
  
  int scanOK = doScan(scanConfig,scanToDo,numOperations);
  
}


