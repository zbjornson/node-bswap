(function () {
	let bswap = function (typedArray) {
		if (ArrayBuffer.isView(typedArray)) {
			switch (typedArray.BYTES_PER_ELEMENT) {
				case 1:
					// no op
					return;
				case 2:
					return flip16(typedArray);
				case 4:
					return flip32(typedArray);
				case 8:
					return flip64(typedArray);
				default:
					throw new Error("Invalid input");
			}
		} else {
			throw new TypeError("expected typed array");
		}
	};

	if (typeof require !== "undefined") {
		try {
			const bindings = require("./build/Release/bswap");
			const bswapJS = bswap;
			bswap = bindings.flipBytes;
			bswap.native = bindings.flipBytes;
			bswap.js = bswapJS;
			bswap.ise = bindings.ISE;
			module.exports = bswap;
			return;
		} catch (e) {}
	}

	function flip16(info) {
		const flipper = new Uint8Array(info.buffer, info.byteOffset, info.length * 2);
		const len = flipper.length;
		for (let i = 0; i < len; i += 2) {
			let t = flipper[i];
			flipper[i] = flipper[i + 1];
			flipper[i + 1] = t;
		}
	}

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

	this.bswap = bswap;
})();
