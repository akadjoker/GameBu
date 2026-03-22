#include "commands/list_command.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

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

    } // namespace

    int runListCommand(const crosside::Context &ctx, const fs::path &repoRoot, const std::vector<std::string> &args)
    {
        std::string what = args.empty() ? "all" : lower(args.front());
        if (what != "all" && what != "projects" && what != "apps")
        {
            ctx.error("Invalid list target: ", what, " (use all|projects)");
            return 1;
        }

        if (what == "all" || what == "apps" || what == "projects")
        {
            ctx.log("Projects:");
            auto files = crosside::io::listProjectFiles(repoRoot / "projects");
            bool hasAnyProject = false;

            for (const auto &file : files)
            {
                hasAnyProject = true;
                auto project = crosside::model::loadProjectFile(file, ctx);
                if (!project.has_value())
                {
                    ctx.log("  ", file.string(), "  [invalid]");
                    continue;
                }
                std::string rootLabel = project->root.filename().string();
                if (rootLabel.empty())
                {
                    rootLabel = file.parent_path().filename().string();
                }
                ctx.log("  ", rootLabel, " (name=", project->name, ")  ", file.string());
            }

            if (auto project = crosside::model::loadKnownWorkspaceProject(repoRoot, "", ctx); project.has_value())
            {
                hasAnyProject = true;
                std::string rootLabel = project->root.filename().string();
                if (rootLabel.empty())
                {
                    rootLabel = "workspace";
                }
                ctx.log("  ", rootLabel, " (name=", project->name, ")  ", project->filePath.string(), "  [known-layout]");
            }

            if (!hasAnyProject)
            {
                ctx.log("  <none>");
            }
        }

        return 0;
    }

} // namespace crosside::commands
