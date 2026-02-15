// BuLang Test Runner - Console only (no graphics)
// Runs all .bu scripts in a directory, reports pass/fail/crash

#include "interpreter.hpp"
#include "platform.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

// ============================================================
// Colors
// ============================================================
#define C_RESET   "\033[0m"
#define C_RED     "\033[1;31m"
#define C_GREEN   "\033[1;32m"
#define C_YELLOW  "\033[1;33m"
#define C_CYAN    "\033[1;36m"

// ============================================================
// File loader for includes
// ============================================================
struct FileLoaderContext
{
    const char *searchPaths[8];
    int pathCount;
    char fullPath[512];
    char buffer[1024 * 1024];
};

const char *multiPathFileLoader(const char *filename, size_t *outSize, void *userdata)
{
    FileLoaderContext *ctx = (FileLoaderContext *)userdata;
    for (int i = 0; i < ctx->pathCount; i++)
    {
        snprintf(ctx->fullPath, sizeof(ctx->fullPath), "%s/%s", ctx->searchPaths[i], filename);
        FILE *f = fopen(ctx->fullPath, "rb");
        if (!f) continue;
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        if (size <= 0 || size >= (long)sizeof(ctx->buffer)) { fclose(f); continue; }
        fseek(f, 0, SEEK_SET);
        size_t bytesRead = fread(ctx->buffer, 1, size, f);
        fclose(f);
        if (bytesRead != (size_t)size) continue;
        ctx->buffer[bytesRead] = '\0';
        *outSize = bytesRead;
        return ctx->buffer;
    }
    *outSize = 0;
    return nullptr;
}

