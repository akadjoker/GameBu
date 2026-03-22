#include "model/loader.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "io/fs_utils.hpp"
#include "io/json_reader.hpp"

namespace fs = std::filesystem;
using nlohmann::json;

namespace crosside::model
{

    namespace
    {

        std::vector<std::string> toStringList(const json &node)
        {
            std::vector<std::string> out;
            if (!node.is_array())
            {
                return out;
            }

            for (const auto &item : node)
            {
                if (!item.is_string())
                {
                    continue;
                }
                std::string value = item.get<std::string>();
                if (!value.empty())
                {
                    out.push_back(value);
                }
            }
            return out;
        }

        std::string lower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        bool isAtSourcePattern(const std::string &value)
        {
            fs::path src(value);
            const std::string filename = src.filename().string();
            if (filename.size() < 3)
            {
                return false;
            }
            return filename[0] == '@' && src.has_extension();
        }

        void appendUnique(std::vector<std::string> &items, const std::string &value)
        {
            if (value.empty())
            {
                return;
            }
            if (std::find(items.begin(), items.end(), value) == items.end())
            {
                items.push_back(value);
            }
        }

        std::vector<std::string> expandAtSourceEntries(const fs::path &baseDir, const std::vector<std::string> &items)
        {
            std::vector<std::string> out;

            for (const auto &item : items)
            {
                if (item.empty())
                {
                    continue;
                }

                if (!isAtSourcePattern(item))
                {
                    appendUnique(out, item);
                    continue;
                }

                const fs::path patternPath(item);
                const std::string expectedExt = lower(patternPath.extension().string());
                const fs::path searchRoot = fs::absolute(baseDir / patternPath.parent_path());

                std::error_code ec;
                if (!fs::exists(searchRoot, ec) || !fs::is_directory(searchRoot, ec))
                {
                    continue;
                }

                std::vector<fs::path> matches;
                for (const auto &entry : fs::recursive_directory_iterator(searchRoot, ec))
                {
                    if (ec || !entry.is_regular_file(ec))
                    {
                        continue;
                    }
                    const fs::path file = entry.path();
                    if (lower(file.extension().string()) != expectedExt)
                    {
                        continue;
                    }
                    matches.push_back(file);
                }

                std::sort(matches.begin(), matches.end(), [](const fs::path &a, const fs::path &b)
                          { return a.generic_string() < b.generic_string(); });

                for (const auto &match : matches)
                {
                    fs::path rel;
                    try
                    {
                        rel = fs::relative(match, baseDir);
                    }
                    catch (...)
                    {
                        rel = match;
                    }
                    appendUnique(out, rel.generic_string());
                }
            }

            return out;
        }

        std::unordered_map<std::string, std::string> toStringMap(const json &node)
        {
            std::unordered_map<std::string, std::string> out;
            if (!node.is_object())
            {
                return out;
            }

            for (auto it = node.begin(); it != node.end(); ++it)
            {
                if (!it.value().is_string())
                {
                    continue;
                }
                out[it.key()] = it.value().get<std::string>();
            }

            return out;
        }

        void mergeJsonObjects(json &base, const json &overlay)
        {
            if (!base.is_object() || !overlay.is_object())
            {
                return;
            }

            for (auto it = overlay.begin(); it != overlay.end(); ++it)
            {
                const std::string key = it.key();
                const json &value = it.value();
                if (base.contains(key) && base[key].is_object() && value.is_object())
                {
                    mergeJsonObjects(base[key], value);
                }
                else
                {
                    base[key] = value;
                }
            }
        }

        fs::path resolveProjectRootFromData(const fs::path &projectFile, const json &data)
        {
            const fs::path rootBase = fs::absolute(projectFile.parent_path());
            if (data.contains("Path") && data["Path"].is_string())
            {
                fs::path fromJson(data["Path"].get<std::string>());
                return (fromJson.is_absolute() ? fromJson : fs::absolute(rootBase / fromJson)).lexically_normal();
            }
            return rootBase.lexically_normal();
        }

        std::optional<fs::path> resolveReleaseFile(const fs::path &projectRoot, const std::string &releaseRef)
        {
            if (releaseRef.empty())
            {
                return std::nullopt;
            }

            fs::path raw(releaseRef);
            std::vector<fs::path> candidates;
            if (raw.is_absolute())
            {
                candidates.push_back(raw);
            }
            else
            {
                candidates.push_back(projectRoot / raw);

                const bool hasDirHints = releaseRef.find('/') != std::string::npos || releaseRef.find('\\') != std::string::npos;
                if (!hasDirHints && raw.extension().empty())
                {
                    candidates.push_back(projectRoot / "releases" / (releaseRef + ".json"));
                }
            }

            std::error_code ec;
            for (const auto &candidate : candidates)
            {
                const fs::path path = fs::absolute(candidate).lexically_normal();
                if (fs::exists(path, ec) && fs::is_regular_file(path, ec))
                {
                    return path;
                }
            }

            return std::nullopt;
        }

