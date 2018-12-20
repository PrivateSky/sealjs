#ifndef WRAPPER_GLOBALS_H
#define WRAPPER_GLOBALS_H

#include <sstream>
#include "wrapper.h"

NAMESPACE_SEAL_WRAPPER_BEGIN

/******************************************************************************/
//global helper functions

//N-API String => std::istringstream
std::istringstream Convert(Napi::String const &objJS);

//std::ostringstream => N-API String
Napi::String Convert(Napi::Env env, std::ostringstream const &oss);


NAMESPACE_SEAL_WRAPPER_END

#endif
