#include <cassert>
#include <node_api.h>

#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

static napi_value Add(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  if (argc < 2) {
    napi_throw_type_error(env, NULL, "Wrong number of arguments");
    return NULL;
  }

  napi_valuetype firstValueType;
  status = napi_typeof(env, args[0], &firstValueType);
  assert(status == napi_ok);

  napi_valuetype secondValueType;
  status = napi_typeof(env, args[1], &secondValueType);
  assert(status == napi_ok);

  if (firstValueType != napi_number || secondValueType != napi_number) {
    napi_throw_type_error(env, NULL, "Wrong arguments");
    return NULL;
  }

  double firstValue;
  status = napi_get_value_double(env, args[0], &firstValue);
  assert(status == napi_ok);

  double secondValue;
  status = napi_get_value_double(env, args[1], &secondValue);
  assert(status == napi_ok);

  napi_value sum;
  status = napi_create_double(env, firstValue + secondValue, &sum);
  assert(status == napi_ok);
  return sum;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor addDescriptor = DECLARE_NAPI_METHOD("add", Add);
  status = napi_define_properties(env, exports, 1, &addDescriptor);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
