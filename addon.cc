#include <napi.h>
#include "sync.h"   // NOLINT(build/include)


/**
  Expose synchronous access to our function (from SEAL demo):
    int example_bfv_basics_i(int, int)
  
  Hints:
    node-addon-examples\2_function_arguments
	node-addon-examples\async_pi_estimate
*/
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "calculateSync"),
              Napi::Function::New(env, CalculateSync));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
