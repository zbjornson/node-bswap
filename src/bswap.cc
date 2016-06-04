#include <nan.h>

#if defined(__GNUC__) || defined(__clang__)
#define BSWAP_INTRINSIC_2(x) __builtin_bswap16(x)
#define BSWAP_INTRINSIC_4(x) __builtin_bswap32(x)
#define BSWAP_INTRINSIC_8(x) __builtin_bswap64(x)
#elif defined(__linux__)
#include <byteswap.h>
#define BSWAP_INTRINSIC_2(x) bswap_16(x)
#define BSWAP_INTRINSIC_4(x) bswap_32(x)
#define BSWAP_INTRINSIC_8(x) bswap_64(x)
#elif defined(_MSC_VER)
#include <intrin.h>
#define BSWAP_INTRINSIC_2(x) _byteswap_ushort(x);
#define BSWAP_INTRINSIC_4(x) _byteswap_ulong(x);
#define BSWAP_INTRINSIC_8(x) _byteswap_uint64(x);
#else
#define BSWAP_INTRINSIC_2(x) (x << 8) | (x >> 8)
#define BSWAP_INTRINSIC_4(x)                                                  \
  ((x & 0xFF) << 24) |                                                        \
  ((x & 0xFF00) << 8) |                                                       \
  ((x >> 8) & 0xFF00) |                                                       \
  ((x >> 24) & 0xFF)
#define BSWAP_INTRINSIC_8(x)                                                  \
  ((x & 0xFF00000000000000ull) >> 56) |                                       \
  ((x & 0x00FF000000000000ull) >> 40) |                                       \
  ((x & 0x0000FF0000000000ull) >> 24) |                                       \
  ((x & 0x000000FF00000000ull) >> 8)  |                                       \
  ((x & 0x00000000FF000000ull) << 8)  |                                       \
  ((x & 0x0000000000FF0000ull) << 24) |                                       \
  ((x & 0x000000000000FF00ull) << 40) |                                       \
  ((x & 0x00000000000000FFull) << 56)
#endif

using namespace v8;

void flip16(Local<TypedArray> in) {
  Nan::TypedArrayContents<uint16_t> events(in);
  size_t eventsLength = in->Length();
  for (size_t i = 0; i < eventsLength; i++) {
    (*events)[i] = BSWAP_INTRINSIC_2((*events)[i]);
  }
}

void flip32(Local<TypedArray> in) {
  Nan::TypedArrayContents<uint32_t> events(in);
  size_t eventsLength = in->Length();
  for (size_t i = 0; i < eventsLength; i++) {
    (*events)[i] = BSWAP_INTRINSIC_4((*events)[i]);
  }
}

void flip64(Local<TypedArray> in) {
  Nan::TypedArrayContents<uint64_t> events(in);
  size_t eventsLength = in->Length();

  for (size_t i = 0; i < eventsLength; i++) {
    (*events)[i] = BSWAP_INTRINSIC_8((*events)[i]);
  }
}

NAN_METHOD(flipBytes) {
  if (info[0]->IsInt8Array() || info[0]->IsUint8Array() || info[0]->IsUint8ClampedArray()) {
    // noop
  } else if (info[0]->IsInt16Array() || info[0]->IsUint16Array()) {
    flip16(info[0].As<TypedArray>());
  } else if (info[0]->IsFloat32Array() || info[0]->IsInt32Array() || info[0]->IsUint32Array()) {
    flip32(info[0].As<TypedArray>());
  } else if (info[0]->IsFloat64Array()) {
    flip64(info[0].As<TypedArray>());
  } else {
    Nan::ThrowTypeError("Expected typed array");
  }
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("flipBytes").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(flipBytes)).ToLocalChecked());
}

NODE_MODULE(bswap, Init);
