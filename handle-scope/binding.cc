#include <cassert>
#include <iostream>

#include <node_api.h>

static napi_value TestScope(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_handle_scope scope;

  status = napi_open_handle_scope(env, &scope);
  napi_value oldVal;
  assert(status == napi_ok);
  status = napi_create_string_utf8(env, "old value", NAPI_AUTO_LENGTH, &oldVal);
  assert(status == napi_ok);
  // closing the scope can indicate to the GC that all napi_values created during the lifetime
  // of the handle scope are no longer referenced from the current stack frame
  status = napi_close_handle_scope(env, scope);
  assert(status == napi_ok);

  napi_value newVal;
  status = napi_create_string_utf8(env, "new value", NAPI_AUTO_LENGTH, &newVal);
  assert(status == napi_ok);
  return oldVal; // new value
}

#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

static napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor desc = DECLARE_NAPI_METHOD("testscope", TestScope);
  status = napi_define_properties(env, exports, 1, &desc);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)