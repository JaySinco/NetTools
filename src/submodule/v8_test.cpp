#include "common.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#define V8_COMPRESS_POINTERS
#include <libplatform/libplatform.h>
#include <v8.h>
#define CHECK_EXCEPTION(isolate, try_catch, maybe) \
    if (maybe.IsEmpty()) {                         \
        report_exception(isolate, &try_catch);     \
        return -1;                                 \
    }

void report_exception(v8::Isolate *isolate, v8::TryCatch *try_catch)
{
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(isolate, try_catch->Exception());
    v8::Local<v8::Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        std::cerr << *exception << std::endl;
        return;
    }
    v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    int linenum = message->GetLineNumber(context).FromJust();
    std::cerr << "{}:{}: {}"_format(*filename, linenum, *exception) << std::endl;
    v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
    std::cerr << *sourceline << std::endl;
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++) {
        std::cerr << " ";
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++) {
        std::cerr << "^";
    }
    std::cerr << std::endl;
    v8::Local<v8::Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString() && stack_trace_string.As<v8::String>()->Length() > 0) {
        v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
        std::cerr << *stack_trace << std::endl;
    }
}

void log(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    if (args.Length() < 1) {
        isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "bad parameters"));
        return;
    }
    v8::String::Utf8Value value(isolate, args[0]);
    std::cerr << *value << std::endl;
}

void get_version(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    v8::Isolate *isolate = info.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    info.GetReturnValue().Set(
        v8::String::NewFromUtf8(isolate, v8::V8::GetVersion()).ToLocalChecked());
}

v8::Local<v8::Context> create_context(v8::Isolate *isolate)
{
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    global->Set(isolate, "log", v8::FunctionTemplate::New(isolate, log));
    global->SetAccessor(v8::String::NewFromUtf8Literal(isolate, "__version__"), get_version);
    return v8::Context::New(isolate, nullptr, global);
}

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);
    if (argc < 2) {
        std::cerr << "please input js code snippet or source file" << std::endl;
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
        std::string source;
        std::string filename;
        if (boost::algorithm::ends_with(argv[1], ".js")) {
            std::ifstream in_file(s2ws(argv[1]));
            if (!in_file) {
                std::cerr << "can't read js source file: " << argv[1] << std::endl;
                return -1;
            }
            std::stringstream ss;
            ss << in_file.rdbuf();
            source = ss.str();
            filename = argv[1];
        } else {
            source = argv[1];
            filename = "<anonymous>";
        }
        v8::TryCatch try_catch(isolate);
        v8::ScriptOrigin origin(
            v8::String::NewFromUtf8(isolate, filename.c_str()).ToLocalChecked());
        v8::MaybeLocal<v8::Script> script = v8::Script::Compile(
            context, v8::String::NewFromUtf8(isolate, source.c_str()).ToLocalChecked(), &origin);
        CHECK_EXCEPTION(isolate, try_catch, script);
        v8::MaybeLocal<v8::Value> result = script.ToLocalChecked()->Run(context);
        CHECK_EXCEPTION(isolate, try_catch, result);
        v8::String::Utf8Value value(isolate, result.ToLocalChecked());
        std::cerr << *value << std::endl;
    }
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    NT_CATCH
}