        BuildArgs parseBuildArgs(const json &node)
        {
            BuildArgs out;
            if (!node.is_object())
            {
                return out;
            }

            if (node.contains("CPP"))
            {
                if (node["CPP"].is_string())
                {
                    out.cpp = io::splitFlags(node["CPP"].get<std::string>());
                }
                else
                {
                    out.cpp = toStringList(node["CPP"]);
                }
            }
            if (node.contains("CC"))
            {
                if (node["CC"].is_string())
                {
                    out.cc = io::splitFlags(node["CC"].get<std::string>());
                }
                else
                {
                    out.cc = toStringList(node["CC"]);
                }
            }
            if (node.contains("LD"))
            {
                if (node["LD"].is_string())
                {
                    out.ld = io::splitFlags(node["LD"].get<std::string>());
                }
                else
                {
                    out.ld = toStringList(node["LD"]);
                }
            }
            return out;
        }

        PlatformBlock parsePlatformBlock(const json &node, const fs::path &moduleDir)
        {
            PlatformBlock out;
            if (!node.is_object())
            {
                return out;
            }

            if (node.contains("src"))
            {
                out.src = expandAtSourceEntries(moduleDir, toStringList(node["src"]));
            }
            if (node.contains("include"))
            {
                out.include = toStringList(node["include"]);
            }

            if (node.contains("CPP_ARGS"))
            {
                if (node["CPP_ARGS"].is_string())
                {
                    out.cppArgs = io::splitFlags(node["CPP_ARGS"].get<std::string>());
                }
                else
                {
                    out.cppArgs = toStringList(node["CPP_ARGS"]);
                }
            }
            if (node.contains("CC_ARGS"))
            {
                if (node["CC_ARGS"].is_string())
                {
                    out.ccArgs = io::splitFlags(node["CC_ARGS"].get<std::string>());
                }
                else
                {
                    out.ccArgs = toStringList(node["CC_ARGS"]);
                }
            }
            if (node.contains("LD_ARGS"))
            {
                if (node["LD_ARGS"].is_string())
                {
                    out.ldArgs = io::splitFlags(node["LD_ARGS"].get<std::string>());
                }
                else
                {
                    out.ldArgs = toStringList(node["LD_ARGS"]);
                }
            }

            if (node.contains("template") && node["template"].is_string())
            {
                out.shellTemplate = node["template"].get<std::string>();
            }

            if (node.contains("static") && node["static"].is_boolean())
            {
                out.staticLib = node["static"].get<bool>();
            }
            else if (node.contains("shared") && node["shared"].is_boolean())
            {
                out.staticLib = !node["shared"].get<bool>();
            }

            return out;
        }

        fs::path toAbsolute(const fs::path &base, const std::string &value)
        {
            fs::path path(value);
            if (path.is_absolute())
            {
                return path;
            }
            return fs::absolute(base / path);
        }

        bool pathExists(const fs::path &path)
        {
            std::error_code ec;
            return fs::exists(path, ec);
        }

        bool hasKnownBuGameWorkspaceLayout(const fs::path &repoRoot)
        {
            return pathExists(repoRoot / "main" / "src" / "main.cpp") &&
                   pathExists(repoRoot / "libbu" / "src") &&
                   pathExists(repoRoot / "graphics" / "src") &&
                   pathExists(repoRoot / "vendor" / "raylib" / "src" / "rcore.c") &&
                   pathExists(repoRoot / "vendor" / "miniz" / "src") &&
                   pathExists(repoRoot / "vendor" / "poly2tri" / "src") &&
                   pathExists(repoRoot / "vendor" / "box2d" / "src");
        }

