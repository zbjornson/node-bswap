#include <nan.h>
#include <stdint.h>

using namespace v8;

#if defined(__GNUC__) // GCC, clang
#ifdef __clang__
#if __clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ < 4)
#error("Requires clang >= 3.4")
#endif // clang >=3.4
#else
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#error("Requires GCC >= 4.8")
#endif // gcc >=4.8
#endif // __clang__

#include <immintrin.h>
#define BSWAP_INTRINSIC_2(x) __builtin_bswap16(x)
#define BSWAP_INTRINSIC_4(x) __builtin_bswap32(x)
#define BSWAP_INTRINSIC_8(x) __builtin_bswap64(x)

#elif defined(_MSC_VER)
#include <intrin.h>
#define BSWAP_INTRINSIC_2(x) _byteswap_ushort(x);
#define BSWAP_INTRINSIC_4(x) _byteswap_ulong(x);
#define BSWAP_INTRINSIC_8(x) _byteswap_uint64(x);

#endif

template <typename STYPE>
static inline __m128i getMask128() {
	switch (sizeof(STYPE)) {
	case 2: return _mm_setr_epi8(1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14);
	case 4: return _mm_setr_epi8(3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);
	case 8: return _mm_setr_epi8(7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8);
	default: return _mm_setzero_si128(); // squash warnings.
	}
}

#ifdef __AVX2__
template <typename STYPE>
static inline __m256i getMask256() { return _mm256_broadcastsi128_si256(getMask128<STYPE>()); }
#endif

// GCC ABI < 4 mangles vectors and prevents overloading
static inline void swap128(char* addr, __m128i mask) {
	__m128i vec = _mm_loadu_si128((__m128i*)addr);
	vec = _mm_shuffle_epi8(vec, mask);
	_mm_storeu_si128((__m128i*)addr, vec);
}

#ifdef __AVX2__
static inline void swap256(char* addr, __m256i mask) {
	__m256i vec = _mm256_loadu_si256((__m256i*)addr);
	vec = _mm256_shuffle_epi8(vec, mask);
	_mm256_storeu_si256((__m256i*)addr, vec);
}
#endif

static inline void swap(uint16_t* val) { *val = BSWAP_INTRINSIC_2(*val); }
static inline void swap(uint32_t* val) { *val = BSWAP_INTRINSIC_4(*val); }
static inline void swap(uint64_t* val) { *val = BSWAP_INTRINSIC_8(*val); }

template<typename STYPE>
static void shuffle(Local<TypedArray> data_ta) {
	Nan::TypedArrayContents<STYPE> data(data_ta);
	char* bytes = reinterpret_cast<char*>(*data);

	size_t byteLength = data_ta->ByteLength();
	size_t elemLength = byteLength / sizeof(STYPE);

	// Scalar until aligned to cache line
	size_t sIdx = 0;
	size_t preLength = ((uintptr_t)(void *)(bytes) % 64) / sizeof(STYPE);
	if (elemLength < preLength) preLength = elemLength;
	while (sIdx < preLength) swap(&(*data)[sIdx++]);

	size_t bIdx = sIdx * sizeof(STYPE);
	size_t vectLength;

#ifdef __AVX2__
	size_t vecSize = 32;
	size_t tailLength = (byteLength - bIdx) % vecSize;
	vectLength = byteLength - tailLength;

	__m256i mask = getMask256<STYPE>();
	while (bIdx < vectLength) {
		swap256(&bytes[bIdx], mask);
		bIdx += vecSize;
	}
#else
	size_t vecSize = 16;
	size_t tailLength = (byteLength - bIdx) % vecSize;
	vectLength = byteLength - tailLength;

	__m128i mask = getMask128<STYPE>();
	while (bIdx < vectLength) {
		swap128(&bytes[bIdx], mask);
		bIdx += vecSize;
	}
#endif

	sIdx = vectLength / sizeof(STYPE);
	while (sIdx < elemLength) swap(&(*data)[sIdx++]);
}

NAN_METHOD(flipBytes) {
	Local<Value> arr = info[0];
	if (arr->IsInt16Array() || arr->IsUint16Array()) {
		shuffle<uint16_t>(arr.As<TypedArray>());
	} else if (arr->IsFloat32Array() || arr->IsInt32Array() || arr->IsUint32Array()) {
		shuffle<uint32_t>(arr.As<TypedArray>());
	} else if (arr->IsFloat64Array()) {
		shuffle<uint64_t>(arr.As<TypedArray>());
	} else if (arr->IsInt8Array() || arr->IsUint8Array() || arr->IsUint8ClampedArray()) {
		// noop
	} else {
		Nan::ThrowTypeError("Expected typed array");
	}
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("flipBytes").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(flipBytes)).ToLocalChecked());
}

NODE_MODULE(bswap, Init);
