#ifndef CONTEXT_LP_HPP
#define CONTEXT_LP_HPP


#include<gmp.h>
#include "circuit.hpp"
#include "context.hpp"
extern "C" {
  #include "paillier.h"
}

#include <cmath>
#include <sstream>

namespace SHEEP {

template <typename PlaintextT>
class ContextLP : public Context<PlaintextT, paillier_ciphertext_t> {

public:
	typedef PlaintextT Plaintext;
	typedef paillier_ciphertext_t Ciphertext;

	// constructor

	ContextLP(const long n = 256):


		m_n(n)
	{
		this->m_param_name_map.insert({"n", m_n});
		this->m_private_key_size = 0;
		this->m_public_key_size = 0;
		this->m_ciphertext_size = 0;
		configure();

	}

	void configure() {

		paillier_keygen(m_n, &pubKey, &secKey, paillier_get_rand_devurandom);
		this->m_public_key_size = sizeof(pubKey);
		this->m_private_key_size = sizeof(secKey);
		this->m_configured = true;


	}

	Ciphertext encrypt(Plaintext p) {



		paillier_plaintext_t* m;
		m = paillier_plaintext_from_ui(p);

		paillier_ciphertext_t *ctxt;
		ctxt = paillier_enc(NULL, pubKey, m, paillier_get_rand_devurandom);
		return *ctxt;

	}

	Plaintext decrypt(Ciphertext ct) {

		paillier_ciphertext_t* encrypted_sum = paillier_create_enc_zero();
		paillier_plaintext_t* dec;
		dec = paillier_dec(NULL, this->pubKey, this->secKey, &ct);
		return mpz_get_ui(dec->m);

	}

	Ciphertext Add(Ciphertext a, Ciphertext b) {

		paillier_ciphertext_t* encrypted_sum = paillier_create_enc_zero();
		paillier_mul(this->pubKey, encrypted_sum, &a, &b);
		return *encrypted_sum;

	}

	Ciphertext Negate(Ciphertext a) {

		return MultByConstant(a, -1);
	}



	Ciphertext MultByConstant(Ciphertext a, long b) {


		paillier_ciphertext_t* product = paillier_create_enc_zero();
		paillier_plaintext_t* pt = paillier_plaintext_from_ui(b);
		paillier_exp( this->pubKey, product, &a, pt);
		return *product;

	}


	// destructor
	virtual ~ContextLP() {


	};

protected:
	paillier_pubkey_t* pubKey;
	paillier_prvkey_t* secKey;
	long m_n;


};

}  // Leaving Sheep namespace

#endif // CONTEXT_SEAL_HPP