        std::string lowerCopy(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        bool matchesKnownProjectHint(const fs::path &repoRoot, const std::string &projectHint)
        {
            if (projectHint.empty())
            {
                return true;
            }

            const std::string key = lowerCopy(projectHint);
            if (key == "bugame" || key == "main" || key == "app")
            {
                return true;
            }

            const std::string rootName = lowerCopy(repoRoot.filename().string());
            if (!rootName.empty() && key == rootName)
            {
                return true;
            }

            fs::path hintPath(projectHint);
            const std::string fileName = lowerCopy(hintPath.filename().string());
            if (fileName == "cmakelists.txt")
            {
                return true;
            }

            return false;
        }

        void appendPathIfExists(std::vector<fs::path> &out, const fs::path &path)
        {
            std::error_code ec;
            if (fs::exists(path, ec) && fs::is_regular_file(path, ec))
            {
                out.push_back(fs::absolute(path));
            }
        }

        void appendGlobPaths(std::vector<fs::path> &out, const fs::path &baseDir, const std::vector<std::string> &patterns)
        {
            const auto expanded = expandAtSourceEntries(baseDir, patterns);
            for (const auto &item : expanded)
            {
                appendPathIfExists(out, baseDir / item);
            }
        }

        ProjectSpec makeKnownBuGameProject(const fs::path &repoRoot)
        {
            ProjectSpec project;
            project.name = "bugame";
            project.root = fs::absolute(repoRoot);
            project.filePath = project.root / "CMakeLists.txt";
            project.exportName = project.name;
            project.buildCache = project.name;

            appendGlobPaths(project.src, project.root, {
                "main/src/@.cpp",
                "libbu/src/@.cpp",
                "graphics/src/@.cpp",
                "vendor/miniz/src/@.c",
                "vendor/poly2tri/src/common/@.cc",
                "vendor/poly2tri/src/sweep/@.cc",
                "vendor/box2d/src/@.cpp",
                "vendor/box2d/src/collision/@.cpp",
                "vendor/box2d/src/common/@.cpp",
                "vendor/box2d/src/dynamics/@.cpp",
                "vendor/box2d/src/rope/@.cpp",
            });

            appendPathIfExists(project.src, project.root / "vendor" / "raylib" / "src" / "rcore.c");
            appendPathIfExists(project.src, project.root / "vendor" / "raylib" / "src" / "rmodels.c");
            appendPathIfExists(project.src, project.root / "vendor" / "raylib" / "src" / "rshapes.c");
            appendPathIfExists(project.src, project.root / "vendor" / "raylib" / "src" / "rtext.c");
            appendPathIfExists(project.src, project.root / "vendor" / "raylib" / "src" / "rtextures.c");
            appendPathIfExists(project.src, project.root / "vendor" / "raylib" / "src" / "utils.c");

            project.include = {
                project.root / "main" / "src",
                project.root / "libbu" / "include",
                project.root / "libbu" / "src",
                project.root / "graphics" / "src",
                project.root / "vendor" / "raylib" / "src",
                project.root / "vendor" / "box2d" / "include",
                project.root / "vendor" / "poly2tri",
                project.root / "vendor" / "poly2tri" / "include",
                project.root / "vendor" / "miniz" / "include",
                project.root / "vendor" / "eigen",
                project.root / "vendor" / "MiniDNN" / "include",
            };

            project.main.cc = {
                "-std=gnu11",
            };

            project.main.cpp = {
                "-std=c++17",
                "-fexceptions",
                "-frtti",
                "-fno-strict-aliasing",
            };

#ifdef _WIN32
            project.desktop.cc = {
                "-DPLATFORM_DESKTOP",
                "-DGRAPHICS_API_OPENGL_33",
                "-D_CRT_SECURE_NO_WARNINGS",
            };
            project.desktop.cpp = project.desktop.cc;
            project.desktop.ld = {
                "-lopengl32",
                "-lgdi32",
                "-lwinmm",
                "-lshell32",
                "-luser32",
            };
#else
            project.desktop.cc = {
                "-DPLATFORM_DESKTOP",
                "-DGRAPHICS_API_OPENGL_33",
            };
            project.desktop.cpp = project.desktop.cc;
            project.desktop.ld = {
                "-lm",
                "-lpthread",
                "-lGL",
                "-ldl",
                "-lrt",
                "-lX11",
            };
#endif

            project.web.cc = {
                "-DPLATFORM_WEB",
                "-DGRAPHICS_API_OPENGL_ES2",
                "-DBU_ENABLE_OS_PROCESS=0",
                "-DBU_ENABLE_OS_EXEC=0",
                "-DBU_ENABLE_SOCKETS=0",
            };
            project.web.cpp = project.web.cc;
            project.web.ld = {
                "-sUSE_GLFW=3",
            };
            {
                const fs::path templateShell = project.root / "Templates" / "Web" / "shell.html";
                std::error_code shellEc;
                if (fs::exists(templateShell, shellEc))
                {
                    project.webShell = templateShell.string();
                }
                else
                {
                    project.webShell = (project.root / "vendor" / "raylib" / "src" / "shell.html").string();
                }
            }
            project.webContentRoot = project.root;

            project.android.cc = {
                "-DPLATFORM_ANDROID",
                "-DGRAPHICS_API_OPENGL_ES2",
                "-DSTBIR_NO_SIMD",
                "-DBU_ENABLE_OS_PROCESS=0",
                "-DBU_ENABLE_OS_EXEC=0",
                "-DBU_ENABLE_SOCKETS=0",
            };
            project.android.cpp = project.android.cc;
            project.android.ld = {
                "-lm",
                "-llog",
                "-landroid",
                "-lEGL",
                "-lGLESv2",
                "-latomic",
                "-lc",
            };
            project.androidPackage = "com.djokersoft.bugame";
            project.androidLabel = "BuGame";
            project.androidContentRoot = project.root;
            project.desktopContentRoot = project.root;

            return project;
        }

        std::optional<fs::path> resolveReleaseFolderPath(const fs::path &repoRoot, const std::string &releaseHint)
        {
            if (releaseHint.empty())
            {
                return std::nullopt;
            }

            fs::path raw(releaseHint);
            std::vector<fs::path> candidates;
            if (raw.is_absolute())
            {
                candidates.push_back(raw);
            }
            else
            {
                candidates.push_back(fs::absolute(repoRoot / raw));
                candidates.push_back(fs::absolute(repoRoot / "releases" / raw));
            }

            std::error_code ec;
            for (const auto &candidate : candidates)
            {
                if (fs::exists(candidate, ec) && fs::is_directory(candidate, ec))
                {
                    return candidate.lexically_normal();
                }
            }

            return std::nullopt;
        }

    } // namespace

