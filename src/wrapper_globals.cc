#include "wrapper.h"
#include "base64.h"

NAMESPACE_SEAL_WRAPPER_BEGIN

/******************************************************************************/
std::istringstream Convert(Napi::String const &objJS)
{
	const auto strBase64 = (std::string)objJS;
	const auto strPersistent = base64_decode(strBase64);
	return std::istringstream(strPersistent, std::ios_base::in | std::ios_base::binary);
}

/******************************************************************************/
Napi::String Convert(Napi::Env env, std::ostringstream const &oss)
{
	const auto strPersistent = oss.str();
	const auto strBase64 = base64_encode(reinterpret_cast<const unsigned char *>(strPersistent.c_str()), strPersistent.length());
	return Napi::String::New(env, strBase64);
}

NAMESPACE_SEAL_WRAPPER_END