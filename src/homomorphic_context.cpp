#include "homomorphic_context.h"
#include "base64.h"
#include <set>
#include <map>

#include <emscripten.h>
#include "emscripten/bind.h"

/******************************************************************************/
std::istringstream Convert(std::string strBase64)
{
	const auto strSerialized = base64_decode(strBase64);
	return std::istringstream(strSerialized, std::ios_base::in | std::ios_base::binary);
}
std::string Convert(std::ostringstream const &oss)
{
	const auto strSerialized = oss.str();
	return base64_encode(reinterpret_cast<const unsigned char *>(strSerialized.c_str()), strSerialized.length());
}

/******************************************************************************/
///Construction: scheme_type is 'BFV' by default

///EncryptionParameters - default values: HomomorphicContext(2048, "coeff_modulus_128", 1 << 8)
HomomorphicContext::HomomorphicContext()
{
	m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::scheme_type::BFV);
	if (!m_EncryptionParameters)
		throw std::runtime_error("Cannot initialize EncryptionParameters");
	
	m_EncryptionParameters->set_poly_modulus_degree(2048);
	m_EncryptionParameters->set_coeff_modulus(seal::coeff_modulus_128(2048));
	m_EncryptionParameters->set_plain_modulus(1 << 8);

	_InitContext();
}

///EncryptionParameters - serialized string
HomomorphicContext::HomomorphicContext(std::string str)
{
	std::istringstream iss(std::move(Convert(str)));
	m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::EncryptionParameters::Load(iss));
	if (!m_EncryptionParameters)
		throw std::runtime_error("Cannot initialize EncryptionParameters");

	_InitContext();
}

///EncryptionParameters - initialization values
HomomorphicContext::HomomorphicContext(std::size_t poly_modulus_degree, std::string coeff_modulus, std::uint64_t plain_modulus)
{
	//available values for poly_modulus_degree:
	static std::set<size_t> s_setPMD{1024, 2048, 4096, 8192, 16384, 32768};

	//map of available values for coeff_modulus:
	static std::map<std::string, std::vector<seal::SmallModulus> (*)(size_t)> s_mapCM{
		{"coeff_modulus_128", seal::coeff_modulus_128},
		{"coeff_modulus_192", seal::coeff_modulus_192},
		{"coeff_modulus_256", seal::coeff_modulus_256},
	};

	if (s_setPMD.find(poly_modulus_degree) == s_setPMD.end())
		throw std::out_of_range("1st argument should be one of: 2^10, 2^11, ..., 2^15");

	auto strCoeffModulus = coeff_modulus;
	std::transform(strCoeffModulus.begin(), strCoeffModulus.end(), strCoeffModulus.begin(), ::tolower); //lowercase
	if (s_mapCM.find(strCoeffModulus) == s_mapCM.end())
		throw std::out_of_range("2nd argument should be one of: `coeff_modulus_128`, `coeff_modulus_192` or `coeff_modulus_256`");

	m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::scheme_type::BFV);
	if (!m_EncryptionParameters)
		throw std::runtime_error("Cannot initialize EncryptionParameters");

	m_EncryptionParameters->set_poly_modulus_degree(poly_modulus_degree);
	m_EncryptionParameters->set_coeff_modulus(s_mapCM[strCoeffModulus](poly_modulus_degree));
	m_EncryptionParameters->set_plain_modulus(plain_modulus);

	_InitContext();
}

///private helper - complete the instance initialization
void HomomorphicContext::_InitContext()
{
	m_SEALContextPtr = std::make_shared<decltype(m_SEALContextPtr)::element_type>(seal::SEALContext::Create(*m_EncryptionParameters));
	if (!m_SEALContextPtr)
		throw std::runtime_error("Cannot initialize THE cryptographic context for homomorphic encryption");
	if (!(*m_SEALContextPtr)->parameters_set())
		throw std::runtime_error("Invalid encryption parameters");

	//!!! IMPORTANT: IntegerEncoder AND FractionalEncoder ARE NOT AVAILABLE IN 'CKKS' SCHEME !!!
	m_IntegerEncoder = std::make_shared<seal::IntegerEncoder>(m_EncryptionParameters->plain_modulus());
	if (!m_IntegerEncoder)
		throw std::runtime_error("Cannot initialize IntegerEncoder");

	m_KeyGenerator = std::make_shared<seal::KeyGenerator>(*m_SEALContextPtr);
	if (!m_KeyGenerator)
		throw std::runtime_error("Cannot initialize KeyGenerator");

	m_PublicKey = std::make_shared<seal::PublicKey>(m_KeyGenerator->public_key());
	if (!m_PublicKey)
		throw std::runtime_error("Cannot initialize PublicKey");

	m_SecretKey = std::make_shared<seal::SecretKey>(m_KeyGenerator->secret_key());
	if (!m_SecretKey)
		throw std::runtime_error("Cannot initialize SecretKey");
}

/******************************************************************************/
std::string HomomorphicContext::getEncryptionParameters()
{
	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	seal::EncryptionParameters::Save(*m_EncryptionParameters, oss);
	return Convert(oss);
}

/******************************************************************************/
std::string HomomorphicContext::getPublicKey()
{
	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	m_PublicKey->save(oss);
	return Convert(oss);
}
void HomomorphicContext::setPublicKey(std::string str)
{
	std::istringstream iss(std::move(Convert(str)));
	m_PublicKey->load(*m_SEALContextPtr, iss);
}

/******************************************************************************/
std::string HomomorphicContext::getSecretKey()
{
	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	m_SecretKey->save(oss);
	return Convert(oss);
}
void HomomorphicContext::setSecretKey(std::string str)
{
	std::istringstream iss(std::move(Convert(str)));
	m_SecretKey->load(*m_SEALContextPtr, iss);
}

