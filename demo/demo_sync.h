#ifndef DEMO_SYNC_H_
#define DEMO_SYNC_H_

#include <napi.h>

// N-API Adapter: simple synchronous access to the `example_bfv_basics_i()` function
Napi::Value CalculateSync(const Napi::CallbackInfo &info);

#endif // DEMO_SYNC_H_
