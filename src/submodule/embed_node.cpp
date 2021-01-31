#pragma warning(disable : 4005)
#include "common.h"
#include <boost/algorithm/string.hpp>
#define NODE_WANT_INTERNALS 1
#include <uv.h>
#include <node.h>
#include <node_main_instance.h>
#include <node_native_module_env.h>
#include <iostream>
#include <filesystem>

using namespace fmt::literals;

namespace node
{
v8::StartupData *NodeMainInstance::GetEmbeddedSnapshotBlob() { return nullptr; }

const std::vector<size_t> *NodeMainInstance::GetIsolateDataIndexes() { return nullptr; }

namespace native_module
{
const bool has_code_cache = false;

void NativeModuleEnv::InitializeCodeCache() {}

}  // namespace native_module

}  // namespace node

int run_script(node::MultiIsolatePlatform *platform, const std::vector<std::string> &args,
               const std::vector<std::string> &exec_args, const std::string &source_path)
{
    int exit_code = 0;
    uv_loop_t loop;
    if (int ret = uv_loop_init(&loop); ret != 0) {
        std::cerr << "failed to init uv loop: {}"_format(uv_err_name(ret)) << std::endl;
        return 1;
    }
    std::shared_ptr<node::ArrayBufferAllocator> allocator = node::ArrayBufferAllocator::Create();
    v8::Isolate *isolate = NewIsolate(allocator.get(), &loop, platform);
    if (isolate == nullptr) {
        std::cerr << "failed to create v8 isolate" << std::endl;
        return 1;
    }
    {
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        std::unique_ptr<node::IsolateData, decltype(&node::FreeIsolateData)> isolate_data(
            node::CreateIsolateData(isolate, &loop, platform, allocator.get()),
            node::FreeIsolateData);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = node::NewContext(isolate);
        if (context.IsEmpty()) {
            std::cerr << "failed to create v8 context" << std::endl;
            return 1;
        }
        v8::Context::Scope context_scope(context);
        std::unique_ptr<node::Environment, decltype(&node::FreeEnvironment)> env(
            node::CreateEnvironment(isolate_data.get(), context, args, exec_args),
            node::FreeEnvironment);
        std::string code = R"(
            globalThis.require = require('module').createRequire(process.cwd() + '/');
            require('vm').runInThisContext(require('fs').readFileSync(`{0}`, encoding='utf8'), `{0}`);
        )"_format(source_path);
        v8::MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(env.get(), code.c_str());
        if (loadenv_ret.IsEmpty()) {
            std::cerr << "failed to load env, there has been a JS exception" << std::endl;
            return 1;
        }
        {
            v8::SealHandleScope seal(isolate);
            bool more;
            do {
                uv_run(&loop, UV_RUN_DEFAULT);
                platform->DrainTasks(isolate);
                more = uv_loop_alive(&loop);
                if (more) {
                    continue;
                }
                node::EmitBeforeExit(env.get());
                more = uv_loop_alive(&loop);
            } while (more == true);
        }
        exit_code = node::EmitExit(env.get());
        node::Stop(env.get());
    }
    bool platform_finished = false;
    platform->AddIsolateFinishedCallback(
        isolate, [](void *data) { *static_cast<bool *>(data) = true; }, &platform_finished);
    platform->UnregisterIsolate(isolate);
    isolate->Dispose();
    while (!platform_finished) {
        uv_run(&loop, UV_RUN_ONCE);
    }
    if (int err = uv_loop_close(&loop); err != 0) {
        std::cerr << "failed to close uv loop" << std::endl;
    }
    return exit_code;
}

void print_usage(const char *arg0)
{
    std::string exec_name = std::filesystem::path(arg0).filename().string();
    std::cerr << "Usage: {} [options] [ script.js ]"_format(exec_name) << std::endl;
}

int main(int argc, char **argv)
{
    argv = uv_setup_args(argc, argv);
    std::vector<std::string> args(argv, argv + argc);
    std::vector<std::string> exec_args;
    std::vector<std::string> errors;
    int exit_code = node::InitializeNodeWithArgs(&args, &exec_args, &errors);
    for (const std::string &error : errors) {
        std::cerr << "failed to init node with args: {}"_format(error.c_str()) << std::endl;
    }
    if (exit_code != 0) {
        return exit_code;
    }
    std::unique_ptr<node::MultiIsolatePlatform> platform = node::MultiIsolatePlatform::Create(4);
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    if (argc < 2 || !boost::algorithm::ends_with(argv[argc - 1], ".js")) {
        print_usage(argv[0]);
        return 1;
    }
    std::string source_path = std::filesystem::path(argv[argc - 1]).generic_string();
    int ret = run_script(platform.get(), args, exec_args, source_path);
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    return ret;
}
