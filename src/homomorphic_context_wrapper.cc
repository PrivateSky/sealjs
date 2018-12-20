#include "homomorphic_context_wrapper.h"
#include "wrapper_globals.h"
//#include "base64.h"

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

	auto infoLength = static_cast<unsigned int>(info.Length());
	switch (infoLength)
	{
	case 0U: //no EncryptionParameters
	{
		SEAL_WRAPPER_TRY
		{
			m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::scheme_type::BFV);
			m_EncryptionParameters->set_poly_modulus_degree(2048);
			m_EncryptionParameters->set_coeff_modulus(seal::coeff_modulus_128(2048));
			m_EncryptionParameters->set_plain_modulus(1 << 8);
		}
		SEAL_WRAPPER_CATCH_ALL
		break;
	}

	case 1U: //EncryptionParameters provided as String
	{
		if (!info[0].IsString()) //TODO: validator
			Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();

		SEAL_WRAPPER_TRY
		{
			std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
			m_EncryptionParameters = std::make_shared<seal::EncryptionParameters>(seal::EncryptionParameters::Load(iss));
		}
		SEAL_WRAPPER_CATCH_ALL
		break;
	}

	default:
		Napi::TypeError::New(env, "Wrong arguments number").ThrowAsJavaScriptException();
	} //end switch (infoLength)

	SEAL_WRAPPER_TRY
	{
		m_SEALContextPtr = std::make_shared<decltype(m_SEALContextPtr)::element_type>(seal::SEALContext::Create(*m_EncryptionParameters));
	}
	SEAL_WRAPPER_CATCH_ALL
	if (!m_SEALContextPtr) //TODO: validator
		Napi::Error::New(env, "Cannot initialize THE cryptographic context for homomorphic encryption").ThrowAsJavaScriptException();
	if (!(*m_SEALContextPtr)->parameters_set())
		Napi::Error::New(env, "Invalid encryption parameters").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		m_IntegerEncoder = std::make_shared<seal::IntegerEncoder>(m_EncryptionParameters->plain_modulus());
		m_KeyGenerator = std::make_shared<seal::KeyGenerator>(*m_SEALContextPtr);
		m_PublicKey = std::make_shared<seal::PublicKey>(m_KeyGenerator->public_key());
		m_SecretKey = std::make_shared<seal::SecretKey>(m_KeyGenerator->secret_key());
	}
	SEAL_WRAPPER_CATCH_ALL
}



/*******************************************************************************
 * HomomorphicContextWrapper::getPublicKey
 * Returns: PublicKey as JS String
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::getPublicKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 0) //TODO: validator
		Napi::TypeError::New(env, "No arguments expected").ThrowAsJavaScriptException();

	if (!m_PublicKey) //TODO: validator
		Napi::Error::New(env, "No public key has been set yet").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		m_PublicKey->save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}
/*******************************************************************************
 * HomomorphicContextWrapper::setPublicKey
 * info[0]: PublicKey as JS String
 * Returns: JS undefined
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::setPublicKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1 || !info[0].IsString())
		Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
		m_PublicKey->load(*m_SEALContextPtr, iss);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}

/*******************************************************************************
 * HomomorphicContextWrapper::getSecretKey
 * Returns: SecretKey as JS String
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::getSecretKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 0) //TODO: validator
		Napi::TypeError::New(env, "No arguments expected").ThrowAsJavaScriptException();

	if (!m_SecretKey) //TODO: validator
		Napi::Error::New(env, "No public key has been set yet").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		m_SecretKey->save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}
/*******************************************************************************
 * HomomorphicContextWrapper::setSecretKey
 * info[0]: SecretKey as JS String
 * Returns: JS undefined
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::setSecretKey(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1 || !info[0].IsString())
		Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
		m_SecretKey->load(*m_SEALContextPtr, iss);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}

/*******************************************************************************
 * HomomorphicContextWrapper::encrypt
 * info[0]: Number (BigInt is still experimental)
 * Returns: Ciphertext as JS String (base 64 encoded)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::encrypt(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1 || !info[0].IsNumber())
		Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		Napi::Number objJS = info[0].As<Napi::Number>();
		const auto inputValue = objJS.Int32Value();				//??? objJS.DoubleValue()
		const auto plaintext = m_IntegerEncoder->encode(inputValue);
		seal::Ciphertext ciphertext;
		seal::Encryptor encryptor(*m_SEALContextPtr, *m_PublicKey);
		encryptor.encrypt(plaintext, ciphertext); // ==> ciphertext

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertext.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}

/*******************************************************************************
 * HomomorphicContextWrapper::decrypt
 * info[0]: Ciphertext as JS String (base 64 encoded)
 * Returns: JS Number
*******************************************************************************/
Napi::Value HomomorphicContextWrapper::decrypt(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1 || !info[0].IsString())
		Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		seal::Ciphertext ciphertext;
		std::istringstream iss(std::move(Convert(info[0].As<Napi::String>())));
		ciphertext.load(*m_SEALContextPtr, iss); //unsafe_load()

		seal::Plaintext plaintext;
		seal::Decryptor decryptor(*m_SEALContextPtr, *m_SecretKey);
		decryptor.decrypt(ciphertext, plaintext); // ==> plaintext

		const auto outputValue = m_IntegerEncoder->decode_int32(plaintext);
		return Napi::Number::New(env, outputValue);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}



/*******************************************************************************
 * HomomorphicContextWrapper::negate
 * info[0]: Ciphertext (op #1)
 * Returns: Ciphertext (result)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::negate(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1 || !info[0].IsString())
		Napi::TypeError::New(env, "String expected - 1 parameter").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		seal::Ciphertext ciphertext1, ciphertextResult;
		std::istringstream iss1(std::move(Convert(info[0].As<Napi::String>())));
		ciphertext1.load(*m_SEALContextPtr, iss1);

		seal::Evaluator evaluator(*m_SEALContextPtr);
		evaluator.negate(ciphertext1, ciphertextResult);

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertextResult.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
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

	if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
		Napi::TypeError::New(env, "String expected - 2 parameters").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
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
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
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

	if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
		Napi::TypeError::New(env, "String expected - 2 parameters").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
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
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
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

	if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
		Napi::TypeError::New(env, "String expected - 2 parameters").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
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
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}

/*******************************************************************************
 * HomomorphicContextWrapper::square
 * info[0]: Ciphertext (op #1)
 * Returns: Ciphertext (result)
 *******************************************************************************/
Napi::Value HomomorphicContextWrapper::square(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1 || !info[0].IsString())
		Napi::TypeError::New(env, "String expected - 1 parameter").ThrowAsJavaScriptException();

	SEAL_WRAPPER_TRY
	{
		seal::Ciphertext ciphertext1, ciphertextResult;
		std::istringstream iss1(std::move(Convert(info[0].As<Napi::String>())));
		ciphertext1.load(*m_SEALContextPtr, iss1);

		seal::Evaluator evaluator(*m_SEALContextPtr);
		evaluator.square(ciphertext1, ciphertextResult);

		std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
		ciphertextResult.save(oss);
		return Convert(env, oss);
	}
	SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED
}


NAMESPACE_SEAL_WRAPPER_END
