[![CircleCI](https://circleci.com/gh/zbjornson/node-bswap.svg?style=svg)](https://circleci.com/gh/zbjornson/node-bswap)
[![Build status](https://ci.appveyor.com/api/projects/status/ddnemfveens34b12/branch/master?svg=true)](https://ci.appveyor.com/project/zbjornson/node-bswap/branch/master)


# node-bswap

A function to quickly swap bytes (a.k.a. reverse the byte ordering, change
endianness) of typed arrays in-place for node.js and browsers. Works with all
of the typed array types. For node.js 4.x and later, this also works on
Buffers.

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

In node.js when native code is available, this library uses x86 SIMD
instructions ([PSHUFB (SSSE3) and VPSHUFB (AVX)](http://www.felixcloutier.com/x86/PSHUFB.html)),
which allow processing multiple array elements simultaneously for maximum
speed.

Tested with MSVC 2015, GCC 4.4.7 - 7.0.0 and clang 3.4.1 - 3.9.0. Clang <3.4.0
is not supported.

In the browser or when native code is unavailable, this library falls back to
a fast, pure javascript implementation. The javascript implementation is also
always available:

```js
> var bswap = require("bswap").js; // Use javascript implementation explicitly
```

## Benchmarks

Showing elements processed per second for varying array sizes.

##### Windows + Microsoft Visual Studio
Run on an Intel i7-6700K 4.0 GHz processor; compiled with MSVC 2015; node.js v6.3.1.

```
$ node benchmark/benchmark.js
16 bit types (Uint16Array, Int16Array)
array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
         1       9,613,755      11,826,989       0.81       8,475,564       1,700,083       3,874,999
        10      97,042,860     101,716,521       0.95      76,003,795      15,804,110      32,134,103
       100     953,008,231     464,119,545       2.05     476,201,619     116,807,394     153,226,065
      1000   6,426,743,793     798,765,235       8.05   3,867,224,701     343,781,438     246,060,748
     10000  18,453,693,952     870,322,876      21.20  14,249,355,741     456,976,905     275,911,914

32 bits types (Uint32Array, Int32Array, Float32Array)
array size          Native              JS  Native:JS            node  network-byte-o         etoggle
         1       8,097,057      11,826,481       0.68       8,386,270       1,771,717       3,908,308
        10      81,797,773     100,940,371       0.81      74,507,375      17,057,524      30,400,790
       100     737,441,176     403,581,517       1.83     483,920,374     102,595,513     102,228,587
      1000   4,585,211,237     605,539,513       7.57   3,301,909,784     273,677,920     129,688,099
     10000   9,420,802,017     654,845,585      14.39   7,040,256,671     306,534,733     137,147,637

64 bit types (Float64Array)
array size          Native              JS  Native:JS            node  network-byte-o         etoggle
         1       7,700,237      11,052,820       0.70       7,278,691                       3,755,845
        10      76,533,589      89,571,447       0.85      61,877,117                      25,662,763
       100     652,991,006     319,891,219       2.04     427,374,201                      58,348,806
      1000   3,198,724,039     439,108,024       7.28   2,399,710,498                      74,203,420
     10000   5,043,258,651     440,390,304      11.45   3,882,154,296                      77,389,275
```

##### Linux + GCC
Run on an Intel Xeon (Haswell) 2.3 GHz processor; compiled with gcc 5.4; node.js 6.6.0

```
$ node benchmark/benchmark.js
16 bit types (Uint16Array, Int16Array)
array size          Native              JS  Native:JS            node        nbo-hton         etoggle
         1       5,244,644       5,026,932       1.04       3,580,626         744,053       1,539,757
        10      51,791,104      42,322,691       1.22      33,075,131       6,781,430      12,801,456
       100     527,499,597     222,458,085       2.37     155,244,427      42,646,759      47,466,223
      1000   3,259,448,912     419,806,035       7.76     659,131,889     139,357,829      57,895,409
     10000   9,422,767,318     421,840,993      22.34   1,328,409,914     238,130,717      81,827,561

32 bit types (Uint32Array, Int32Array, Float32Array)
array size          Native              JS  Native:JS            node        nbo-hton         etoggle
         1       4,717,200       4,758,587       0.99       3,442,169         819,240       1,518,156
        10      48,461,691      41,443,893       1.17      30,884,029       8,030,576      11,171,093
       100     452,300,997     182,326,619       2.48     148,421,110      34,824,975      26,433,741
      1000   2,513,577,048     314,465,980       7.99     772,784,152      89,412,062      36,650,725
     10000   4,504,804,963     361,385,763      12.47   1,459,533,539     145,833,158      49,651,697

64 bit types (Float64Array)
array size          Native              JS  Native:JS            node        nbo-hton         etoggle
         1       4,531,083       4,305,089       1.05       2,817,672                       1,331,291
        10      44,578,940      37,572,752       1.19      25,876,374                       8,423,013
       100     382,249,709     137,271,339       2.78     135,520,956                      18,949,443
      1000   1,600,655,189     203,786,262       7.85     739,446,806                      22,149,947
     10000   2,475,442,656     239,289,309      10.34   1,173,594,391                      33,553,118
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
Slower except on Windows.

  In 6.3.0 I added some optimizations to node.js's implementation. The optimizations are effective
  on Windows, but GCC does not do the same automatic vectorization that MSVC does, nor does node's
  default build config enable all SSE/AVX instructions. This library's build config enables those
  extensions and uses builtins that are not worth attempting to support on all of the architectures
  that node.js needs to support. -- That is to say, this library is faster, but is only tested on
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
