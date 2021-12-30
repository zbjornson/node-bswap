#include <nan.h>
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
static void shuffle(v8::Local<v8::TypedArray> data_ta) {
	Nan::TypedArrayContents<STYPE> data(data_ta);
	uint8_t* bytes = reinterpret_cast<uint8_t*>(*data);

	size_t byteLength = data_ta->ByteLength();
	size_t elemLength = byteLength / sizeof(STYPE);
	size_t byteIdx = 0;
	size_t elemIdx = 0;
	size_t vectSize = VTYPE::size();

	// 1. Head: Scalar until aligned.
	size_t preLength = (vectSize - ((uintptr_t)(void *)(bytes) % vectSize)) / sizeof(STYPE);
	if (elemLength < preLength) preLength = elemLength;
	while (elemIdx < preLength) swap(&(*data)[elemIdx++]);
	byteIdx = elemIdx * sizeof(STYPE);

	VTYPE mask = VTYPE::template getMask<STYPE>();

	// 2. Main body: vectors unrolled by 8
	size_t unrolledEndIdx = byteLength - ((byteLength - byteIdx) % (vectSize * 8));
	while (byteIdx < unrolledEndIdx) {
		VTYPE::swap(&bytes[byteIdx + 0 * vectSize], mask);
		VTYPE::swap(&bytes[byteIdx + 1 * vectSize], mask);
		VTYPE::swap(&bytes[byteIdx + 2 * vectSize], mask);
		VTYPE::swap(&bytes[byteIdx + 3 * vectSize], mask);
		VTYPE::swap(&bytes[byteIdx + 4 * vectSize], mask);
		VTYPE::swap(&bytes[byteIdx + 5 * vectSize], mask);
		VTYPE::swap(&bytes[byteIdx + 6 * vectSize], mask);
		VTYPE::swap(&bytes[byteIdx + 7 * vectSize], mask);
		byteIdx += 8 * vectSize;
	}

	// 3. Tail A: vectors without unrolling
	size_t vectEndIdx = byteLength - ((byteLength - byteIdx) % vectSize);
	while (byteIdx < vectEndIdx) {
		VTYPE::swap(&bytes[byteIdx], mask);
		byteIdx += vectSize;
	}

	// 4. Tail B: scalar until end
	elemIdx = byteIdx / sizeof(STYPE);
	while (elemIdx < elemLength) swap(&(*data)[elemIdx++]);
}

template <class VTYPE>
NAN_METHOD(flipBytes) {
	// consider this to warm up the wider registers:
	// asm volatile("vxorps ymm0, ymm0, ymm0" : : "ymm0")
	v8::Local<v8::Value> arr = info[0];
	if (arr->IsInt16Array() || arr->IsUint16Array()) {
		shuffle<uint16_t, VTYPE>(arr.As<v8::TypedArray>());
	} else if (arr->IsFloat32Array() || arr->IsInt32Array() || arr->IsUint32Array()) {
		shuffle<uint32_t, VTYPE>(arr.As<v8::TypedArray>());
	} else if (arr->IsFloat64Array()) {
		shuffle<uint64_t, VTYPE>(arr.As<v8::TypedArray>());
	} else if (arr->IsInt8Array() || arr->IsUint8Array() || arr->IsUint8ClampedArray()) {
		// noop
	} else {
		Nan::ThrowTypeError("Expected typed array");
	}
}

NAN_MODULE_INIT(Init) {
	v8::Local<v8::FunctionTemplate> ft;
	v8::MaybeLocal<v8::String> ise;

	// MSVC doesn't have any equivalent to -march=native, but it will emit
	// instructions from any instruction set when intrinsics are used. This lets
	// us set EnableEnhancedInstructionSet to a low/compatible value while using
	// run-time dispatch to pick a faster version.
#ifdef _MSC_VER
	// Warning: Do not put the ternary outside of the New. Performance will tank.
	ft = Nan::New<v8::FunctionTemplate>(supportsAVX2() ? flipBytes<Vec256> : flipBytes<Vec128>);
	ise = Nan::New(supportsAVX2() ? "AVX2" : "SSSE3");
#else
	// GNU-compatible compilers have -march=native, and refuse to emit
	// instructions from an instruction set less than the -m flags allow.
# if defined(__AVX512BW__) && defined(BSWAP_USE_AVX512)
	// Disabled by default because it is slower than AVX2.
	ft = Nan::New<v8::FunctionTemplate>(flipBytes<Vec512>);
	ise = Nan::New("AVX512");
# elif defined(__AVX2__)
	ft = Nan::New<v8::FunctionTemplate>(flipBytes<Vec256>);
	ise = Nan::New("AVX2");
# elif defined(__SSSE3__)
	ft = Nan::New<v8::FunctionTemplate>(flipBytes<Vec128>);
	ise = Nan::New("SSSE3");
# elif defined(__ARM_NEON)
	ft = Nan::New<v8::FunctionTemplate>(flipBytes<VecNeon>);
	ise = Nan::New("NEON");
# endif
#endif

	Nan::Set(target, Nan::New("flipBytes").ToLocalChecked(), Nan::GetFunction(ft).ToLocalChecked());
	Nan::Set(target, Nan::New("ISE").ToLocalChecked(), ise.ToLocalChecked());
}

NODE_MODULE(bswap, Init);
