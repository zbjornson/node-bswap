#ifdef __ARM_NEON

#include <arm_neon.h>

#ifndef _MSC_VER
// No MSVC support:
// https://developercommunity.visualstudio.com/content/problem/201662/arm-neonh-doenst-support-arm64-compiler.html

class VecNeon {
public:
	uint8_t bw;
	static uint8_t size() { return 16; }
	template<typename STYPE> static VecNeon getMask() {
		auto rv = VecNeon();
		rv.bw = sizeof(STYPE);
		return rv;
	}

	VecNeon() {};
	static inline void swap(uint8_t* addr, VecNeon mask) {
		uint8x16_t v = vld1q_u8(addr);
		// Sort of a hack, but the switch optimizes away.
		switch (mask.bw) {
		case 2: v = vrev16q_u8(v); break;
		case 4: v = vrev32q_u8(v); break;
		case 8: v = vrev64q_u8(v); break;
		}
		vst1q_u8(addr, v);
	}
};

#endif // _MSC_VER
#endif // __ARM_NEON