    std::string hostDesktopKey()
    {
#ifdef _WIN32
        return "windows";
#else
        return "linux";
#endif
    }

    std::string defaultTargetFromConfig(const fs::path &repoRoot)
    {
        fs::path configPath = repoRoot / "config.json";
        if (!fs::exists(configPath))
        {
            return "desktop";
        }

        try
        {
            json data = io::loadJsonFile(configPath);
            json root = data;
            if (data.contains("Configuration") && data["Configuration"].is_object())
            {
                root = data["Configuration"];
            }
            if (!root.contains("Session") || !root["Session"].is_object())
            {
                return "desktop";
            }
            int value = root["Session"].value("CurrentPlatform", 0);
            if (value == 1)
            {
                return "android";
            }
            if (value == 2)
            {
                return "web";
            }
        }
        catch (...)
        {
            return "desktop";
        }
        return "desktop";
    }

    std::optional<ModuleSpec> loadModuleFile(const fs::path &moduleFile, const crosside::Context &ctx)
    {
        try
        {
            json data = io::loadJsonFile(moduleFile);

            ModuleSpec module;
            module.dir = fs::absolute(moduleFile.parent_path());
            module.name = data.value("module", module.dir.filename().string());
            module.staticLib = data.value("static", true);

            module.depends = toStringList(data.value("depends", json::array()));
            module.systems = toStringList(data.value("system", json::array()));

            module.main.src = expandAtSourceEntries(module.dir, toStringList(data.value("src", json::array())));
            module.main.include = toStringList(data.value("include", json::array()));

            if (data.contains("CPP_ARGS") && data["CPP_ARGS"].is_string())
            {
                module.main.cppArgs = io::splitFlags(data["CPP_ARGS"].get<std::string>());
            }
            if (data.contains("CC_ARGS") && data["CC_ARGS"].is_string())
            {
                module.main.ccArgs = io::splitFlags(data["CC_ARGS"].get<std::string>());
            }
            if (data.contains("LD_ARGS") && data["LD_ARGS"].is_string())
            {
                module.main.ldArgs = io::splitFlags(data["LD_ARGS"].get<std::string>());
            }

            if (data.contains("plataforms") && data["plataforms"].is_object())
            {
                const auto &platforms = data["plataforms"];

                std::string desktopKey = hostDesktopKey();
                if (platforms.contains(desktopKey))
                {
                    module.desktop = parsePlatformBlock(platforms[desktopKey], module.dir);
                }
                if (platforms.contains("android"))
                {
                    module.android = parsePlatformBlock(platforms["android"], module.dir);
                }
                if (platforms.contains("emscripten"))
                {
                    module.web = parsePlatformBlock(platforms["emscripten"], module.dir);
                }
            }

            return module;
        }
        catch (const std::exception &e)
        {
            ctx.error("Failed parse module ", moduleFile.string(), " : ", e.what());
            return std::nullopt;
        }
    }

