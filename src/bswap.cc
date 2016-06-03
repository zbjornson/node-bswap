#include <nan.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

using namespace v8;

void flip16(Local<TypedArray> in) {
  Nan::TypedArrayContents<uint16_t> events(in);
  size_t eventsLength = in->Length();

  for (size_t i = 0; i < eventsLength; i++) {
#if defined(_MSC_VER)
    (*events)[i] = _byteswap_ushort((*events)[i]);
#elif defined(__GNUC__)
    (*events)[i] = __builtin_bswap16((*events)[i]);
#endif
  }
}

void flip32(Local<TypedArray> in) {
  Nan::TypedArrayContents<uint32_t> events(in);
  size_t eventsLength = in->Length();

  for (size_t i = 0; i < eventsLength; i++) {
#if defined(_MSC_VER)
    (*events)[i] = _byteswap_ulong((*events)[i]);
#elif defined(__GNUC__)
    (*events)[i] = __builtin_bswap32((*events)[i]);
#endif
  }
}

void flip64(Local<TypedArray> in) {
  Nan::TypedArrayContents<uint64_t> events(in);
  size_t eventsLength = in->Length();

  for (size_t i = 0; i < eventsLength; i++) {
#if defined(_MSC_VER)
    (*events)[i] = _byteswap_uint64((*events)[i]);
#elif defined(__GNUC__)
    (*events)[i] = __builtin_bswap64((*events)[i]);
#endif
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
