#include <tuinator/testing/json_protocol.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <stdexcept>

namespace tuinator::testing {

namespace {

Key parse_key(std::string_view name) {
    if (name == "Enter") {
        return Key::Enter;
    }
    if (name == "Escape") {
        return Key::Escape;
    }
    if (name == "Tab") {
        return Key::Tab;
    }
    if (name == "BackTab") {
        return Key::BackTab;
    }
    if (name == "Backspace") {
        return Key::Backspace;
    }
    if (name == "Delete") {
        return Key::Delete;
    }
    if (name == "Up") {
        return Key::Up;
    }
    if (name == "Down") {
        return Key::Down;
    }
    if (name == "Left") {
        return Key::Left;
    }
    if (name == "Right") {
        return Key::Right;
    }
    if (name == "Home") {
        return Key::Home;
    }
    if (name == "End") {
        return Key::End;
    }
    if (name == "PageUp") {
        return Key::PageUp;
    }
    if (name == "PageDown") {
        return Key::PageDown;
    }
    if (name == "Space") {
        return Key::None;
    }
    throw std::runtime_error("Unknown key name: " + std::string(name));
}

nlohmann::json ok_response(const nlohmann::json& payload = nlohmann::json::object()) {
    nlohmann::json out = payload;
    out["ok"] = true;
    return out;
}

nlohmann::json error_response(const std::string& message) {
    return nlohmann::json{{"ok", false}, {"error", message}};
}

} // namespace

std::string handle_json_command(AppDriver& driver, std::string_view line) {
    nlohmann::json request;
    try {
        request = nlohmann::json::parse(line);
    } catch (const std::exception& ex) {
        return error_response(std::string("Invalid JSON: ") + ex.what()).dump();
    }

    const std::string cmd = request.value("cmd", "");
    try {
        if (cmd == "snapshot") {
            return ok_response({{"snapshot", driver.snapshot_text()}}).dump();
        }
        if (cmd == "ansi") {
            return ok_response({{"ansi", driver.snapshot_ansi()}}).dump();
        }
        if (cmd == "tree") {
            const bool include_screen = request.value("include_screen", false);
            const bool include_render = request.value("include_render", false);
            UiTreeOptions options;
            options.include_screen_text = include_screen;
            options.include_render_styles = include_render;
            return ok_response({{"tree", nlohmann::json::parse(driver.ui_tree_json(options))}}).dump();
        }
        if (cmd == "styles") {
            const std::string id = request.value("id", "");
            if (id.empty()) {
                return error_response("styles requires id").dump();
            }
            return ok_response({{"styles", nlohmann::json::parse(driver.selector_styles_json(id))}}).dump();
        }
        if (cmd == "hover") {
            const std::string id = request.value("id", "");
            if (id.empty()) {
                return error_response("hover requires id").dump();
            }
            driver.hover(id);
            return ok_response().dump();
        }
        if (cmd == "png") {
            const std::string path = request.value("path", "");
            if (path.empty()) {
                return error_response("png requires path").dump();
            }
            if (!driver.write_snapshot_png(path)) {
                return error_response("Failed to write PNG").dump();
            }
            return ok_response({{"path", path}}).dump();
        }
        if (cmd == "click" || cmd == "activate") {
            const std::string id = request.value("id", "");
            if (id.empty()) {
                return error_response(cmd + " requires id").dump();
            }
            if (cmd == "click") {
                driver.click(id);
            } else {
                driver.activate(id);
            }
            return ok_response().dump();
        }
        if (cmd == "focus") {
            const std::string id = request.value("id", "");
            if (id.empty()) {
                return error_response("focus requires id").dump();
            }
            driver.focus(id);
            return ok_response().dump();
        }
        if (cmd == "type") {
            const std::string id = request.value("id", "");
            const std::string text = request.value("text", "");
            if (id.empty()) {
                driver.type_text(text);
            } else {
                driver.type_into(id, text);
            }
            return ok_response().dump();
        }
        if (cmd == "key") {
            const std::string key_name = request.value("key", "");
            if (key_name.empty()) {
                return error_response("key requires key").dump();
            }
            if (key_name == "Space") {
                driver.press_key(Key::None, ' ');
            } else if (key_name.size() == 1) {
                driver.press_key(Key::None, key_name[0]);
            } else {
                driver.press_key(parse_key(key_name), '\0');
            }
            return ok_response().dump();
        }
        if (cmd == "step") {
            driver.step_frame();
            return ok_response().dump();
        }
        if (cmd == "present") {
            driver.present();
            return ok_response().dump();
        }
        if (cmd == "quit" || cmd == "exit") {
            return ok_response({{"quit", true}}).dump();
        }
        if (cmd.empty()) {
            return error_response("Missing cmd").dump();
        }
        return error_response("Unknown cmd: " + cmd).dump();
    } catch (const std::exception& ex) {
        return error_response(ex.what()).dump();
    }
}

int run_json_protocol_loop(AppDriver& driver) {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        const std::string response = handle_json_command(driver, line);
        std::cout << response << '\n';
        std::cout.flush();

        nlohmann::json parsed = nlohmann::json::parse(response, nullptr, false);
        if (parsed.is_discarded()) {
            continue;
        }
        if (parsed.value("quit", false)) {
            return 0;
        }
        if (parsed.contains("ok") && parsed["ok"] == false) {
            return 1;
        }
        const std::string cmd = nlohmann::json::parse(line).value("cmd", "");
        if (cmd == "quit" || cmd == "exit") {
            return 0;
        }
    }
    return 0;
}

} // namespace tuinator::testing
