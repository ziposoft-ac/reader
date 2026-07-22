#define NAPI_VERSION 9
#include <napi.h>
#include <cstdint>

// For GCC/Clang, __rdtsc() is available through <x86intrin.h>
#if defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#endif

Napi::Value GetRDTSC(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Read the time-stamp counter
    uint64_t cycles = __rdtsc();

    // Convert to a JS BigInt to avoid 64-bit overflow issues in JavaScript
    return Napi::BigInt::New(env, cycles);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "rdtsc"), Napi::Function::New(env, GetRDTSC));
    return exports;
}

NODE_API_MODULE(rdtsc_addon, Init)