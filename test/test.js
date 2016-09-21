var assert = require("chai").assert;
var bswap = require("../bswap.js");
var bswapFn;

var tests = [
	{label: "Flips an Int16Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var expect = new Uint8Array([210, 4, 9, 7]);
		var y = new Int16Array(x.buffer, x.byteOffset, 2);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips a Uint16Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var expect = new Uint8Array([210, 4, 9, 7]);
		var y = new Uint16Array(x.buffer, x.byteOffset, 2);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips a Float32Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var expect = new Uint8Array([9, 7, 210, 4]);
		var y = new Float32Array(x.buffer, x.byteOffset, 1);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips an Int32Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var expect = new Uint8Array([9, 7, 210, 4]);
		var y = new Int32Array(x.buffer, x.byteOffset, 1);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips a Uint32Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var expect = new Uint8Array([9, 7, 210, 4]);
		var y = new Uint32Array(x.buffer, x.byteOffset, 1);
		bswapFn(y);
		assert.deepEqual(x, expect);
	}},

	{label: "Flips a Float64Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9, 11, 63, 81, 12]);
		var expect = new Uint8Array([12, 81, 63, 11, 9, 7, 210, 4]);
		var y = new Float64Array(x.buffer, x.byteOffset, 1);
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
