#include <napi.h>
#include "example_bfv_basics_i.h"  // NOLINT(build/include)
#include "sync.h"  // NOLINT(build/include)


// Simple synchronous access to the `example_bfv_basics_i()` function
 Napi::Value CalculateSync(const Napi::CallbackInfo& info) {

  Napi::Env env = info.Env();

  // check function params:
  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // expect numbers as arguments:
  int val1 = info[0].As<Napi::Number>().Uint32Value();
  int val2 = info[1].As<Napi::Number>().Uint32Value();

  //invoke the C/C++ function (SEAL high level API):
  int est = example_bfv_basics_i(val1, val2);

  //return the result
  return Napi::Number::New(info.Env(), est);
}
