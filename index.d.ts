type TypedArray8 = Uint8Array|Int8Array|Uint8ClampedArray;
type TypedArray16 = Uint16Array|Int16Array;
type TypedArray32 = Uint32Array|Int32Array|Float32Array;
type TypedArray64 = BigInt64Array|BigUint64Array|Float64Array;

/** Swaps the bytes of the input TypedArray using the fastest available implementation. */
declare function bswap(arr: TypedArray8|TypedArray16|TypedArray32|TypedArray64): void;

export = bswap;

declare namespace bswap {
	/** The implementation used. */
	export const ise: "JS"|"SSSE3"|"AVX2"|"AVX512"|"NEON";
	/** Swaps the bytes of the input TypedArray using the JavaScript implementation. */
	export const js: typeof bswap;
	/** Swaps the bytes of the input TypedArray using the C++ implementation. */
	export const native: typeof bswap | undefined;
}
