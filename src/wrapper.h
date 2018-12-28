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

//you must return immediately after ThrowAsJavaScriptException() in any `catch` block:
#define SEAL_WRAPPER_CATCH_ALL(RETVAL)  catch (const Napi::Error &e) { e.ThrowAsJavaScriptException(); return (RETVAL); }\
                                        catch (const std::exception &e)	{ Napi::Error::New(env, e.what()).ThrowAsJavaScriptException(); return (RETVAL); }\
                                        catch (...) { Napi::Error::New(env, "Unknown exception").ThrowAsJavaScriptException(); return (RETVAL); }
//for ctor():
#define SEAL_WRAPPER_CATCH_ALL_RETNONE  catch (const Napi::Error &e) { e.ThrowAsJavaScriptException(); return; }\
                                        catch (const std::exception &e)	{ Napi::Error::New(env, e.what()).ThrowAsJavaScriptException(); return; }\
                                        catch (...) { Napi::Error::New(env, "Unknown exception").ThrowAsJavaScriptException(); return; }

//finally return RETVAL even if no exception has been thrown:
#define SEAL_WRAPPER_CATCH_ALL_THEN_RETURN(RETVAL)  SEAL_WRAPPER_CATCH_ALL(RETVAL); return (RETVAL);

#endif