    std::optional<ProjectSpec> loadProjectFile(
        const fs::path &projectFile,
        const crosside::Context &ctx,
        const std::string &releaseOverride,
        bool useProjectDefaultRelease)
    {
        try
        {
            json data = io::loadJsonFile(projectFile);
            const fs::path projectRoot = resolveProjectRootFromData(projectFile, data);
            std::string releaseRef = releaseOverride;
            const bool hasExplicitReleaseOverride = !releaseOverride.empty();
            bool autoExportNameFromReleaseProfile = false;
            std::string releaseProfileName;
            if (releaseRef.empty() && useProjectDefaultRelease && data.contains("Release") && data["Release"].is_string())
            {
                releaseRef = data["Release"].get<std::string>();
            }
            if (!releaseRef.empty())
            {
                const auto releaseFile = resolveReleaseFile(projectRoot, releaseRef);
                if (!releaseFile.has_value())
                {
                    ctx.error("Release file not found: ", releaseRef);
                    return std::nullopt;
                }

                json releaseData = io::loadJsonFile(releaseFile.value());
                if (!releaseData.is_object())
                {
                    ctx.error("Release file must be a JSON object: ", releaseFile->string());
                    return std::nullopt;
                }

                // If user explicitly asked --release <profile> and profile doesn't define Name,
                // use release filename stem (e.g. piano.json -> piano) for outputs/artifacts.
                if (hasExplicitReleaseOverride && !releaseData.contains("Name"))
                {
                    releaseProfileName = releaseFile->stem().string();
                    autoExportNameFromReleaseProfile = !releaseProfileName.empty();
                }

                mergeJsonObjects(data, releaseData);
            }

            ProjectSpec project;
            project.filePath = fs::absolute(projectFile);
            project.name = data.value("Name", projectFile.stem().string());
            project.exportName = project.name;
            if (autoExportNameFromReleaseProfile)
            {
                project.exportName = releaseProfileName;
            }
            project.buildCache = data.value("BuildCache", "");
            if (project.buildCache.empty())
            {
                project.buildCache = data.value("BUILD_CACHE", "");
            }
            if (project.buildCache.empty())
            {
                project.buildCache = data.value("CACHE_KEY", "");
            }

            project.root = resolveProjectRootFromData(projectFile, data);

            project.modules = toStringList(data.value("Modules", json::array()));

            const auto resolvedProjectSrc = expandAtSourceEntries(project.root, toStringList(data.value("Src", json::array())));
            for (const auto &item : resolvedProjectSrc)
            {
                project.src.push_back(toAbsolute(project.root, item));
            }
            for (const auto &item : toStringList(data.value("Include", json::array())))
            {
                project.include.push_back(toAbsolute(project.root, item));
            }

            project.main = parseBuildArgs(data.value("Main", json::object()));
            project.desktop = parseBuildArgs(data.value("Desktop", json::object()));
            project.android = parseBuildArgs(data.value("Android", json::object()));
            project.web = parseBuildArgs(data.value("Web", json::object()));

            if (data.contains("Android") && data["Android"].is_object())
            {
                const auto &android = data["Android"];
                project.androidPackage = android.value("PACKAGE", "");
                project.androidActivity = android.value("ACTIVITY", "");
                project.androidLabel = android.value("LABEL", "");

                auto parsePathField = [&](const json &node, const std::string &key, fs::path &outPath)
                {
                    if (!node.contains(key) || !node[key].is_string())
                    {
                        return;
                    }
                    const std::string rel = node[key].get<std::string>();
                    if (rel.empty())
                    {
                        return;
                    }
                    outPath = toAbsolute(project.root, rel);
                };

                auto parsePathMapField = [&](const json &node, const std::string &key, std::unordered_map<std::string, fs::path> &outMap)
                {
                    if (!node.contains(key) || !node[key].is_object())
                    {
                        return;
                    }
                    for (auto it = node[key].begin(); it != node[key].end(); ++it)
                    {
                        if (!it.value().is_string())
                        {
                            continue;
                        }
                        const std::string rel = it.value().get<std::string>();
                        if (rel.empty())
                        {
                            continue;
                        }
                        outMap[it.key()] = toAbsolute(project.root, rel);
                    }
                };

                auto parsePathListField = [&](const json &node, const std::string &key, std::vector<fs::path> &outList)
                {
                    if (!node.contains(key))
                    {
                        return;
                    }

                    if (node[key].is_string())
                    {
                        const std::string rel = node[key].get<std::string>();
                        if (!rel.empty())
                        {
                            outList.push_back(toAbsolute(project.root, rel));
                        }
                        return;
                    }

                    if (!node[key].is_array())
                    {
                        return;
                    }

                    for (const auto &item : node[key])
                    {
                        if (!item.is_string())
                        {
                            continue;
                        }
                        const std::string rel = item.get<std::string>();
                        if (!rel.empty())
                        {
                            outList.push_back(toAbsolute(project.root, rel));
                        }
                    }
                };

                parsePathField(android, "ICON", project.androidIcon);
                parsePathMapField(android, "ICONS", project.androidIcons);
                parsePathField(android, "ROUND_ICON", project.androidRoundIcon);
                parsePathMapField(android, "ROUND_ICONS", project.androidRoundIcons);
                project.androidManifestMode = android.value("MANIFEST_MODE", "");
                if (project.androidManifestMode.empty())
                {
                    project.androidManifestMode = android.value("MANIFEST_TYPE", "");
                }

                parsePathListField(android, "JAVA_SOURCES", project.androidJavaSources);
                parsePathListField(android, "JAVA", project.androidJavaSources);
                parsePathListField(android, "JAVA_DIRS", project.androidJavaSources);

                std::string contentRoot = android.value("CONTENT_ROOT", "");
                if (contentRoot.empty())
                {
                    contentRoot = android.value("ASSET_ROOT", "");
                }
                if (contentRoot.empty())
                {
                    contentRoot = android.value("RELEASE_ROOT", "");
                }
                if (!contentRoot.empty())
                {
                    project.androidContentRoot = toAbsolute(project.root, contentRoot);
                }

                if (android.contains("ADAPTIVE_ICON") && android["ADAPTIVE_ICON"].is_object())
                {
                    const auto &adaptive = android["ADAPTIVE_ICON"];
                    parsePathField(adaptive, "FOREGROUND", project.androidAdaptiveForeground);
                    parsePathField(adaptive, "MONOCHROME", project.androidAdaptiveMonochrome);
                    if (adaptive.contains("BACKGROUND") && adaptive["BACKGROUND"].is_string())
                    {
                        const std::string value = adaptive["BACKGROUND"].get<std::string>();
                        if (!value.empty())
                        {
                            if (value.front() == '#')
                            {
                                project.androidAdaptiveBackgroundColor = value;
                            }
                            else
                            {
                                project.androidAdaptiveBackgroundImage = toAbsolute(project.root, value);
                            }
                        }
                    }
                    project.androidAdaptiveRound = adaptive.value("ROUND", true);
                }

                parsePathField(android, "ADAPTIVE_FOREGROUND", project.androidAdaptiveForeground);
                parsePathField(android, "ADAPTIVE_MONOCHROME", project.androidAdaptiveMonochrome);
                if (android.contains("ADAPTIVE_BACKGROUND") && android["ADAPTIVE_BACKGROUND"].is_string())
                {
                    const std::string value = android["ADAPTIVE_BACKGROUND"].get<std::string>();
                    if (!value.empty())
                    {
                        if (value.front() == '#')
                        {
                            project.androidAdaptiveBackgroundColor = value;
                            project.androidAdaptiveBackgroundImage.clear();
                        }
                        else
                        {
                            project.androidAdaptiveBackgroundImage = toAbsolute(project.root, value);
                            project.androidAdaptiveBackgroundColor.clear();
                        }
                    }
                }
                if (android.contains("ADAPTIVE_ROUND") && android["ADAPTIVE_ROUND"].is_boolean())
                {
                    project.androidAdaptiveRound = android["ADAPTIVE_ROUND"].get<bool>();
                }

                std::string manifestTemplate = android.value("MANIFEST_TEMPLATE", "");
                if (manifestTemplate.empty())
                {
                    manifestTemplate = android.value("MANIFEST", "");
                }
                if (!manifestTemplate.empty())
                {
                    project.androidManifestTemplate = toAbsolute(project.root, manifestTemplate);
                }

                if (android.contains("MANIFEST_VARS"))
                {
                    project.androidManifestVars = toStringMap(android["MANIFEST_VARS"]);
                }
            }
            if (data.contains("Desktop") && data["Desktop"].is_object())
            {
                const auto &desktop = data["Desktop"];

                std::string contentRoot = desktop.value("CONTENT_ROOT", "");
                if (contentRoot.empty())
                {
                    contentRoot = desktop.value("ASSET_ROOT", "");
                }
                if (contentRoot.empty())
                {
                    contentRoot = desktop.value("RELEASE_ROOT", "");
                }
                if (!contentRoot.empty())
                {
                    project.desktopContentRoot = toAbsolute(project.root, contentRoot);
                }
            }
            if (data.contains("Web") && data["Web"].is_object())
            {
                const auto &web = data["Web"];
                project.webShell = web.value("SHELL", "");

                std::string contentRoot = web.value("CONTENT_ROOT", "");
                if (contentRoot.empty())
                {
                    contentRoot = web.value("ASSET_ROOT", "");
                }
                if (contentRoot.empty())
                {
                    contentRoot = web.value("RELEASE_ROOT", "");
                }
                if (!contentRoot.empty())
                {
                    project.webContentRoot = toAbsolute(project.root, contentRoot);
                }
            }

            return project;
        }
        catch (const std::exception &e)
        {
            ctx.error("Failed parse project ", projectFile.string(), " : ", e.what());
            return std::nullopt;
        }
    }