// ============================================================
// Helpers
// ============================================================
static std::string loadFile(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

static std::vector<std::string> listBuFiles(const char *dir)
{
    std::vector<std::string> files;
    DIR *d = opendir(dir);
    if (!d) return files;
    struct dirent *entry;
    while ((entry = readdir(d)) != nullptr)
    {
        std::string name = entry->d_name;
        if (name.size() > 3 && name.substr(name.size() - 3) == ".bu")
        {
            files.push_back(std::string(dir) + "/" + name);
        }
    }
    closedir(d);
    std::sort(files.begin(), files.end());
    return files;
}

static std::string getBasename(const std::string &path)
{
    size_t pos = path.find_last_of('/');
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

// ============================================================
// Test Native Struct: Point (x, y as floats)
// ============================================================
struct TestPoint
{
    float x;
    float y;
};

static void point_ctor(Interpreter *vm, void *buffer, int argc, Value *args)
{
    TestPoint *p = (TestPoint *)buffer;
    p->x = (argc > 0) ? (float)args[0].asNumber() : 0.0f;
    p->y = (argc > 1) ? (float)args[1].asNumber() : 0.0f;
}

// ============================================================
// Test Native Struct: Rect (x, y, w, h as floats + readOnly area)
// ============================================================
struct TestRect
{
    float x, y, w, h;
};

static void rect_ctor(Interpreter *vm, void *buffer, int argc, Value *args)
{
    TestRect *r = (TestRect *)buffer;
    r->x = (argc > 0) ? (float)args[0].asNumber() : 0.0f;
    r->y = (argc > 1) ? (float)args[1].asNumber() : 0.0f;
    r->w = (argc > 2) ? (float)args[2].asNumber() : 0.0f;
    r->h = (argc > 3) ? (float)args[3].asNumber() : 0.0f;
}

// ============================================================
// Test Native Class: Accumulator (methods + properties)
// ============================================================
struct TestAccumulator
{
    double value;
    int callCount;
    double minVal;
    double maxVal;
};

static void *accum_ctor(Interpreter *vm, int argCount, Value *args)
{
    TestAccumulator *a = new TestAccumulator();
    a->value = (argCount > 0) ? args[0].asNumber() : 0.0;
    a->callCount = 0;
    a->minVal = a->value;
    a->maxVal = a->value;
    return a;
}

static void accum_dtor(Interpreter *vm, void *instance)
{
    delete static_cast<TestAccumulator *>(instance);
}

// method: add(n) - returns self for chaining
static int accum_add(Interpreter *vm, void *data, int argCount, Value *args)
{
    TestAccumulator *a = static_cast<TestAccumulator *>(data);
    if (argCount < 1) return 0;
    double n = args[0].asNumber();
    a->value += n;
    a->callCount++;
    if (a->value < a->minVal) a->minVal = a->value;
    if (a->value > a->maxVal) a->maxVal = a->value;
    vm->pushFloat((float)a->value);
    return 1;
}

// method: sub(n)
static int accum_sub(Interpreter *vm, void *data, int argCount, Value *args)
{
    TestAccumulator *a = static_cast<TestAccumulator *>(data);
    if (argCount < 1) return 0;
    double n = args[0].asNumber();
    a->value -= n;
    a->callCount++;
    if (a->value < a->minVal) a->minVal = a->value;
    if (a->value > a->maxVal) a->maxVal = a->value;
    vm->pushFloat((float)a->value);
    return 1;
}

// method: reset()
static int accum_reset(Interpreter *vm, void *data, int argCount, Value *args)
{
    TestAccumulator *a = static_cast<TestAccumulator *>(data);
    a->value = 0;
    a->callCount = 0;
    a->minVal = 0;
    a->maxVal = 0;
    return 0;
}

// method: get_stats() - returns 3 values (value, min, max)
static int accum_get_stats(Interpreter *vm, void *data, int argCount, Value *args)
{
    TestAccumulator *a = static_cast<TestAccumulator *>(data);
    vm->pushFloat((float)a->value);
    vm->pushFloat((float)a->minVal);
    vm->pushFloat((float)a->maxVal);
    return 3;
}

// property getter: value
static Value accum_get_value(Interpreter *vm, void *instance)
{
    TestAccumulator *a = static_cast<TestAccumulator *>(instance);
    return vm->makeFloat((float)a->value);
}

// property setter: value
static void accum_set_value(Interpreter *vm, void *instance, Value val)
{
    TestAccumulator *a = static_cast<TestAccumulator *>(instance);
    a->value = val.asNumber();
}

// property getter: count (read-only)
static Value accum_get_count(Interpreter *vm, void *instance)
{
    TestAccumulator *a = static_cast<TestAccumulator *>(instance);
    return vm->makeInt(a->callCount);
}

// ============================================================
// Bridge test: C++ reads field from script class instance
// native_read_field(instance, fieldName) -> value
// ============================================================
static int native_read_field(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 2 || !args[1].isString())
    {
        vm->pushNil();
        return 1;
    }

    String *fieldName = args[1].asString();

    // Script class instance
    if (args[0].isClassInstance())
    {
        ClassInstance *inst = args[0].asClassInstance();
        uint8_t idx;
        if (inst->klass->fieldNames.get(fieldName, &idx))
        {
            vm->push(inst->fields[idx]);
            return 1;
        }
    }
    // Script struct instance
    else if (args[0].isStructInstance())
    {
        StructInstance *inst = args[0].asStructInstance();
        uint8 idx;
        if (inst->def->names.get(fieldName, &idx))
        {
            vm->push(inst->values[idx]);
            return 1;
        }
    }

    vm->pushNil();
    return 1;
}

// ============================================================
// Bridge test: C++ writes field on script class instance
// native_write_field(instance, fieldName, value)
// ============================================================
static int native_write_field(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 3 || !args[1].isString())
    {
        vm->pushBool(false);
        return 1;
    }

    String *fieldName = args[1].asString();
    Value newVal = args[2];

    if (args[0].isClassInstance())
    {
        ClassInstance *inst = args[0].asClassInstance();
        uint8_t idx;
        if (inst->klass->fieldNames.get(fieldName, &idx))
        {
            inst->fields[idx] = newVal;
            vm->pushBool(true);
            return 1;
        }
    }
    else if (args[0].isStructInstance())
    {
        StructInstance *inst = args[0].asStructInstance();
        uint8 idx;
        if (inst->def->names.get(fieldName, &idx))
        {
            inst->values[idx] = newVal;
            vm->pushBool(true);
            return 1;
        }
    }

    vm->pushBool(false);
    return 1;
}

