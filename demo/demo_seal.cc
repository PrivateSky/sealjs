#include <cstdlib>
#include "demo_seal.h" // NOLINT(build/include)

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <memory>
#include <limits>

#include "seal/seal.h"

using namespace std;
using namespace seal;

//forward function declarations:
void print_example_banner(string title);
void print_parameters(shared_ptr<SEALContext> context);


int example_bfv_basics_i(int value1, int value2)
{
  print_example_banner("Example: BFV Basics I");

  EncryptionParameters parms(scheme_type::BFV);
  parms.set_poly_modulus_degree(2048);
  parms.set_coeff_modulus(coeff_modulus_128(2048));
  parms.set_plain_modulus(1 << 8);

  auto context = SEALContext::Create(parms);

  print_parameters(context);

  IntegerEncoder encoder(parms.plain_modulus());

  KeyGenerator keygen(context);
  PublicKey public_key = keygen.public_key();
  SecretKey secret_key = keygen.secret_key();

  Encryptor encryptor(context, public_key);

  Evaluator evaluator(context);

  Decryptor decryptor(context, secret_key);

  Plaintext plain1 = encoder.encode(value1);
  cout << "Encoded " << value1 << " as polynomial " << plain1.to_string()
       << " (plain1)" << endl;

  Plaintext plain2 = encoder.encode(value2);
  cout << "Encoded " << value2 << " as polynomial " << plain2.to_string()
       << " (plain2)" << endl;

  Ciphertext encrypted1, encrypted2;

  cout << "Encrypting plain1: ";
  encryptor.encrypt(plain1, encrypted1);
  cout << "Done (encrypted1)" << endl;

  cout << "Encrypting plain2: ";
  encryptor.encrypt(plain2, encrypted2);
  cout << "Done (encrypted2)" << endl;

  cout << "Noise budget in encrypted1: "
       << decryptor.invariant_noise_budget(encrypted1) << " bits" << endl;
  cout << "Noise budget in encrypted2: "
       << decryptor.invariant_noise_budget(encrypted2) << " bits" << endl;

  evaluator.negate_inplace(encrypted1);
  cout << "Noise budget in -encrypted1: "
       << decryptor.invariant_noise_budget(encrypted1) << " bits" << endl;

  evaluator.add_inplace(encrypted1, encrypted2);
  cout << "Noise budget in -encrypted1 + encrypted2: "
       << decryptor.invariant_noise_budget(encrypted1) << " bits" << endl;

  evaluator.multiply_inplace(encrypted1, encrypted2);
  cout << "Noise budget in (-encrypted1 + encrypted2) * encrypted2: "
       << decryptor.invariant_noise_budget(encrypted1) << " bits" << endl;

  Plaintext plain_result;
  cout << "Decrypting result: ";
  decryptor.decrypt(encrypted1, plain_result);
  cout << "Done" << endl;

  cout << "Plaintext polynomial: " << plain_result.to_string() << endl;

  // valRet = (-val1 + val2) * val2
  int valRet = encoder.decode_int32(plain_result);
  cout << "Decoded integer: " << encoder.decode_int32(plain_result) << endl;

  return valRet;
}


/*
Helper function: Prints the name of the example in a fancy banner.
*/
void print_example_banner(string title)
{
  if (!title.empty())
  {
    size_t title_length = title.length();
    size_t banner_length = title_length + 2 + 2 * 10;
    string banner_top(banner_length, '*');
    string banner_middle = string(10, '*') + " " + title + " " + string(10, '*');

    cout << endl
         << banner_top << endl
         << banner_middle << endl
         << banner_top << endl
         << endl;
  }
}

/*
Helper function: Prints the parameters in a SEALContext.
*/
void print_parameters(shared_ptr<SEALContext> context)
{
  // Verify parameters
  if (!context)
  {
    throw invalid_argument("context is not set");
  }
  auto &context_data = *context->context_data();

  /*
	Which scheme are we using?
	*/
  string scheme_name;
  switch (context_data.parms().scheme())
  {
  case scheme_type::BFV:
    scheme_name = "BFV";
    break;
  case scheme_type::CKKS:
    scheme_name = "CKKS";
    break;
  default:
    throw invalid_argument("unsupported scheme");
  }

  cout << "/ Encryption parameters:" << endl;
  cout << "| scheme: " << scheme_name << endl;
  cout << "| poly_modulus_degree: " << context_data.parms().poly_modulus_degree() << endl;

  /*
	Print the size of the true (product) coefficient modulus.
	*/
  cout << "| coeff_modulus size: " << context_data.total_coeff_modulus_bit_count() << " bits" << endl;

  /*
	For the BFV scheme print the plain_modulus parameter.
	*/
  if (context_data.parms().scheme() == scheme_type::BFV)
  {
    cout << "| plain_modulus: " << context_data.parms().plain_modulus().value() << endl;
  }

  cout << "\\ noise_standard_deviation: " << context_data.parms().noise_standard_deviation() << endl;
  cout << endl;
}