    bool hasKnownWorkspaceProject(const fs::path &repoRoot, const std::string &projectHint)
    {
        return hasKnownBuGameWorkspaceLayout(repoRoot) && matchesKnownProjectHint(repoRoot, projectHint);
    }

    std::optional<ProjectSpec> loadKnownWorkspaceProject(
        const fs::path &repoRoot,
        const std::string &projectHint,
        const crosside::Context &ctx)
    {
        if (!hasKnownWorkspaceProject(repoRoot, projectHint))
        {
            return std::nullopt;
        }

        try
        {
            return makeKnownBuGameProject(repoRoot);
        }
        catch (const std::exception &e)
        {
            ctx.error("Failed synthesize known workspace project: ", e.what());
            return std::nullopt;
        }
    }

    std::optional<ReleaseSpec> loadReleaseFolder(
        const fs::path &repoRoot,
        const std::string &releaseHint,
        const crosside::Context &ctx)
    {
        const auto resolved = resolveReleaseFolderPath(repoRoot, releaseHint);
        if (!resolved.has_value())
        {
            ctx.error("Release folder not found: ", releaseHint);
            return std::nullopt;
        }

        ReleaseSpec release;
        release.root = resolved.value();
        release.name = release.root.filename().string();
        release.scriptsRoot = release.root / "scripts";
        if (release.name.empty())
        {
            release.name = "release";
        }

        const std::vector<fs::path> entryCandidates = {
            release.scriptsRoot / "main.buc",
            release.scriptsRoot / "main.bc",
            release.scriptsRoot / "main.bytecode",
        };

        std::error_code ec;
        for (const auto &candidate : entryCandidates)
        {
            if (fs::exists(candidate, ec) && fs::is_regular_file(candidate, ec))
            {
                release.entryBytecode = candidate;
                break;
            }
        }

        if (release.entryBytecode.empty())
        {
            ctx.error("Release entry bytecode not found in ", release.root.string(), " (expected scripts/main.buc)");
            return std::nullopt;
        }

        const fs::path assetsRoot = release.root / "assets";
        if (fs::exists(assetsRoot, ec) && fs::is_directory(assetsRoot, ec))
        {
            release.assetsRoot = assetsRoot;
        }

        const fs::path iconFile = release.root / "icon.png";
        if (fs::exists(iconFile, ec) && fs::is_regular_file(iconFile, ec))
        {
            release.iconFile = iconFile;
        }

        return release;
    }

