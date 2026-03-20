#pragma once
// SceneDocument — pure data model for a level/scene, with JSON serialization.
// No dependency on raylib, ImGui, or the engine at this level.

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace le // level editor
{

// ── Collision shape description ───────────────────────────────────────────────
enum class CollisionType { None, Circle, Rectangle, Polygon };

struct CollisionDesc
{
    CollisionType type = CollisionType::None;
    float width  = 32.0f;
    float height = 32.0f;
    float radius = 16.0f;
    std::vector<float> polygon_xs;
    std::vector<float> polygon_ys;
};

// ── Game object placed in a layer ─────────────────────────────────────────────
struct SceneObject
{
    uint32_t    uid        = 0;       // unique within the scene
    std::string name;
    int         graph      = -1;      // GraphLib id (-1 = no sprite)
    double      x          = 0.0;
    double      y          = 0.0;
    int         z          = 0;       // z-order within layer
    double      angle      = 0.0;     // degrees
    double      size_x     = 100.0;   // percentage (100 = 1x)
    double      size_y     = 100.0;
    uint8_t     r = 255, g = 255, b = 255, a = 255;
    bool        flip_x     = false;
    bool        flip_y     = false;
    bool        visible    = true;
    CollisionDesc collision;
    uint32_t    collision_layer = 1;
    uint32_t    collision_mask  = 0xFFFFFFFF;

    // User-defined key-value properties (for scripts)
    std::vector<std::pair<std::string, std::string>> properties;
};

// ── Tilemap data for one layer ────────────────────────────────────────────────
struct TilemapData
{
    int tileset_graph = -1;   // Graph ID of the tileset image
    int tile_width    = 32;
    int tile_height   = 32;
    int cols          = 0;
    int rows          = 0;
    std::vector<int> data;    // col-major or row-major tile indices (0 = empty)
};

// ── Layer ─────────────────────────────────────────────────────────────────────
struct SceneLayer
{
    std::string name;
    int         index       = 0;
    bool        visible     = true;
    bool        locked      = false;
    double      scroll_factor_x = 1.0;
    double      scroll_factor_y = 1.0;
    int         back_graph  = -1;     // parallax background graph
    int         front_graph = -1;     // parallax foreground graph

    // Optional tilemap
    bool        has_tilemap = false;
    TilemapData tilemap;

    // Objects on this layer
    std::vector<SceneObject> objects;
};

// ── Solid rectangle (static collision) ────────────────────────────────────────
struct SceneSolid
{
    float       x = 0, y = 0, w = 0, h = 0;
    std::string name;
};

// ── Top-level scene document ──────────────────────────────────────────────────
struct SceneDocument
{
    std::string name      = "untitled";
    std::string file_path;
    int         width     = 1280;
    int         height    = 720;
    double      camera_x  = 0.0;
    double      camera_y  = 0.0;
    double      camera_zoom = 1.0;

    std::vector<SceneLayer> layers;
    std::vector<SceneSolid> solids;

    // Editor state (not serialized to file)
    bool        dirty     = false;
    uint32_t    next_uid  = 1;

    uint32_t AllocUID() { dirty = true; return next_uid++; }

    // ── Convenience ──
    void EnsureDefaultLayers();
    SceneObject* FindObject(uint32_t uid);
    SceneLayer*  FindLayerByIndex(int index);

    // ── Serialization ──
    nlohmann::json ToJson() const;
    static SceneDocument FromJson(const nlohmann::json& j);

    bool SaveToFile(const std::string& path);
    static bool LoadFromFile(const std::string& path, SceneDocument& out);
};

} // namespace le