// ============================================================
// Bridge test: C++ calls a script function by name
// native_call_fn(funcName, arg) -> result
// ============================================================
static int native_call_fn(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 2 || !args[0].isString())
    {
        vm->pushNil();
        return 1;
    }

    const char *fnName = args[0].asString()->chars();
    vm->push(args[1]); // push the argument onto stack

    if (vm->callFunctionAuto(fnName, 1))
    {
        return 1; // result already on stack
    }

    vm->pushNil();
    return 1;
}

// ============================================================
// Bridge test: C++ calls a script method on class instance
// native_call_method(instance, methodName, arg) -> result
// ============================================================
static int native_call_method(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 3 || !args[1].isString())
    {
        vm->pushNil();
        return 1;
    }

    Value instance = args[0];
    const char *methodName = args[1].asString()->chars();
    Value methodArgs[1] = { args[2] };

    if (vm->callMethod(instance, methodName, 1, methodArgs))
    {
        return 1;
    }

    vm->pushNil();
    return 1;
}

// ============================================================
// Bridge test: C++ reads/writes global variables
// native_get_global(name) -> value
// native_set_global(name, value) -> bool
// ============================================================
static int native_get_global(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 1 || !args[0].isString())
    {
        vm->pushNil();
        return 1;
    }
    const char *name = args[0].asString()->chars();
    Value val;
    if (vm->tryGetGlobal(name, &val))
    {
        vm->push(val);
    }
    else
    {
        vm->pushNil();
    }
    return 1;
}

static int native_set_global(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 2 || !args[0].isString())
    {
        vm->pushBool(false);
        return 1;
    }
    const char *name = args[0].asString()->chars();
    bool ok = vm->setGlobal(name, args[1]);
    vm->pushBool(ok);
    return 1;
}

// ============================================================
// Bridge test: C++ creates and returns an array to script
// native_make_range(start, end) -> [start, start+1, ..., end-1]
// ============================================================
static int native_make_range(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 2)
    {
        vm->pushNil();
        return 1;
    }
    int start = (int)args[0].asNumber();
    int end = (int)args[1].asNumber();

    Value arrVal = vm->makeArray();
    ArrayInstance *arr = arrVal.as.array;
    for (int i = start; i < end; i++)
    {
        arr->values.push(vm->makeInt(i));
    }
    vm->push(arrVal);
    return 1;
}

// ============================================================
// Bridge test: C++ creates and returns a map to script
// native_make_info(name, age) -> {name: name, age: age}
// ============================================================
static int native_make_info(Interpreter *vm, int argCount, Value *args)
{
    if (argCount < 2)
    {
        vm->pushNil();
        return 1;
    }

    Value mapVal = vm->makeMap();
    MapInstance *map = mapVal.as.map;
    map->table.set(vm->createString("name"), args[0]);
    map->table.set(vm->createString("age"), args[1]);
    vm->push(mapVal);
    return 1;
}

// ============================================================
// Native process test hook: callable only from process context
// native_proc_ping(x) -> x + 1
// ============================================================
static int native_proc_ping(Interpreter *vm, Process *proc, int argCount, Value *args)
{
    if (!proc)
    {
        vm->push(vm->makeInt(-999));
        return 1;
    }

    if (argCount < 1 || !args[0].isNumber())
    {
        vm->push(vm->makeInt(-1));
        return 1;
    }

    vm->push(vm->makeInt(args[0].asInt() + 1));
    return 1;
}

