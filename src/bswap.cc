#include <nan.h>

using namespace v8;

// GCC/clang: __builtin_bswapN emits MOVBE, but does not vectorize the loop,
// so we have to do it by hand.
//
// MSVC: _byteswap_ushort and friends emits PSHUFB, but MOVDQU. This variant
// emits MOVDQA. Overall it's about 20% faster. (Doubt it's MOVDQU vs MOVDQA
// alone that's responsible given that MOVDQU is generally the same as MOVDQA
// for aligned data.)

#if defined(__GNUC__) // GCC, clang
typedef char v16qi __attribute__((vector_size(16)));
#define pshufb128 __builtin_ia32_pshufb128
#define BSWAP_INTRINSIC_2(x) __builtin_bswap16(x)
#define BSWAP_INTRINSIC_4(x) __builtin_bswap32(x)
#define BSWAP_INTRINSIC_8(x) __builtin_bswap64(x)

#elif defined(_MSC_VER)
typedef __m128i v16qi;
#define pshufb128 _mm_shuffle_epi8
#include <intrin.h>
#define BSWAP_INTRINSIC_2(x) _byteswap_ushort(x);
#define BSWAP_INTRINSIC_4(x) _byteswap_ulong(x);
#define BSWAP_INTRINSIC_8(x) _byteswap_uint64(x);

#endif

static v16qi shuf16 = { 1,0, 3,2, 5,4, 7,6, 9,8, 11,10, 13,12, 15,14 };
static v16qi shuf32 = { 3,2,1,0, 7,6,5,4, 11,10,9,8, 15,14,13,12 };
static v16qi shuf64 = { 7,6,5,4,3,2,1,0, 15,14,13,12,11,10,9,8 };

#define IOPT_SHUFF(n, t, mask, intrin, b)                                     \
void n(Local<TypedArray> in) {                                                \
  Nan::TypedArrayContents<t> events(in);                                      \
  char* bytes = reinterpret_cast<char*>((*events));                           \
  size_t length = in->ByteLength();                                           \
  size_t vlength = length - length % 16;                                      \
  size_t i;                                                                   \
  for (i = 0; i < vlength; i+= 16) {                                          \
    v16qi vec = *(v16qi*)&bytes[i];                                           \
    vec = pshufb128(vec, mask);                                               \
    *(v16qi*)(bytes + i) = vec;                                               \
  }                                                                           \
  size_t vlengthB = vlength / b;                                              \
  size_t lengthB = length / b;                                                \
  for ( ; i < lengthB; i++) {                                                 \
    size_t vlengthBI = vlengthB + i;                                          \
    (*events)[vlengthBI] = intrin((*events)[vlengthBI]);                      \
  }                                                                           \
}

IOPT_SHUFF(flip16, uint16_t, shuf16, BSWAP_INTRINSIC_2, 2)
IOPT_SHUFF(flip32, uint32_t, shuf32, BSWAP_INTRINSIC_4, 4)
IOPT_SHUFF(flip64, uint64_t, shuf64, BSWAP_INTRINSIC_8, 8)

NAN_METHOD(flipBytes) {
  Local<Value> arr = info[0];
  if (arr->IsInt16Array() || arr->IsUint16Array()) {
    flip16(arr.As<TypedArray>());
  } else if (arr->IsFloat32Array() || arr->IsInt32Array() || arr->IsUint32Array()) {
    flip32(arr.As<TypedArray>());
  } else if (arr->IsFloat64Array()) {
    flip64(arr.As<TypedArray>());
  } else if (arr->IsInt8Array() || arr->IsUint8Array() || arr->IsUint8ClampedArray()) {
    // noop
  } else {
    Nan::ThrowTypeError("Expected typed array");
  }
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("flipBytes").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(flipBytes)).ToLocalChecked());
}

NODE_MODULE(bswap, Init);
