#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <random>

#include <node_api.h>

using namespace std::chrono;

#define CHECK(expr) \
  { \
    if ((expr) == 0) { \
      std::cout << __FILE__ << ":" << __LINE__ << "" <<" failed assertion " << #expr << std::endl; \
      abort(); \
    } \
  }

struct AsyncWorkerData {
  napi_async_work work;
  napi_deferred deferred;
  int32_t result;

  ~AsyncWorkerData() {
    work = nullptr;
    deferred = nullptr;
  }
};

std::mutex workerMutex;

// This function runs on a worker thread. It has no access to the JavaScript.
static void ExecuteWork(napi_env env, void* data) {
  std::lock_guard<std::mutex> autoLock(workerMutex);
  std::cout << "[worker thread]" << std::this_thread::get_id() << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  AsyncWorkerData* asyncWorkerData = reinterpret_cast<AsyncWorkerData*>(data);
  std::default_random_engine generator;
  std::uniform_int_distribution<int32_t> distribution(1, 6);
  asyncWorkerData->result = distribution(generator);
}

// This function runs on the main thread after `ExecuteWork` exits.
static void WorkComplete(napi_env env, napi_status status, void* data) {
  if (status != napi_ok) {
    return;
  }
  std::cout << "[main thread]" << std::this_thread::get_id() << std::endl;
  AsyncWorkerData* asyncWorkerData = reinterpret_cast<AsyncWorkerData*>(data);
  napi_value result;
  CHECK(napi_create_int32(env, asyncWorkerData->result, &result) == napi_ok);
  CHECK(napi_resolve_deferred(env, asyncWorkerData->deferred, result) == napi_ok);
  // Clean up the work item associated with this run.
  CHECK(napi_delete_async_work(env, asyncWorkerData->work) == napi_ok);

  delete asyncWorkerData;
}

// Create a deferred promise and an async queue work item.
static napi_value StartWork(napi_env env, napi_callback_info info) {
  napi_value workName;
  napi_value promise;
  AsyncWorkerData* asyncWorkerData = new AsyncWorkerData();

  // Create a string to describe this asynchronous operation.
  CHECK(napi_create_string_utf8(env,
                                 "Node-API Deferred Promise from Async Work Item",
                                 NAPI_AUTO_LENGTH,
                                 &workName) == napi_ok);

  // Create a deferred promise which we will resolve at the completion of the work.
  CHECK(napi_create_promise(env,
                             &(asyncWorkerData->deferred),
                             &promise) == napi_ok);

  // Create an async work item, passing in the addon data, which will give the
  // worker thread access to the above-created deferred promise.
  CHECK(napi_create_async_work(env,
                                NULL,
                                workName,
                                ExecuteWork,
                                WorkComplete,
                                asyncWorkerData,
                                &(asyncWorkerData->work)) == napi_ok);

  // Queue the work item for execution.
  CHECK(napi_queue_async_work(env, asyncWorkerData->work) == napi_ok);

  // This causes created `promise` to be returned to JavaScript.
  return promise;
}

// The commented-out return type and the commented out formal function
// parameters below help us keep in mind the signature of the addon
// initialization function. We write the body as though the return value were as
// commented below and as though there were parameters passed in as commented
// below.
/*napi_value*/ NAPI_MODULE_INIT(/*napi_env env, napi_value exports*/) {

  // Define the properties that will be set on exports.
  napi_property_descriptor start_work = {
    "startWork",
    nullptr,
    StartWork,
    nullptr,
    nullptr,
    nullptr,
    napi_default,
    nullptr
  };

  // Decorate exports with the above-defined properties.
  CHECK(napi_define_properties(env, exports, 1, &start_work) == napi_ok);
  return exports;
}
