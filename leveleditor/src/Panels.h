#pragma once
// Panels — ImGui panels for the level editor: Hierarchy, Inspector, Asset Browser, Toolbar.
// Kept in a single header for simplicity (all inline/template-like).

#include "SceneDocument.h"
#include "Gizmo2D.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <engine.hpp>
#include <string>
#include <functional>
#include <algorithm>
#include <cmath>

namespace le
{

// ── Hierarchy Panel ─────────────────────────────────────────────────────────
// Shows layers as tree nodes with their objects as children.
struct HierarchyPanel
{
    float width = 220.0f;

    // Returns true if selection changed
    bool Render(SceneDocument& doc, int& activeLayer, uint32_t& selectedUID, std::string& status)
    {
        bool changed = false;
        ImGui::BeginChild("##hierarchy", ImVec2(width, -1), true);
        ImGui::TextUnformatted("Hierarchy");
        ImGui::Separator();

        for (auto& layer : doc.layers)
        {
            ImGuiTreeNodeFlags layerFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (layer.index == activeLayer)
                layerFlags |= ImGuiTreeNodeFlags_Selected;

            // Layer visibility toggle
            bool vis = layer.visible;
            std::string eye = vis ? "[V]" : "[H]";
            std::string layerLabel = eye + " " + layer.name + " (L" + std::to_string(layer.index) + ")";
            bool open = ImGui::TreeNodeEx(("##layer" + std::to_string(layer.index)).c_str(), layerFlags, "%s", layerLabel.c_str());

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                activeLayer = layer.index;
                changed = true;
            }
            // Right-click context for layer
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                ImGui::OpenPopup(("layer_ctx_" + std::to_string(layer.index)).c_str());

            if (ImGui::BeginPopup(("layer_ctx_" + std::to_string(layer.index)).c_str()))
            {
                if (ImGui::MenuItem(vis ? "Hide Layer" : "Show Layer"))
                {
                    layer.visible = !layer.visible;
                    doc.dirty = true;
                }
                if (ImGui::MenuItem(layer.locked ? "Unlock Layer" : "Lock Layer"))
                {
                    layer.locked = !layer.locked;
                    doc.dirty = true;
                }
                ImGui::InputText("Name", &layer.name);
                ImGui::EndPopup();
            }

            if (open)
            {
                for (int i = 0; i < (int)layer.objects.size(); i++)
                {
                    auto& obj = layer.objects[i];
                    ImGuiTreeNodeFlags objFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (obj.uid == selectedUID)
                        objFlags |= ImGuiTreeNodeFlags_Selected;

                    std::string objLabel = obj.name.empty()
                        ? ("Object #" + std::to_string(obj.uid))
                        : obj.name;
                    objLabel += " [g:" + std::to_string(obj.graph) + " z:" + std::to_string(obj.z) + "]";

                    ImGui::TreeNodeEx(("##obj" + std::to_string(obj.uid)).c_str(), objFlags, "%s", objLabel.c_str());
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    {
                        selectedUID = obj.uid;
                        activeLayer = layer.index;
                        changed = true;
                    }
                    // Right-click to delete
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                        ImGui::OpenPopup(("obj_ctx_" + std::to_string(obj.uid)).c_str());
                    if (ImGui::BeginPopup(("obj_ctx_" + std::to_string(obj.uid)).c_str()))
                    {
                        if (ImGui::MenuItem("Delete"))
                        {
                            if (selectedUID == obj.uid)
                                selectedUID = 0;
                            layer.objects.erase(layer.objects.begin() + i);
                            doc.dirty = true;
                            status = "Object deleted";
                            ImGui::EndPopup();
                            --i;
                            continue;
                        }
                        if (ImGui::MenuItem("Duplicate"))
                        {
                            SceneObject dup = obj;
                            dup.uid  = doc.AllocUID();
                            dup.name = obj.name + "_copy";
                            dup.x   += 20;
                            dup.y   += 20;
                            layer.objects.push_back(dup);
                            selectedUID = dup.uid;
                            doc.dirty = true;
                            status = "Object duplicated";
                        }
                        ImGui::EndPopup();
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndChild();
        return changed;
    }
};

// ── Inspector Panel ─────────────────────────────────────────────────────────
// Shows and edits properties of the selected object.
struct InspectorPanel
{
    float width = 280.0f;

    void Render(SceneDocument& doc, uint32_t selectedUID)
    {
        ImGui::BeginChild("##inspector", ImVec2(width, -1), true);
        ImGui::TextUnformatted("Inspector");
        ImGui::Separator();

        SceneObject* obj = doc.FindObject(selectedUID);
        if (!obj)
        {
            ImGui::TextDisabled("No object selected");
            ImGui::EndChild();
            return;
        }

        bool changed = false;

        // Name
        changed |= ImGui::InputText("Name", &obj->name);

        // Transform
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            double px = obj->x, py = obj->y;
            if (ImGui::InputDouble("X", &px, 1.0, 10.0, "%.1f")) { obj->x = px; changed = true; }
            if (ImGui::InputDouble("Y", &py, 1.0, 10.0, "%.1f")) { obj->y = py; changed = true; }

            int z = obj->z;
            if (ImGui::InputInt("Z", &z)) { obj->z = z; changed = true; }

            double angle = obj->angle;
            if (ImGui::InputDouble("Angle", &angle, 1.0, 15.0, "%.1f")) { obj->angle = angle; changed = true; }

            double sx = obj->size_x, sy = obj->size_y;
            if (ImGui::InputDouble("Size X%", &sx, 1.0, 10.0, "%.1f")) { obj->size_x = sx; changed = true; }
            if (ImGui::InputDouble("Size Y%", &sy, 1.0, 10.0, "%.1f")) { obj->size_y = sy; changed = true; }

            changed |= ImGui::Checkbox("Flip X", &obj->flip_x);
            ImGui::SameLine();
            changed |= ImGui::Checkbox("Flip Y", &obj->flip_y);
        }

        // Appearance
        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int graph = obj->graph;
            if (ImGui::InputInt("Graph ID", &graph)) { obj->graph = graph; changed = true; }

            float col[4] = { obj->r / 255.0f, obj->g / 255.0f, obj->b / 255.0f, obj->a / 255.0f };
            if (ImGui::ColorEdit4("Color", col))
            {
                obj->r = (uint8_t)(col[0] * 255);
                obj->g = (uint8_t)(col[1] * 255);
                obj->b = (uint8_t)(col[2] * 255);
                obj->a = (uint8_t)(col[3] * 255);
                changed = true;
            }

            changed |= ImGui::Checkbox("Visible", &obj->visible);
        }

        // Collision
        if (ImGui::CollapsingHeader("Collision"))
        {
            const char* types[] = { "None", "Circle", "Rectangle", "Polygon" };
            int ct = (int)obj->collision.type;
            if (ImGui::Combo("Shape", &ct, types, 4))
            {
                obj->collision.type = (CollisionType)ct;
                changed = true;
            }

            if (obj->collision.type == CollisionType::Circle)
            {
                changed |= ImGui::InputFloat("Radius", &obj->collision.radius, 1.0f, 5.0f);
            }
            else if (obj->collision.type == CollisionType::Rectangle)
            {
                changed |= ImGui::InputFloat("Width", &obj->collision.width, 1.0f, 5.0f);
                changed |= ImGui::InputFloat("Height", &obj->collision.height, 1.0f, 5.0f);
            }

            int clayer = (int)obj->collision_layer;
            int cmask  = (int)obj->collision_mask;
            if (ImGui::InputInt("Coll. Layer", &clayer)) { obj->collision_layer = (uint32_t)clayer; changed = true; }
            if (ImGui::InputInt("Coll. Mask", &cmask))   { obj->collision_mask  = (uint32_t)cmask; changed = true; }
        }

        // Custom properties
        if (ImGui::CollapsingHeader("Properties"))
        {
            for (int i = 0; i < (int)obj->properties.size(); i++)
            {
                ImGui::PushID(i);
                ImGui::SetNextItemWidth(100);
                changed |= ImGui::InputText("##key", &obj->properties[i].first);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-40);
                changed |= ImGui::InputText("##val", &obj->properties[i].second);
                ImGui::SameLine();
                if (ImGui::SmallButton("X"))
                {
                    obj->properties.erase(obj->properties.begin() + i);
                    --i;
                    changed = true;
                }
                ImGui::PopID();
            }
            if (ImGui::SmallButton("+ Add Property"))
            {
                obj->properties.push_back({ "key", "value" });
                changed = true;
            }
        }

        if (changed)
            doc.dirty = true;

        ImGui::EndChild();
    }
};

// ── Toolbar ─────────────────────────────────────────────────────────────────
enum class EditorTool { Select, Move, Create, Delete };

struct Toolbar
{
    EditorTool activeTool = EditorTool::Select;
    GizmoMode  gizmoMode  = GizmoMode::Translate;

