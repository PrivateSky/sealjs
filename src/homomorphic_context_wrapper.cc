#include "homomorphic_context_wrapper.h"
#include "wrapper_globals.h"
#include <set>

NAMESPACE_SEAL_WRAPPER_BEGIN

/*******************************************************************************
 * generateHomomorficContext - factory method for HomomorphicContextWrapper
 *******************************************************************************/
Napi::Object generateHomomorficContext(const Napi::CallbackInfo &info)
{
	return HomomorphicContextWrapper::NewInstance(); //!!! no params
}

////////////////////////////////////////////////////////////////////////////////
//class HomomorphicContextWrapper

/*static*/
Napi::FunctionReference HomomorphicContextWrapper::constructor;

/*******************************************************************************
 * HomomorphicContextWrapper::Init - N-API specific
 *******************************************************************************/
/*static*/
Napi::Object HomomorphicContextWrapper::Init(Napi::Env env, Napi::Object exports)
{
	Napi::HandleScope scope(env);

	Napi::Function func = DefineClass(env, "HomomorphicContextWrapper", {
		InstanceMethod("getEncryptionParameters", &HomomorphicContextWrapper::getEncryptionParameters), //no setter

		InstanceMethod("getPublicKey", &HomomorphicContextWrapper::getPublicKey),
		InstanceMethod("setPublicKey", &HomomorphicContextWrapper::setPublicKey),
		InstanceMethod("getSecretKey", &HomomorphicContextWrapper::getSecretKey),
		InstanceMethod("setSecretKey", &HomomorphicContextWrapper::setSecretKey),
		
		InstanceMethod("encrypt", &HomomorphicContextWrapper::encrypt),
		InstanceMethod("decrypt", &HomomorphicContextWrapper::decrypt),

		InstanceMethod("negate", &HomomorphicContextWrapper::negate),
		InstanceMethod("add", &HomomorphicContextWrapper::add),
		InstanceMethod("sub", &HomomorphicContextWrapper::sub),
		InstanceMethod("multiply", &HomomorphicContextWrapper::multiply),
		InstanceMethod("square", &HomomorphicContextWrapper::square)
	});

	constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

	exports.Set("HomomorphicContext", func);

	return exports;
}

/*******************************************************************************
 * HomomorphicContextWrapper::NewInstance
 *******************************************************************************/
/*static*/
Napi::Object HomomorphicContextWrapper::NewInstance()
{
	Napi::Object objWrapped = constructor.New({}); //!!! no params
	return objWrapped;
}

/*******************************************************************************
 * HomomorphicContextWrapper::HomomorphicContextWrapper - ctor
 *******************************************************************************/
