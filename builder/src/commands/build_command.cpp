#include "commands/build_command.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "build/android_builder.hpp"
#include "build/desktop_builder.hpp"
#include "build/web_builder.hpp"
#include "io/fs_utils.hpp"
#include "model/loader.hpp"

namespace fs = std::filesystem;

namespace crosside::commands
{
    namespace
    {

        struct BuildOptions
        {
            std::string kind;
            std::string name;
            std::vector<std::string> targets;

            std::string mode = "release";
            std::string projectFile;
            std::string release;

            bool full = false;
            bool run = false;
            bool detach = false;
            bool skipModules = true;
            bool dryRun = false;
            std::vector<int> abis = {0, 1};
            int port = 8080;
        };

        constexpr const char *kDesktopOutputFolder =
#ifdef _WIN32
            "Windows";
#else
            "Linux";
#endif

        std::string lower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        std::string normalizeTarget(const std::string &value)
        {
            const std::string key = lower(value);
            if (key == "desktop" || key == "linux" || key == "windows" || key == "native")
            {
                return "desktop";
            }
            if (key == "android")
            {
                return "android";
            }
            if (key == "web" || key == "emscripten")
            {
                return "web";
            }
            return "";
        }

        bool isCompilableSourcePath(const fs::path &path)
        {
            const std::string ext = lower(path.extension().string());
            return ext == ".c" || ext == ".cc" || ext == ".cpp" || ext == ".cxx" || ext == ".mm" || ext == ".xpp";
        }

        std::vector<int> parseAbis(const std::string &value)
        {
            std::vector<int> out;
            std::string token;
            for (size_t i = 0; i <= value.size(); ++i)
            {
                if (i == value.size() || value[i] == ',')
                {
                    const std::string key = lower(token);
                    if (!key.empty())
                    {
                        if (key == "arm7" || key == "armeabi" || key == "armeabi-v7a")
                        {
                            if (std::find(out.begin(), out.end(), 0) == out.end())
                            {
                                out.push_back(0);
                            }
                        }
                        else if (key == "arm64" || key == "arm64-v8a" || key == "aarch64")
                        {
                            if (std::find(out.begin(), out.end(), 1) == out.end())
                            {
                                out.push_back(1);
                            }
                        }
                    }
                    token.clear();
                    continue;
                }
                token.push_back(value[i]);
            }
            if (out.empty())
            {
                out = {0, 1};
            }
            return out;
        }

        std::optional<fs::path> resolveSingleSourceFile(const fs::path &repoRoot, const std::string &hint)
        {
            if (hint.empty())
            {
                return std::nullopt;
            }

            fs::path raw(hint);
            std::vector<fs::path> candidates;
            if (raw.is_absolute())
            {
                candidates.push_back(raw);
            }
            else
            {
                candidates.push_back(fs::current_path() / raw);
                candidates.push_back(repoRoot / raw);
                candidates.push_back(repoRoot / "projects" / raw);
            }

            for (const auto &candidate : candidates)
            {
                std::error_code ec;
                const fs::path abs = fs::absolute(candidate, ec);
                if (ec || !isCompilableSourcePath(abs))
                {
                    continue;
                }

                if (fs::exists(abs, ec) && fs::is_regular_file(abs, ec))
                {
                    return abs;
                }
            }

            return std::nullopt;
        }

        std::optional<crosside::model::ProjectSpec> tryCreateSingleFileProject(
            const crosside::Context &ctx,
            const fs::path &repoRoot,
            const BuildOptions &opt)
        {
            if (opt.kind != "app" || !opt.projectFile.empty())
            {
                return std::nullopt;
            }

            const auto sourceFile = resolveSingleSourceFile(repoRoot, opt.name);
            if (!sourceFile.has_value())
            {
                return std::nullopt;
            }

            crosside::model::ProjectSpec project;
            project.name = sourceFile->stem().string();
            if (project.name.empty())
            {
                project.name = "app";
            }
            project.root = sourceFile->parent_path();
            project.filePath = sourceFile.value();
            project.src.push_back(sourceFile.value());
            project.modules = crosside::model::loadSingleFileModules(repoRoot, ctx);
            return project;
        }

        std::vector<std::string> normalizeAbiNames(const std::vector<int> &abis)
        {
            std::vector<std::string> out;
            for (int abi : abis)
            {
                const std::string name = abi == 1 ? "arm64-v8a" : "armeabi-v7a";
                if (std::find(out.begin(), out.end(), name) == out.end())
                {
                    out.push_back(name);
                }
            }
            if (out.empty())
            {
                out = {"armeabi-v7a", "arm64-v8a"};
            }
            return out;
        }

