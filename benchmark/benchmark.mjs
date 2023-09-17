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
	const hz = event.hz * times;
	const stats = event.stats;
	const size = stats.sample.length;
	const pm = '\xb1';

	let result = " (array length " + times + ")";

	result += ' x ' + chalk.cyan(formatNumber(hz.toFixed(hz < 100 ? 2 : 0))) + ' ops/sec ' + pm +
		stats.rme.toFixed(2) + '% (' + size + ' run' + (size == 1 ? '' : 's') + ' sampled)';

	return result;
}

const byteSizes = [
	{label: "16 bit", Ctor: Uint16Array},
	{label: "32 bit", Ctor: Uint32Array},
	{label: "64 bit", Ctor: Float64Array}
];

const arrayLengths = [1, 10, 100, 1000, 10000];

const methods = [
	{label: "native", fn: bswap.native},
	{label: "js", fn: bswap.js}
];

try {
	const nbo = await import("network-byte-order");
	const fn = function (typedArray) {
		let b;
		const al = typedArray.length;
		switch (typedArray.BYTES_PER_ELEMENT) {
			case 2:
				b = new Uint8Array(al * 2);
				for (let j = 0; j < al; j++) {
					nbo.htons(b, j * 2, typedArray[j]);
				}
				return b;

			case 4:
				b = new Uint8Array(al * 4);
				for (let j = 0; j < al; j++) {
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
	const toggle = await import("endian-toggle");
	const fn = function (typedArray) {
		return toggle(Buffer.from(typedArray.buffer), typedArray.BYTES_PER_ELEMENT * 8);
	};
	methods.push({label: "etoggle", fn: fn});
} catch (ex) {
	console.log("endian-toggle unavailable, skipping");
}

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
		"node".padStart(15),
		"nbo-hton".padStart(15),
		"etoggle".padStart(15),
	);

	for (const len of arrayLengths) {
		const input = new byteSize.Ctor(len);
		for (let i = 0; i < input.length; i++) {
			input[i] = i % 255;
		}

		const results = {};

		for (const method of methods) {
			if (byteSize.label === "64 bit" && method.label === "nbo") continue;

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
			("node" in results ? formatNumber(results.node.toFixed(0)) : "").padStart(15),
			("nbo" in results ? formatNumber(results.nbo.toFixed(0)) : "").padStart(15),
			("etoggle" in results ? formatNumber(results.etoggle.toFixed(0)) : "").padStart(15)
		);
	}

	console.log();
}
