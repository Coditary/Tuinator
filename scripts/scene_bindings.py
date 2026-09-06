"""Property binding codegen for scripts/generate_scene_cpp.py."""

from __future__ import annotations

import json
from dataclasses import dataclass
from typing import Any, Callable

ValueType = str  # string | int | bool | double


@dataclass(frozen=True)
class SourceProperty:
    value_type: ValueType
    subscribe: str  # set_on_change | set_on_select | constructor
    callback_param: str
    read_expr: str  # uses {ptr}


@dataclass(frozen=True)
class TargetProperty:
    value_type: ValueType
    write_expr: str  # uses {ptr} and {expr}


@dataclass
class BindingSpec:
    source_id: str
    target_id: str
    source_property: str | None
    target_property: str | None
    event: str
    template: str | None
    fallback: str | None
    converter: str | None
    initial: bool


SOURCE_PROPERTIES: dict[tuple[str, str], SourceProperty] = {
    ("TextInput", "value"): SourceProperty(
        "string", "set_on_change", "const std::string& value", "{ptr}->value()"
    ),
    ("TextArea", "value"): SourceProperty(
        "string", "set_on_change", "const std::string& value", "{ptr}->value()"
    ),
    ("Checkbox", "checked"): SourceProperty(
        "bool", "constructor", "bool value", "{ptr}->checked()"
    ),
    ("Toggle", "checked"): SourceProperty(
        "bool", "constructor", "bool value", "{ptr}->checked()"
    ),
    ("Slider", "value"): SourceProperty(
        "int", "constructor", "int value", "{ptr}->value()"
    ),
    ("Spinner", "value"): SourceProperty(
        "int", "constructor", "int value", "{ptr}->value()"
    ),
    ("ComboBox", "selectedIndex"): SourceProperty(
        "int", "set_on_select", "int index, const std::string& item", "{ptr}->selected_index()"
    ),
    ("ComboBox", "selectedItem"): SourceProperty(
        "string",
        "set_on_select",
        "int index, const std::string& item",
        "tuinator::scene_binding::combo_selected_item({ptr})",
    ),
    ("ListView", "selectedIndex"): SourceProperty(
        "int", "set_on_select", "int index, const std::string& item", "{ptr}->selected_index()"
    ),
    ("ListView", "selectedItem"): SourceProperty(
        "string",
        "set_on_select",
        "int index, const std::string& item",
        "tuinator::scene_binding::list_selected_item({ptr})",
    ),
}

TARGET_PROPERTIES: dict[tuple[str, str], TargetProperty] = {
    ("Label", "text"): TargetProperty("string", "{ptr}->set_text({expr})"),
    ("StatusBar", "text"): TargetProperty("string", "{ptr}->set_text({expr})"),
    ("BigText", "text"): TargetProperty("string", "{ptr}->set_text({expr})"),
    ("TextInput", "value"): TargetProperty("string", "{ptr}->set_value({expr})"),
    ("TextArea", "value"): TargetProperty("string", "{ptr}->set_value({expr})"),
    ("Checkbox", "checked"): TargetProperty("bool", "{ptr}->set_checked({expr})"),
    ("Toggle", "checked"): TargetProperty("bool", "{ptr}->set_checked({expr})"),
    ("Slider", "value"): TargetProperty("int", "{ptr}->set_value({expr})"),
    ("Spinner", "value"): TargetProperty("int", "{ptr}->set_value({expr})"),
    ("ProgressBar", "value"): TargetProperty("double", "{ptr}->set_value({expr})"),
}

DEFAULT_SOURCE_PROPERTY: dict[str, str] = {
    "TextInput": "value",
    "TextArea": "value",
    "Checkbox": "checked",
    "Toggle": "checked",
    "Slider": "value",
    "Spinner": "value",
    "ComboBox": "selectedItem",
    "ListView": "selectedItem",
}

DEFAULT_TARGET_PROPERTY: dict[str, str] = {
    "Label": "text",
    "StatusBar": "text",
    "BigText": "text",
    "TextInput": "value",
    "TextArea": "value",
    "Checkbox": "checked",
    "Toggle": "checked",
    "Slider": "value",
    "Spinner": "value",
    "ProgressBar": "value",
}

SELECT_EVENTS = frozenset({"select", "onSelect", "on_select"})
CHANGE_EVENTS = frozenset({"change", "onChange", "on_change"})


def cpp_string(value: str) -> str:
    return json.dumps(value, ensure_ascii=True)


def parse_bindings(bindings: list[Any]) -> list[BindingSpec]:
    specs: list[BindingSpec] = []
    for binding in bindings:
        if not isinstance(binding, dict):
            raise ValueError("Each binding must be an object")
        source_id = binding.get("from")
        target_id = binding.get("to")
        if not source_id or not target_id:
            raise ValueError("Binding requires 'from' and 'to' node ids")
        specs.append(
            BindingSpec(
                source_id=str(source_id),
                target_id=str(target_id),
                source_property=_first_str(
                    binding,
                    "fromProperty",
                    "sourceProperty",
                    "source_property",
                ),
                target_property=_first_str(
                    binding,
                    "toProperty",
                    "targetProperty",
                    "target_property",
                ),
                event=str(binding.get("event", "change")),
                template=binding.get("template"),
                fallback=binding.get("fallback"),
                converter=binding.get("converter"),
                initial=bool(binding.get("initial", True)),
            )
        )
    return specs


