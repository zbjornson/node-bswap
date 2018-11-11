#if defined(__x86_64__) || defined(_M_X64)

#ifdef __GNUC__
# include <immintrin.h>
#elif defined(_MSC_VER)
# include <intrin.h>
#endif

///// CPU feature detection
const static uint8_t EAX = 0;
const static uint8_t EBX = 1;
const static uint8_t ECX = 2;
const static uint8_t EDX = 3;

static bool cpuid(uint8_t outreg, uint8_t bit, uint32_t initEax, uint32_t initEcx = 0) {
	uint32_t regs[4];
#ifdef _MSC_VER
	__cpuidex(reinterpret_cast<int32_t*>(regs), initEax, initEcx);
#else
	asm volatile("cpuid"
		: "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
		: "0" (initEax), "2" (initEcx));
#endif
	return regs[outreg] & (1 << bit);
}

// __builtin_cpu_supports("avx2") for GCC 4.8+, Clang 3.7+, ICC 16+ only
static bool supportsAVX2() { return cpuid(EBX, 5, 7); }
static bool supportsAVX512BW() { return cpuid(EBX, 30, 7); }

class Vec128 {
public:
	__m128i v;
	static uint8_t size() { return 16; }
	template<typename STYPE> static Vec128 getMask() {
		switch (sizeof(STYPE)) {
		case 2: return Vec128(_mm_setr_epi8(1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14));
		case 4: return Vec128(_mm_setr_epi8(3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12));
		case 8: return Vec128(_mm_setr_epi8(7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8));
		default: return Vec128(_mm_setzero_si128()); // squash warnings.
		}
	}

	Vec128() {};
	Vec128(__m128i const & _v) : v(_v) {};
	static inline void swap(uint8_t* addr, Vec128 mask) {
		__m128i v = _mm_load_si128((__m128i*)addr);
		v = _mm_shuffle_epi8(v, mask.v);
		_mm_store_si128((__m128i*)addr, v);
	}
};

class Vec256 {
public:
	__m256i v;
	static uint8_t size() { return 32; }
	template<typename STYPE> static Vec256 getMask() {
		return Vec256(_mm256_broadcastsi128_si256(Vec128::getMask<STYPE>().v));
	}

	Vec256() {};
	Vec256(__m256i const & _v) : v(_v) {};
	static inline void swap(uint8_t* addr, Vec256 mask) {
		__m256i v = _mm256_load_si256((__m256i*)addr);
		v = _mm256_shuffle_epi8(v, mask.v);
		_mm256_store_si256((__m256i*)addr, v);
	}
};

#ifndef _MSC_VER
// v15.3 starts to have AVX-512 support, but my install doesn't have proper
// zmmintrin intrinsics.
class Vec512 {
public:
	__m512i v;
	static uint8_t size() { return 64; }
	template<typename STYPE> static Vec512 getMask() {
		return Vec512(_mm512_broadcast_i32x4(Vec128::getMask<STYPE>().v));
	}

	Vec512() {};
	Vec512(__m512i const & _v) : v(_v) {};
	static inline void swap(uint8_t* addr, Vec512 mask) {
		__m512i v = _mm512_load_si512((void*)addr);
		v = _mm512_shuffle_epi8(v, mask.v);
		_mm512_store_si512((void*)addr, v);
	}
};
#endif // _MSC_VER

#endif // __x86_64__
