#include "SceneDocument.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace le
{

using json = nlohmann::json;

// ── Helpers ──────────────────────────────────────────────────────────────────

void SceneDocument::EnsureDefaultLayers()
{
    if (!layers.empty()) return;
    const char* names[] = { "Background", "Main", "Foreground", "UI", "Layer4", "Layer5" };
    for (int i = 0; i < 6; i++)
    {
        SceneLayer layer;
        layer.name  = names[i];
        layer.index = i;
        layers.push_back(std::move(layer));
    }
}

SceneObject* SceneDocument::FindObject(uint32_t uid)
{
    for (auto& layer : layers)
        for (auto& obj : layer.objects)
            if (obj.uid == uid) return &obj;
    return nullptr;
}

SceneLayer* SceneDocument::FindLayerByIndex(int index)
{
    for (auto& layer : layers)
        if (layer.index == index) return &layer;
    return nullptr;
}

// ── Collision JSON ──────────────────────────────────────────────────────────

static json CollisionToJson(const CollisionDesc& c)
{
    json j;
    switch (c.type)
    {
    case CollisionType::Circle:
        j["type"]   = "circle";
        j["radius"] = c.radius;
        break;
    case CollisionType::Rectangle:
        j["type"]   = "rectangle";
        j["width"]  = c.width;
        j["height"] = c.height;
        break;
    case CollisionType::Polygon:
        j["type"] = "polygon";
        j["xs"]   = c.polygon_xs;
        j["ys"]   = c.polygon_ys;
        break;
    default:
        j["type"] = "none";
        break;
    }
    return j;
}

static CollisionDesc CollisionFromJson(const json& j)
{
    CollisionDesc c;
    std::string type = j.value("type", "none");
    if (type == "circle")
    {
        c.type   = CollisionType::Circle;
        c.radius = j.value("radius", 16.0f);
    }
    else if (type == "rectangle")
    {
        c.type   = CollisionType::Rectangle;
        c.width  = j.value("width", 32.0f);
        c.height = j.value("height", 32.0f);
    }
    else if (type == "polygon")
    {
        c.type       = CollisionType::Polygon;
        c.polygon_xs = j.value("xs", std::vector<float>{});
        c.polygon_ys = j.value("ys", std::vector<float>{});
    }
    return c;
}

// ── SceneObject JSON ────────────────────────────────────────────────────────

static json ObjectToJson(const SceneObject& o)
{
    json j;
    j["uid"]       = o.uid;
    j["name"]      = o.name;
    j["graph"]     = o.graph;
    j["x"]         = o.x;
    j["y"]         = o.y;
    j["z"]         = o.z;
    j["angle"]     = o.angle;
    j["size_x"]    = o.size_x;
    j["size_y"]    = o.size_y;
    j["color"]     = { o.r, o.g, o.b, o.a };
    j["flip_x"]    = o.flip_x;
    j["flip_y"]    = o.flip_y;
    j["visible"]   = o.visible;
    j["collision"]       = CollisionToJson(o.collision);
    j["collision_layer"] = o.collision_layer;
    j["collision_mask"]  = o.collision_mask;

    if (!o.properties.empty())
    {
        json props = json::object();
        for (auto& [k, v] : o.properties)
            props[k] = v;
        j["properties"] = props;
    }
    return j;
}

static SceneObject ObjectFromJson(const json& j)
{
    SceneObject o;
    o.uid    = j.value("uid", 0u);
    o.name   = j.value("name", std::string());
    o.graph  = j.value("graph", -1);
    o.x      = j.value("x", 0.0);
    o.y      = j.value("y", 0.0);
    o.z      = j.value("z", 0);
    o.angle  = j.value("angle", 0.0);
    o.size_x = j.value("size_x", 100.0);
    o.size_y = j.value("size_y", 100.0);
    if (j.contains("color") && j["color"].is_array() && j["color"].size() >= 4)
    {
        o.r = j["color"][0]; o.g = j["color"][1];
        o.b = j["color"][2]; o.a = j["color"][3];
    }
    o.flip_x  = j.value("flip_x", false);
    o.flip_y  = j.value("flip_y", false);
    o.visible = j.value("visible", true);
    if (j.contains("collision"))
        o.collision = CollisionFromJson(j["collision"]);
    o.collision_layer = j.value("collision_layer", 1u);
    o.collision_mask  = j.value("collision_mask", 0xFFFFFFFFu);
    if (j.contains("properties") && j["properties"].is_object())
    {
        for (auto& [k, v] : j["properties"].items())
            o.properties.push_back({ k, v.get<std::string>() });
    }
    return o;
}

// ── Tilemap JSON ────────────────────────────────────────────────────────────

static json TilemapToJson(const TilemapData& t)
{
    json j;
    j["tileset_graph"] = t.tileset_graph;
    j["tile_width"]    = t.tile_width;
    j["tile_height"]   = t.tile_height;
    j["cols"]          = t.cols;
    j["rows"]          = t.rows;
    j["data"]          = t.data;
    return j;
}

static TilemapData TilemapFromJson(const json& j)
{
    TilemapData t;
    t.tileset_graph = j.value("tileset_graph", -1);
    t.tile_width    = j.value("tile_width", 32);
    t.tile_height   = j.value("tile_height", 32);
    t.cols          = j.value("cols", 0);
    t.rows          = j.value("rows", 0);
    t.data          = j.value("data", std::vector<int>{});
    return t;
}

// ── Layer JSON ──────────────────────────────────────────────────────────────

static json LayerToJson(const SceneLayer& l)
{
    json j;
    j["name"]            = l.name;
    j["index"]           = l.index;
    j["visible"]         = l.visible;
    j["locked"]          = l.locked;
    j["scroll_factor_x"] = l.scroll_factor_x;
    j["scroll_factor_y"] = l.scroll_factor_y;
    j["back_graph"]      = l.back_graph;
    j["front_graph"]     = l.front_graph;
    if (l.has_tilemap)
        j["tilemap"] = TilemapToJson(l.tilemap);
    json objs = json::array();
    for (auto& o : l.objects)
        objs.push_back(ObjectToJson(o));
    j["objects"] = objs;
    return j;
}

static SceneLayer LayerFromJson(const json& j)
{
    SceneLayer l;
    l.name              = j.value("name", std::string("Layer"));
    l.index             = j.value("index", 0);
    l.visible           = j.value("visible", true);
    l.locked            = j.value("locked", false);
    l.scroll_factor_x   = j.value("scroll_factor_x", 1.0);
    l.scroll_factor_y   = j.value("scroll_factor_y", 1.0);
    l.back_graph        = j.value("back_graph", -1);
    l.front_graph       = j.value("front_graph", -1);
    if (j.contains("tilemap") && j["tilemap"].is_object())
    {
        l.has_tilemap = true;
        l.tilemap     = TilemapFromJson(j["tilemap"]);
    }
    if (j.contains("objects") && j["objects"].is_array())
    {
        for (auto& oj : j["objects"])
            l.objects.push_back(ObjectFromJson(oj));
    }
    return l;
}

// ── SceneDocument JSON ──────────────────────────────────────────────────────

json SceneDocument::ToJson() const
{
    json j;
    j["name"]   = name;
    j["width"]  = width;
    j["height"] = height;
    j["camera"] = { {"x", camera_x}, {"y", camera_y}, {"zoom", camera_zoom} };
    j["next_uid"] = next_uid;

    json jlayers = json::array();
    for (auto& l : layers)
        jlayers.push_back(LayerToJson(l));
    j["layers"] = jlayers;

    json jsolids = json::array();
    for (auto& s : solids)
        jsolids.push_back(json{ {"x", s.x}, {"y", s.y}, {"w", s.w}, {"h", s.h}, {"name", s.name} });
    j["solids"] = jsolids;

    return j;
}

SceneDocument SceneDocument::FromJson(const json& j)
{
    SceneDocument doc;
    doc.name        = j.value("name", std::string("untitled"));
    doc.width       = j.value("width", 1280);
    doc.height      = j.value("height", 720);
    doc.next_uid    = j.value("next_uid", 1u);

    if (j.contains("camera") && j["camera"].is_object())
    {
        doc.camera_x    = j["camera"].value("x", 0.0);
        doc.camera_y    = j["camera"].value("y", 0.0);
        doc.camera_zoom = j["camera"].value("zoom", 1.0);
    }

    if (j.contains("layers") && j["layers"].is_array())
    {
        for (auto& lj : j["layers"])
            doc.layers.push_back(LayerFromJson(lj));
    }

    if (j.contains("solids") && j["solids"].is_array())
    {
        for (auto& sj : j["solids"])
        {
            SceneSolid s;
            s.x    = sj.value("x", 0.0f);
            s.y    = sj.value("y", 0.0f);
            s.w    = sj.value("w", 0.0f);
            s.h    = sj.value("h", 0.0f);
            s.name = sj.value("name", std::string());
            doc.solids.push_back(s);
        }
    }

    if (doc.layers.empty())
        doc.EnsureDefaultLayers();

    return doc;
}

// ── File I/O ────────────────────────────────────────────────────────────────

bool SceneDocument::SaveToFile(const std::string& path)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out << ToJson().dump(2) << '\n';
    if (!out.good()) return false;
    file_path = path;
    dirty     = false;
    return true;
}

bool SceneDocument::LoadFromFile(const std::string& path, SceneDocument& out)
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;
    std::ostringstream buf;
    buf << in.rdbuf();
    try
    {
        json j = json::parse(buf.str());
        out = SceneDocument::FromJson(j);
        out.file_path = path;
        out.dirty     = false;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace le