    void Render()
    {
        auto toolButton = [&](const char* label, EditorTool tool)
        {
            bool selected = (activeTool == tool);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            if (ImGui::Button(label, ImVec2(80, 0)))
                activeTool = tool;
            if (selected)
                ImGui::PopStyleColor();
            ImGui::SameLine();
        };

        toolButton("Select", EditorTool::Select);
        toolButton("Move",   EditorTool::Move);
        toolButton("Create", EditorTool::Create);
        toolButton("Delete", EditorTool::Delete);

        // Separator between tools and gizmo mode
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        auto gizmoButton = [&](const char* label, GizmoMode gm)
        {
            bool selected = (gizmoMode == gm);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
            if (ImGui::Button(label, ImVec2(70, 0)))
                gizmoMode = gm;
            if (selected)
                ImGui::PopStyleColor();
            ImGui::SameLine();
        };

        gizmoButton("Transl", GizmoMode::Translate);
        gizmoButton("Rotate", GizmoMode::Rotate);
        gizmoButton("Scale",  GizmoMode::Scale);

        ImGui::NewLine();
    }
};

// ── Scene Settings Popup ────────────────────────────────────────────────────
struct SceneSettingsPopup
{
    bool visible = false;

    void Render(SceneDocument& doc)
    {
        if (visible)
        {
            ImGui::OpenPopup("Scene Settings");
            visible = false;
        }
        if (ImGui::BeginPopupModal("Scene Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText("Scene Name", &doc.name);
            ImGui::InputInt("Width", &doc.width);
            ImGui::InputInt("Height", &doc.height);

            ImGui::Separator();
            ImGui::Text("Solids (%d)", (int)doc.solids.size());
            for (int i = 0; i < (int)doc.solids.size(); i++)
            {
                ImGui::PushID(i);
                auto& s = doc.solids[i];
                ImGui::InputText("Name", &s.name);
                ImGui::InputFloat("X", &s.x); ImGui::SameLine();
                ImGui::InputFloat("Y", &s.y);
                ImGui::InputFloat("W", &s.w); ImGui::SameLine();
                ImGui::InputFloat("H", &s.h);
                if (ImGui::SmallButton("Delete Solid"))
                {
                    doc.solids.erase(doc.solids.begin() + i);
                    --i;
                    doc.dirty = true;
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            if (ImGui::Button("+ Add Solid"))
            {
                doc.solids.push_back({ 0, 0, 100, 20, "solid" });
                doc.dirty = true;
            }

            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }
};

// ── Bottom Panel Tabs ───────────────────────────────────────────────────────
enum class BottomTab { Assets, Tiles, Particles };

// ── Asset Browser Panel ─────────────────────────────────────────────────────
// Shows thumbnails of all loaded graphs in a scrollable grid.
// Click to select for the Create tool. Right-click for details.
struct AssetBrowserPanel
{
    float thumbnailSize = 64.0f;
    float padding       = 8.0f;
    std::string filter;
    int  hoveredGraph    = -1;
    bool showDetails     = false;
    int  detailsGraphId  = -1;

    // Returns the graph ID that was just selected (or -1 if none)
    int Render(GraphLib& graphLib, int selectedGraphId, EditorTool& tool, std::string& status)
    {
        int result = -1;

        // Filter bar
        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("##asset_filter", "Filter graphs...", &filter);
        ImGui::SameLine();
        ImGui::Text("Graphs: %d | Textures: %d", graphLib.getGraphCount(), graphLib.getTextureCount());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::SliderFloat("##thumb_size", &thumbnailSize, 32.0f, 128.0f, "%.0f px");
        ImGui::SameLine();
        ImGui::Text("Size");

        ImGui::Separator();

        // Thumbnail grid
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int cols = std::max(1, (int)((panelWidth + padding) / (thumbnailSize + padding)));
        int col = 0;

        ImGui::BeginChild("##asset_grid", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        std::string filterLower = filter;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

        for (int i = 0; i < (int)graphLib.graphs.size(); i++)
        {
            const Graph& g = graphLib.graphs[i];

            // Apply filter
            if (!filterLower.empty())
            {
                std::string nameLower(g.name);
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                if (nameLower.find(filterLower) == std::string::npos)
                    continue;
            }

            ImGui::PushID(i);

            // Thumbnail button
            bool isSelected = (i == selectedGraphId);
            if (isSelected)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 0.9f));
            }

            bool hasTexture = (g.texture >= 0 && g.texture < (int)graphLib.textures.size());
            bool clicked = false;

            if (hasTexture)
            {
                const Texture2D& tex = graphLib.textures[g.texture];
                // UV coords for the clip region
                float u0 = g.clip.x / tex.width;
                float v0 = g.clip.y / tex.height;
                float u1 = (g.clip.x + g.clip.width)  / tex.width;
                float v1 = (g.clip.y + g.clip.height) / tex.height;

                clicked = ImGui::ImageButton(
                    ("##graph_" + std::to_string(i)).c_str(),
                    (ImTextureID)(intptr_t)tex.id,
                    ImVec2(thumbnailSize, thumbnailSize),
                    ImVec2(u0, v0), ImVec2(u1, v1));
            }
            else
            {
                // No texture — placeholder button
                clicked = ImGui::Button("?", ImVec2(thumbnailSize, thumbnailSize));
            }

            if (isSelected)
                ImGui::PopStyleColor(2);

            // Tooltip on hover
            if (ImGui::IsItemHovered())
            {
                hoveredGraph = i;
                ImGui::BeginTooltip();
                ImGui::Text("ID: %d", i);
                ImGui::Text("Name: %s", g.name);
                ImGui::Text("Size: %d x %d", (int)g.clip.width, (int)g.clip.height);
                ImGui::Text("Points: %d", (int)g.points.size());
                // Show larger preview
                if (hasTexture)
                {
                    const Texture2D& tex = graphLib.textures[g.texture];
                    float u0 = g.clip.x / tex.width;
                    float v0 = g.clip.y / tex.height;
                    float u1 = (g.clip.x + g.clip.width)  / tex.width;
                    float v1 = (g.clip.y + g.clip.height) / tex.height;
                    float previewSize = 128.0f;
                    float aspect = g.clip.width / std::max(g.clip.height, 1.0f);
                    ImVec2 sz = (aspect >= 1.0f)
                        ? ImVec2(previewSize, previewSize / aspect)
                        : ImVec2(previewSize * aspect, previewSize);
                    ImGui::Image((ImTextureID)(intptr_t)tex.id, sz, ImVec2(u0, v0), ImVec2(u1, v1));
                }
                ImGui::EndTooltip();
            }

            // Left click: select for create tool
            if (clicked)
            {
                result = i;
                tool = EditorTool::Create;
                status = "Selected graph " + std::to_string(i) + " (" + std::string(g.name) + ")";
            }

            // Right click: show details popup
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                detailsGraphId = i;
                showDetails = true;
            }

            // Label under thumbnail
            std::string label = std::string(g.name);
            if (label.length() > 10) label = label.substr(0, 9) + "..";
            float textW = ImGui::CalcTextSize(label.c_str()).x;
            float thumbCenter = thumbnailSize / 2.0f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + thumbCenter - textW / 2.0f);
            ImGui::TextDisabled("%s", label.c_str());

            ImGui::PopID();

            col++;
            if (col < cols)
                ImGui::SameLine(0, padding);
            else
                col = 0;
        }

        // Empty state
        if (graphLib.graphs.empty())
        {
            ImGui::TextDisabled("No graphs loaded.\nDrag & drop PNG/JPG images here.");
        }

        ImGui::EndChild();

        // Graph details popup
        if (showDetails && detailsGraphId >= 0)
        {
            ImGui::OpenPopup("Graph Details");
            showDetails = false;
        }
        if (ImGui::BeginPopup("Graph Details"))
        {
            if (detailsGraphId >= 0 && detailsGraphId < (int)graphLib.graphs.size())
            {
                Graph& g = graphLib.graphs[detailsGraphId];
                ImGui::Text("Graph #%d", detailsGraphId);
                ImGui::Separator();
                ImGui::Text("Name: %s", g.name);
                ImGui::Text("Texture: %d", g.texture);
                ImGui::Text("Clip: %.0f, %.0f, %.0f, %.0f", g.clip.x, g.clip.y, g.clip.width, g.clip.height);
                ImGui::Text("Points: %d", (int)g.points.size());
                for (int p = 0; p < (int)g.points.size(); p++)
                    ImGui::Text("  [%d] (%.1f, %.1f)", p, g.points[p].x, g.points[p].y);

                bool hasTexture = (g.texture >= 0 && g.texture < (int)graphLib.textures.size());
                if (hasTexture)
                {
                    const Texture2D& tex = graphLib.textures[g.texture];
                    float u0 = g.clip.x / tex.width;
                    float v0 = g.clip.y / tex.height;
                    float u1 = (g.clip.x + g.clip.width)  / tex.width;
                    float v1 = (g.clip.y + g.clip.height) / tex.height;
                    ImGui::Image((ImTextureID)(intptr_t)tex.id, ImVec2(200, 200), ImVec2(u0, v0), ImVec2(u1, v1));
                }

                ImGui::Separator();
                if (ImGui::Button("Use for Create Tool"))
                {
                    result = detailsGraphId;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Close##details"))
                    ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        return result;
    }
};

// ── Bottom Panel ────────────────────────────────────────────────────────────
// Tab bar at the bottom with Assets, Tiles, Particles tabs.
struct BottomPanel
{
    float height = 180.0f;
    BottomTab activeTab = BottomTab::Assets;
    AssetBrowserPanel assetBrowser;
    bool visible = true;

    void Render(GraphLib& graphLib, int selectedGraphId, int& createGraphId,
                EditorTool& tool, std::string& status)
    {
        if (!visible) return;

        ImGui::BeginChild("##bottom_panel", ImVec2(-1, height), true);

        if (ImGui::BeginTabBar("##bottom_tabs"))
        {
            if (ImGui::BeginTabItem("Assets"))
            {
                activeTab = BottomTab::Assets;
                int picked = assetBrowser.Render(graphLib, createGraphId, tool, status);
                if (picked >= 0)
                    createGraphId = picked;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tiles"))
            {
                activeTab = BottomTab::Tiles;
                ImGui::TextDisabled("Tile editor coming soon.");
                ImGui::TextDisabled("Select a layer with a tilemap, then paint tiles here.");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Particles"))
            {
                activeTab = BottomTab::Particles;
                ImGui::TextDisabled("Particle preview coming soon.");
                ImGui::TextDisabled("Create and test particle emitters visually.");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::EndChild();
    }
};

} // namespace le