// ============================================================
// Register all test native bindings
// ============================================================
static void registerTestBindings(Interpreter &vm)
{
    // --- Bridge functions (C++ <-> script objects) ---
    vm.registerNative("native_read_field", native_read_field, -1);
    vm.registerNative("native_write_field", native_write_field, -1);
    vm.registerNative("native_call_fn", native_call_fn, 2);
    vm.registerNative("native_call_method", native_call_method, -1);
    vm.registerNative("native_get_global", native_get_global, 1);
    vm.registerNative("native_set_global", native_set_global, 2);
    vm.registerNative("native_make_range", native_make_range, 2);
    vm.registerNative("native_make_info", native_make_info, 2);
    vm.registerNativeProcess("native_proc_ping", native_proc_ping, 1);

    // --- Native Struct: Point ---
    auto *point = vm.registerNativeStruct("Point", sizeof(TestPoint), point_ctor);
    vm.addStructField(point, "x", offsetof(TestPoint, x), FieldType::FLOAT);
    vm.addStructField(point, "y", offsetof(TestPoint, y), FieldType::FLOAT);

    // --- Native Struct: Rect ---
    auto *rect = vm.registerNativeStruct("Rect", sizeof(TestRect), rect_ctor);
    vm.addStructField(rect, "x", offsetof(TestRect, x), FieldType::FLOAT);
    vm.addStructField(rect, "y", offsetof(TestRect, y), FieldType::FLOAT);
    vm.addStructField(rect, "w", offsetof(TestRect, w), FieldType::FLOAT);
    vm.addStructField(rect, "h", offsetof(TestRect, h), FieldType::FLOAT);

    // --- Native Class: Accumulator ---
    auto *accum = vm.registerNativeClass("Accumulator", accum_ctor, accum_dtor, 1);
    vm.addNativeMethod(accum, "add", accum_add);
    vm.addNativeMethod(accum, "sub", accum_sub);
    vm.addNativeMethod(accum, "reset", accum_reset);
    vm.addNativeMethod(accum, "get_stats", accum_get_stats);
    vm.addNativeProperty(accum, "value", accum_get_value, accum_set_value);
    vm.addNativeProperty(accum, "count", accum_get_count); // read-only (no setter)
}

static void configureTestInterpreter(Interpreter &vm, FileLoaderContext &ctx)
{
    vm.registerAll();
    registerTestBindings(vm);

    ctx.searchPaths[0] = "scripts";
    ctx.searchPaths[1] = "scripts/test";
    ctx.searchPaths[2] = ".";
    ctx.pathCount = 3;
    vm.setFileLoader(multiPathFileLoader, &ctx);
}

// ============================================================
// Run a single script in-process
// Returns: 0=OK, 1=compile/runtime error
// ============================================================
static int runScript(const char *path)
{
    std::string code = loadFile(path);
    if (code.empty())
    {
        fprintf(stderr, "  Could not read: %s\n", path);
        return 1;
    }

    Interpreter vm;
    FileLoaderContext ctx;
    configureTestInterpreter(vm, ctx);

    bool ok = vm.run(code.c_str(), false);
    return ok ? 0 : 1;
}

static bool runLoadedMainProcess(Interpreter &vm, int maxSteps)
{
    Process *proc = vm.callProcess("__main_process__", 0);
    if (!proc)
    {
        fprintf(stderr, "  bytecode roundtrip: failed to spawn '__main_process__'\n");
        return false;
    }

    if (proc->frameCount <= 0)
    {
        fprintf(stderr, "  bytecode roundtrip: process has no execution context\n");
        return false;
    }

    vm.setCurrentProcess(proc);
    vm.setCurrentExec(proc);

    for (int i = 0; i < maxSteps; i++)
    {
        ProcessResult result = vm.run_process(proc);

        if (result.reason == ProcessResult::ERROR)
        {
            return false;
        }

        if (result.reason == ProcessResult::PROCESS_DONE)
        {
            return true;
        }
    }

    fprintf(stderr, "  bytecode roundtrip: execution exceeded %d steps\n", maxSteps);
    return false;
}

