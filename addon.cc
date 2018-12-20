#include <napi.h>

#include "demo/demo_sync.h"   // NOLINT(build/include)

/*
#include "src/seal_context_wrapper.h"
#include "src/encryption_parameters_wrapper.h"
#include "src/integer_encoder_wrapper.h"
#include "src/plaintext_wrapper.h"
#include "src/key_generator_wrapper.h"
#include "src/public_key_wrapper.h"
#include "src/secret_key_wrapper.h"
*/
#include "src/homomorphic_context_wrapper.h"

//using namespace nsSEALWrapper;

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
	//export for 'demo' script:
	exports.Set(Napi::String::New(env, "calculateSync"),
				Napi::Function::New(env, CalculateSync));


	//nsSEALWrapper specific exports:
	/*
	exports.Set(Napi::String::New(env, "generateSEALContext"),
				Napi::Function::New(env, nsSEALWrapper::generateSEALContextWrapper)); //factory
	nsSEALWrapper::SEALContextWrapper::Init(env, exports); //class

	nsSEALWrapper::EncryptionParametersWrapper	::Init(env, exports);
	nsSEALWrapper::IntegerEncoderWrapper		::Init(env, exports);
	nsSEALWrapper::PlaintextWrapper				::Init(env, exports);
	nsSEALWrapper::KeyGeneratorWrapper			::Init(env, exports);
	nsSEALWrapper::PublicKeyWrapper				::Init(env, exports);
	nsSEALWrapper::SecretKeyWrapper				::Init(env, exports);
	*/

	//the light JS version
	exports.Set(Napi::String::New(env, "generateHomomorficContext"),
				Napi::Function::New(env, nsSEALWrapper::generateHomomorficContext)); //factory
	nsSEALWrapper::HomomorphicContextWrapper::Init(env, exports);

	return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