    ModuleMap discoverModules(const fs::path &modulesRoot, const crosside::Context &ctx)
    {
        ModuleMap modules;
        if (!fs::exists(modulesRoot) && hasKnownBuGameWorkspaceLayout(modulesRoot.parent_path()))
        {
            return modules;
        }
        for (const auto &file : io::listModuleJsonFiles(modulesRoot))
        {
            auto spec = loadModuleFile(file, ctx);
            if (spec.has_value())
            {
                modules[spec->name] = spec.value();
            }
        }
        return modules;
    }

    fs::path resolveModuleFile(
        const fs::path &repoRoot,
        const std::string &moduleName,
        const std::string &explicitFile)
    {
        if (!explicitFile.empty())
        {
            fs::path path(explicitFile);
            if (!path.is_absolute())
            {
                path = fs::absolute(repoRoot / path);
            }
            return path;
        }
        return fs::absolute(repoRoot / "modules" / moduleName / "module.json");
    }

    fs::path resolveProjectFile(
        const fs::path &repoRoot,
        const std::string &projectHint,
        const std::string &explicitFile)
    {
        auto resolveFromDir = [](const fs::path &dir) -> fs::path
        {
            const fs::path mainMk = dir / "main.mk";
            if (fs::exists(mainMk))
            {
                return fs::absolute(mainMk);
            }

            const fs::path projectMk = dir / "project.mk";
            if (fs::exists(projectMk))
            {
                return fs::absolute(projectMk);
            }

            return fs::absolute(mainMk);
        };

        if (!explicitFile.empty())
        {
            fs::path path(explicitFile);
            if (!path.is_absolute())
            {
                path = fs::absolute(repoRoot / path);
            }
            return path;
        }

        fs::path hint(projectHint);
        if (hint.is_absolute())
        {
            if (fs::is_directory(hint))
            {
                return resolveFromDir(hint);
            }
            return fs::absolute(hint);
        }

        fs::path fromRepo = fs::absolute(repoRoot / hint);
        if (fs::exists(fromRepo))
        {
            if (fs::is_directory(fromRepo))
            {
                return resolveFromDir(fromRepo);
            }
            return fromRepo;
        }

        const fs::path projectsDir = fs::absolute(repoRoot / "projects" / projectHint);
        if (fs::exists(projectsDir) && fs::is_directory(projectsDir))
        {
            return resolveFromDir(projectsDir);
        }

        return fs::absolute(repoRoot / "projects" / projectHint / "main.mk");
    }