HomomorphicContextWrapper::HomomorphicContextWrapper(const Napi::CallbackInfo &info)
	: Napi::ObjectWrap<HomomorphicContextWrapper>(info)
{
	Napi::Env env = info.Env();
	Napi::HandleScope scope(env);

	static std::set<size_t> s_setPMD{1024, 2048, 4096, 8192, 16384, 32768}; //available values for poly_modulus_degree
	static std::map<std::string, std::vector<seal::SmallModulus> (*)(size_t)> s_mapCM{
		{"coeff_modulus_128", seal::coeff_modulus_128},
		{"coeff_modulus_192", seal::coeff_modulus_192},
		{"coeff_modulus_256", seal::coeff_modulus_256},
	};

	SEAL_WRAPPER_TRY
	{

		auto infoLength = static_cast<unsigned int>(info.Length());
		switch (infoLength)
		{
		case 0U: //no EncryptionParameters
		{
			m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::scheme_type::BFV);
			m_EncryptionParameters->set_poly_modulus_degree(2048);
			m_EncryptionParameters->set_coeff_modulus(seal::coeff_modulus_128(2048));
			m_EncryptionParameters->set_plain_modulus(1 << 8);
		}
		break;

		case 1U: //EncryptionParameters serialized as String
		{
			if (!info[0].IsString()) //TODO: validator
				// { Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException(); return; } //MUST RETURN!!!
				throw Napi::TypeError::New(env, "String expected");

			std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
			m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::EncryptionParameters::Load(iss));
		}
		break;

		case 3U: //Init parameters for EncryptionParameters in "BFV" scheme: poly_modulus_degree, coeff_modulus and plain_modulus
		{
			if (!info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber()) //TODO: validator
				throw Napi::TypeError::New(env, "(Number, String, Number) expected for BFV scheme");

			const auto poly_modulus_degree = static_cast<size_t>(info[0].As<Napi::Number>().Uint32Value());
			auto strCoeffModulus = (std::string)info[1].As<Napi::String>();
			const auto plain_modulus = info[2].As<Napi::Number>().Int64Value(); //Number::Uint64Value() -- no such a method?

			if (s_setPMD.find(poly_modulus_degree) == s_setPMD.end())
				throw Napi::RangeError::New(env, "1st argument should be one of: 2^10, 2^11, ..., 2^15");

			std::transform(strCoeffModulus.begin(), strCoeffModulus.end(), strCoeffModulus.begin(), ::tolower); //lowercase
			if (s_mapCM.find(strCoeffModulus) == s_mapCM.end())
				throw Napi::RangeError::New(env, "2nd argument should be one of: 'coeff_modulus_128', 'coeff_modulus_192' or 'coeff_modulus_256'");

			//scheme_type: 'BFV' by default
			m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::scheme_type::BFV);
			m_EncryptionParameters->set_poly_modulus_degree(poly_modulus_degree);
			m_EncryptionParameters->set_coeff_modulus(s_mapCM[strCoeffModulus](poly_modulus_degree));
			m_EncryptionParameters->set_plain_modulus(plain_modulus);
		}
		break;

		default:
			throw Napi::TypeError::New(env, "Wrong arguments number");
		} //end switch (infoLength)

		m_SEALContextPtr = std::make_shared<decltype(m_SEALContextPtr)::element_type>(seal::SEALContext::Create(*m_EncryptionParameters));
		if (!m_SEALContextPtr) //TODO: validator
			throw Napi::Error::New(env, "Cannot initialize THE cryptographic context for homomorphic encryption");
		if (!(*m_SEALContextPtr)->parameters_set())
			throw Napi::Error::New(env, "Invalid encryption parameters");

		//!!! IMPORTANT: IntegerEncoder AND FractionalEncoder ARE NOT AVAILABLE IN 'CKKS' SCHEME !!!
		m_IntegerEncoder = std::make_shared<seal::IntegerEncoder>(m_EncryptionParameters->plain_modulus());

		m_KeyGenerator = std::make_shared<seal::KeyGenerator>(*m_SEALContextPtr);
		m_PublicKey = std::make_shared<seal::PublicKey>(m_KeyGenerator->public_key());
		m_SecretKey = std::make_shared<seal::SecretKey>(m_KeyGenerator->secret_key());
	}
	SEAL_WRAPPER_CATCH_ALL_RETNONE; //ctor returns nothing
}

