#include "common.h"
#include <boost/algorithm/string.hpp>
#define V8_COMPRESS_POINTERS
#include <libplatform/libplatform.h>
#include <v8.h>

void log_js(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    if (args.Length() < 1) {
        return;
    }
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    v8::Local<v8::Value> arg = args[0];
    v8::String::Utf8Value value(isolate, arg);
    LOG(INFO) << *value;
}

v8::Local<v8::Context> create_context(v8::Isolate *isolate)
{
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    global->Set(isolate, "log", v8::FunctionTemplate::New(isolate, log_js));
    return v8::Context::New(isolate, nullptr, global);
}

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);
    if (argc < 2) {
        LOG(ERROR) << "please input js code snippet or source file";
        return -1;
    }

    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = create_context(isolate);
        v8::Context::Scope context_scope(context);
        v8::Local<v8::String> source;
        if (boost::algorithm::ends_with(argv[1], ".js")) {
            std::ifstream in_file(argv[1]);
            if (!in_file) {
                LOG(ERROR) << "can't read js source file: " << argv[1];
                return -1;
            }
            std::stringstream ss;
            ss << in_file.rdbuf();
            std::string code = ss.str();
            source = v8::String::NewFromUtf8(isolate, code.c_str()).ToLocalChecked();
        } else {
            source = v8::String::NewFromUtf8(isolate, argv[1]).ToLocalChecked();
        }
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        v8::String::Utf8Value value(isolate, result);
        LOG(INFO) << *value;
    }
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    NT_CATCH
}