    std::vector<std::string> moduleClosure(
        const std::vector<std::string> &seedModules,
        const ModuleMap &modules,
        const crosside::Context &ctx)
    {
        std::vector<std::string> ordered;
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> active;

        std::function<void(const std::string &)> visit = [&](const std::string &name)
        {
            if (name.empty())
            {
                return;
            }
            if (visited.count(name) != 0U)
            {
                return;
            }
            if (active.count(name) != 0U)
            {
                ctx.warn("Circular dependency at ", name);
                return;
            }

            auto it = modules.find(name);
            if (it == modules.end())
            {
                ctx.warn("Missing module dependency: ", name);
                return;
            }

            active.insert(name);
            for (const auto &dep : it->second.depends)
            {
                if (!dep.empty() && dep != name)
                {
                    visit(dep);
                }
            }
            active.erase(name);

            visited.insert(name);
            ordered.push_back(name);
        };

        for (const auto &seed : seedModules)
        {
            visit(seed);
        }

        return ordered;
    }

    std::vector<std::string> loadGlobalModules(const fs::path &repoRoot, const crosside::Context &)
    {
        fs::path configPath = repoRoot / "config.json";
        if (!fs::exists(configPath))
        {
            return {};
        }

        try
        {
            json data = io::loadJsonFile(configPath);
            json root = data;
            if (data.contains("Configuration") && data["Configuration"].is_object())
            {
                root = data["Configuration"];
            }
            return toStringList(root.value("Modules", json::array()));
        }
        catch (...)
        {
            return {};
        }
    }

    std::vector<std::string> loadSingleFileModules(
        const fs::path &repoRoot,
        const crosside::Context &ctx)
    {
        fs::path configPath = repoRoot / "config.json";
        if (!fs::exists(configPath))
        {
            return loadGlobalModules(repoRoot, ctx);
        }

        try
        {
            json data = io::loadJsonFile(configPath);
            json root = data;
            if (data.contains("Configuration") && data["Configuration"].is_object())
            {
                root = data["Configuration"];
            }

            if (root.contains("SingleFileModules"))
            {
                const std::vector<std::string> singleModules =
                    root["SingleFileModules"].is_array() ? toStringList(root["SingleFileModules"]) : std::vector<std::string>{};
                if (!singleModules.empty())
                {
                    return singleModules;
                }
            }
        }
        catch (...)
        {
            return loadGlobalModules(repoRoot, ctx);
        }

        return loadGlobalModules(repoRoot, ctx);
    }

    std::optional<fs::path> loadDefaultWebShell(const fs::path &repoRoot)
    {
        fs::path configPath = repoRoot / "config.json";
        if (!fs::exists(configPath))
        {
            return std::nullopt;
        }

        try
        {
            json data = io::loadJsonFile(configPath);
            json root = data;
            if (data.contains("Configuration") && data["Configuration"].is_object())
            {
                root = data["Configuration"];
            }

            std::string shellPath;
            if (root.contains("Web") && root["Web"].is_object())
            {
                const auto &web = root["Web"];
                auto readString = [&](const char *key)
                {
                    if (shellPath.empty() && web.contains(key) && web[key].is_string())
                    {
                        shellPath = web[key].get<std::string>();
                    }
                };

                readString("SHELL");
                readString("Shell");
                readString("ShellTemplate");
                readString("Template");
            }

            if (shellPath.empty() && root.contains("WebShell") && root["WebShell"].is_string())
            {
                shellPath = root["WebShell"].get<std::string>();
            }

            if (shellPath.empty())
            {
                return std::nullopt;
            }

            fs::path shell = fs::path(shellPath);
            if (!shell.is_absolute())
            {
                shell = fs::absolute(repoRoot / shell);
            }

            return shell;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

} // namespace crosside::model
