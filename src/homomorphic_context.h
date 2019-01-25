#ifndef HOMOMORPHIC_CONTEXT_H_
#define HOMOMORPHIC_CONTEXT_H_

#include "seal/seal.h"

//BY DEFAULT: seal::scheme_type::BFV
class HomomorphicContext
{
public:
	//no EncryptionParameters ==> HomomorphicContext(2048, "coeff_modulus_128", 1 << 8)
	HomomorphicContext();

	//EncryptionParameters serialized as String
	HomomorphicContext(std::string);

	//Init parameters for EncryptionParameters in "BFV" scheme: poly_modulus_degree, coeff_modulus and plain_modulus
	HomomorphicContext(std::size_t poly_modulus_degree, std::string coeff_modulus, std::uint64_t plain_modulus);

private:
	void _InitContext();

public:
	std::string getEncryptionParameters();

	std::string getPublicKey();
	void setPublicKey(std::string);

	std::string getSecretKey();
	void setSecretKey(std::string);

	std::string encrypt(std::int32_t value); //ACTUALLY: Plaintext IntegerEncoder::encode(int32_t)
	std::int32_t decrypt(std::string);		 //ACTUALLY: int32_t IntegerEncoder::decode_int32(Plaintext)

	std::string negate(std::string);
	std::string add(std::string, std::string);
	std::string sub(std::string, std::string);
	std::string multiply(std::string, std::string); //relinearization can apply here
	std::string square(std::string);				//relinearization can apply here

	//std::string	exponentiate(???);				//we need explicit relinearization here!

protected:
	std::shared_ptr<seal::EncryptionParameters> m_EncryptionParameters;
	std::shared_ptr<std::shared_ptr<seal::SEALContext>> m_SEALContextPtr; //THE Cryptographic Context

	std::shared_ptr<seal::IntegerEncoder> m_IntegerEncoder; //do we really need it?

	std::shared_ptr<seal::KeyGenerator> m_KeyGenerator; //necessary only to generate the public & secret keys; //TODO: RelinKeys
	std::shared_ptr<seal::PublicKey> m_PublicKey;
	std::shared_ptr<seal::SecretKey> m_SecretKey;
};

#endif
