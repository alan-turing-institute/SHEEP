
#include <seal/seal.h>

using namespace std;
using namespace seal;

/*
Helper function: Prints the parameters in a SEALContext.
*/
void print_parameters(const SEALContext &context) {
  cout << "/ Encryption parameters:" << endl;
  cout << "| poly_modulus: " << context.poly_modulus().to_string() << endl;

  /*
  Print the size of the true (product) coefficient modulus
  */
  cout << "| coeff_modulus size: "
       << context.total_coeff_modulus().significant_bit_count() << " bits"
       << endl;

  cout << "| plain_modulus: " << context.plain_modulus().value() << endl;
  cout << "\\ noise_standard_deviation: " << context.noise_standard_deviation()
       << endl;
  cout << endl;
}

void run_chain_test(const string op, size_t depth, SEALContext context) {
  KeyGenerator keygen(context);
  PublicKey public_key = keygen.public_key();
  SecretKey secret_key = keygen.secret_key();

  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);
  IntegerEncoder encoder(context.plain_modulus());
  cout << "Encrypting " << depth << " values...";
  vector<Ciphertext> inputs(depth);
  for (size_t i = 0; i < depth; ++i) {
    encryptor.encrypt(encoder.encode(2), inputs[i]);
  }
  cout << "Done" << endl;
  cout << "Computing " << depth << " nested " << op << "s on encrypted data...";
  Ciphertext c;
  for (size_t i = 0; i < depth - 1; ++i) {
    Ciphertext c1, c2;
    if (i == 0) {
      c1 = inputs[0];
      c2 = inputs[1];
    } else {
      c1 = c;
      c2 = inputs[i + 1];
    }
    if (op == "MULTIPLY") {
      evaluator.multiply(c1, c2, c);
    } else if (op == "ADD") {
      evaluator.add(c1, c2, c);
    }
  }
  cout << "Done" << endl;
  Plaintext result;
  cout << "Decrypting...";
  decryptor.decrypt(c, result);
  cout << "Done" << endl;
  cout << "Result = " << encoder.decode_int32(result) << endl;
}

void custom_context_nested_op(const string op, size_t depth,
                              const string poly_modulus,
                              uint64_t plain_modulus) {
  EncryptionParameters parms;
  parms.set_poly_modulus(poly_modulus);
  parms.set_coeff_modulus(
      coeff_modulus_128(parms.poly_modulus().coeff_count() - 1));
  parms.set_plain_modulus(plain_modulus);
  SEALContext context(parms);
  print_parameters(context);
  run_chain_test(op, depth, context);
}

void parameters_selection_nested_op(const string op, size_t depth) {
  ChooserEncoder chooser_encoder(3);
  ChooserEvaluator chooser_evaluator;
  vector<ChooserPoly> c_inputs;
  for (size_t i = 0; i < depth; ++i) {
    ChooserPoly c_input(10, 1);
    c_inputs.push_back(c_input);
  }
  ChooserPoly t;
  for (size_t i = 0; i < depth - 1; ++i) {
    ChooserPoly p1, p2;
    if (i == 0) {
      p1 = c_inputs[0];
      p2 = c_inputs[1];
    } else {
      p1 = t;
      p2 = c_inputs[i + 1];
    }
    if (op == "MULTIPLY") {
      t = chooser_evaluator.multiply(p1, p2);
    } else if (op == "ADD") {
      t = chooser_evaluator.add(p1, p2);
    }
  }
  EncryptionParameters optimal_parms;
  chooser_evaluator.select_parameters({t}, 0, optimal_parms);
  cout << "Done" << endl;
  print_parameters(optimal_parms);

  SEALContext optimal_context(optimal_parms);
  run_chain_test(op, depth, optimal_context);
}

