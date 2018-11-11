/**
 * Swaps the bytes of the input TypedArray using the fastest available method.
 */
export function bswap<T extends ArrayBufferView>(input: T): T;

export namespace bswap {
	/**
	 * The best available instruction set extension used. (C++ version only.)
	 */
	export const ISE: "SSSE3"|"AVX2"|"AVX512"|"NEON";
	/** The C++ implementation. */
	export function native<T extends ArrayBufferView>(input: T): T;
	/** The JavaScript implementation. */
	export function js<T extends ArrayBufferView>(input: T): T;
}

export = bswap;
