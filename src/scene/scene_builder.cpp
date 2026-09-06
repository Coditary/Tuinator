#include <tuinator/core/action_registry.hpp>
#include <tuinator/core/application.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/scene/detail/binding_engine.hpp>
#include <tuinator/scene/detail/json_node.hpp>
#include <tuinator/scene/detail/widget_builder.generated.hpp>
#include <tuinator/scene/scene.hpp>
#include <tuinator/scene/scene_context.hpp>

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

namespace tuinator::scene {

namespace {

Theme resolve_scene_theme(const nlohmann::json& scene, const Theme& fallback) {
    if (!scene.contains("theme") || !scene["theme"].is_object()) {
        return fallback;
    }
    const auto& theme_block = scene["theme"];
    ThemeOptions options{};
    if (const nlohmann::json* glyphs = json::find(theme_block, "glyphs", "glyphSet", "glyph_set")) {
        const std::string name = json::as_string(*glyphs);
        if (name == "Ascii")
            options.glyphs = GlyphSet::Ascii;
        else if (name == "Unicode")
            options.glyphs = GlyphSet::Unicode;
        else if (name == "UnicodeRounded")
            options.glyphs = GlyphSet::UnicodeRounded;
        else
            options.glyphs = GlyphSet::Auto;
    }
    if (const nlohmann::json* border = json::find(theme_block, "borderStyle", "border_style")) {
        const std::string name = json::as_string(*border);
        if (name == "Ascii")
            options.border_style = BorderStyle::Ascii;
        else if (name == "Light")
            options.border_style = BorderStyle::Light;
        else if (name == "Heavy")
            options.border_style = BorderStyle::Heavy;
        else if (name == "Double")
            options.border_style = BorderStyle::Double;
        else if (name == "Rounded")
            options.border_style = BorderStyle::Rounded;
    }
    const std::string preset = json::node_string_default(theme_block, "dark", "preset");
    if (preset == "light") {
        return light_theme(options);
    }
    return dark_theme(options);
}

void register_actions(const nlohmann::json& scene, detail::SceneContext& ctx) {
    if (!scene.contains("actions")) {
        return;
    }
    const auto& actions = scene["actions"];
    if (!actions.is_object()) {
        throw std::runtime_error("Scene actions must be an object");
    }
    ctx.actions = std::make_shared<ActionRegistry>();
    for (auto it = actions.begin(); it != actions.end(); ++it) {
        const nlohmann::json& action = it.value();
        RegisteredAction entry;
        entry.id = it.key();
        entry.label = json::node_string(action, "label");
        entry.category = json::node_string(action, "category");
        entry.shortcut = json::node_string(action, "shortcut");
        entry.hint = json::node_string(action, "hint");
        entry.icon = json::node_string(action, "icon");
        entry.enabled = json::node_bool(action, true, "enabled");
        const std::string handler = json::node_string(action, "handler");
        if (handler.empty()) {
            if (const nlohmann::json* value = json::find(action, "onTrigger", "on_trigger")) {
                entry.callback = [&ctx, name = json::as_string(*value)]() { ctx.invoke_handler(name); };
            } else if (const nlohmann::json* click = json::find(action, "onClick", "on_click")) {
                entry.callback = [&ctx, name = json::as_string(*click)]() { ctx.invoke_handler(name); };
            }
        } else {
            entry.callback = [&ctx, handler]() { ctx.invoke_handler(handler); };
        }
        ctx.actions->register_action(std::move(entry));
    }
}

void register_timers(const nlohmann::json& scene, detail::SceneContext& ctx) {
    if (!scene.contains("timers") || !scene["timers"].is_array()) {
        return;
    }
    for (const auto& timer : scene["timers"]) {
        const int interval = json::node_int(timer, 1000, "intervalMs", "interval_ms", "interval");
        const bool repeat = json::node_bool(timer, true, "repeat");
        std::string handler = json::node_string(timer, "onTick", "on_tick");
        if (handler.empty()) {
            handler = json::node_string(timer, "handler");
        }
        if (handler.empty()) {
            throw std::runtime_error("Timer requires onTick handler");
        }
        auto callback = [&ctx, handler]() { ctx.invoke_handler(handler); };
        if (repeat) {
            ctx.app.set_interval(interval, callback);
        } else {
            ctx.app.set_timeout(interval, callback);
        }
    }
}

BuildResult build_scene_document(const nlohmann::json& scene, Application& app, const Theme& theme,
                                 const LoadOptions& options) {
    if (!scene.is_object()) {
        throw std::runtime_error("Scene root must be a JSON object");
    }
    if (scene.value("version", 1) != 1) {
        throw std::runtime_error("Only scene version 1 is supported");
    }
    if (!scene.contains("root") || !scene["root"].is_object()) {
        throw std::runtime_error("Scene must contain a 'root' object");
    }

    Theme active_theme = resolve_scene_theme(scene, theme);
    detail::SceneContext ctx(app, std::move(active_theme), options.handlers);
    register_actions(scene, ctx);
    register_timers(scene, ctx);

    BuildResult result;
    result.actions = ctx.actions;
    result.root = detail::build_widget(scene["root"], ctx);
    if (scene.contains("bindings")) {
        detail::apply_bindings(scene["bindings"], ctx);
    }
    ctx.run_post_init();
    result.desktop = ctx.desktop;
    result.window_host = ctx.window_host;
    for (const auto& [id, entry] : ctx.widgets()) {
        result.widgets.emplace(id, WidgetRef{entry.type, entry.ptr});
    }
    return result;
}

} // namespace

BuildResult load_and_build(const std::filesystem::path& path, Application& app, const Theme& theme,
                           const LoadOptions& options) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Failed to open scene file: " + path.string());
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return load_and_build_from_string(buffer.str(), app, theme, options);
}

BuildResult load_and_build_from_string(const std::string& json_text, Application& app, const Theme& theme,
                                       const LoadOptions& options) {
    const nlohmann::json scene = nlohmann::json::parse(json_text);
    return build_scene_document(scene, app, theme, options);
}

} // namespace tuinator::scene
