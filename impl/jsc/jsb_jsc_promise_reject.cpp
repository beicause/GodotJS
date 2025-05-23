#include "jsb_jsc_promise_reject.h"
#include "jsb_jsc_handle.h"

namespace v8
{
    Local<Promise> PromiseRejectMessage::GetPromise() const
    {
        return Local<Promise>(Data(isolate_, promise_pos_));
    }

    Local<Value> PromiseRejectMessage::GetValue() const
    {
        return Local<Value>(Data(isolate_, reason_pos_));
    }

}
