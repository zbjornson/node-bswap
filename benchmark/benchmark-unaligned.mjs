import Benchmark from "benchmark";
import * as bswap from "../node.mjs";
import chalk from "chalk";


// From benchmark.js
function formatNumber(number) {
	number = String(number).split('.');
	return number[0].replace(/(?=(?:\d{3})+$)(?!\b)/g, ',') +
	(number[1] ? '.' + number[1] : '');
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

const byteSizes = [
	{label: "16 bit", Ctor: Uint16Array},
	{label: "32 bit", Ctor: Float32Array},
	{label: "64 bit", Ctor: Float64Array}
];

const arrayLengths = [1, 10, 100, 1000, 10000];

const methods = [
	{label: "native", fn: bswap.native},
	{label: "js", fn: bswap.js}
];

try {
	const fn = function (typedArray) {
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

for (const byteSize of byteSizes) {
	console.log(chalk.blue(byteSize.label));

	console.log(
		"array size".padStart(10),
		"Native".padStart(15),
		"JS".padStart(15),
		"Native:JS".padStart(10),
		"node".padStart(15)
	);

	for (const len of arrayLengths) {
		const preinput = new byteSize.Ctor(len + 1);
		for (let i = 0; i < preinput.length; i++) {
			preinput[i] = i % 255;
		}
		// Make unaligned (to the 16-byte xmm register) by moving one
		// element's-worth of bytes.
		const input = new byteSize.Ctor(preinput.buffer,
			preinput.byteOffset + byteSize.Ctor.BYTES_PER_ELEMENT,
			len);

		const results = {};

		for (const method of methods) {
			new Benchmark.Suite()
				.add({
					name: method.label,
					fn() { method.fn(input); }
				})
				.on("cycle", event => {
					results[method.label] = event.target.hz * len;
				})
				.run();
		}

		console.log(
			String(len).padStart(10),
			formatNumber(results.native.toFixed(0)).padStart(15),
			formatNumber(results.js.toFixed(0)).padStart(15),
			(results.native / results.js).toFixed(2).padStart(10),
			("node" in results ? formatNumber(results.node.toFixed(0)) : "").padStart(15)
		);
	}

	console.log();
}
