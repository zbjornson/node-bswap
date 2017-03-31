[![CircleCI](https://circleci.com/gh/zbjornson/node-bswap.svg?style=svg)](https://circleci.com/gh/zbjornson/node-bswap)
[![Build status](https://ci.appveyor.com/api/projects/status/ddnemfveens34b12/branch/master?svg=true)](https://ci.appveyor.com/project/zbjornson/node-bswap/branch/master)


# node-bswap

A function to quickly swap bytes (a.k.a. reverse the byte ordering, change
endianness) of typed arrays in-place for node.js and browsers. Works with all
of the typed array types. For node.js 4.x and later, this also works on
Buffers if you construct a TypedArray view on the underlying array buffer (see below).

Install:
```
$ npm install bswap
```

Use (node.js):
```js
> var bswap = require("bswap");
> var x = new Uint16Array([1, 2, 3, 4, 5, 6, 7, 8]);
> bswap(x);
> x
Uint16Array [ 256, 512, 768, 1024, 1280, 1536, 1792, 2048 ]

// With buffers:
> var b = Buffer.alloc(128);
// This constructs a "view" on the same memory; it does not allocate new memory:
> var ui32 = new Uint32Array(b.buffer, b.byteOffset, b.byteLength / Uint32Array.BYTES_PER_ELEMENT);
> bswap(ui32);
```

Use (browser):
```html
<script src="bswap.js" />
<script type="text/javascript">
  var x = new Uint16Array([1, 2, 3, 4, 5, 6, 7, 8]);
  bswap(x);
  console.log(x); // [ 256, 512, 768, 1024, 1280, 1536, 1792, 2048 ]
</script>
```

In node.js when native code and an x86 processor is available, this library uses
x86 SIMD instructions ([PSHUFB (SSSE3) and VPSHUFB (AVX2)](http://www.felixcloutier.com/x86/PSHUFB.html)),
which allow processing multiple array elements simultaneously for maximum speed.

Native code requires:
* MSVC 2015 or later
* Clang 3.4.x or later
* GCC 4.8.x or later
* ICC 16 or later

In the browser or when native code is unavailable, this library falls back to
a fast, pure javascript implementation. The javascript implementation is also
always explicitly available:

```js
> var bswap = require("bswap").js; // Use javascript implementation explicitly
```

## Benchmarks

Showing elements processed per second for varying array sizes.

##### Windows + Microsoft Visual Studio
Run on an Intel i7-6700K 4.0 GHz processor; compiled with MSVC 2015; node.js v6.9.1.

```
$ node benchmark/benchmark.js
16 bit types (Uint16Array, Int16Array)
array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
         1       9,623,128      11,744,194       0.82      10,321,414       1,722,619       5,007,272
        10      92,939,729     105,099,791       0.88      97,937,560      16,434,764      39,068,393
       100     836,706,259     478,135,777       1.75     531,663,161     117,835,015     122,989,862
      1000   7,103,465,043     780,571,487       9.10   4,125,276,485     348,755,856     158,190,387
     10000  30,311,532,432     870,230,276      34.83  14,508,789,926     416,698,205     174,576,092

32 bits types (Uint32Array, Int32Array, Float32Array)
array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
         1       8,424,850      11,766,511       0.72      10,246,743       1,788,284       4,747,687
        10      80,099,734      99,091,399       0.81      89,075,993      16,762,539      34,038,929
       100     778,506,169     392,606,447       1.98     490,808,650      96,666,526      83,384,793
      1000   5,755,961,382     584,114,433       9.85   3,396,910,232     257,698,664     106,384,258
     10000  13,990,895,416     649,662,228      21.54   7,111,943,703     308,314,686     120,570,771

64 bit types (Float64Array)
array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
         1       8,247,345      10,953,469       0.75       8,553,544                       4,615,601
        10      78,144,438      89,228,121       0.88      69,683,579                      26,828,479
       100     741,464,085     309,504,200       2.40     449,366,982                      56,739,112
      1000   4,541,352,325     430,736,326      10.54   2,389,748,759                      67,823,257
     10000   7,972,476,354     433,632,544      18.39   3,877,818,734                      71,182,283
```

##### Linux + GCC
Run on an Intel Xeon (Haswell) 2.3 GHz processor; compiled with gcc 5.4; node.js 6.10.0

```
$ node benchmark/benchmark.js
16 bit types (Uint16Array, Int16Array)
array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
         1       7,165,478       7,312,635       0.98       6,484,115       1,335,542       2,768,400
        10      68,776,049      63,314,781       1.09      59,665,460      12,773,941      22,364,736
       100     688,617,555     309,237,407       2.23     274,180,440      80,521,009      71,115,057
      1000   5,188,942,005     536,719,907       9.67   1,062,689,936     214,415,320      95,796,719
     10000  19,275,597,207     599,452,514      32.16   1,585,267,860     327,547,954     104,935,171

32 bits types (Uint32Array, Int32Array, Float32Array)
array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
         1       6,422,036       6,937,930       0.93       6,394,318       1,346,763       2,762,375
        10      62,386,195      59,012,938       1.06      55,481,606      13,121,369      18,696,615
       100     596,551,383     252,945,128       2.36     290,599,325      63,995,412      51,085,414
      1000   4,029,053,228     422,846,152       9.53   1,244,689,635     165,566,848      67,920,636
     10000   6,033,072,610     448,254,354      13.46   1,879,609,356     237,455,933      74,017,061

64 bit types (Float64Array)
array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
         1       6,107,068       6,880,232       0.89       5,507,570                       2,642,804
        10      59,792,589      55,598,979       1.08      45,608,568                      16,137,452
       100     517,743,147     196,702,625       2.63     266,074,952                      34,681,753
      1000   2,683,745,928     286,584,407       9.36   1,087,181,187                      40,301,519
     10000   3,227,705,828     296,483,220      10.89   1,555,902,990                      43,150,733
```

Note that there's an inflection point between the penalty for crossing into C++ and the
performance gain from using native code (around 32 elements in the Windows benchmark, or 2 elements
in the Linux benchmark). If you want to explicitly use the javascript implementation, e.g. because
you're processing small arrays, it is available as `bswap.js(arr)`.

## Other libraries

| Library | Operand | In-Place | 64-bit Type Support | Browser | Speed (vs bswap)* |
| --- | --- | --- | --- | --- | --- |
| bswap (this) | TypedArray | yes | yes | yes | 1.00 |
| node [`buffer.swap16/32/64`](https://nodejs.org/api/buffer.html#buffer_buf_swap16) | Buffer | yes | since 6.3.0 | no | 0.14 |
| [endian-toggle](https://github.com/substack/endian-toggle) | Buffer | no | yes | no | 0.03 |
| [network-byte-order](https://github.com/mattcg/network-byte-order) | Number/\[Octet\] | no | no | yes | 0.009 |

\* Higher is better. For 16-bit types, 10k-element arrays.

* **Node.js's built-in [buffer.swap16|32|64](https://nodejs.org/api/buffer.html#buffer_buf_swap16)
methods** (16/32 added in v5.10.0; 64 added by me in 6.3.0). Operates in-place. No browser support.
Slower except for small arrays (where it uses the JS implementation).

  In 6.3.0 I added some optimizations to node.js's implementation. The optimizations are effective
  on Windows, but GCC does not do the same automatic vectorization that MSVC does, nor does node's
  default build config enable all SSE/AVX instructions. This library's build config enables those
  extensions and uses builtins that are not worth attempting to support on all of the architectures
  that node.js needs to support. -- That is to say, this library is faster, but is only supported on
  x86.

```js
Buffer.from(typedArray.buffer).swap16()
```

* **[endian-toggle](https://github.com/substack/endian-toggle)**. Simple usage, operates
on a node.js buffer, handles any byte size, returns a new buffer (does not operate
in-place), slow.

```js
> var x = new Uint16Array([2048])
> toggle(Buffer.from(x.buffer), x.BYTES_PER_ELEMENT * 8)
<Buffer d2 04 09 07>
```

* **[network-byte-order](https://github.com/mattcg/network-byte-order)** has a different
syntax: it operates on a single value at a time (i.e. needs to be looped to operate
on an array) and has separate hton and ntoh methods, which do effectively the same
thing but have different syntaxes. It can operate on strings, but it cannot swap
64-bit types.

```js
// Using hton
> var b = [];
> nbo.htons(b, 0, 2048);
> b
[8, 0]

// bswap:
> var x = new Uint16Array([2048])
> bswap(x)
> new Uint8Array(x.buffer, x.byteOffset, 2) // inspect underlying ArrayBuffer
Uint8Array [ 8, 0 ]

// or using ntoh
> var x = new Uint16Array([2048])
> nbo.ntohs(new Uint8Array(x.buffer, x.byteOffset, 2), 0)
8
> var z = new Uint16Array([8])
> new Uint8Array(z.buffer, z.byteOffset, 2)
Uint8Array [ 8, 0 ]
```
