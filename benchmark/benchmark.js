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
	{label: "32 bit", Ctor: Uint32Array},
	{label: "64 bit", Ctor: Float64Array}
];

var arrayLengths = [1, 10, 100, 1000, 10000];

var methods = [
	{label: "native", fn: bswap.native},
	{label: "js", fn: bswap.js}
];

try {
	var nbo = require("network-byte-order");
	var fn = function (typedArray) {
		var j, b;
		var al = typedArray.length;
		switch (typedArray.BYTES_PER_ELEMENT) {
			case 2:
				b = new Uint8Array(al * 2);
				for (j = 0; j < al; j++) {
					nbo.htons(b, j * 2, typedArray[j]);
				}
				return b;

			case 4:
				b = new Uint8Array(al * 4);
				for (j = 0; j < al; j++) {
					nbo.htonl(b, j * 4, typedArray[j]);
				}
				return b;

			case 8:
				return;
		}
	};
	methods.push({label: "nbo", fn: fn});
} catch (ex) {
	console.log("network-byte-order unavailable, skipping");
}

try {
	var toggle = require("endian-toggle");
	var fn = function (typedArray) {
		return toggle(Buffer.from(typedArray.buffer), typedArray.BYTES_PER_ELEMENT * 8);
	};
	methods.push({label: "etoggle", fn: fn});
} catch (ex) {
	console.log("endian-toggle unavailable, skipping");
}

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
		padLeft("node", 15),
		padLeft("nbo-hton", 15),
		padLeft("etoggle", 15)
	);

	for (var al = 0; al < arrayLengths.length; al++) {
		var len = arrayLengths[al];

		var input = new byteSize.Ctor(len);
		for (var i = 0; i < input.length; i++) {
			input[i] = i % 255;
		}

		var results = {};

		for (var m = 0; m < methods.length; m++) {
			var method = methods[m];

			if (byteSize.label === "64 bit" && method.label === "nbo") continue;

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
			padLeft("node" in results ? formatNumber(results.node.toFixed(0)) : "", 15),
			padLeft("nbo" in results ? formatNumber(results.nbo.toFixed(0)) : "", 15),
			padLeft("etoggle" in results ? formatNumber(results.etoggle.toFixed(0)) : "", 15)
		);
	}

	console.log();
}
