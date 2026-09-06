#include <tuinator/scene/detail/binding_engine.hpp>

#include <tuinator/scene/detail/json_node.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/display/spinner.hpp>
#include <tuinator/widgets/controls/text_area.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/controls/toggle.hpp>
#include <tuinator/widgets/display/big_text.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>
#include <tuinator/widgets/chrome/status_bar.hpp>
#include <tuinator/widgets/views/list_view.hpp>

#include <stdexcept>
#include <string>

namespace tuinator::scene::detail {

namespace {

struct BindingSpec {
    std::string source_id;
    std::string target_id;
    std::string source_property;
    std::string target_property;
    std::string event;
    std::string template_text;
    std::string fallback;
    std::string converter;
    bool initial = true;
};

std::string first_string(const nlohmann::json& node, const char* key1, const char* key2 = nullptr) {
    if (json::find(node, key1)) {
        return json::node_string(node, key1);
    }
    if (key2 != nullptr && json::find(node, key2)) {
        return json::node_string(node, key2);
    }
    return {};
}

BindingSpec parse_binding(const nlohmann::json& binding) {
    BindingSpec spec;
    spec.source_id = json::node_string(binding, "from");
    spec.target_id = json::node_string(binding, "to");
    if (spec.source_id.empty() || spec.target_id.empty()) {
        throw std::runtime_error("Binding requires 'from' and 'to' node ids");
    }
    spec.source_property = first_string(binding, "fromProperty", "sourceProperty");
    spec.target_property = first_string(binding, "toProperty", "targetProperty");
    spec.event = json::node_string(binding, "change", "event");
    if (binding.contains("template") && binding["template"].is_string()) {
        spec.template_text = binding["template"].get<std::string>();
    }
    if (binding.contains("fallback") && binding["fallback"].is_string()) {
        spec.fallback = binding["fallback"].get<std::string>();
    }
    if (binding.contains("converter") && binding["converter"].is_string()) {
        spec.converter = binding["converter"].get<std::string>();
    }
    spec.initial = binding.value("initial", true);
    return spec;
}

std::string default_source_property(const std::string& type, const std::string& event) {
    if ((type == "ComboBox" || type == "ListView") &&
        (event == "select" || event == "onSelect" || event == "on_select")) {
        return "selectedItem";
    }
    if (type == "Checkbox" || type == "Toggle") return "checked";
    if (type == "Slider" || type == "Spinner") return "value";
    return "value";
}

std::string default_target_property(const std::string& type) {
    if (type == "Label" || type == "StatusBar" || type == "BigText") return "text";
    if (type == "TextInput" || type == "TextArea") return "value";
    if (type == "Checkbox" || type == "Toggle") return "checked";
    if (type == "Slider" || type == "Spinner" || type == "ProgressBar") return "value";
    return "text";
}

std::string format_template(
    const std::string& template_text,
    const std::string& string_value,
    const std::string& fallback) {
    const std::string placeholder = "{value}";
    const auto pos = template_text.find(placeholder);
    if (pos == std::string::npos) {
        return template_text;
    }
    std::string value = string_value;
    if (!fallback.empty() && value.empty()) {
        value = fallback;
    }
    return template_text.substr(0, pos) + value + template_text.substr(pos + placeholder.size());
}

std::string bool_to_string(bool value, const std::string& converter) {
    if (converter == "yesNo") return value ? "Yes" : "No";
    if (converter == "onOff") return value ? "ON" : "OFF";
    return value ? "true" : "false";
}

std::string combo_selected_item(const ComboBox* widget) {
    if (widget == nullptr) return {};
    const auto& items = widget->items();
    const int index = widget->selected_index();
    if (index < 0 || index >= static_cast<int>(items.size())) return {};
    return items[index];
}

std::string list_selected_item(const ListView* widget) {
    if (widget == nullptr) return {};
    const auto& items = widget->items();
    const int index = widget->selected_index();
    if (index < 0 || index >= static_cast<int>(items.size())) return {};
    return items[index];
}

void set_target_string(
    WidgetEntry& target,
    const std::string& value,
    const BindingSpec& spec) {
    std::string text = value;
    if (!spec.template_text.empty()) {
        text = format_template(spec.template_text, value, spec.fallback);
    } else if (!spec.fallback.empty() && text.empty()) {
        text = spec.fallback;
    }
    if (target.type == "Label") {
        static_cast<Label*>(target.ptr)->set_text(text);
    } else if (target.type == "StatusBar") {
        static_cast<StatusBar*>(target.ptr)->set_text(text);
    } else if (target.type == "BigText") {
        static_cast<BigText*>(target.ptr)->set_text(text);
    } else if (target.type == "TextInput") {
        static_cast<TextInput*>(target.ptr)->set_value(text);
    } else if (target.type == "TextArea") {
        static_cast<TextArea*>(target.ptr)->set_value(text);
    } else {
        throw std::runtime_error("Unsupported string binding target: " + target.type);
    }
}

void set_target_bool(WidgetEntry& target, bool value) {
    if (target.type == "Checkbox") {
        static_cast<Checkbox*>(target.ptr)->set_checked(value);
    } else if (target.type == "Toggle") {
        static_cast<Toggle*>(target.ptr)->set_checked(value);
    } else {
        throw std::runtime_error("Unsupported bool binding target: " + target.type);
    }
}

void set_target_int(WidgetEntry& target, int value) {
    if (target.type == "Slider") {
        static_cast<Slider*>(target.ptr)->set_value(value);
    } else if (target.type == "Spinner") {
        static_cast<Spinner*>(target.ptr)->set_value(value);
    } else {
        throw std::runtime_error("Unsupported int binding target: " + target.type);
    }
}

void set_target_double(WidgetEntry& target, double value) {
    if (target.type == "ProgressBar") {
        static_cast<ProgressBar*>(target.ptr)->set_value(value);
    } else {
        throw std::runtime_error("Unsupported double binding target: " + target.type);
    }
}

void apply_text_binding_from_string(
    WidgetEntry& target,
    const std::string& value,
    const BindingSpec& spec) {
    std::string text = value;
    if (!spec.fallback.empty() && text.empty()) {
        text = spec.fallback;
    }
    if (!spec.template_text.empty()) {
        text = format_template(spec.template_text, text, spec.fallback);
    }
    set_target_string(target, text, {});
}

void apply_binding_once(const BindingSpec& spec, SceneContext& ctx) {
    WidgetEntry* source = ctx.find_widget(spec.source_id);
    WidgetEntry* target = ctx.find_widget(spec.target_id);
    if (source == nullptr || target == nullptr) {
        throw std::runtime_error("Binding references unknown widget id");
    }
    const std::string source_property = spec.source_property.empty()
        ? default_source_property(source->type, spec.event)
        : spec.source_property;
    const std::string target_property = spec.target_property.empty()
        ? default_target_property(target->type)
        : spec.target_property;

    if (source->type == "TextInput" && source_property == "value") {
        const auto* input = static_cast<TextInput*>(source->ptr);
        apply_text_binding_from_string(*target, input->value(), spec);
        return;
    }
    if (source->type == "TextArea" && source_property == "value") {
        const auto* area = static_cast<TextArea*>(source->ptr);
        apply_text_binding_from_string(*target, area->value(), spec);
        return;
    }
    if (source->type == "Checkbox" && source_property == "checked") {
        const auto* checkbox = static_cast<Checkbox*>(source->ptr);
        if (target_property == "text") {
            set_target_string(*target, bool_to_string(checkbox->checked(), spec.converter), spec);
        } else if (target_property == "checked") {
            set_target_bool(*target, checkbox->checked());
        }
        return;
    }
    if (source->type == "Toggle" && source_property == "checked") {
        const auto* toggle = static_cast<Toggle*>(source->ptr);
        if (target_property == "text") {
            set_target_string(*target, bool_to_string(toggle->checked(), spec.converter), spec);
        } else if (target_property == "checked") {
            set_target_bool(*target, toggle->checked());
        }
        return;
    }
    if (source->type == "Slider" && source_property == "value") {
        const int value = static_cast<Slider*>(source->ptr)->value();
        if (target_property == "value" && target->type == "ProgressBar") {
            double converted = static_cast<double>(value);
            if (spec.converter == "percent01" || spec.converter == "ratio") {
                converted /= 100.0;
            }
            set_target_double(*target, converted);
        } else if (target_property == "value") {
            set_target_int(*target, value);
        } else if (target_property == "text") {
            std::string text = std::to_string(value);
            if (spec.converter == "percent") text += "%";
            set_target_string(*target, text, spec);
        }
        return;
    }
    if (source->type == "Spinner" && source_property == "value") {
        const int value = static_cast<Spinner*>(source->ptr)->value();
        if (target_property == "value" && target->type == "ProgressBar") {
            set_target_double(*target, static_cast<double>(value));
        } else if (target_property == "value") {
            set_target_int(*target, value);
        } else if (target_property == "text") {
            set_target_string(*target, std::to_string(value), spec);
        }
        return;
    }
    if (source->type == "ComboBox") {
        const auto* combo = static_cast<ComboBox*>(source->ptr);
        if (source_property == "selectedIndex") {
            if (target_property == "value") set_target_int(*target, combo->selected_index());
        } else {
            apply_text_binding_from_string(*target, combo_selected_item(combo), spec);
        }
        return;
    }
    if (source->type == "ListView") {
        const auto* list = static_cast<ListView*>(source->ptr);
        if (source_property == "selectedIndex") {
            if (target_property == "value") set_target_int(*target, list->selected_index());
        } else {
            apply_text_binding_from_string(*target, list_selected_item(list), spec);
        }
        return;
    }
    throw std::runtime_error(
        "Unsupported binding from " + source->type + "." + source_property + " to " + target->type + "." +
        target_property);
}

void subscribe_binding(const BindingSpec& spec, SceneContext& ctx) {
    WidgetEntry* source = ctx.find_widget(spec.source_id);
    WidgetEntry* target = ctx.find_widget(spec.target_id);
    if (source == nullptr || target == nullptr) {
        throw std::runtime_error("Binding references unknown widget id");
    }
    const std::string source_property = spec.source_property.empty()
        ? default_source_property(source->type, spec.event)
        : spec.source_property;

    if (source->type == "TextInput") {
        auto* input = static_cast<TextInput*>(source->ptr);
        input->set_on_change([target, spec](const std::string& value) {
            apply_text_binding_from_string(*target, value, spec);
        });
        return;
    }
    if (source->type == "TextArea") {
        auto* area = static_cast<TextArea*>(source->ptr);
        area->set_on_change([target, spec](const std::string& value) {
            apply_text_binding_from_string(*target, value, spec);
        });
        return;
    }
    if (source->type == "ComboBox") {
        auto* combo = static_cast<ComboBox*>(source->ptr);
        combo->set_on_select([target, spec, source_property](int index, const std::string& item) {
            if (source_property == "selectedIndex") {
                if (target->type == "Slider" || target->type == "Spinner") {
                    set_target_int(*target, index);
                }
            } else {
                apply_text_binding_from_string(*target, item, spec);
            }
        });
        return;
    }
    if (source->type == "ListView") {
        auto* list = static_cast<ListView*>(source->ptr);
        list->set_on_select([target, spec, source_property](int index, const std::string& item) {
            if (source_property == "selectedIndex") {
                if (target->type == "Slider" || target->type == "Spinner") {
                    set_target_int(*target, index);
                }
            } else {
                apply_text_binding_from_string(*target, item, spec);
            }
        });
        return;
    }
}

} // namespace

void apply_bindings(const nlohmann::json& bindings, SceneContext& ctx) {
    if (!bindings.is_array()) {
        return;
    }
    std::vector<BindingSpec> specs;
    for (const auto& binding : bindings) {
        specs.push_back(parse_binding(binding));
    }
    for (const BindingSpec& spec : specs) {
        if (spec.initial) {
            apply_binding_once(spec, ctx);
        }
        WidgetEntry* source = ctx.find_widget(spec.source_id);
        if (source == nullptr) continue;
        if (source->type == "TextInput" || source->type == "TextArea" ||
            source->type == "ComboBox" || source->type == "ListView") {
            subscribe_binding(spec, ctx);
        }
    }
}

} // namespace tuinator::scene::detail
