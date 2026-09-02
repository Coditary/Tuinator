#!/usr/bin/env python3
"""Shared editor icon domains for Codicons + Octicons."""

from __future__ import annotations

EDITOR_SOCIAL_SUFFIXES = {
    "heart",
    "heart_filled",
    "mention",
    "reactions",
    "smiley",
    "thumbsdown",
    "thumbsup",
    "star_empty",
    "star_full",
    "star_half",
    "star",
    "unverified",
}

EDITOR_COMMUNICATION_PREFIXES = ("mail", "inbox", "send", "rss", "call_", "broadcast")
EDITOR_SHAPE_PREFIXES = ("circle", "primitive_square", "square", "triangle_", "dash")
EDITOR_MEDIA_PREFIXES = ("device_camera", "device_mobile", "mic", "record", "radio")
EDITOR_CHART_PREFIXES = ("graph", "pulse", "dashboard", "pie_chart", "organization")


def shared_editor_domain(suffix: str) -> str | None:
    if suffix.startswith("star_") or suffix in EDITOR_SOCIAL_SUFFIXES:
        return "EditorSocial"
    if suffix.startswith(EDITOR_COMMUNICATION_PREFIXES) or suffix in {"mail", "inbox", "send", "rss"}:
        return "EditorCommunication"
    if suffix.startswith(EDITOR_SHAPE_PREFIXES) or suffix in {"circle", "square"}:
        return "EditorShape"
    if suffix.startswith(EDITOR_MEDIA_PREFIXES):
        return "EditorMedia"
    if suffix.startswith(EDITOR_CHART_PREFIXES):
        return "EditorChart"
    return None


SHARED_EDITOR_DOMAINS = (
    "EditorSocial",
    "EditorCommunication",
    "EditorShape",
    "EditorMedia",
    "EditorChart",
)

SHARED_EDITOR_PATH_PREFIX = {
    "EditorSocial": "editor-social",
    "EditorCommunication": "editor-communication",
    "EditorShape": "editor-shape",
    "EditorMedia": "editor-media",
    "EditorChart": "editor-chart",
}

SHARED_EDITOR_CPP_STEM = {
    "EditorSocial": "editor_social_icon",
    "EditorCommunication": "editor_communication_icon",
    "EditorShape": "editor_shape_icon",
    "EditorMedia": "editor_media_icon",
    "EditorChart": "editor_chart_icon",
}

SHARED_EDITOR_ENUM = {
    "EditorSocial": "EditorSocialIcon",
    "EditorCommunication": "EditorCommunicationIcon",
    "EditorShape": "EditorShapeIcon",
    "EditorMedia": "EditorMediaIcon",
    "EditorChart": "EditorChartIcon",
}

SHARED_EDITOR_CATEGORY_ENUM = {
    "EditorSocial": "EditorSocialIconCategory",
    "EditorCommunication": "EditorCommunicationIconCategory",
    "EditorShape": "EditorShapeIconCategory",
    "EditorMedia": "EditorMediaIconCategory",
    "EditorChart": "EditorChartIconCategory",
}