void example_parameter_selection() {
  /*
  SEAL contains an automatic parameter selection tool that can help the user
  select optimal parameters that support a particular computation. In this
  example we show how the tool can be used to find parameters for evaluating
  the degree 3 polynomial 42x^3-27x+1 on an encrypted input encoded with the
  IntegerEncoder. For this to be possible, we need to know an upper bound on
  the size of the input, and in this example assume that x is an integer with
  base-3 representation of length at most 10.
  */
  cout << "Finding optimized parameters for computing 42x^3-27x+1: ";

  /*
  The set of tools in the parameter selector are ChooserPoly, ChooserEvaluator,
  ChooserEncoder, ChooserEncryptor, and ChooserDecryptor. Of these the most
  important ones are ChooserPoly, which is an object representing the input
  data both in plaintext and encrypted form, and ChooserEvaluator, which
  simulates plaintext coefficient growth and noise budget consumption in the
  computations. Here we use also the ChooserEncoder to conveniently obtain
  ChooserPoly objects modeling the plaintext coefficients 42, -27, and 1.

  Note that we are using the IntegerEncoder with base 3.
  */
  ChooserEncoder chooser_encoder(3);
  ChooserEvaluator chooser_evaluator;

  /*
  First we create a ChooserPoly representing the input data. You can think of
  this modeling a freshly encrypted ciphertext of a plaintext polynomial of
  length at most 10 coefficients, where the coefficients have absolute value
  at most 1 (as is the case when using IntegerEncoder with base 3).
  */
  ChooserPoly c_input(10, 1);

  /*
  Normally Evaluator::exponentiate takes the evaluation keys as argument. Since
  no keys exist here, we simply pass the desired decomposition bit count (15)
  to the ChooserEvaluator::exponentiate function.

  Here we compute the first term.
  */
  ChooserPoly c_cubed_input = chooser_evaluator.exponentiate(c_input, 3, 15);
  ChooserPoly c_term1 = chooser_evaluator.multiply_plain(
      c_cubed_input, chooser_encoder.encode(42));

  /*
  Then compute the second term.
  */
  ChooserPoly c_term2 =
      chooser_evaluator.multiply_plain(c_input, chooser_encoder.encode(27));

  /*
  Subtract the first two terms.
  */
  ChooserPoly c_sum12 = chooser_evaluator.sub(c_term1, c_term2);

  /*
  Finally add a plaintext constant 1.
  */
  ChooserPoly c_result =
      chooser_evaluator.add_plain(c_sum12, chooser_encoder.encode(1));

  /*
  The optimal parameters are now computed using the select_parameters
  function in ChooserEvaluator. It is possible to give this function the
  results of several distinct computations (as ChooserPoly objects), all
  of which are supposed to be possible to perform with the resulting set
  of parameters. However, here we have only one input ChooserPoly.
  */
  EncryptionParameters optimal_parms;
  chooser_evaluator.select_parameters({c_result}, 0, optimal_parms);
  cout << "Done" << endl;

  /*
  Create an SEALContext object for the returned parameters
  */
  SEALContext optimal_context(optimal_parms);
  print_parameters(optimal_parms);

  /*
  Do the parameters actually make any sense? We can try to perform the
  homomorphic computation using the given parameters and see what happens.
  */
  KeyGenerator keygen(optimal_context);
  PublicKey public_key = keygen.public_key();
  SecretKey secret_key = keygen.secret_key();
  EvaluationKeys ev_keys;
  keygen.generate_evaluation_keys(15, ev_keys);

  Encryptor encryptor(optimal_context, public_key);
  Evaluator evaluator(optimal_context);
  Decryptor decryptor(optimal_context, secret_key);
  IntegerEncoder encoder(optimal_context.plain_modulus(), 3);

  /*
  Now perform the computations on some real data.
  */
  int input_value = 12345;
  Plaintext plain_input = encoder.encode(input_value);
  cout << "Encoded " << input_value << " as polynomial "
       << plain_input.to_string() << endl;

  Ciphertext input;
  cout << "Encrypting: ";
  encryptor.encrypt(plain_input, input);
  cout << "Done" << endl;

  cout << "Computing 42x^3-27x+1 on encrypted x=12345: ";
  Ciphertext deg3_term;
  evaluator.exponentiate(input, 3, ev_keys, deg3_term);
  evaluator.multiply_plain(deg3_term, encoder.encode(42));
  Ciphertext deg1_term;
  evaluator.multiply_plain(input, encoder.encode(27), deg1_term);
  evaluator.sub(deg3_term, deg1_term);
  evaluator.add_plain(deg3_term, encoder.encode(1));
  cout << "Done" << endl;

  /*
  Now deg3_term holds the result. We decrypt, decode, and print the result.
  */
  Plaintext plain_result;
  cout << "Decrypting: ";
  decryptor.decrypt(deg3_term, plain_result);
  cout << "Done" << endl;
  cout << "Polynomial 42x^3-27x+1 evaluated at x=12345: "
       << encoder.decode_int64(plain_result) << endl;

  /*
  We should have a reasonable amount of noise room left if the parameter
  selection was done properly. The user can experiment for instance by
  changing the decomposition bit count, and observing how it affects the
  result. Typically the budget should never be even close to 0. Instead,
  SEAL uses heuristic upper bound estimates on the noise budget consumption,
  which ensures that the computation will succeed with very high probability
  with the selected parameters.
  */
  cout << "Noise budget in result: "
       << decryptor.invariant_noise_budget(deg3_term) << " bits" << endl;
}

int main() {
  // example_parameter_selection();
  int depth = 8;
  const string op = "MULTIPLY";
  parameters_selection_nested_op(op, depth);
  custom_context_nested_op(op, depth, "1x^16384 + 1", (1ll << 32));

  // parameters_selection_nested_op("ADD", 2);
}