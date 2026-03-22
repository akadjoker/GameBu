#include "commands/release_command.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "build/android_builder.hpp"
#include "io/fs_utils.hpp"
#include "model/loader.hpp"

namespace fs = std::filesystem;

namespace crosside::commands
{
    namespace
    {

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

        fs::path runtimeTemplateRoot(const fs::path &repoRoot, const std::string &target)
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

        bool copyFileIfExists(const fs::path &src, const fs::path &dst, const crosside::Context &ctx)
        {
            std::error_code ec;
            if (!fs::exists(src, ec) || !fs::is_regular_file(src, ec))
            {
                return false;
            }

            crosside::io::ensureDir(dst.parent_path());
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
            if (ec)
            {
                ctx.error("Failed copy file ", src.string(), " -> ", dst.string(), " : ", ec.message());
                return false;
            }
            copyFilePermissions(src, dst);
            return true;
        }

        std::size_t copyDirectoryTree(const fs::path &src, const fs::path &dst, const crosside::Context &ctx)
        {
            std::error_code ec;
            if (!fs::exists(src, ec) || !fs::is_directory(src, ec))
            {
                return 0;
            }

            std::size_t copied = 0;
            for (const auto &entry : fs::recursive_directory_iterator(src, ec))
            {
                if (ec)
                {
                    break;
                }

                const fs::path rel = fs::relative(entry.path(), src, ec);
                if (ec)
                {
                    ctx.error("Failed compute relative path while copying ", src.string());
                    return copied;
                }

                const fs::path outPath = dst / rel;
                if (entry.is_directory(ec))
                {
                    crosside::io::ensureDir(outPath);
                    continue;
                }
                if (!entry.is_regular_file(ec))
                {
                    continue;
                }

                crosside::io::ensureDir(outPath.parent_path());
                fs::copy_file(entry.path(), outPath, fs::copy_options::overwrite_existing, ec);
                if (ec)
                {
                    ctx.error("Failed copy file ", entry.path().string(), " -> ", outPath.string(), " : ", ec.message());
                    return copied;
                }
                ++copied;
            }

            return copied;
        }

