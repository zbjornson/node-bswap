var assert = require("chai").assert;
var bswap = require("../bswap.js");
var bswapFn;

var tests = [
	{label: "Flips Int16Array", test: function () {
		var x = new Uint32Array(258).fill(0x04030201);
		var expect = new Uint32Array(258).fill(0x03040102);
		var y = new Int16Array(x.buffer, x.byteOffset, 516);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips Uint16Array", test: function () {
		var x = new Uint32Array(258).fill(0x04030201);
		var expect = new Uint32Array(258).fill(0x03040102);
		var y = new Uint16Array(x.buffer, x.byteOffset, 516);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips Float32Array", test: function () {
		var x = new Uint32Array(258).fill(0x04030201);
		var expect = new Uint32Array(258).fill(0x01020304);
		var y = new Float32Array(x.buffer, x.byteOffset, 258);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips Float32Array (unaligned)", test: function () {
		var x = new Uint32Array(258).fill(0x04030201);
		var expect = new Uint32Array(258).fill(0x04030201);
		var temp = new Uint32Array(expect.buffer, expect.byteOffset + 4, 257).fill(0x01020304);
		var y = new Float32Array(x.buffer, x.byteOffset + 4, 257);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips Int32Array", test: function () {
		var x = new Uint32Array(258).fill(0x04030201);
		var expect = new Uint32Array(258).fill(0x01020304);
		var y = new Int32Array(x.buffer, x.byteOffset, 258);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips Uint32Array", test: function () {
		var x = new Uint32Array(258).fill(0x04030201);
		var expect = new Uint32Array(258).fill(0x01020304);
		var y = new Uint32Array(x.buffer, x.byteOffset, 258);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips Float64Array", test: function () {
		var x = new Uint8Array(256 * 8);
		var expect = new Uint8Array(256 * 8);
		for (var i = 0; i < x.length; i++) {
			x[i] = i % 8;
			expect[expect.length - i - 1] = i % 8;
		}
		var y = new Float64Array(x.buffer, x.byteOffset, 256);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Does not touch surrounding memory", test: function () {
		// Node buffers can come from a shared pool, and typed arrays
		// can reference the same ArrayBuffer. Vectorized code works on
		// 16 byte vectors. Check that we don't always do 16 bytes.
		var a = new Uint16Array([1, 2, 3, 4, 5, 6, 7, 8]);
		var sub = a.subarray(0, 3);
		var expect = new Uint16Array([256, 512, 768, 4, 5, 6, 7, 8]);
		bswapFn(sub);
		assert.deepEqual(a, expect);
	}},

	{label: "Does nothing to an Int8Array", test: function () {
		var x = new Int8Array([4, 210]);
		var expect = new Int8Array([4, 210]);
		bswapFn(x);
		assert.deepEqual(x, expect);
	}},

	{label: "Does nothing to a Uint8Array", test: function () {
		var x = new Uint8Array([4, 210]);
		var expect = new Uint8Array([4, 210]);
		bswapFn(x);
		assert.deepEqual(x, expect);
	}},

	{label: "Does nothing to a Uint8ClampedArray", test: function () {
		var x = new Uint8ClampedArray([4, 210]);
		var expect = new Uint8ClampedArray([4, 210]);
		bswapFn(x);
		assert.deepEqual(x, expect);
	}}
];

describe("Native", function () {
	before(function () {
		bswapFn = bswap.native;
	});

	for (var i = 0; i < tests.length; i++) {
		it(tests[i].label, tests[i].test);
	}
});

describe("JS", function () {
	before(function () {
		bswapFn = bswap.js;
	});

	for (var i = 0; i < tests.length; i++) {
		it(tests[i].label, tests[i].test);
	}
});

it("Throws for non-TypedArray input", function () {
	assert.throws(function () {
		bswap([]);
	}, TypeError);
	assert.throws(function () {
		bswap(1);
	}, TypeError);
	assert.throws(function () {
		bswap("a");
	}, TypeError);
	assert.throws(function () {
		bswap({});
	}, TypeError);
});
