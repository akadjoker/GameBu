// BuLang Script Runner (direct execution, no child processes)

#include "interpreter.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>

#define C_RESET "\033[0m"
#define C_RED "\033[1;31m"
#define C_GREEN "\033[1;32m"
#define C_CYAN "\033[1;36m"

struct FileLoaderContext
{
    const char *searchPaths[8];
    int pathCount;
    char fullPath[512];
    char buffer[1024 * 1024];
};

static const char *multiPathFileLoader(const char *filename, size_t *outSize, void *userdata)
{
    FileLoaderContext *ctx = (FileLoaderContext *)userdata;

    for (int i = 0; i < ctx->pathCount; i++)
    {
        snprintf(ctx->fullPath, sizeof(ctx->fullPath), "%s/%s", ctx->searchPaths[i], filename);

        FILE *f = fopen(ctx->fullPath, "rb");
        if (!f)
            continue;

        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        if (size <= 0 || size >= (long)sizeof(ctx->buffer))
        {
            fclose(f);
            continue;
        }

        fseek(f, 0, SEEK_SET);
        size_t bytesRead = fread(ctx->buffer, 1, size, f);
        fclose(f);

        if (bytesRead != (size_t)size)
            continue;

        ctx->buffer[bytesRead] = '\0';
        *outSize = bytesRead;
        return ctx->buffer;
    }

    *outSize = 0;
    return nullptr;
}

static std::string loadFile(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return "";

    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

static std::vector<std::string> listBuFiles(const char *dir)
{
    std::vector<std::string> files;
    DIR *d = opendir(dir);
    if (!d)
        return files;

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

static void configureInterpreter(Interpreter &vm, FileLoaderContext &ctx)
{
    vm.registerAll();

    ctx.searchPaths[0] = "scripts";
    ctx.searchPaths[1] = "scripts/test";
    ctx.searchPaths[2] = ".";
    ctx.pathCount = 3;

    vm.setFileLoader(multiPathFileLoader, &ctx);
}

static int runScript(const char *path)
{
    std::string code = loadFile(path);
    if (code.empty())
    {
        fprintf(stderr, "Could not read: %s\n", path);
        return 1;
    }

    Interpreter vm;
    FileLoaderContext ctx;
    configureInterpreter(vm, ctx);

    bool ok = vm.run(code.c_str(), false);
    if (!ok)
        return 1;

    // Headless game loop: needed so process/frame/fiber scheduling advances.
    static constexpr float kFixedDeltaSeconds = 1.0f / 60.0f;
    static constexpr int kMaxUpdateSteps = 120000; // Safety cap

    int steps = 0;
    while (vm.getTotalAliveProcesses() > 0 && steps < kMaxUpdateSteps)
    {
        vm.update(kFixedDeltaSeconds);
        steps++;
    }

    if (vm.getTotalAliveProcesses() > 0)
    {
        fprintf(stderr, "Script exceeded max update steps (%d): %s\n", kMaxUpdateSteps, path);
        return 1;
    }

    return 0;
}

static void usage(const char *prog)
{
    printf("BuLang Script Runner\n\n");
    printf("Usage: %s [file.bu | directory]\n\n", prog);
    printf("  -h          Help\n\n");
    printf("Default: runs all scripts/test/*.bu in-process\n");
}

int main(int argc, char *argv[])
{
    const char *target = nullptr;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            usage(argv[0]);
            return 0;
        }

        target = argv[i];
    }

    std::vector<std::string> files;

    if (target)
    {
        DIR *d = opendir(target);
        if (d)
        {
            closedir(d);
            files = listBuFiles(target);
        }
        else
        {
            files.push_back(target);
        }
    }
    else
    {
        const char *dirs[] = {"scripts/test", "../scripts/test"};
        for (auto &dir : dirs)
        {
            files = listBuFiles(dir);
            if (!files.empty())
                break;
        }
    }

    if (files.empty())
    {
        fprintf(stderr, "No .bu script files found.\n");
        return 1;
    }

    printf(C_CYAN "BuLang Script Runner" C_RESET " - %zu script(s)\n\n", files.size());

    int passed = 0;
    int failed = 0;

    for (const auto &file : files)
    {
        std::string name = getBasename(file);
        printf("  %-45s", name.c_str());
        fflush(stdout);

        int result = runScript(file.c_str());
        if (result == 0)
        {
            printf(C_GREEN "PASS" C_RESET "\n");
            passed++;
        }
        else
        {
            printf(C_RED "FAIL" C_RESET "\n");
            failed++;
        }
    }

    printf("\n" C_CYAN "%d/%d passed" C_RESET, passed, passed + failed);
    if (failed > 0)
        printf(", " C_RED "%d failed" C_RESET, failed);
    printf("\n");

    return failed > 0 ? 1 : 0;
}