/*******************************************************************************
 * HomomorphicContextWrapper::getEncryptionParameters
 * Returns: EncryptionParameters as JS String
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::getEncryptionParameters(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 0) //TODO: validator
			throw Napi::TypeError::New(env, "No arguments expected");

		if (!m_EncryptionParameters) //TODO: validator
			throw Napi::Error::New(env, "No EncryptionParameters have been set yet");

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		seal::EncryptionParameters::Save(*m_EncryptionParameters, oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::getPublicKey
 * Returns: PublicKey as JS String
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::getPublicKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 0) //TODO: validator
			throw Napi::TypeError::New(env, "No arguments expected");

		if (!m_PublicKey) //TODO: validator
			throw Napi::Error::New(env, "No public key has been set yet");

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		m_PublicKey->save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}
/*******************************************************************************
 * HomomorphicContextWrapper::setPublicKey
 * info[0]: PublicKey as JS String
 * Returns: JS undefined
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::setPublicKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 1 || !info[0].IsString())
			throw Napi::TypeError::New(env, "String expected");

		std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
		m_PublicKey->load(*m_SEALContextPtr, iss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::getSecretKey
 * Returns: SecretKey as JS String
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::getSecretKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 0) //TODO: validator
			throw Napi::TypeError::New(env, "No arguments expected");

		if (!m_SecretKey) //TODO: validator
			throw Napi::Error::New(env, "No secret key has been set yet");

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		m_SecretKey->save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}
/*******************************************************************************
 * HomomorphicContextWrapper::setSecretKey
 * info[0]: SecretKey as JS String
 * Returns: JS undefined
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::setSecretKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 1 || !info[0].IsString())
			throw Napi::TypeError::New(env, "String expected");

		std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
		m_SecretKey->load(*m_SEALContextPtr, iss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::encrypt
 * info[0]: Number (BigInt is still experimental)
 * Returns: Ciphertext as JS String (base 64 encoded)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::encrypt(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 1 || !info[0].IsNumber())
			throw Napi::TypeError::New(env, "Number expected");

		Napi::Number objJS = info[0].As<Napi::Number>();
		const auto inputValue = objJS.Int32Value(); //??? objJS.DoubleValue()
		const auto plaintext = m_IntegerEncoder->encode(inputValue);
		seal::Ciphertext ciphertext;
		seal::Encryptor encryptor(*m_SEALContextPtr, *m_PublicKey);
		encryptor.encrypt(plaintext, ciphertext); // ==> ciphertext

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertext.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}
/*******************************************************************************
 * HomomorphicContextWrapper::decrypt
 * info[0]: Ciphertext as JS String (base 64 encoded)
 * Returns: JS Number
*******************************************************************************/
Napi::Value HomomorphicContextWrapper::decrypt(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 1 || !info[0].IsString())
			throw Napi::TypeError::New(env, "String expected");

		seal::Ciphertext ciphertext;
		std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
		ciphertext.load(*m_SEALContextPtr, iss); //unsafe_load()

		seal::Plaintext plaintext;
		seal::Decryptor decryptor(*m_SEALContextPtr, *m_SecretKey);
		decryptor.decrypt(ciphertext, plaintext); // ==> plaintext

		const auto outputValue = m_IntegerEncoder->decode_int32(plaintext);
		return Napi::Number::New(env, outputValue);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::negate
 * info[0]: Ciphertext (op #1)
 * Returns: Ciphertext (result)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::negate(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 1 || !info[0].IsString())
			throw Napi::TypeError::New(env, "String expected - 1 parameter");

		seal::Ciphertext ciphertext1, ciphertextResult;
		std::istringstream iss1(std::move(Convert(info[0].As<Napi::String>())));
		ciphertext1.load(*m_SEALContextPtr, iss1);

		seal::Evaluator evaluator(*m_SEALContextPtr);
		evaluator.negate(ciphertext1, ciphertextResult);

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertextResult.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::add
 * info[0]: Ciphertext (op #1)
 * info[1]: Ciphertext (op #2)
 * Returns: Ciphertext (result)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::add(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
			throw Napi::TypeError::New(env, "String expected - 2 parameters");

		seal::Ciphertext ciphertext1, ciphertext2, ciphertextResult;
		std::istringstream iss1(std::move(Convert(info[0].As<Napi::String>())));
		std::istringstream iss2(std::move(Convert(info[1].As<Napi::String>())));
		ciphertext1.load(*m_SEALContextPtr, iss1);
		ciphertext2.load(*m_SEALContextPtr, iss2);

		seal::Evaluator evaluator(*m_SEALContextPtr);
		evaluator.add(ciphertext1, ciphertext2, ciphertextResult);

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertextResult.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::sub
 * info[0]: Ciphertext (op #1)
 * info[1]: Ciphertext (op #2)
 * Returns: Ciphertext (result)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::sub(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
			throw Napi::TypeError::New(env, "String expected - 2 parameters");

		seal::Ciphertext ciphertext1, ciphertext2, ciphertextResult;
		std::istringstream iss1(std::move(Convert(info[0].As<Napi::String>())));
		std::istringstream iss2(std::move(Convert(info[1].As<Napi::String>())));
		ciphertext1.load(*m_SEALContextPtr, iss1);
		ciphertext2.load(*m_SEALContextPtr, iss2);

		seal::Evaluator evaluator(*m_SEALContextPtr);
		evaluator.sub(ciphertext1, ciphertext2, ciphertextResult);

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertextResult.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::multiply
 * info[0]: Ciphertext (op #1)
 * info[1]: Ciphertext (op #2)
 * Returns: Ciphertext (result)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::multiply(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
			throw Napi::TypeError::New(env, "String expected - 2 parameters");

		seal::Ciphertext ciphertext1, ciphertext2, ciphertextResult;
		std::istringstream iss1(std::move(Convert(info[0].As<Napi::String>())));
		std::istringstream iss2(std::move(Convert(info[1].As<Napi::String>())));
		ciphertext1.load(*m_SEALContextPtr, iss1);
		ciphertext2.load(*m_SEALContextPtr, iss2);

		seal::Evaluator evaluator(*m_SEALContextPtr);
		evaluator.multiply(ciphertext1, ciphertext2, ciphertextResult);

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertextResult.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

/*******************************************************************************
 * HomomorphicContextWrapper::square
 * info[0]: Ciphertext (op #1)
 * Returns: Ciphertext (result)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::square(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	SEAL_WRAPPER_TRY
	{
		if (info.Length() != 1 || !info[0].IsString())
			throw Napi::TypeError::New(env, "String expected - 1 parameter");

		seal::Ciphertext ciphertext1, ciphertextResult;
		std::istringstream iss1(std::move(Convert(info[0].As<Napi::String>())));
		ciphertext1.load(*m_SEALContextPtr, iss1);

		seal::Evaluator evaluator(*m_SEALContextPtr);
		evaluator.square(ciphertext1, ciphertextResult);

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertextResult.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(env.Undefined());
}

NAMESPACE_SEAL_WRAPPER_END