        bool hasModuleBinaryInDir(const crosside::model::ModuleSpec &module, const fs::path &dir)
        {
            const fs::path staticLib = dir / ("lib" + module.name + ".a");
            const fs::path sharedLib = dir / ("lib" + module.name + ".so");
            return fs::exists(staticLib) || fs::exists(sharedLib);
        }

        bool validateProjectModuleArtifacts(
            const crosside::Context &ctx,
            const crosside::model::ModuleMap &modules,
            const std::vector<std::string> &activeModules,
            const std::string &target,
            const std::vector<int> &abis)
        {
            const std::vector<std::string> allModules = crosside::model::moduleClosure(activeModules, modules, ctx);
            bool ok = true;

            for (const auto &moduleName : allModules)
            {
                auto it = modules.find(moduleName);
                if (it == modules.end())
                {
                    ctx.error("Missing module definition: ", moduleName);
                    ok = false;
                    continue;
                }

                const auto &module = it->second;
                if (target == "desktop")
                {
                    const fs::path outDir = module.dir / kDesktopOutputFolder;
                    if (!hasModuleBinaryInDir(module, outDir))
                    {
                        ctx.error(
                            "Missing desktop module binary for ", module.name,
                            " (expected ", (outDir / ("lib" + module.name + ".a")).string(),
                            " or ", (outDir / ("lib" + module.name + ".so")).string(), ")");
                        ok = false;
                    }
                    continue;
                }

                if (target == "web")
                {
                    // For web, modules may be provided by Emscripten flags
                    // (e.g. SDL2 via -s USE_SDL=2) without local static/shared libs.
                    continue;
                }

                if (target == "android")
                {
                    for (const auto &abiName : normalizeAbiNames(abis))
                    {
                        const fs::path outDir = module.dir / "Android" / abiName;
                        if (!hasModuleBinaryInDir(module, outDir))
                        {
                            ctx.error(
                                "Missing android module binary for ", module.name, " [", abiName, "]",
                                " (expected ", (outDir / ("lib" + module.name + ".a")).string(),
                                " or ", (outDir / ("lib" + module.name + ".so")).string(), ")");
                            ok = false;
                        }
                    }
                    continue;
                }
            }

            return ok;
        }

        std::vector<std::string> normalizeTargets(
            const std::vector<std::string> &input,
            const std::string &fallback,
            const crosside::Context &ctx)
        {
            std::vector<std::string> out;
            if (input.empty())
            {
                out.push_back(fallback);
                return out;
            }

            for (const auto &value : input)
            {
                std::string normalized = normalizeTarget(value);
                if (normalized.empty())
                {
                    ctx.error("Unknown target: ", value);
                    continue;
                }
                if (std::find(out.begin(), out.end(), normalized) == out.end())
                {
                    out.push_back(normalized);
                }
            }

            if (out.empty())
            {
                out.push_back(fallback);
            }
            return out;
        }

        bool parseSubject(
            const std::vector<std::string> &positionals,
            std::string &kind,
            std::string &name,
            std::vector<std::string> &targets,
            const crosside::Context &ctx)
        {
            if (positionals.empty())
            {
                ctx.error("build: missing subject");
                return false;
            }

            const std::string first = lower(positionals[0]);
            if (first == "runner")
            {
                kind = "runner";
                name = "bugame";
                targets.assign(positionals.begin() + 1, positionals.end());
                return true;
            }

            if (first == "app" || first == "project" || first == "proj")
            {
                if (positionals.size() < 2)
                {
                    ctx.error("build app: missing project name");
                    return false;
                }
                kind = "app";
                name = positionals[1];
                targets.assign(positionals.begin() + 2, positionals.end());
                return true;
            }

            kind = "app";
            name = positionals[0];
            targets.assign(positionals.begin() + 1, positionals.end());
            return true;
        }

