# node-bswap

A function to quickly swap bytes (a.k.a. reverse the byte ordering, change endianness)
of typed arrays in-place for node.js and browsers. Works with all of the typed array
types.

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

In node.js when native code is available, this library uses builtins that can make
use of BSWAP (32/64 bits), PSHUFB (SSE3) (64/128) and/or VPSHUFB (AVX2) (128/256),
which allow processing multiple array elements simultaneously. At the time of
writing, MSVC emits PSHUFB.

In the browser or when native code is unavailable, this library falls back to
a fast, pure javascript implementation. The javascript implementation is always
available:

```js
> var bswap = require("bswap").js; // Use javascript implementation explicitly
```

## Benchmarks

Elements processed per second for varying array sizes:

```
$ node benchmark/benchmark.js
16 bit types (Uint16Array, Int16Array)
  array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
           1       8,441,130      11,086,234       0.76       3,764,998       1,623,341       1,845,096
          10      79,993,782      98,280,565       0.81      36,034,049      15,511,274      16,800,614
         100     762,042,912     438,169,569       1.74     264,074,657     101,040,915     100,146,873
        1000   5,360,029,816     742,170,343       7.22   1,068,422,840     331,524,214     211,556,340
       10000  15,354,166,732     821,145,689      18.70   1,747,250,318     345,137,386     236,394,916

32 bits (Uint32Array, Int32Array, Float32Array)
  array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
           1       7,407,920      10,804,187       0.69       3,732,767       1,612,897       1,933,172
          10      74,050,368      87,439,979       0.85      35,080,767      15,810,413      16,649,373
         100     659,876,661     375,634,810       1.76     236,032,153      94,687,628      71,320,300
        1000   3,906,214,035     575,589,756       6.79     678,166,120     253,622,320     122,729,165
       10000   7,203,311,138     615,401,742      11.71     885,411,222     297,031,906     127,451,274

64 bytes (Float64Array)
  array size    bswap.native        bswap.js  Native:JS            node  network-byte-o   endian-toggle
           1       7,193,015      10,652,337       0.68                                       1,910,540
          10      69,285,071      83,629,025       0.83                                      14,732,739
         100     607,663,302     302,590,563       2.01                                      44,715,417
        1000   2,778,599,092     415,668,122       6.68                                      66,873,235
       10000   3,847,320,548     412,785,327       9.32                                      71,712,786

```
(Run on an intel I7-6700K 4.0 GHz processor.)

Note that there's an inflection point between the penalty for crossing into C++
and the performance gain from using native code right around 32 elements. If you
want to explicitly use the javascript implementation, e.g. because you're processing
small arrays, it is available as `bson.js(arr)`.

## Other libraries

| Library | Operand | In-Place | 64-bit Type Support | Browser | Speed (vs bswap)* |
| --- | --- | --- | --- | --- | --- |
| bswap (this) | TypedArray | yes | yes | yes | 1.00 |
| node [`buffer.swap16/32`](https://nodejs.org/api/buffer.html#buffer_buf_swap16) | Buffer | yes | no | no | 0.11 |
| [endian-toggle](https://github.com/substack/endian-toggle) | Buffer | no | yes | no | 0.02 |
| [network-byte-order](https://github.com/mattcg/network-byte-order) | Number/\[Octet\] | no | no | yes | 0.02 |

\* Higher is better. For 16-bit types, 10k-element arrays.

* node's built-in [buffer.swap16|32](https://nodejs.org/api/buffer.html#buffer_buf_swap16)
methods (added in v5.10.0). Does not work with `Float64Array`s (only handles 16 and 32-bit
types). Operates in-place. Surprisingly slow. No browser support.

```js
Buffer.from(typedArray.buffer).swap16()
```

* [endian-toggle](https://github.com/substack/endian-toggle). Simple usage, operates
on a node.js buffer, handles any byte size, returns a new buffer (does not operate
in-place), slow.

```js
> var x = new Uint16Array([2048])
> toggle(Buffer.from(x.buffer), x.BYTES_PER_ELEMENT * 8)
<Buffer d2 04 09 07>
```

* [network-byte-order](https://github.com/mattcg/network-byte-order) has a different
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