/******************************************************************************/
std::string HomomorphicContext::encrypt(std::int32_t value)
{
	const auto plaintext = m_IntegerEncoder->encode(value);
	seal::Ciphertext ciphertext;
	seal::Encryptor encryptor(*m_SEALContextPtr, *m_PublicKey);
	encryptor.encrypt(plaintext, ciphertext); // ==> ciphertext

	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	ciphertext.save(oss);
	return Convert(oss);
}
std::int32_t HomomorphicContext::decrypt(std::string str)
{
	try
	{
		seal::Ciphertext ciphertext;
		std::istringstream iss(std::move(Convert(str)));
		ciphertext.load(*m_SEALContextPtr, iss); //unsafe_load()

		seal::Plaintext plaintext;
		seal::Decryptor decryptor(*m_SEALContextPtr, *m_SecretKey);
		decryptor.decrypt(ciphertext, plaintext); // ==> plaintext
		return m_IntegerEncoder->decode_int32(plaintext);
	}
	catch(const std::exception& e)
	{
		EM_ASM({
			throw(UTF8ToString($0));
		}, e.what());
	}
	catch(...)
	{
		EM_ASM(throw('Unknown exception'));
	}
	return int32_t(0); //handled
}

/******************************************************************************/
std::string HomomorphicContext::negate(std::string str)
{
	seal::Ciphertext ciphertext1, ciphertextResult;
	std::istringstream iss1(std::move(Convert(str)));
	ciphertext1.load(*m_SEALContextPtr, iss1);

	seal::Evaluator evaluator(*m_SEALContextPtr);
	evaluator.negate(ciphertext1, ciphertextResult);

	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	ciphertextResult.save(oss);
	return Convert(oss);
}
/******************************************************************************/
std::string HomomorphicContext::add(std::string str1, std::string str2)
{
	seal::Ciphertext ciphertext1, ciphertext2, ciphertextResult;
	std::istringstream iss1(std::move(Convert(str1)));
	std::istringstream iss2(std::move(Convert(str2)));
	ciphertext1.load(*m_SEALContextPtr, iss1);
	ciphertext2.load(*m_SEALContextPtr, iss2);

	seal::Evaluator evaluator(*m_SEALContextPtr);
	evaluator.add(ciphertext1, ciphertext2, ciphertextResult);

	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	ciphertextResult.save(oss);
	return Convert(oss);
}
/******************************************************************************/
std::string HomomorphicContext::sub(std::string str1, std::string str2)
{
	seal::Ciphertext ciphertext1, ciphertext2, ciphertextResult;
	std::istringstream iss1(std::move(Convert(str1)));
	std::istringstream iss2(std::move(Convert(str2)));
	ciphertext1.load(*m_SEALContextPtr, iss1);
	ciphertext2.load(*m_SEALContextPtr, iss2);

	seal::Evaluator evaluator(*m_SEALContextPtr);
	evaluator.sub(ciphertext1, ciphertext2, ciphertextResult);

	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	ciphertextResult.save(oss);
	return Convert(oss);
}
/******************************************************************************/
std::string HomomorphicContext::multiply(std::string str1, std::string str2)
{
	seal::Ciphertext ciphertext1, ciphertext2, ciphertextResult;
	std::istringstream iss1(std::move(Convert(str1)));
	std::istringstream iss2(std::move(Convert(str2)));
	ciphertext1.load(*m_SEALContextPtr, iss1);
	ciphertext2.load(*m_SEALContextPtr, iss2);

	seal::Evaluator evaluator(*m_SEALContextPtr);
	evaluator.multiply(ciphertext1, ciphertext2, ciphertextResult);

	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	ciphertextResult.save(oss);
	return Convert(oss);
}
/******************************************************************************/
std::string HomomorphicContext::square(std::string str)
{
	seal::Ciphertext ciphertext1, ciphertextResult;
	std::istringstream iss1(std::move(Convert(str)));
	ciphertext1.load(*m_SEALContextPtr, iss1);

	seal::Evaluator evaluator(*m_SEALContextPtr);
	evaluator.square(ciphertext1, ciphertextResult);

	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	ciphertextResult.save(oss);
	return Convert(oss);
}

/******************************************************************************/
std::shared_ptr<HomomorphicContext> generateHomomorficContext()
{
	return std::make_shared<HomomorphicContext>();
}



/******************************************************************************/
using namespace emscripten;

EMSCRIPTEN_BINDINGS(seal_wasm_module)
{

	/// Class HomomorphicContext:
	class_<HomomorphicContext>("HomomorphicContext")
		.smart_ptr<std::shared_ptr<HomomorphicContext>>("shared_ptr<HomomorphicContext>")
		.constructor(&std::make_shared<HomomorphicContext>)
		.constructor(&std::make_shared<HomomorphicContext, std::string>)
		.constructor(&std::make_shared<HomomorphicContext, unsigned long, std::string, unsigned long>)

		.function("getEncryptionParameters", &HomomorphicContext::getEncryptionParameters)

		.function("getPublicKey", &HomomorphicContext::getPublicKey)
		.function("setPublicKey", &HomomorphicContext::setPublicKey)

		.function("getSecretKey", &HomomorphicContext::getSecretKey)
		.function("setSecretKey", &HomomorphicContext::setSecretKey)

		.function("encrypt", &HomomorphicContext::encrypt)
		.function("decrypt", &HomomorphicContext::decrypt)

		.function("negate", &HomomorphicContext::negate)
		.function("add", &HomomorphicContext::add)
		.function("sub", &HomomorphicContext::sub)
		.function("multiply", &HomomorphicContext::multiply)
		.function("square", &HomomorphicContext::square);

	/// Other classes:

	/// Other global functions:
	function("generateHomomorficContext", generateHomomorficContext);
}
