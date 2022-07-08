/** @typedef {Uint8Array|Int8Array|Uint8ClampedArray} TypedArray8 */
/** @typedef {Uint16Array|Int16Array} TypedArray16 */
/** @typedef {Uint32Array|Int32Array|Float32Array} TypedArray32 */
/** @typedef {BigInt64Array|BigUint64Array|Float64Array} TypedArray64 */

/**
 * The implementation used.
 * @type {"JS"|"SSSE3"|"AVX2"|"AVX512"|"NEON"}
 */
export let ise = "JS";

/** Swaps the bytes of the input TypedArray using the fastest available method. */
export let bswap = bswapJS;
/** Swaps the bytes of the input TypedArray using the JavaScript implementation. */
export const js = bswapJS;
/**
 * Swaps the bytes of the input TypedArray using the C++ implementation.
 * @type {typeof bswapJS|undefined}
 */
export let native = undefined;

try {
	const mod = {exports: {}};
	process.dlopen(mod, "./build/Release/bswap.node"/*, 2=RTLD_NOW*/);
	bswap = native = mod.exports.bswap;
	ise = mod.exports.ise;
} catch (err) {
	console.log(err);
}

/** @param {TypedArray8|TypedArray16|TypedArray32|TypedArray64} arr */
function bswapJS(arr) {
	if (ArrayBuffer.isView(arr)) {
		switch (arr.BYTES_PER_ELEMENT) {
			case 1:
				// no op
				return;
			case 2:
				return flip16(arr);
			case 4:
				return flip32(arr);
			case 8:
				return flip64(arr);
			default:
				throw new Error("Invalid input");
		}
	} else {
		throw new TypeError("expected typed array");
	}
}

/** @param {TypedArray16} */
function flip16(info) {
	const flipper = new Uint8Array(info.buffer, info.byteOffset, info.length * 2);
	const len = flipper.length;
	for (let i = 0; i < len; i += 2) {
		let t = flipper[i];
		flipper[i] = flipper[i + 1];
		flipper[i + 1] = t;
	}
}

/** @param {TypedArray32} */
function flip32(info) {
	const flipper = new Uint8Array(info.buffer, info.byteOffset, info.length * 4);
	const len = flipper.length;
	for (let i = 0; i < len; i += 4) {
		let t = flipper[i];
		flipper[i] = flipper[i + 3];
		flipper[i + 3] = t;
		t = flipper[i + 1];
		flipper[i + 1] = flipper[i + 2];
		flipper[i + 2] = t;
	}
}

/** @param {TypedArray64} */
function flip64(info) {
	const flipper = new Uint8Array(info.buffer, info.byteOffset, info.length * 8);
	const len = flipper.length;
	for (let i = 0; i < len; i += 8) {
		let t = flipper[i];
		flipper[i] = flipper[i + 7];
		flipper[i + 7] = t;
		t = flipper[i + 1];
		flipper[i + 1] = flipper[i + 6];
		flipper[i + 6] = t;
		t = flipper[i + 2];
		flipper[i + 2] = flipper[i + 5];
		flipper[i + 5] = t;
		t = flipper[i + 3];
		flipper[i + 3] = flipper[i + 4];
		flipper[i + 4] = t;
	}
}
