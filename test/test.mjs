import assert from "assert";
import * as bswap from "../bswap.mjs";
let bswapFn;

const tests = [
	{label: "Flips Int16Array", test: function () {
		const x = new Uint32Array(258).fill(0x04030201);
		const expect = new Uint32Array(258).fill(0x03040102);
		const y = new Int16Array(x.buffer, x.byteOffset, 516);
		bswapFn(y);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Flips Uint16Array", test: function () {
		const x = new Uint32Array(258).fill(0x04030201);
		const expect = new Uint32Array(258).fill(0x03040102);
		const y = new Uint16Array(x.buffer, x.byteOffset, 516);
		bswapFn(y);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Flips Float32Array", test: function () {
		const x = new Uint32Array(258).fill(0x04030201);
		const expect = new Uint32Array(258).fill(0x01020304);
		const y = new Float32Array(x.buffer, x.byteOffset, 258);
		bswapFn(y);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Flips Float32Array (unaligned)", test: function () {
		const x = new Uint32Array(258).fill(0x04030201);
		const expect = new Uint32Array(258).fill(0x04030201);
		const temp = new Uint32Array(expect.buffer, expect.byteOffset + 4, 257).fill(0x01020304);
		const y = new Float32Array(x.buffer, x.byteOffset + 4, 257);
		bswapFn(y);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Flips Int32Array", test: function () {
		const x = new Uint32Array(258).fill(0x04030201);
		const expect = new Uint32Array(258).fill(0x01020304);
		const y = new Int32Array(x.buffer, x.byteOffset, 258);
		bswapFn(y);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Flips Uint32Array", test: function () {
		const x = new Uint32Array(258).fill(0x04030201);
		const expect = new Uint32Array(258).fill(0x01020304);
		const y = new Uint32Array(x.buffer, x.byteOffset, 258);
		bswapFn(y);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Flips Float64Array", test: function () {
		const x = new Uint8Array(256 * 8);
		const expect = new Uint8Array(256 * 8);
		for (let i = 0; i < x.length; i++) {
			x[i] = i % 8;
			expect[expect.length - i - 1] = i % 8;
		}
		var y = new Float64Array(x.buffer, x.byteOffset, 256);
		bswapFn(y);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Does not touch surrounding memory", test: function () {
		// Node buffers can come from a shared pool, and typed arrays
		// can reference the same ArrayBuffer. Vectorized code works on
		// 16 byte vectors. Check that we don't always do 16 bytes.
		const a = new Uint16Array([1, 2, 3, 4, 5, 6, 7, 8]);
		const sub = a.subarray(0, 3);
		const expect = new Uint16Array([256, 512, 768, 4, 5, 6, 7, 8]);
		bswapFn(sub);
		assert.deepStrictEqual(a, expect);
	}},

	{label: "Does nothing to an Int8Array", test: function () {
		const x = new Int8Array([4, 210]);
		const expect = new Int8Array([4, 210]);
		bswapFn(x);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Does nothing to a Uint8Array", test: function () {
		const x = new Uint8Array([4, 210]);
		const expect = new Uint8Array([4, 210]);
		bswapFn(x);
		assert.deepStrictEqual(x, expect);
	}},

	{label: "Does nothing to a Uint8ClampedArray", test: function () {
		const x = new Uint8ClampedArray([4, 210]);
		const expect = new Uint8ClampedArray([4, 210]);
		bswapFn(x);
		assert.deepStrictEqual(x, expect);
	}}
];

describe("Native", function () {
	before(function () {
		bswapFn = bswap.native;
		console.log(`Testing ${bswap.ise} instruction set.`);
	});

	for (let i = 0; i < tests.length; i++) {
		it(tests[i].label, tests[i].test);
	}
});

describe("JS", function () {
	before(function () {
		bswapFn = bswap.js;
	});

	for (let i = 0; i < tests.length; i++) {
		it(tests[i].label, tests[i].test);
	}
});

describe("General", function () {
	it("Throws for non-TypedArray input", function () {
		assert.throws(() => bswap([]), TypeError);
		assert.throws(() => bswap(1), TypeError);
		assert.throws(() => bswap("a"), TypeError);
		assert.throws(() => bswap({}), TypeError);
	});

	it("Defines ise constant", function () {
		assert(typeof bswap.ise === "string");
	});
});
