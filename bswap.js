(function () {
	var bswap = function (typedArray) {
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
			var bswapNative = require("./build/Release/bswap").flipBytes;
			var bswapJS = bswap;
			bswap = bswapNative;
			bswap.native = bswapNative;
			bswap.js = bswapJS;
			module.exports = bswap;
			return;
		} catch (e) {}
	}

	function flip16(info) {
		var flipper = new Uint8Array(info.buffer, info.byteOffset, info.length * 2);
		var len = flipper.length;
		for (var i = 0; i < len; i += 2) {
			var t = flipper[i];
			flipper[i] = flipper[i + 1];
			flipper[i + 1] = t;
		}
	}

	function flip32(info) {
		var flipper = new Uint8Array(info.buffer, info.byteOffset, info.length * 4);
		var len = flipper.length;
		for (var i = 0; i < len; i += 4) {
			var t = flipper[i];
			flipper[i] = flipper[i + 3];
			flipper[i + 3] = t;
			t = flipper[i + 1];
			flipper[i + 1] = flipper[i + 2];
			flipper[i + 2] = t;
		}
	}

	function flip64(info) {
		var flipper = new Uint8Array(info.buffer, info.byteOffset, info.length * 8);
		var len = flipper.length;
		for (var i = 0; i < len; i += 8) {
			var t = flipper[i];
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
