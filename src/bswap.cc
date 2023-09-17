#include "napi.h"
#include <cstdint>
#include <cstddef>

///// Min compiler support and bswap intrins used in scalar region
#if defined(__GNUC__) // GCC, clang
# ifdef __clang__
#  if __clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ < 4)
#   error("Requires clang >= 3.4")
#  endif
# else
#  if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#   error("Requires GCC >= 4.8")
#  endif
# endif

# define BSWAP_INTRINSIC_2(x) __builtin_bswap16(x)
# define BSWAP_INTRINSIC_4(x) __builtin_bswap32(x)
# define BSWAP_INTRINSIC_8(x) __builtin_bswap64(x)

#elif defined(_MSC_VER)
# define BSWAP_INTRINSIC_2(x) _byteswap_ushort(x);
# define BSWAP_INTRINSIC_4(x) _byteswap_ulong(x);
# define BSWAP_INTRINSIC_8(x) _byteswap_uint64(x);
#endif // _MSC_VER

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::size_t;
using std::uintptr_t;

#include "x86.h"
#include "neon.h"

static inline void swap(uint16_t* val) { *val = BSWAP_INTRINSIC_2(*val); }
static inline void swap(uint32_t* val) { *val = BSWAP_INTRINSIC_4(*val); }
static inline void swap(uint64_t* val) { *val = BSWAP_INTRINSIC_8(*val); }

template<typename STYPE, class VTYPE>
static void shuffle(STYPE* data, size_t elemLength) {
	uint8_t* bytes = reinterpret_cast<uint8_t*>(data);

	size_t elemIdx = 0;
	constexpr size_t vectSize = VTYPE::size();

	// 1. Head: Scalar until aligned to SIMD register. Not optimized for buffers
	//    < VTYPE::size() bytes, where we could use narrower vectors than the
	//    widest supported by the ISA.
	while ((reinterpret_cast<uintptr_t>(data + elemIdx) % vectSize) && elemIdx < elemLength) {
		swap(&data[elemIdx++]);
	}

	VTYPE mask = VTYPE::template getMask<STYPE>();

	// 2. Main body: vectors unrolled by 8
	constexpr size_t elemsPerVec = vectSize / sizeof(STYPE);
	constexpr size_t elemsPerIter = 8 * elemsPerVec;
	while (elemIdx + elemsPerIter < elemLength) {
		VTYPE::swap(&data[elemIdx + 0 * elemsPerVec], mask);
		VTYPE::swap(&data[elemIdx + 1 * elemsPerVec], mask);
		VTYPE::swap(&data[elemIdx + 2 * elemsPerVec], mask);
		VTYPE::swap(&data[elemIdx + 3 * elemsPerVec], mask);
		VTYPE::swap(&data[elemIdx + 4 * elemsPerVec], mask);
		VTYPE::swap(&data[elemIdx + 5 * elemsPerVec], mask);
		VTYPE::swap(&data[elemIdx + 6 * elemsPerVec], mask);
		VTYPE::swap(&data[elemIdx + 7 * elemsPerVec], mask);
		elemIdx += elemsPerIter;
	}

	// 3. Tail A: vectors without unrolling
	while (elemIdx + elemsPerVec < elemLength) {
		VTYPE::swap(data + elemIdx, mask);
		elemIdx += vectSize / sizeof(STYPE);
	}

	// 4. Tail B: scalar until end
	while (elemIdx < elemLength) swap(&data[elemIdx++]);
}

template <class VTYPE>
void flipBytes(const Napi::CallbackInfo& info) {
	// TODO(perf): consider this to warm up the wider registers:
	// asm volatile("vxorps ymm0, ymm0, ymm0" : : "ymm0")

	if (!info[0].IsTypedArray()) {
		Napi::Error::New(info.Env(), "Expected typed array").ThrowAsJavaScriptException();
		return;
	}

	auto arr = info[0].As<Napi::TypedArray>();
	napi_typedarray_type type;
	size_t elemLength;
	void* data;
	napi_status ok = napi_get_typedarray_info(
		info.Env(), arr, &type, &elemLength, &data, nullptr, nullptr);
	if (ok != napi_ok) {
		Napi::Error::New(info.Env(), "Failed to get typed array info").ThrowAsJavaScriptException();
		return;
	}

	switch (type) {
		case napi_int16_array:
		case napi_uint16_array:
			return shuffle<uint16_t, VTYPE>(reinterpret_cast<uint16_t*>(data), elemLength);
		case napi_int32_array:
		case napi_uint32_array:
		case napi_float32_array:
			return shuffle<uint32_t, VTYPE>(reinterpret_cast<uint32_t*>(data), elemLength);
		case napi_float64_array:
#if (NAPI_VERSION > 5)
		case napi_bigint64_array:
		case napi_biguint64_array:
#endif  // (NAPI_VERSION > 5)
			return shuffle<uint64_t, VTYPE>(reinterpret_cast<uint64_t*>(data), elemLength);
		case napi_int8_array:
		case napi_uint8_array:
		case napi_uint8_clamped_array:
		default:
			return;
	}
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	Napi::Function fn;
	Napi::String ise;

	// MSVC doesn't have any equivalent to -march=native, but it will emit
	// instructions from any instruction set when intrinsics are used. This lets
	// us set EnableEnhancedInstructionSet to a low/compatible value while using
	// run-time dispatch to pick a faster version.
#ifdef _MSC_VER
	// Warning: Do not put the ternary outside of the New. Performance will tank.
# ifdef BSWAP_USE_AVX512
	fn = Napi::Function::New(env,
		supportsAVX512BW() ? flipBytes<Vec512> :
		supportsAVX2() ? flipBytes<Vec256> : flipBytes<Vec128>);
	ise = Napi::String::New(env,
		supportsAVX512BW() ? "AVX512" : supportsAVX2() ? "AVX2" : "SSSE3");
# else
	fn = Napi::Function::New(env,
		supportsAVX2() ? flipBytes<Vec256> : flipBytes<Vec128>);
	ise = Napi::String::New(env,
		supportsAVX2() ? "AVX2" : "SSSE3");
# endif // BSWAP_USE_AVX512
#else
	// GNU-compatible compilers have -march=native, and refuse to emit
	// instructions from an instruction set less than the -m flags allow.
# if defined(__AVX512BW__) && defined(BSWAP_USE_AVX512)
	// Disabled by default because it is slower than AVX2.
	fn = Napi::Function::New(env, flipBytes<Vec512>);
	ise = Napi::String::New(env, "AVX512");
# elif defined(__AVX2__)
	fn = Napi::Function::New(env, flipBytes<Vec256>);
	ise = Napi::String::New(env, "AVX2");
# elif defined(__SSSE3__)
	fn = Napi::Function::New(env, flipBytes<Vec128>);
	ise = Napi::String::New(env, "SSSE3");
# elif defined(__ARM_NEON)
	fn = Napi::Function::New(env, flipBytes<VecNeon>);
	ise = Napi::String::New(env, "NEON");
# endif
#endif

	exports.Set(Napi::String::New(env, "bswap"), fn);
	exports.Set(Napi::String::New(env, "ise"), ise);
	return exports;
}

NODE_API_MODULE(bswap, Init);