        bool copyDirectoryContentsExcept(
            const fs::path &src,
            const fs::path &dst,
            const std::vector<std::string> &skipNames,
            const crosside::Context &ctx)
        {
            std::error_code ec;
            if (!fs::exists(src, ec) || !fs::is_directory(src, ec))
            {
                ctx.error("Template directory not found: ", src.string());
                return false;
            }

            crosside::io::ensureDir(dst);
            for (const auto &entry : fs::directory_iterator(src, ec))
            {
                if (ec)
                {
                    break;
                }

                const std::string name = entry.path().filename().string();
                if (std::find(skipNames.begin(), skipNames.end(), name) != skipNames.end())
                {
                    continue;
                }

                const fs::path outPath = dst / entry.path().filename();
                if (entry.is_directory(ec))
                {
                    if (copyDirectoryTree(entry.path(), outPath, ctx) == 0 && !fs::is_empty(entry.path(), ec))
                    {
                        return false;
                    }
                    continue;
                }
                if (entry.is_regular_file(ec))
                {
                    if (!copyFileIfExists(entry.path(), outPath, ctx))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        bool copyDirectoryContents(const fs::path &src, const fs::path &dst, const crosside::Context &ctx)
        {
            std::error_code ec;
            if (!fs::exists(src, ec) || !fs::is_directory(src, ec))
            {
                ctx.error("Template directory not found: ", src.string());
                return false;
            }

            crosside::io::ensureDir(dst);
            for (const auto &entry : fs::directory_iterator(src, ec))
            {
                if (ec)
                {
                    break;
                }

                const fs::path outPath = dst / entry.path().filename();
                if (entry.is_directory(ec))
                {
                    if (copyDirectoryTree(entry.path(), outPath, ctx) == 0 && !fs::is_empty(entry.path(), ec))
                    {
                        return false;
                    }
                    continue;
                }
                if (entry.is_regular_file(ec))
                {
                    if (!copyFileIfExists(entry.path(), outPath, ctx))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        std::string toWebPath(const fs::path &path)
        {
            return path.generic_string();
        }

        std::string escapeJsString(const std::string &value)
        {
            std::string out;
            out.reserve(value.size() + 8);
            for (char ch : value)
            {
                switch (ch)
                {
                case '\\':
                    out += "\\\\";
                    break;
                case '"':
                    out += "\\\"";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    out.push_back(ch);
                    break;
                }
            }
            return out;
        }

        std::vector<std::pair<std::string, std::string>> collectWebPreloadFiles(const fs::path &outRoot)
        {
            std::vector<std::pair<std::string, std::string>> files;
            const std::vector<std::string> roots = {"scripts", "assets", "resources", "data", "media"};

            std::error_code ec;
            for (const auto &folder : roots)
            {
                const fs::path root = outRoot / folder;
                if (!fs::exists(root, ec) || !fs::is_directory(root, ec))
                {
                    continue;
                }

                for (const auto &entry : fs::recursive_directory_iterator(root, ec))
                {
                    if (ec || !entry.is_regular_file(ec))
                    {
                        continue;
                    }

                    const fs::path rel = fs::relative(entry.path(), outRoot, ec);
                    if (ec)
                    {
                        continue;
                    }

                    files.emplace_back(toWebPath(rel), "/" + toWebPath(rel));
                }
            }

            std::sort(files.begin(), files.end(), [](const auto &a, const auto &b)
                      { return a.first < b.first; });
            return files;
        }

        bool writeTextFile(const fs::path &path, const std::string &content, const crosside::Context &ctx)
        {
            crosside::io::ensureDir(path.parent_path());
            std::ofstream out(path);
            if (!out)
            {
                ctx.error("Failed write file: ", path.string());
                return false;
            }
            out << content;
            return static_cast<bool>(out);
        }

        bool writeWebPreloadScript(const fs::path &outRoot, const crosside::Context &ctx)
        {
            const auto files = collectWebPreloadFiles(outRoot);

            std::ostringstream js;
            js << "(function () {\n";
            js << "  if (typeof Module === \"undefined\") { window.Module = {}; }\n";
            js << "  Module.preRun = Module.preRun || [];\n";
            js << "  Module.preRun.push(function () {\n";
            js << "    function ensureDir(path) {\n";
            js << "      if (!path || path === \"/\") return;\n";
            js << "      try {\n";
            js << "        if (!FS.analyzePath(path).exists) FS.mkdirTree(path);\n";
            js << "      } catch (e) {}\n";
            js << "    }\n";

            for (const auto &[hostPath, mountPath] : files)
            {
                const fs::path mountFs(mountPath);
                const std::string parent = escapeJsString(toWebPath(mountFs.parent_path()));
                const std::string name = escapeJsString(mountFs.filename().string());
                const std::string host = escapeJsString(hostPath);
                js << "    ensureDir(\"" << parent << "\");\n";
                js << "    FS.createPreloadedFile(\"" << parent << "\", \"" << name << "\", \"" << host << "\", true, false);\n";
            }

            js << "  });\n";
            js << "})();\n";

            return writeTextFile(outRoot / "preload.js", js.str(), ctx);
        }

        bool injectWebPreloadScriptTag(const fs::path &htmlPath, const crosside::Context &ctx)
        {
            std::ifstream in(htmlPath);
            if (!in)
            {
                ctx.error("Failed open HTML for preload injection: ", htmlPath.string());
                return false;
            }

            std::ostringstream buffer;
            buffer << in.rdbuf();
            std::string html = buffer.str();

            const std::string needle = "<script async type=\"text/javascript\" src=\"bugame.js\"></script>";
            const std::string insert = "<script src=\"preload.js\"></script>\n    " + needle;

            const std::size_t pos = html.find(needle);
            if (pos == std::string::npos)
            {
                ctx.warn("Could not inject preload.js into ", htmlPath.string(), " (bugame.js tag not found)");
                return true;
            }

            html.replace(pos, needle.size(), insert);
            return writeTextFile(htmlPath, html, ctx);
        }

        bool packageWebRelease(
            const crosside::Context &ctx,
            const fs::path &repoRoot,
            const crosside::model::ReleaseSpec &release)
        {
            const fs::path templateRoot = runtimeTemplateRoot(repoRoot, "web");
            const fs::path outRoot = repoRoot / "dist" / release.name / "web";

            std::error_code ec;
            fs::remove_all(outRoot, ec);
            crosside::io::ensureDir(outRoot);

            if (!copyDirectoryContents(templateRoot, outRoot, ctx))
            {
                return false;
            }

            const fs::path scriptsDir = outRoot / "scripts";
            crosside::io::ensureDir(scriptsDir);
            if (!release.scriptsRoot.empty())
            {
                copyDirectoryTree(release.scriptsRoot, scriptsDir, ctx);
            }
            if (!copyFileIfExists(release.entryBytecode, scriptsDir / "main.buc", ctx))
            {
                return false;
            }

            if (!release.assetsRoot.empty())
            {
                copyDirectoryTree(release.assetsRoot, outRoot / "assets", ctx);
            }

            if (!release.iconFile.empty())
            {
                copyFileIfExists(release.iconFile, outRoot / "icon.png", ctx);
            }

            if (!writeWebPreloadScript(outRoot, ctx))
            {
                return false;
            }

            const std::vector<fs::path> htmlFiles = {
                outRoot / "index.html",
                outRoot / "bugame.html",
            };
            for (const auto &htmlPath : htmlFiles)
            {
                std::error_code ec;
                if (fs::exists(htmlPath, ec) && fs::is_regular_file(htmlPath, ec))
                {
                    if (!injectWebPreloadScriptTag(htmlPath, ctx))
                    {
                        return false;
                    }
                }
            }

            ctx.log("Web release packaged at ", outRoot.string());
            return true;
        }

        bool packageDesktopRelease(
            const crosside::Context &ctx,
            const fs::path &repoRoot,
            const crosside::model::ReleaseSpec &release)
        {
            const fs::path templateRoot = runtimeTemplateRoot(repoRoot, "desktop");
            const fs::path outRoot = repoRoot / "dist" / release.name / "desktop";

            std::error_code ec;
            fs::remove_all(outRoot, ec);
            crosside::io::ensureDir(outRoot);

            if (!copyDirectoryContents(templateRoot, outRoot, ctx))
            {
                return false;
            }

            const fs::path scriptsDir = outRoot / "scripts";
            crosside::io::ensureDir(scriptsDir);
            if (!release.scriptsRoot.empty())
            {
                copyDirectoryTree(release.scriptsRoot, scriptsDir, ctx);
            }
            if (!copyFileIfExists(release.entryBytecode, scriptsDir / "main.buc", ctx))
            {
                return false;
            }

            if (!release.assetsRoot.empty())
            {
                copyDirectoryTree(release.assetsRoot, outRoot / "assets", ctx);
            }

            if (!release.iconFile.empty())
            {
                copyFileIfExists(release.iconFile, outRoot / "icon.png", ctx);
            }

            ctx.log("Desktop release packaged at ", outRoot.string());
            return true;
        }

        bool packageAndroidRelease(
            const crosside::Context &ctx,
            const fs::path &repoRoot,
            const crosside::model::ReleaseSpec &release)
        {
            const fs::path templateBaseRoot = repoRoot / "Templates" / "Android";
            const fs::path templateRuntimeRoot = runtimeTemplateRoot(repoRoot, "android");
            const fs::path outRoot = repoRoot / "dist" / release.name / "android";

            std::error_code ec;
            fs::remove_all(outRoot, ec);
            crosside::io::ensureDir(outRoot);

            if (!copyDirectoryContentsExcept(templateBaseRoot, outRoot, {"runtime"}, ctx))
            {
                return false;
            }

            if (!copyDirectoryContents(templateRuntimeRoot, outRoot, ctx))
            {
                return false;
            }

            crosside::model::ProjectSpec project;
            project.name = "bugame";
            project.exportName = release.name;
            project.root = outRoot;
            project.filePath = release.root;
            project.androidContentRoot = release.root;
            project.androidLabel = release.name;
            project.androidPackage = "com.djokersoft." + lower(release.name);
            if (!release.iconFile.empty())
            {
                project.androidIcon = release.iconFile;
            }

            crosside::model::ModuleMap modules;
            const std::vector<std::string> activeModules;
            const std::vector<int> abis = {0, 1};
            if (!crosside::build::buildProjectAndroid(ctx, repoRoot, project, modules, activeModules, false, false, false, abis))
            {
                return false;
            }

            ctx.log("Android release packaged at ", outRoot.string());
            return true;
        }

    } // namespace

    int runReleaseCommand(const crosside::Context &ctx, const fs::path &repoRoot, const std::vector<std::string> &args)
    {
        if (args.size() < 2)
        {
            ctx.error("release: usage release <folder> <target>");
            return 1;
        }

        const auto release = crosside::model::loadReleaseFolder(repoRoot, args[0], ctx);
        if (!release.has_value())
        {
            return 1;
        }

        const std::string target = normalizeTarget(args[1]);
        if (target.empty())
        {
            ctx.error("Unknown release target: ", args[1]);
            return 1;
        }

        if (target == "web")
        {
            return packageWebRelease(ctx, repoRoot, release.value()) ? 0 : 1;
        }
        if (target == "desktop")
        {
            return packageDesktopRelease(ctx, repoRoot, release.value()) ? 0 : 1;
        }
        if (target == "android")
        {
            return packageAndroidRelease(ctx, repoRoot, release.value()) ? 0 : 1;
        }

        ctx.error("Unsupported release target: ", target);
        return 1;
    }

} // namespace crosside::commands