def _first_str(binding: dict[str, Any], *keys: str) -> str | None:
    for key in keys:
        value = binding.get(key)
        if value is not None:
            return str(value)
    return None


def resolve_source_property(widget_type: str, property_name: str | None, event: str) -> str:
    if property_name:
        return property_name
    if widget_type in {"ComboBox", "ListView"} and event in SELECT_EVENTS:
        return "selectedItem"
    return DEFAULT_SOURCE_PROPERTY.get(widget_type, "value")


def resolve_target_property(widget_type: str, property_name: str | None) -> str:
    if property_name:
        return property_name
    return DEFAULT_TARGET_PROPERTY.get(widget_type, "text")


def source_property_def(widget_type: str, property_name: str) -> SourceProperty:
    key = (widget_type, property_name)
    if key not in SOURCE_PROPERTIES:
        allowed = sorted({prop for widget, prop in SOURCE_PROPERTIES if widget == widget_type})
        raise ValueError(
            f"Unsupported source property '{property_name}' on {widget_type}. "
            f"Allowed: {', '.join(allowed) or 'none'}"
        )
    return SOURCE_PROPERTIES[key]


def target_property_def(widget_type: str, property_name: str) -> TargetProperty:
    key = (widget_type, property_name)
    if key not in TARGET_PROPERTIES:
        allowed = sorted({prop for widget, prop in TARGET_PROPERTIES if widget == widget_type})
        raise ValueError(
            f"Unsupported target property '{property_name}' on {widget_type}. "
            f"Allowed: {', '.join(allowed) or 'none'}"
        )
    return TARGET_PROPERTIES[key]


def callback_value_expr(
    source_def: SourceProperty,
    source_property: str,
    param_name: str,
) -> str:
    if source_def.subscribe == "set_on_select":
        if source_property.endswith("Index") or source_property == "selectedIndex":
            return "index"
        return "item"
    return param_name


def convert_to_type(
    value_expr: str,
    source_type: ValueType,
    target_type: ValueType,
    converter: str | None,
) -> str:
    if source_type == target_type:
        return value_expr
    if target_type == "string":
        return convert_to_string(value_expr, source_type, converter)
    if target_type == "double" and source_type == "int":
        if converter in {"percent01", "ratio"}:
            return f"(static_cast<double>({value_expr}) / 100.0)"
        return f"static_cast<double>({value_expr})"
    if target_type == "int" and source_type == "double":
        return f"static_cast<int>({value_expr})"
    if target_type == "bool" and source_type == "string":
        return f"!{value_expr}.empty()"
    raise ValueError(
        f"Cannot convert binding value from {source_type} to {target_type}"
        + (f" (converter={converter})" if converter else "")
    )


def convert_to_string(value_expr: str, source_type: ValueType, converter: str | None) -> str:
    if source_type == "string":
        return value_expr
    if converter == "yesNo":
        return f"({value_expr} ? std::string(\"Yes\") : std::string(\"No\"))"
    if converter == "onOff":
        return f"({value_expr} ? std::string(\"ON\") : std::string(\"OFF\"))"
    if converter == "percent":
        return f"(std::to_string({value_expr}) + \"%\")"
    if source_type == "bool":
        return f"({value_expr} ? std::string(\"true\") : std::string(\"false\"))"
    if source_type in {"int", "double"}:
        return f"std::to_string({value_expr})"
    raise ValueError(f"Cannot convert {source_type} to string")


def format_template(template: str, value_expr: str, fallback: str | None, string_expr: str) -> str:
    if "{value}" not in template:
        return cpp_string(template)
    parts = template.split("{value}")
    if len(parts) != 2:
        raise ValueError(f"Binding template must contain exactly one {{value}} placeholder: {template}")
    if fallback:
        string_expr = (
            f"({string_expr}.empty() ? std::string({cpp_string(fallback)}) : {string_expr})"
        )
    return cpp_string(parts[0]) + " + " + string_expr + " + " + cpp_string(parts[1])


def emit_assignment(
    source_ctx: Any,
    target_ctx: Any,
    spec: BindingSpec,
    value_expr: str,
) -> str:
    source_property = resolve_source_property(
        source_ctx.cpp_type, spec.source_property, spec.event
    )
    target_property = resolve_target_property(target_ctx.cpp_type, spec.target_property)
    source_def = source_property_def(source_ctx.cpp_type, source_property)
    target_def = target_property_def(target_ctx.cpp_type, target_property)

    typed_value = value_expr
    if target_def.value_type == "string":
        string_value = convert_to_string(value_expr, source_def.value_type, spec.converter)
        if spec.template:
            final_expr = format_template(spec.template, value_expr, spec.fallback, string_value)
        elif spec.fallback and source_def.value_type == "string":
            final_expr = (
                f"({value_expr}.empty() ? std::string({cpp_string(spec.fallback)}) : {value_expr})"
            )
        else:
            final_expr = string_value
    else:
        final_expr = convert_to_type(
            value_expr, source_def.value_type, target_def.value_type, spec.converter
        )

    write = target_def.write_expr.format(ptr=target_ctx.ptr_name, expr=final_expr)
    return f"{write};"