        bool parseOptions(const std::vector<std::string> &args, BuildOptions &opt, const crosside::Context &ctx)
        {
            std::vector<std::string> positionals;

            for (size_t i = 0; i < args.size(); ++i)
            {
                const std::string &arg = args[i];
                if (arg == "--full")
                {
                    opt.full = true;
                    continue;
                }
                if (arg == "--run")
                {
                    opt.run = true;
                    continue;
                }
                if (arg == "--detach")
                {
                    opt.detach = true;
                    continue;
                }
                if (arg == "--skip-modules")
                {
                    opt.skipModules = true;
                    continue;
                }
                if (arg == "--build-modules")
                {
                    opt.skipModules = false;
                    continue;
                }
                if (arg == "--dry-run")
                {
                    opt.dryRun = true;
                    continue;
                }
                if (arg == "--mode")
                {
                    if (i + 1 >= args.size())
                    {
                        ctx.error("--mode requires value");
                        return false;
                    }
                    opt.mode = lower(args[++i]);
                    if (opt.mode != "release" && opt.mode != "debug")
                    {
                        ctx.error("Invalid --mode: ", opt.mode, " (use release|debug)");
                        return false;
                    }
                    continue;
                }
                if (arg == "--abis")
                {
                    if (i + 1 >= args.size())
                    {
                        ctx.error("--abis requires value");
                        return false;
                    }
                    opt.abis = parseAbis(args[++i]);
                    continue;
                }
                if (arg == "--project-file")
                {
                    if (i + 1 >= args.size())
                    {
                        ctx.error("--project-file requires value");
                        return false;
                    }
                    opt.projectFile = args[++i];
                    continue;
                }
                if (arg == "--release")
                {
                    if (i + 1 >= args.size())
                    {
                        ctx.error("--release requires value");
                        return false;
                    }
                    opt.release = args[++i];
                    continue;
                }
                if (arg == "--port")
                {
                    if (i + 1 >= args.size())
                    {
                        ctx.error("--port requires value");
                        return false;
                    }
                    try
                    {
                        opt.port = std::stoi(args[++i]);
                    }
                    catch (...)
                    {
                        ctx.error("Invalid --port value");
                        return false;
                    }
                    if (opt.port <= 0 || opt.port > 65535)
                    {
                        ctx.error("Invalid --port: ", opt.port);
                        return false;
                    }
                    continue;
                }

                if (arg.rfind("--", 0) == 0)
                {
                    ctx.error("Unknown build option: ", arg);
                    return false;
                }
                positionals.push_back(arg);
            }

            std::vector<std::string> rawTargets;
            if (!parseSubject(positionals, opt.kind, opt.name, rawTargets, ctx))
            {
                return false;
            }

            opt.targets = normalizeTargets(rawTargets, crosside::model::defaultTargetFromConfig(fs::current_path()), ctx);
            return true;
        }

        bool buildProjectForTarget(
            const crosside::Context &ctx,
            const fs::path &repoRoot,
            const crosside::model::ProjectSpec &project,
            const crosside::model::ModuleMap &modules,
            const std::vector<std::string> &activeModules,
            const std::string &target,
            const BuildOptions &opt,
            const std::string &effectiveMode)
        {
            if (target == "desktop")
            {
                return crosside::build::buildProjectDesktop(
                    ctx,
                    project,
                    modules,
                    activeModules,
                    opt.full,
                    effectiveMode,
                    opt.run,
                    opt.detach,
                    !opt.skipModules);
            }
            if (target == "android")
            {
                return crosside::build::buildProjectAndroid(
                    ctx,
                    repoRoot,
                    project,
                    modules,
                    activeModules,
                    opt.full,
                    opt.run,
                    !opt.skipModules,
                    opt.abis);
            }
            if (target == "web")
            {
                return crosside::build::buildProjectWeb(
                    ctx,
                    repoRoot,
                    project,
                    modules,
                    activeModules,
                    opt.full,
                    opt.run,
                    opt.detach,
                    !opt.skipModules,
                    opt.port);
            }
            ctx.error("Unsupported target: ", target);
            return false;
        }

        fs::path runnerTemplateRoot(const fs::path &repoRoot, const std::string &target)
        {
            if (target == "desktop")
            {
                return repoRoot / "Templates" / "Desktop" / "runtime";
            }
            if (target == "web")
            {
                return repoRoot / "Templates" / "Web" / "runtime";
            }
            if (target == "android")
            {
                return repoRoot / "Templates" / "Android" / "runtime";
            }
            return repoRoot / "Templates" / target / "runtime";
        }

        fs::path runnerBinRoot(const fs::path &repoRoot, const std::string &target)
        {
            return repoRoot / "bin" / target;
        }

        void copyFilePermissions(const fs::path &src, const fs::path &dst)
        {
            std::error_code ec;
            const fs::perms perms = fs::status(src, ec).permissions();
            if (ec)
            {
                return;
            }
            fs::permissions(dst, perms, ec);
        }

        bool copyFileTo(const fs::path &src, const fs::path &dst, const crosside::Context &ctx)
        {
            std::error_code ec;
            if (!fs::exists(src, ec) || !fs::is_regular_file(src, ec))
            {
                ctx.error("Template source file missing: ", src.string());
                return false;
            }
            crosside::io::ensureDir(dst.parent_path());
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
            if (ec)
            {
                ctx.error("Failed copy ", src.string(), " -> ", dst.string(), " : ", ec.message());
                return false;
            }
            copyFilePermissions(src, dst);
            return true;
        }

