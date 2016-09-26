var Benchmark = require("benchmark");
var bswap = require("../bswap.js");
var chalk = require("chalk");

// From benchmark.js
function formatNumber(number) {
	number = String(number).split('.');
	return number[0].replace(/(?=(?:\d{3})+$)(?!\b)/g, ',') +
	(number[1] ? '.' + number[1] : '');
}

function padLeft(input, size) {
	return Array(size - input.length + 1).join(" ") + input;
}

// Modified from benchmark.js
function formatResult(event, times) {
	var hz = event.hz * times;
	var stats = event.stats;
	var size = stats.sample.length;
	var pm = '\xb1';

	var result = " (array length " + times + ")";

	result += ' x ' + chalk.cyan(formatNumber(hz.toFixed(hz < 100 ? 2 : 0))) + ' ops/sec ' + pm +
		stats.rme.toFixed(2) + '% (' + size + ' run' + (size == 1 ? '' : 's') + ' sampled)';

	return result;
}

var byteSizes = [
	{label: "16 bit", Ctor: Uint16Array},
	{label: "32 bit", Ctor: Float32Array},
	{label: "64 bit", Ctor: Float64Array}
];

var arrayLengths = [1, 10, 100, 1000, 10000];

var methods = [
	{label: "native", fn: bswap.native},
	{label: "js", fn: bswap.js}
];

try {
	var fn = function (typedArray) {
		switch (typedArray.BYTES_PER_ELEMENT) {
			case 2: return Buffer.from(typedArray.buffer).swap16();
			case 4: return Buffer.from(typedArray.buffer).swap32();
			case 8: return Buffer.from(typedArray.buffer).swap64();
		}
	};
	methods.push({label: "node", fn: fn});
} catch (ex) {
	console.log("Buffer not available in browser, skipping");
}

for (var s = 0; s < byteSizes.length; s++) {
	var byteSize = byteSizes[s];

	console.log(chalk.blue(byteSize.label));

	console.log(
		padLeft("array size", 10),
		padLeft("Native", 15),
		padLeft("JS", 15),
		padLeft("Native:JS", 10),
		padLeft("node", 15)
	);

	for (var al = 0; al < arrayLengths.length; al++) {
		var len = arrayLengths[al];

		var preinput = new byteSize.Ctor(len + 1);
		for (var i = 0; i < preinput.length; i++) {
			preinput[i] = i % 255;
		}
		// Make unaligned (to the 16-byte MMX register) by moving one
		// element's-worth of bytes.
		var input = new byteSize.Ctor(preinput.buffer,
			preinput.byteOffset + byteSize.Ctor.BYTES_PER_ELEMENT,
			len);

		var results = {};

		for (var m = 0; m < methods.length; m++) {
			var method = methods[m];

			new Benchmark.Suite()
				.add({
					name: method.label,
					fn: function () {
						method.fn(input);
					}
				})
				.on("cycle", function (event) {
					results[method.label] = event.target.hz * len;
				})
				.run();
		}

		console.log(
			padLeft(String(len), 10),
			padLeft(formatNumber(results.native.toFixed(0)), 15),
			padLeft(formatNumber(results.js.toFixed(0)), 15),
			padLeft((results.native / results.js).toFixed(2), 10),
			padLeft("node" in results ? formatNumber(results.node.toFixed(0)) : "", 15)
		);
	}

	console.log();
}
