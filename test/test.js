var assert = require("chai").assert;
var bswap = require("../bswap.js");
var bswapFn;

var tests = [
	{label: "Flips an Int16Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var y = new Int16Array(x.buffer, x.byteOffset, 2);
		bswapFn(y);
		assert.strictEqual(x[0], 210);
		assert.strictEqual(x[1], 4);
		assert.strictEqual(x[2], 9);
		assert.strictEqual(x[3], 7);
	}},

	{label: "Flips a Uint16Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var y = new Uint16Array(x.buffer, x.byteOffset, 2);
		bswapFn(y);
		assert.strictEqual(x[0], 210);
		assert.strictEqual(x[1], 4);
		assert.strictEqual(x[2], 9);
		assert.strictEqual(x[3], 7);
	}},

	{label: "Flips a Float32Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var y = new Float32Array(x.buffer, x.byteOffset, 1);
		bswapFn(y);
		assert.strictEqual(x[0], 9);
		assert.strictEqual(x[1], 7);
		assert.strictEqual(x[2], 210);
		assert.strictEqual(x[3], 4);
	}},

	{label: "Flips an Int32Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var y = new Int32Array(x.buffer, x.byteOffset, 1);
		bswapFn(y);
		assert.strictEqual(x[0], 9);
		assert.strictEqual(x[1], 7);
		assert.strictEqual(x[2], 210);
		assert.strictEqual(x[3], 4);
	}},

	{label: "Flips a Uint32Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9]);
		var y = new Uint32Array(x.buffer, x.byteOffset, 1);
		bswapFn(y);
		assert.strictEqual(x[0], 9);
		assert.strictEqual(x[1], 7);
		assert.strictEqual(x[2], 210);
		assert.strictEqual(x[3], 4);
	}},

	{label: "Flips a Float64Array", test: function () {
		var x = new Uint8Array([4, 210, 7, 9, 11, 63, 81, 12]);
		var y = new Float64Array(x.buffer, x.byteOffset, 1);
		bswapFn(y);
		assert.strictEqual(x[0], 12);
		assert.strictEqual(x[1], 81);
		assert.strictEqual(x[2], 63);
		assert.strictEqual(x[3], 11);
		assert.strictEqual(x[4], 9);
		assert.strictEqual(x[5], 7);
		assert.strictEqual(x[6], 210);
		assert.strictEqual(x[7], 4);
	}},

	{label: "Does nothing to an Int8Array", test: function () {
		var x = new Int8Array([4, 20]);
		bswapFn(x);
		assert.strictEqual(x[0], 4);
		assert.strictEqual(x[1], 20);
	}},

	{label: "Does nothing to a Uint8Array", test: function () {
		var x = new Uint8Array([4, 210]);
		bswapFn(x);
		assert.strictEqual(x[0], 4);
		assert.strictEqual(x[1], 210);
	}},

	{label: "Does nothing to a Uint8ClampedArray", test: function () {
		var x = new Uint8ClampedArray([4, 210]);
		bswapFn(x);
		assert.strictEqual(x[0], 4);
		assert.strictEqual(x[1], 210);
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
