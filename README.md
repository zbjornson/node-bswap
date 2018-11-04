# node-bswap
[![Build Status](https://dev.azure.com/zbjornson/node-bswap/_apis/build/status/zbjornson.node-bswap)](https://dev.azure.com/zbjornson/node-bswap/_build/latest?definitionId=2)

A function to quickly swap bytes (a.k.a. reverse the byte ordering, change
endianness) of TypedArrays in-place for Node.js and browsers. Works with all of
the TypedArray types. For Node.js 4.x and later, also works on Buffers if you
construct a TypedArray view on the underlying ArrayBuffer (see below).

Install:
```
$ npm install bswap
```

Use:
```js
const bswap = require("bswap");
const x = new Uint16Array([1, 2, 3, 4, 5, 6, 7, 8]);
bswap(x);
// now: Uint16Array [ 256, 512, 768, 1024, 1280, 1536, 1792, 2048 ]

// With buffers:
const b = Buffer.alloc(128);
// This constructs a "view" on the same memory; it does not allocate new memory:
const ui32 = new Uint32Array(b.buffer, b.byteOffset, b.byteLength / Uint32Array.BYTES_PER_ELEMENT);
bswap(ui32);
```

In Node.js when native code and an x86 processor is available, this library uses
the fastest available SIMD instructions ([PSHUFB (SSSE3) and VPSHUFB (AVX2)](http://www.felixcloutier.com/x86/PSHUFB.html)),
which allow processing multiple array elements simultaneously for maximum speed.

Native code requires one of:
* MSVC 2015 or later
* Clang 3.4.x or later
* GCC 4.8.x or later
* ICC 16 or later

In the browser or when native code is unavailable, this library falls back to
the fastest JavaScript implementation. The JavaScript implementation is also
always explicitly available:

```js
const bswap = require("bswap").js; // Use javascript implementation explicitly
```

## Benchmarks

Showing elements processed per second with a 10,000-element array. (Run the
benchmark suite to see results for varying array lengths.) Ran on an Intel
i7-7700HQ 2.80 GHz processor (AVX2 supported); Node.js v8.x.

```
16 bit types (Uint16Array, Int16Array)
compiler      bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
 MSVC 2015  26,508,500,333     625,159,076      42.40  12,141,174,254     297,620,311     164,956,926
 GCC 8.1    26,245,813,975     601,541,776      43.63   1,507,520,480     376,959,470     191,667,343

32 bits types (Uint32Array, Int32Array, Float32Array)
compiler      bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
 MSVC 2015  10,144,363,025     342,992,439      29.58   5,840,019,468     196,300,814      99,381,563
 GCC 8.1    11,553,801,641     328,666,596      35.15   2,361,306,808     229,946,224     108,048,505

64 bit types (Float64Array)
compiler      bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
 MSVC 2015   6,244,425,421     179,640,906      34.76   3,043,672,857                      55,486,764
 GCC 8.1     5,453,585,179     182,916,366      29.81   1,790,234,016                      59,658,422
```

## Other libraries

| Library | Operand | In-Place | 64-bit Type Support | Browser | Speed (vs bswap)* |
| --- | --- | --- | --- | --- | --- |
| bswap (this) | TypedArray | yes | yes | yes | 1.00 |
| node [`buffer.swap16/32/64`](https://nodejs.org/api/buffer.html#buffer_buf_swap16) | Buffer | yes | since 6.3.0 | no | 0.06 to 0.45 |
| [endian-toggle](https://github.com/substack/endian-toggle) | Buffer | no | yes | no | 0.014 |
| [network-byte-order](https://github.com/mattcg/network-byte-order) | Number/\[Octet\] | no | no | yes | 0.007 |

\* Higher is better. For 16-bit types, 10k-element arrays. Range given for
Node.js version reflects Windows vs. Linux benchmark.

* **Node.js' built-in
  [buffer.swap16|32|64](https://nodejs.org/api/buffer.html#buffer_buf_swap16)
  methods** (16/32 since v5.10.0; 64 since 6.3.0). Operates in-place. No browser
  support. Slower except for tiny arrays (where it uses the JS implementation).

  In 6.3.0 I added some optimizations to Node.js' implementation. The
  optimizations are effective on Windows, but GCC does not do the same automatic
  vectorization that MSVC does, nor does Node's default build config enable the
  newer SIMD instructions that this library uses.

  <details><summary>Usage</summary>

  ```js
  > Buffer.from(typedArray.buffer).swap16()
  ```

  </details>

* **[endian-toggle](https://github.com/substack/endian-toggle)**. Simple usage,
  operates on a Node.js Buffer, handles any byte size, returns a new buffer
  (does not operate in-place), slow.

  <details><summary>Usage</summary>

  ```js
  > const x = new Uint16Array([2048])
  > toggle(Buffer.from(x.buffer), x.BYTES_PER_ELEMENT * 8)
  <Buffer d2 04 09 07>
  ```

  </details>

* **[network-byte-order](https://github.com/mattcg/network-byte-order)**.
  Operates on a single value at a time (i.e. needs to be looped to operate on an
  array) and has separate `hton` and `ntoh` methods, which do effectively the
  same thing but have different syntaxes. It can operate on strings, but it
  cannot swap 64-bit types.

  <details><summary>Usage</summary>

  ```js
  // Using hton
  > const b = [];
  > nbo.htons(b, 0, 2048);
  > b
  [8, 0]
  
  // or using ntoh
  > const x = new Uint16Array([2048])
  > nbo.ntohs(new Uint8Array(x.buffer, x.byteOffset, 2), 0)
  8
  > const z = new Uint16Array([8])
  > new Uint8Array(z.buffer, z.byteOffset, 2)
  Uint8Array [ 8, 0 ]
  ```

  </details>
