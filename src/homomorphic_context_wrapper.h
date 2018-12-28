#ifndef HOMOMORPHIC_CONTEXT_WRAPPER_H
#define HOMOMORPHIC_CONTEXT_WRAPPER_H

#include "wrapper.h"

NAMESPACE_SEAL_WRAPPER_BEGIN

/*******************************************************************************
 * generateHomomorficContext - factory method for the fat class below
 * Hints:
 *		node-addon-examples\8_passing_wrapped
 * !!! no params
 *******************************************************************************/
	Napi::Object generateHomomorficContext(const Napi::CallbackInfo &info);

/*******************************************************************************
 * HomomorphicContextWrapper - N-API wrapper; fat class over Microsoft SEAL API:
 *    seal::EncryptionParameters         - serializable
 *    shared_ptr<seal::SEALContext>
 *    seal::IntegerEncoder
 *    seal::Plaintext                    - serializable
 *    seal::KeyGenerator
 *    seal::PublicKey                    - serializable
 *    seal::SecretKey                    - serializable
 * 
 *    seal::Ciphertext                   - serializable
 *    seal::Encryptor
 *    seal::Decryptor
 *    seal::Evaluator
 * Hints:
 *		atul\blog-addons-example\cppsrc\main.cpp
 *		node-addon-examples\8_passing_wrapped
 *******************************************************************************/
class HomomorphicContextWrapper : public Napi::ObjectWrap<HomomorphicContextWrapper>
{
private:
	static Napi::FunctionReference constructor;

public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);
	static Napi::Object NewInstance();
	//static Napi::Object NewInstance(Napi::Env env, Napi::Value arg);

public:
	HomomorphicContextWrapper(const Napi::CallbackInfo &info);

private:
	//no setter; use init ctor with EncryptionParameters
	Napi::Value getEncryptionParameters(const Napi::CallbackInfo &info);

	Napi::Value getPublicKey(const Napi::CallbackInfo &info);
	Napi::Value setPublicKey(const Napi::CallbackInfo &info);

	Napi::Value getSecretKey(const Napi::CallbackInfo &info);
	Napi::Value setSecretKey(const Napi::CallbackInfo &info);

	Napi::Value encrypt(const Napi::CallbackInfo &info);	//ACTUALLY: Plaintext IntegerEncoder::encode(int32_t)
	Napi::Value decrypt(const Napi::CallbackInfo &info);	//ACTUALLY: int32_t IntegerEncoder::decode_int32(Plaintext)
private:
	Napi::Value negate(const Napi::CallbackInfo &info);
	Napi::Value add(const Napi::CallbackInfo &info);
	Napi::Value sub(const Napi::CallbackInfo &info);
	Napi::Value multiply(const Napi::CallbackInfo &info);		//relinearization can apply here
	Napi::Value square(const Napi::CallbackInfo &info);			//relinearization can apply here
	//Napi::Value exponentiate(const Napi::CallbackInfo &info);	//we need explicit relinearization here!

public:
	std::shared_ptr<seal::EncryptionParameters>			m_EncryptionParameters;
	std::shared_ptr<std::shared_ptr<seal::SEALContext>>	m_SEALContextPtr;		//THE Cryptographic Context

	std::shared_ptr<seal::IntegerEncoder>	m_IntegerEncoder;	//do we really need it?

	std::shared_ptr<seal::KeyGenerator>		m_KeyGenerator;		//necessary only to generate the public & secret keys; //TODO: RelinKeys
	std::shared_ptr<seal::PublicKey>		m_PublicKey;
	std::shared_ptr<seal::SecretKey>		m_SecretKey;
};

NAMESPACE_SEAL_WRAPPER_END

#endif
