#ifndef WRAPPER_H
#define WRAPPER_H

#include <napi.h>
#include "seal/seal.h"


#define NAMESPACE_SEAL_WRAPPER_BEGIN   namespace nsSEALWrapper {
#define NAMESPACE_SEAL_WRAPPER_END     }

/*
template <class T>
bool isFloatValue(T d)
{
    return (d - floor(d) > 0);
}
*/

#define SEAL_WRAPPER_TRY        try
#define SEAL_WRAPPER_CATCH_ALL  catch (const Napi::Error &e) { e.ThrowAsJavaScriptException(); }\
                                catch (const std::exception &e)	{ Napi::Error::New(env, e.what()).ThrowAsJavaScriptException(); }\
                                catch (...) { Napi::Error::New(env, "Unknown exception").ThrowAsJavaScriptException(); }

#define SEAL_WRAPPER_CATCH_ALL_RETURN_UNDEFINED     SEAL_WRAPPER_CATCH_ALL; return env.Undefined();


#endif