static int runBytecodeRoundtrip()
{
    static const char *kSource = R"BU(
def double_it_bc(x) { return x * 2; }

struct Pair { a, b }

class Counter {
    var value;
    def init(v) { self.value = v; }
    def add(n) { self.value = self.value + n; return self.value; }
}

var p = Pair(10, 32);
if (p.a + p.b != 42) { throw "pair"; }

var c = Counter(40);
if (c.add(2) != 42) { throw "counter"; }

if (double_it_bc(21) != 42) { throw "fn"; }
if (native_proc_ping(41) != 42) { throw "native_proc"; }

var __bytecode_ok = 12345;
)BU";

    char bytecodePath[256];
    snprintf(bytecodePath, sizeof(bytecodePath), "/tmp/bu_roundtrip_%d.bu.bc", (int)getpid());

    {
        Interpreter compileVm;
        FileLoaderContext compileCtx;
        configureTestInterpreter(compileVm, compileCtx);

        if (!compileVm.compileToBytecode(kSource, bytecodePath, false))
        {
            std::remove(bytecodePath);
            return 1;
        }
    }

    Interpreter runVm;
    FileLoaderContext runCtx;
    configureTestInterpreter(runVm, runCtx);

    if (!runVm.loadBytecode(bytecodePath))
    {
        std::remove(bytecodePath);
        return 1;
    }

    std::remove(bytecodePath);

    if (!runLoadedMainProcess(runVm, 4096))
    {
        return 1;
    }

    Value okVal;
    if (!runVm.tryGetGlobal("__bytecode_ok", &okVal))
    {
        fprintf(stderr, "  bytecode roundtrip: missing global '__bytecode_ok'\n");
        return 1;
    }

    if (!okVal.isNumber() || (int)okVal.asNumber() != 12345)
    {
        fprintf(stderr, "  bytecode roundtrip: unexpected '__bytecode_ok' value\n");
        return 1;
    }

    return 0;
}

static int runBytecodeRoundtripSafe(bool verbose, int timeoutSecs)
{
    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 2;
    }

    if (pid == 0)
    {
        if (!verbose)
        {
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
        }
        int result = runBytecodeRoundtrip();
        _exit(result);
    }

    int status = 0;
    int elapsed = 0;
    int maxWait = timeoutSecs * 10;
    while (elapsed < maxWait)
    {
        pid_t ret = waitpid(pid, &status, WNOHANG);
        if (ret == pid) break;
        if (ret < 0) return 2;
        usleep(100000);
        elapsed++;
    }

    if (elapsed >= maxWait)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        return 3;
    }

    if (WIFSIGNALED(status))
    {
        return 2;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    return 2;
}

// ============================================================
// Run in child process to catch crashes/segfaults
// Returns: 0=OK, 1=error, 2=crash, 3=timeout
// ============================================================
static int runScriptSafe(const char *path, bool verbose, int timeoutSecs)
{
    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 2;
    }

    if (pid == 0)
    {
        // Child
        if (!verbose)
        {
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
        }
        int result = runScript(path);
        _exit(result);
    }

    // Parent: wait with timeout
    int status = 0;
    int elapsed = 0;
    int maxWait = timeoutSecs * 10;
    while (elapsed < maxWait)
    {
        pid_t ret = waitpid(pid, &status, WNOHANG);
        if (ret == pid) break;
        if (ret < 0) return 2;
        usleep(100000); // 100ms
        elapsed++;
    }

    if (elapsed >= maxWait)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        return 3;
    }

    if (WIFSIGNALED(status))
    {
        return 2;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    return 2;
}

// ============================================================
// Main
// ============================================================
static void usage(const char *prog)
{
    printf("BuLang Test Runner\n\n");
    printf("Usage: %s [options] [file.bu | directory]\n\n", prog);
    printf("  -v          Verbose (show script output)\n");
    printf("  -t <secs>   Timeout per test (default: 5)\n");
    printf("  -h          Help\n\n");
    printf("Default: runs all scripts/test/*.bu\n");
}

