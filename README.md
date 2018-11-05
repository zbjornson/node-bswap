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

Showing millions of elements processed per second when invoked with a
10,000-element array. (Run the benchmark suite to see results for varying array
lengths and other libraries.) Ran on an Intel i7-7700HQ 2.80 GHz processor (AVX2
supported); Node.js v8.x.

| compiler  |    C++ |   JS | Native:JS |   node |
| --------- | -----: | ---: | --------: | -----: |
| **16 bit types (Uint16Array, Int16Array)**
| MSVC 2015 | 26,508 |  625 |     42.40 | 12,141 |
| GCC 8.1   | 26,245 |  601 |     43.63 |  1,507 |
| **32 bits types (Uint32Array, Int32Array, Float32Array)**
| MSVC 2015 | 10,144 |  342 |     29.58 |  5,840 |
| GCC 8.1   | 11,553 |  328 |     35.15 |  2,361 |
| **64 bit types (Float64Array)**
| MSVC 2015 | 6,244  |  179 |     34.76 |  3,043 |
| GCC 8.1   | 5,453  |  182 |     29.81 |  1,790 |

### Optimization notes

Despite how simple this procedure is, it is very difficult to get all compilers
to produce optimal code for all instruction sets. The current version emits the
best code with GCC and MSVC. LLVM Clang is comparable for 16-bit types with all
ISEs, but significantly slower for 32- and 64-bit types with all ISEs.

There's an AVX-512 implementation available that is disabled by default because
it is slower than the AVX2 implementation.

Below shows GCC 8.1 and LLVM Clang 6.0 run on the same Skylake SP machine with
each of the three ISAs and three scalar types. (The Node.js column is shown so
the Native column can be normalized to it.) Values are ops/sec.

```
╔═══════╤════════╤═════════════════╤════════════════╤═══════════════╤═══════════╗
║ Width │ ISE    │ Compiler        │ This Library   │ Node.js       │ this:node ║
╠═══════╪════════╪═════════════════╪════════════════╪═══════════════╪═══════════╣
║ 16    │ SSSE3  │ GCC8            │ 12,653,020,236 │ 1,216,141,201 │ 10.40     ║
║ 16    │ SSSE3  │ GCC8-al32       │ 12,733,630,917 │ 1,214,958,824 │ 10.48     ║
║ 16    │ AVX2   │ GCC8            │ 15,091,712,556 │ 1,192,133,126 │ 12.66     ║
║ 16    │ AVX2   │ GCC8-al32       │ 21,599,016,828 │ 1,208,407,961 │ 17.87 *   ║
║ 16    │ AVX512 │ GCC8            │ 20,336,576,654 │ 1,198,435,702 │ 16.97     ║
║ 16    │ SSSE3  │ Clang6          │ 12,994,362,801 │ 1,190,614,664 │ 10.91     ║
║ 16    │ AVX2   │ Clang6          │ 21,689,271,126 │ 1,203,133,316 │ 18.03     ║
║ 16    │ AVX512 │ Clang6          │ 20,010,693,365 │ 1,174,994,915 │ 17.03     ║
║ 16    │ AVX512 │ Clang6-nointrin │ 22,923,164,493 │ 1,207,507,344 │ 18.98 *   ║
╠═══════╪════════╪═════════════════╪════════════════╪═══════════════╪═══════════╣
║ 32    │ SSSE3  │ GCC8            │  4,047,932,606 │ 1,885,214,174 │  2.15     ║
║ 32    │ SSSE3  │ GCC8-al32       │  6,059,427,503 │ 1,911,981,279 │  3.17     ║
║ 32    │ AVX2   │ GCC8            │  8,764,230,884 │ 1,871,032,648 │  4.68 *   ║
║ 32    │ AVX2   │ GCC8-al32       │  9,040,984,638 │ 1,891,490,104 │  4.78 *   ║
║ 32    │ AVX512 │ GCC8            │  7,739,234,783 │ 1,879,787,126 │  4.12     ║
║ 32    │ SSSE3  │ Clang6          │  4,157,620,539 │ 1,896,533,115 │  2.19     ║
║ 32    │ AVX2   │ Clang6          │  7,747,506,930 │ 1,869,321,807 │  4.14     ║
║ 32    │ AVX512 │ Clang6          │  8,633,861,272 │ 1,860,963,702 │  4.64     ║
╠═══════╪════════╪═════════════════╪════════════════╪═══════════════╪═══════════╣
║ 64    │ SSSE3  │ GCC8            │  3,101,205,301 │ 1,441,643,749 │  2.15     ║
║ 64    │ SSSE3  │ GCC8-al32       │  3,249,387,192 │ 1,429,194,277 │  2.27     ║
║ 64    │ AVX2   │ GCC8            │  4,927,928,205 │ 1,402,571,030 │  3.51 *   ║
║ 64    │ AVX2   │ GCC8-al32       │  5,066,601,453 │ 1,429,882,304 │  3.54 *   ║
║ 64    │ AVX512 │ GCC8            │  4,300,340,158 │ 1,400,720,898 │  3.07     ║
║ 64    │ SSSE3  │ Clang6          │  2,220,142,836 │ 1,412,992,328 │  1.57     ║
║ 64    │ AVX2   │ Clang6          │  4,085,709,580 │ 1,437,597,532 │  2.84     ║
║ 64    │ AVX512 │ Clang6          │  4,741,903,055 │ 1,410,231,436 │  3.36     ║
╚═══════╧════════╧═════════════════╧════════════════╧═══════════════╧═══════════╝

GCC8-al32: With `-falign-loops=32`. This has a significant impact for
width=16/AVX2 and width=32/SSSE3, and potentially width=64/SSSE3.

Clang6-nointrin: __builtin_bswap16() called in a loop. The bswap builtin is only
fast for 16-bit types; the other sizes are extremely slow. This appears to be
from extensive and very tidy loop unrolling.
```

## Comparison to other libraries

| Library | Operand | In-Place | 64-bit Type Support | Browser | Speed (vs. bswap)* |
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
