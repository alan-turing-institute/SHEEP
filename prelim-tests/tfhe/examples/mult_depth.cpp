#include <cmath>
#include <iostream>
#include <boost/program_options.hpp>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

const double sqrt2_PI = sqrt(2/M_PI);

namespace po = boost::program_options;

struct TFheOptions {
	int32_t N, k, n, l, Bgbit, ks_t, ks_basebit;
	double alpha_min, alpha_max, bk_stdev;
};

void handle_options(int argc, char *argv[], TFheOptions& tfhe_opts);

int main(int argc, char *argv[])
{
	TFheOptions tfhe_opts;
	handle_options(argc, argv, tfhe_opts);

	TLweParams tlwe_params(tfhe_opts.N,
			       tfhe_opts.k,
			       tfhe_opts.bk_stdev,
			       tfhe_opts.alpha_max);

	LweParams lwe_params(tfhe_opts.n,
			     tfhe_opts.alpha_min,
			     tfhe_opts.alpha_max);

	TGswParams tgsw_params(tfhe_opts.l,
			       tfhe_opts.Bgbit,
			       &tlwe_params);
	
	TFheGateBootstrappingParameterSet tfhe_params(
	 	tfhe_opts.ks_t,
	 	tfhe_opts.ks_basebit,
	 	&lwe_params, &tgsw_params);

	///// Generate key
	uint32_t seed[] = { 314, 1592, 657 };
	tfhe_random_generator_setSeed(seed,3);
	TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(&tfhe_params);

	///// Plaintext
	uint16_t plaintext = 0xFFFF;
        const int pt_bitlength = 16;
	LweSample* ciphertext = new_gate_bootstrapping_ciphertext_array(pt_bitlength, &tfhe_params);
	for (int i=0; i<pt_bitlength; i++)
		bootsSymEncrypt(&ciphertext[i], (plaintext >> i) & 1, key);
	
	///// Repeatedly AND the ciphertext with itself
	for (int iter=0; iter<100; iter++) {
		for (int i=0; i<pt_bitlength; i++)
			bootsAND(&ciphertext[i], &ciphertext[i], &ciphertext[i], &key->cloud);

		// Decrypt it
		uint16_t plaintext_decrypt = 0;
		for (int i=0; i<pt_bitlength; i++)
			plaintext_decrypt |= (bootsSymDecrypt(&ciphertext[i], key) << i);

		std::cout << plaintext_decrypt << "\n";
	}

	delete_gate_bootstrapping_ciphertext_array(pt_bitlength, ciphertext);
	delete_gate_bootstrapping_secret_keyset(key);
}

void handle_options(int argc, char *argv[], TFheOptions& tfhe_opts)
{
	po::options_description desc("Options");
	desc.add_options()
		("help,h", "Produce this help message")

		(",N",
		 po::value<int32_t>(&tfhe_opts.N)
		 -> default_value(1024),
		 "Degree of the polynomials")

		(",k",
		 po::value<int32_t>(&tfhe_opts.k)
		 -> default_value(1),
		 "Number of polynomials")

		(",n",
		 po::value<int32_t>(&tfhe_opts.n)
		 -> default_value(500),
		 "?")

		(",l",
		 po::value<int32_t>(&tfhe_opts.l)
		 -> default_value(2),
		 "Decomp length")

		("Bgbit",
		 po::value<int32_t>(&tfhe_opts.Bgbit)
		 -> default_value(10),
		 "log2(decomp base)")

		("ks_t",
		 po::value<int32_t>(&tfhe_opts.ks_t)
		 -> default_value(8),
		 "length of ?")

		("ks_basebit",
		 po::value<int32_t>(&tfhe_opts.ks_basebit)
		 -> default_value(2),
		 "?")

		("alpha_min",
		 po::value<double>(&tfhe_opts.alpha_min)
		 -> default_value(sqrt2_PI * pow(2.0, -15)),
		 "std deviation")

		("alpha_max",
		 po::value<double>(&tfhe_opts.alpha_max)
		 -> default_value(sqrt2_PI * pow(2.0, -4) / 4.0),
		 "max std deviation")

		("bk_stdev",
		 po::value<double>(&tfhe_opts.bk_stdev)
		 -> default_value(sqrt2_PI * 9.0E-9),
		 "std deviation");

	po::variables_map varmap;
	po::store(po::parse_command_line(argc, argv, desc), varmap);
	po::notify(varmap);

	if (varmap.count("help")) {
		std::cout << desc << std::endl;
		exit(1);
	}
}