int main(int argc, char *argv[])
{
    bool verbose = false;
    int timeout = 5;
    const char *target = nullptr;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-v") == 0)                       verbose = true;
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)  timeout = atoi(argv[++i]);
        else if (strcmp(argv[i], "-h") == 0)                  { usage(argv[0]); return 0; }
        else                                                   target = argv[i];
    }

    // Collect .bu files
    std::vector<std::string> files;

    if (target)
    {
        DIR *d = opendir(target);
        if (d) { closedir(d); files = listBuFiles(target); }
        else   { files.push_back(target); }
    }
    else
    {
        const char *dirs[] = { "scripts/test", "../scripts/test" };
        for (auto &dir : dirs)
        {
            files = listBuFiles(dir);
            if (!files.empty()) break;
        }
    }

    if (files.empty())
    {
        fprintf(stderr, "No .bu test files found.\n");
        return 1;
    }

    size_t totalScheduled = files.size() + 1; // +1 internal bytecode roundtrip test
    printf(C_CYAN "BuLang Test Runner" C_RESET " - %zu test(s), timeout %ds\n\n", totalScheduled, timeout);

    int passed = 0, failed = 0, crashed = 0, timedout = 0;
    std::vector<std::string> failedNames;

    printf("  %-45s", "00_bytecode_roundtrip_internal");
    fflush(stdout);
    int bytecodeResult = runBytecodeRoundtripSafe(verbose, timeout);
    switch (bytecodeResult)
    {
    case 0:
        printf(C_GREEN "PASS" C_RESET "\n");
        passed++;
        break;
    case 1:
        printf(C_RED "FAIL" C_RESET "\n");
        failed++;
        failedNames.push_back("00_bytecode_roundtrip_internal");
        break;
    case 2:
        printf(C_RED "CRASH" C_RESET "\n");
        crashed++;
        failedNames.push_back("00_bytecode_roundtrip_internal (CRASH)");
        break;
    case 3:
        printf(C_YELLOW "TIMEOUT" C_RESET " (>%ds)\n", timeout);
        timedout++;
        failedNames.push_back("00_bytecode_roundtrip_internal (TIMEOUT)");
        break;
    }

    for (auto &file : files)
    {
        std::string name = getBasename(file);
        printf("  %-45s", name.c_str());
        fflush(stdout);

        int result = runScriptSafe(file.c_str(), verbose, timeout);

        switch (result)
        {
        case 0:
            printf(C_GREEN "PASS" C_RESET "\n");
            passed++;
            break;
        case 1:
            printf(C_RED "FAIL" C_RESET "\n");
            failed++;
            failedNames.push_back(name);
            break;
        case 2:
            printf(C_RED "CRASH" C_RESET "\n");
            crashed++;
            failedNames.push_back(name + " (CRASH)");
            break;
        case 3:
            printf(C_YELLOW "TIMEOUT" C_RESET " (>%ds)\n", timeout);
            timedout++;
            failedNames.push_back(name + " (TIMEOUT)");
            break;
        }
    }

    // Summary
    int total = passed + failed + crashed + timedout;
    printf("\n");

    if (!failedNames.empty())
    {
        printf(C_RED "Failed:" C_RESET "\n");
        for (auto &n : failedNames)
            printf("  - %s\n", n.c_str());
        printf("\n");
    }

    printf(C_CYAN "%d/%d passed" C_RESET, passed, total);
    if (failed > 0)   printf(", " C_RED "%d failed" C_RESET, failed);
    if (crashed > 0)  printf(", " C_RED "%d crashed" C_RESET, crashed);
    if (timedout > 0) printf(", " C_YELLOW "%d timeout" C_RESET, timedout);
    printf("\n");

    return (failed + crashed + timedout > 0) ? 1 : 0;
}