def emit_read_assignment(source_ctx: Any, target_ctx: Any, spec: BindingSpec) -> str:
    source_property = resolve_source_property(
        source_ctx.cpp_type, spec.source_property, spec.event
    )
    source_def = source_property_def(source_ctx.cpp_type, source_property)
    read_expr = source_def.read_expr.format(ptr=source_ctx.ptr_name)
    return emit_assignment(source_ctx, target_ctx, spec, read_expr)


def emit_callback_updates_for_source(
    source_id: str,
    specs: list[BindingSpec],
    ids: dict[str, Any],
    value_expr: str,
) -> list[str]:
    updates: list[str] = []
    for spec in specs:
        if spec.source_id != source_id:
            continue
        source_ctx = ids[spec.source_id]
        target_ctx = ids[spec.target_id]
        updates.append(emit_assignment(source_ctx, target_ctx, spec, value_expr))
    return updates


def emit_subscribe_binding(
    spec: BindingSpec,
    source_ctx: Any,
    target_ctx: Any,
) -> list[str]:
    source_property = resolve_source_property(
        source_ctx.cpp_type, spec.source_property, spec.event
    )
    source_def = source_property_def(source_ctx.cpp_type, source_property)
    if source_def.subscribe == "constructor":
        raise ValueError(
            f"Binding source '{spec.source_id}' ({source_ctx.cpp_type}) must use widget onChange; "
            "scene-level subscribe is not supported for this widget"
        )
    if spec.event not in CHANGE_EVENTS and spec.event not in SELECT_EVENTS:
        raise ValueError(f"Unsupported binding event: {spec.event}")

    value_expr = callback_value_expr(source_def, source_property, "value")
    assignment = emit_assignment(source_ctx, target_ctx, spec, value_expr)
    captures = [target_ctx.ptr_name]
    if source_def.subscribe == "set_on_select":
        callback = (
            f"    {source_ctx.ptr_name}->set_on_select([{', '.join(captures)}]"
            f"({source_def.callback_param}) {{\n"
            f"        {assignment}\n"
            f"    }});"
        )
    else:
        callback = (
            f"    {source_ctx.ptr_name}->set_on_change([{', '.join(captures)}]"
            f"({source_def.callback_param}) {{\n"
            f"        {assignment}\n"
            f"    }});"
        )
    return [callback]


def wrap_constructor_callback(
    source_id: str | None,
    specs: list[BindingSpec],
    ids: dict[str, Any],
    app_var: str,
    param_type: str,
    param_name: str,
    handler_body: str | None,
) -> str:
    updates: list[str] = []
    if source_id:
        updates.extend(
            emit_callback_updates_for_source(source_id, specs, ids, param_name)
        )
    if handler_body:
        updates.append(handler_body)
    if not updates:
        return "{}"
    body = "\n        ".join(updates)
    return f"[&{app_var}]({param_type} {param_name}) {{\n        {body}\n    }}"


def generate_bindings(
    specs: list[BindingSpec],
    ids: dict[str, Any],
) -> list[str]:
    lines: list[str] = []
    for spec in specs:
        if spec.source_id not in ids:
            raise ValueError(f"Binding source id '{spec.source_id}' was not found in the scene")
        if spec.target_id not in ids:
            raise ValueError(f"Binding target id '{spec.target_id}' was not found in the scene")

    for spec in specs:
        source_ctx = ids[spec.source_id]
        target_ctx = ids[spec.target_id]
        source_property = resolve_source_property(
            source_ctx.cpp_type, spec.source_property, spec.event
        )
        source_def = source_property_def(source_ctx.cpp_type, source_property)

        if spec.initial:
            lines.append(f"    {emit_read_assignment(source_ctx, target_ctx, spec)}")

        if source_def.subscribe == "constructor":
            continue
        lines.extend(emit_subscribe_binding(spec, source_ctx, target_ctx))

    return lines


def binding_helper_header() -> str:
    return """namespace tuinator::scene_binding {

inline std::string combo_selected_item(const ::tuinator::ComboBox* widget) {
    if (widget == nullptr) {
        return {};
    }
    const auto& items = widget->items();
    const int index = widget->selected_index();
    if (index < 0 || index >= static_cast<int>(items.size())) {
        return {};
    }
    return items[index];
}

inline std::string list_selected_item(const ::tuinator::ListView* widget) {
    if (widget == nullptr) {
        return {};
    }
    const auto& items = widget->items();
    const int index = widget->selected_index();
    if (index < 0 || index >= static_cast<int>(items.size())) {
        return {};
    }
    return items[index];
}

} // namespace tuinator::scene_binding
"""
