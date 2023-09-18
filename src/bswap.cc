#include "node_api.h"
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
napi_value flipBytes(napi_env env, napi_callback_info info) {
	// TODO(perf): consider this to warm up the wider registers:
	// asm volatile("vxorps ymm0, ymm0, ymm0" : : "ymm0")

	napi_status status;

	napi_value args[1];
	size_t argc = 1;
	status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
	if (status != napi_ok) goto error;

	if (argc < 1) {
		napi_throw_error(env, NULL, "Expected typed array");
		return nullptr;
	}

	bool isTypedArray;
	status = napi_is_typedarray(env, args[0], &isTypedArray);
	if (status != napi_ok) goto error;
	if (!isTypedArray) {
		napi_throw_error(env, NULL, "Expected typed array");
		return nullptr;
	}

	napi_typedarray_type type;
	size_t elemLength;
	void* data;
	status = napi_get_typedarray_info(env, args[0], &type, &elemLength, &data, nullptr, nullptr);
	if (status != napi_ok) goto error;

	switch (type) {
		case napi_int16_array:
		case napi_uint16_array:
			shuffle<uint16_t, VTYPE>(reinterpret_cast<uint16_t*>(data), elemLength);
			return nullptr;
		case napi_int32_array:
		case napi_uint32_array:
		case napi_float32_array:
			shuffle<uint32_t, VTYPE>(reinterpret_cast<uint32_t*>(data), elemLength);
			return nullptr;
		case napi_float64_array:
#if (NAPI_VERSION > 5)
		case napi_bigint64_array:
		case napi_biguint64_array:
#endif // (NAPI_VERSION > 5)
			shuffle<uint64_t, VTYPE>(reinterpret_cast<uint64_t*>(data), elemLength);
			return nullptr;
		case napi_int8_array:
		case napi_uint8_array:
		case napi_uint8_clamped_array:
		default:
			return nullptr;
	}

	error:
	const napi_extended_error_info* error_info = nullptr;
	napi_get_last_error_info(env, &error_info);
	const char* err_message = error_info->error_message;
	bool is_pending;
	napi_is_exception_pending(env, &is_pending);
	if (!is_pending) {
		const char* message = err_message == nullptr ? "empty error message" : err_message;
		napi_throw_error(env, nullptr, message);
	}
	return nullptr;
}

#define NAPI_CALL(env, call)                                      \
  do {                                                            \
    napi_status status = (call);                                  \
    if (status != napi_ok) {                                      \
      const napi_extended_error_info* error_info = NULL;          \
      napi_get_last_error_info((env), &error_info);               \
      const char* err_message = error_info->error_message;        \
      bool is_pending;                                            \
      napi_is_exception_pending((env), &is_pending);              \
      if (!is_pending) {                                          \
        const char* message = (err_message == NULL)               \
            ? "empty error message"                               \
            : err_message;                                        \
        napi_throw_error((env), NULL, message);                   \
        return NULL;                                              \
      }                                                           \
    }                                                             \
  } while(0)

napi_value Init(napi_env env, napi_value exports) {
	napi_value fn;
	napi_value ise;

	// MSVC doesn't have any equivalent to -march=native, but it will emit
	// instructions from any instruction set when intrinsics are used. This lets
	// us set EnableEnhancedInstructionSet to a low/compatible value while using
	// run-time dispatch to pick a faster version.
#ifdef _MSC_VER
	// Warning: Do not put the ternary outside of the New. Performance will tank.
# ifdef BSWAP_USE_AVX512
	NAPI_CALL(env, napi_create_function(env, "bswap", NAPI_AUTO_LENGTH,
		supportsAVX512BW() ? flipBytes<Vec512> :
		supportsAVX2() ? flipBytes<Vec256> : flipBytes<Vec128>, NULL, &fn));
	NAPI_CALL(env, napi_create_string_latin1(env,
		supportsAVX512BW() ? "AVX512" : supportsAVX2() ? "AVX2" : "SSSE3",
		NAPI_AUTO_LENGTH, &ise));
# else
	NAPI_CALL(env, napi_create_function(env, "bswap", NAPI_AUTO_LENGTH,
		supportsAVX2() ? flipBytes<Vec256> : flipBytes<Vec128>, NULL, &fn));
	NAPI_CALL(env, napi_create_string_latin1(env, supportsAVX2() ? "AVX2" : "SSSE3", NAPI_AUTO_LENGTH, &ise));
# endif // BSWAP_USE_AVX512
#else
	// GNU-compatible compilers have -march=native, and refuse to emit
	// instructions from an instruction set less than the -m flags allow.
# if defined(__AVX512BW__) && defined(BSWAP_USE_AVX512)
	// Disabled by default because it is slower than AVX2.
	NAPI_CALL(env, napi_create_function(env, "bswap", NAPI_AUTO_LENGTH, flipBytes<Vec512>, NULL, &fn));
	NAPI_CALL(env, napi_create_string_latin1(env, "AVX512", NAPI_AUTO_LENGTH, &ise));
# elif defined(__AVX2__)
	NAPI_CALL(env, napi_create_function(env, "bswap", NAPI_AUTO_LENGTH, flipBytes<Vec256>, NULL, &fn));
	NAPI_CALL(env, napi_create_string_latin1(env, "AVX2", NAPI_AUTO_LENGTH, &ise));
# elif defined(__SSSE3__)
	NAPI_CALL(env, napi_create_function(env, "bswap", NAPI_AUTO_LENGTH, flipBytes<Vec128>, NULL, &fn));
	NAPI_CALL(env, napi_create_string_latin1(env, "SSSE3", NAPI_AUTO_LENGTH, &ise));
# elif defined(__ARM_NEON)
	NAPI_CALL(env, napi_create_function(env, "bswap", NAPI_AUTO_LENGTH, flipBytes<VecNeon>, NULL, &fn));
	NAPI_CALL(env, napi_create_string_latin1(env, "NEON", NAPI_AUTO_LENGTH, &ise));
# endif
#endif

	NAPI_CALL(env, napi_set_named_property(env, exports, "bswap", fn));
	NAPI_CALL(env, napi_set_named_property(env, exports, "ise", ise));

	return exports;
}

NAPI_MODULE(bswap, Init)
