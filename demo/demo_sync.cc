//#include <napi.h>
#include "demo_sync.h" // NOLINT(build/include)
#include "demo_seal.h" // NOLINT(build/include)

Napi::Value CalculateSync(const Napi::CallbackInfo &info)
{

  Napi::Env env = info.Env();

  // check the input params:
  if (info.Length() < 2)
  {
    Napi::TypeError::New(env, "Wrong number of arguments!").ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[0].IsNumber() || !info[1].IsNumber())
  {
    Napi::TypeError::New(env, "Wrong argument types!").ThrowAsJavaScriptException();
    return env.Null();
  }

  // expect numbers as arguments:
  int val1 = info[0].As<Napi::Number>().Uint32Value();
  int val2 = info[1].As<Napi::Number>().Uint32Value();

  // call the C/C++ function:
  int est = example_bfv_basics_i(val1, val2); //<==HERE!!!

  // return the result:
  return Napi::Number::New(info.Env(), est);
}