        bool copyDirectoryRecursive(const fs::path &src, const fs::path &dst, const crosside::Context &ctx)
        {
            std::error_code ec;
            if (!fs::exists(src, ec) || !fs::is_directory(src, ec))
            {
                ctx.error("Template source directory missing: ", src.string());
                return false;
            }

            for (const auto &entry : fs::recursive_directory_iterator(src, ec))
            {
                if (ec)
                {
                    ctx.error("Failed walk directory: ", src.string(), " : ", ec.message());
                    return false;
                }

                const fs::path rel = fs::relative(entry.path(), src, ec);
                if (ec)
                {
                    ctx.error("Failed compute relative path for ", entry.path().string());
                    return false;
                }
                const fs::path outPath = dst / rel;
                if (entry.is_directory(ec))
                {
                    crosside::io::ensureDir(outPath);
                    continue;
                }
                if (entry.is_regular_file(ec))
                {
                    if (!copyFileTo(entry.path(), outPath, ctx))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        bool stageRunnerTemplate(
            const crosside::Context &ctx,
            const fs::path &repoRoot,
            const crosside::model::ProjectSpec &project,
            const std::string &target)
        {
            const fs::path templateRoot = runnerTemplateRoot(repoRoot, target);
            std::error_code ec;
            fs::remove_all(templateRoot, ec);
            crosside::io::ensureDir(templateRoot);

            if (target == "desktop")
            {
                const fs::path exe = project.root / project.name;
                return copyFileTo(exe, templateRoot / project.name, ctx);
            }

            if (target == "web")
            {
                const fs::path webRoot = project.root / "Web";
                return copyDirectoryRecursive(webRoot, templateRoot, ctx);
            }

            if (target == "android")
            {
                for (const std::string &abiName : {std::string("armeabi-v7a"), std::string("arm64-v8a")})
                {
                    const fs::path libFile = project.root / "Android" / abiName / ("lib" + project.name + ".so");
                    std::error_code existsEc;
                    if (!fs::exists(libFile, existsEc))
                    {
                        continue;
                    }
                    if (!copyFileTo(libFile, templateRoot / "Android" / abiName / libFile.filename(), ctx))
                    {
                        return false;
                    }
                }
                return true;
            }

            ctx.error("Unsupported runner target: ", target);
            return false;
        }

        bool stageRunnerBinOutput(
            const crosside::Context &ctx,
            const fs::path &repoRoot,
            const std::string &target)
        {
            const fs::path srcRoot = runnerTemplateRoot(repoRoot, target);
            const fs::path dstRoot = runnerBinRoot(repoRoot, target);

            std::error_code ec;
            fs::remove_all(dstRoot, ec);
            crosside::io::ensureDir(dstRoot);

            return copyDirectoryRecursive(srcRoot, dstRoot, ctx);
        }

    } // namespace

    int runBuildCommand(const crosside::Context &ctx, const fs::path &repoRoot, const std::vector<std::string> &args)
    {
        BuildOptions opt;
        if (!parseOptions(args, opt, ctx))
        {
            return 1;
        }

        ctx.log("Build type: ", opt.kind);
        ctx.log("Name: ", opt.name);

        std::string targetText;
        for (size_t i = 0; i < opt.targets.size(); ++i)
        {
            if (i > 0)
            {
                targetText += ", ";
            }
            targetText += opt.targets[i];
        }
        ctx.log("Targets: ", targetText);
        ctx.log("Desktop mode: ", opt.mode);
        if (!opt.release.empty())
        {
            ctx.log("Release profile: ", opt.release);
        }
        if (opt.detach && !opt.run)
        {
            ctx.warn("--detach has no effect without --run");
        }
        std::string abiText;
        for (size_t i = 0; i < opt.abis.size(); ++i)
        {
            if (i > 0)
            {
                abiText += ", ";
            }
            abiText += opt.abis[i] == 1 ? "arm64-v8a" : "armeabi-v7a";
        }
        ctx.log("Android ABIs: ", abiText);

        auto modules = crosside::model::discoverModules(repoRoot / "modules", ctx);
        const auto defaultWebShell = crosside::model::loadDefaultWebShell(repoRoot);

        for (const auto &target : opt.targets)
        {
            const std::string effectiveMode = target == "desktop" ? opt.mode : "release";
            if (target != "desktop" && opt.mode != "release")
            {
                ctx.log("Target ", target, " uses release mode (desktop mode ignored)");
            }
            if (target == "android" && opt.detach && opt.run)
            {
                ctx.warn("--detach ignored for android --run");
            }

            if (opt.kind == "runner")
            {
                auto project = crosside::model::loadKnownWorkspaceProject(repoRoot, "", ctx);
                if (!project.has_value())
                {
                    ctx.error("Known workspace project not found for runner build");
                    return 1;
                }

                const fs::path emptyContentRoot = repoRoot / "build" / "builder" / "_empty_content";
                std::error_code ec;
                fs::remove_all(emptyContentRoot, ec);
                crosside::io::ensureDir(emptyContentRoot);

                project->desktopContentRoot = emptyContentRoot;
                project->androidContentRoot = emptyContentRoot;
                project->webContentRoot = emptyContentRoot;

                std::vector<std::string> activeModules;

                ctx.log("Build runner template -> ", target);
                if (!opt.dryRun)
                {
                    if (!buildProjectForTarget(ctx, repoRoot, project.value(), modules, activeModules, target, opt, effectiveMode))
                    {
                        return 1;
                    }
                    if (!stageRunnerTemplate(ctx, repoRoot, project.value(), target))
                    {
                        return 1;
                    }
                    ctx.log("Runner template staged at ", runnerTemplateRoot(repoRoot, target).string());
                    if (!stageRunnerBinOutput(ctx, repoRoot, target))
                    {
                        return 1;
                    }
                    ctx.log("Runner binary staged at ", runnerBinRoot(repoRoot, target).string());
                }
                continue;
            }

            auto project = tryCreateSingleFileProject(ctx, repoRoot, opt);
            const bool sourceHint = (opt.kind == "app" && opt.projectFile.empty() && isCompilableSourcePath(fs::path(opt.name)));
            if (sourceHint && !project.has_value())
            {
                ctx.error("Single file source not found: ", opt.name);
                return 1;
            }
            if (!project.has_value())
            {
                if (opt.projectFile.empty())
                {
                    project = crosside::model::loadKnownWorkspaceProject(repoRoot, opt.name, ctx);
                    if (project.has_value())
                    {
                        ctx.log("Using known workspace project layout for ", opt.name);
                    }
                }

                if (!project.has_value())
                {
                    const fs::path projectFile = crosside::model::resolveProjectFile(repoRoot, opt.name, opt.projectFile);
                    const bool useProjectDefaultRelease = !(target == "desktop" && opt.release.empty());
                    if (!useProjectDefaultRelease)
                    {
                        ctx.log("Desktop build without --release: using base project content");
                    }
                    project = crosside::model::loadProjectFile(projectFile, ctx, opt.release, useProjectDefaultRelease);
                    if (!project.has_value())
                    {
                        ctx.error("Project not found: ", projectFile.string());
                        return 1;
                    }
                }
            }
            else
            {
                if (!opt.release.empty())
                {
                    ctx.warn("--release ignored in single-file mode");
                }
                ctx.log("Single file mode: ", project->filePath.string(), " (no main.mk)");
                std::string mods;
                for (std::size_t i = 0; i < project->modules.size(); ++i)
                {
                    if (i > 0)
                    {
                        mods += ", ";
                    }
                    mods += project->modules[i];
                }
                if (mods.empty())
                {
                    mods = "(none)";
                }
                ctx.log("Single file modules: ", mods);
            }

            if (project->webShell.empty() && defaultWebShell.has_value())
            {
                project->webShell = defaultWebShell->string();
            }

            std::vector<std::string> activeModules = project->modules.empty()
                                                         ? crosside::model::loadGlobalModules(repoRoot, ctx)
                                                         : project->modules;

            const std::string outputName = crosside::model::projectOutputName(project.value());
            ctx.log("Build app ", project->name, " from ", project->filePath.string());
            if (outputName != project->name)
            {
                ctx.log("Output name: ", outputName, " (core/lib name stays ", project->name, ")");
            }
            {
                const std::string buildCacheKey = crosside::model::projectBuildCacheKey(project.value());
                if (!buildCacheKey.empty() && buildCacheKey != project->name)
                {
                    ctx.log("Build cache key: ", buildCacheKey);
                }
            }
            ctx.log("Auto-build modules: ", opt.skipModules ? "off" : "on");
            if (opt.dryRun)
            {
                continue;
            }

            if (opt.skipModules && !validateProjectModuleArtifacts(ctx, modules, activeModules, target, opt.abis))
            {
                return 1;
            }

            if (!buildProjectForTarget(ctx, repoRoot, project.value(), modules, activeModules, target, opt, effectiveMode))
            {
                return 1;
            }
        }

        return 0;
    }

} // namespace crosside::commands
