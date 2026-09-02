#!/usr/bin/env python3
"""Assign Nerd Fonts weather glyphs (i_weather.sh) to semantic Tuinator domains."""

from __future__ import annotations

import re

from weather_icon_naming import ascii_for, categorize, pascal_case

COMPASS_DIRECTION_ONLY = True  # wind arrows live in Weather, not Compass

WEATHER_WIND_SUFFIXES = {
    "wind_direction",
    "wind_east",
    "wind_west",
    "wind_north",
    "wind_south",
    "wind_north_east",
    "wind_north_west",
    "wind_south_east",
    "wind_south_west",
}

DOMAIN_ORDER = [
    "Weather",
    "Moon",
    "Clock",
    "Compass",
    "Measurement",
    "Astronomy",
    "AirQuality",
    "Hazard",
    "MarineAlert",
    "Action",
]

DOMAIN_PATH_PREFIX = {
    "Weather": "weather",
    "Moon": "moon",
    "Clock": "clock",
    "Compass": "compass",
    "Measurement": "measurement",
    "Astronomy": "astronomy",
    "AirQuality": "air-quality",
    "Hazard": "hazard",
    "MarineAlert": "marine",
    "Action": "action",
}

DOMAIN_CPP_STEM = {
    "Weather": "weather_icon",
    "Moon": "moon_icon",
    "Clock": "clock_icon",
    "Compass": "compass_icon",
    "Measurement": "measurement_icon",
    "Astronomy": "astronomy_icon",
    "AirQuality": "air_quality_icon",
    "Hazard": "hazard_icon",
    "MarineAlert": "marine_alert_icon",
    "Action": "action_icon",
}

DOMAIN_ENUM = {
    "Weather": "WeatherIcon",
    "Moon": "MoonIcon",
    "Clock": "ClockIcon",
    "Compass": "CompassIcon",
    "Measurement": "MeasurementIcon",
    "Astronomy": "AstronomyIcon",
    "AirQuality": "AirQualityIcon",
    "Hazard": "HazardIcon",
    "MarineAlert": "MarineAlertIcon",
    "Action": "ActionIcon",
}

DOMAIN_CATEGORY_ENUM = {
    "Weather": "WeatherIconCategory",
    "Moon": "MoonIconCategory",
    "Clock": "ClockIconCategory",
    "Compass": "CompassIconCategory",
    "Measurement": "MeasurementIconCategory",
    "Astronomy": "AstronomyIconCategory",
    "AirQuality": "AirQualityIconCategory",
    "Hazard": "HazardIconCategory",
    "MarineAlert": "MarineAlertIconCategory",
    "Action": "ActionIconCategory",
}

WEATHER_CATEGORY_ORDER = [
    "Day",
    "NightAlt",
    "Night",
    "Cloud",
    "Precipitation",
    "Storm",
    "Wind",
    "Unavailable",
    "Equipment",
    "Misc",
]

MOON_CATEGORY_ORDER = ["Moon", "MoonAlt"]

COMPASS_CATEGORY_ORDER = ["Direction"]

ASTRONOMY_CATEGORY_ORDER = ["Celestial", "Eclipse", "Sky", "Misc"]

SINGLE_CATEGORY_DOMAINS = {
    "Clock": "Clock",
    "Measurement": "Measurement",
    "AirQuality": "AirQuality",
    "Hazard": "Hazard",
    "MarineAlert": "MarineAlert",
    "Action": "Action",
}


def domain_for(suffix: str, weather_category: str) -> str:
    if suffix.startswith("time_"):
        return "Clock"
    if suffix.startswith("direction_"):
        return "Compass"
    if suffix in WEATHER_WIND_SUFFIXES:
        return "Weather"
    if suffix.startswith("moon_"):
        return "Moon"
    if suffix in (
        "celsius",
        "fahrenheit",
        "degrees",
        "hot",
        "thermometer",
        "thermometer_exterior",
        "thermometer_internal",
        "barometer",
        "humidity",
    ):
        return "Measurement"
    if suffix in (
        "stars",
        "lunar_eclipse",
        "solar_eclipse",
        "moonrise",
        "moonset",
        "sunrise",
        "sunset",
        "horizon",
        "horizon_alt",
        "alien",
        "aliens",
    ):
        return "Astronomy"
    if suffix in ("dust", "smoke", "smog"):
        return "AirQuality"
    if suffix in ("volcano", "earthquake", "fire", "meteor", "flood", "tsunami"):
        return "Hazard"
    if suffix in ("gale_warning", "small_craft_advisory"):
        return "MarineAlert"
    if suffix in ("refresh", "refresh_alt"):
        return "Action"
    if suffix in ("na",):
        return "Weather"
    if suffix in ("umbrella",):
        return "Weather"
    if weather_category == "Wind" and (
        suffix.startswith("wind_beaufort_") or suffix in ("windy", "strong_wind")
    ):
        return "Weather"
    if weather_category in ("Day", "Night", "NightAlt", "Cloud", "Precipitation", "Storm", "Other"):
        return "Weather"
    return "Weather"


def category_for_domain(domain: str, suffix: str, weather_category: str) -> str:
    if domain == "Weather":
        if suffix == "na":
            return "Unavailable"
        if suffix == "umbrella":
            return "Equipment"
        if suffix in ("raindrop", "raindrops", "snowflake_cold"):
            return "Precipitation"
        if suffix == "train":
            return "Misc"
        if suffix in WEATHER_WIND_SUFFIXES or weather_category == "Wind" or suffix.startswith("wind_beaufort_"):
            return "Wind"
        return weather_category if weather_category != "Other" else "Misc"
    if domain == "Moon":
        return "MoonAlt" if suffix.startswith("moon_alt_") else "Moon"
    if domain == "Compass":
        return "Direction"
    if domain == "Astronomy":
        if suffix in ("sunrise", "sunset", "moonrise", "moonset"):
            return "Celestial"
        if suffix in ("lunar_eclipse", "solar_eclipse"):
            return "Eclipse"
        if suffix in ("horizon", "horizon_alt", "stars"):
            return "Sky"
        return "Misc"
    return SINGLE_CATEGORY_DOMAINS[domain]


def path_tail_for_domain(domain: str, suffix: str) -> str:
    if domain == "Moon":
        body = suffix
        if body.startswith("moon_alt_"):
            body = body[len("moon_alt_") :]
        elif body.startswith("moon_"):
            body = body[len("moon_") :]
        return body.replace("_", "-")
    return suffix.replace("_", "-")


def nested_path(domain: str, suffix: str) -> str:
    prefix = DOMAIN_PATH_PREFIX[domain]
    tail = path_tail_for_domain(domain, suffix)
    return f"{prefix}-{tail}"


def category_path_for(domain: str, category: str) -> str:
    prefix = DOMAIN_PATH_PREFIX[domain]
    mapping = {
        "Day": f"{prefix}-day",
        "NightAlt": f"{prefix}-night-alt",
        "Night": f"{prefix}-night",
        "Moon": f"{prefix}",
        "MoonAlt": f"{prefix}-alt",
        "Wind": f"{prefix}-wind",
        "Storm": f"{prefix}-storm",
        "Cloud": f"{prefix}-cloud",
        "Precipitation": f"{prefix}-precipitation",
        "Unavailable": f"{prefix}-unavailable",
        "Equipment": f"{prefix}-equipment",
        "Misc": f"{prefix}-misc",
        "Direction": f"{prefix}-direction",
        "Celestial": f"{prefix}-celestial",
        "Eclipse": f"{prefix}-eclipse",
        "Sky": f"{prefix}-sky",
        "Misc": f"{prefix}-misc",
        "Clock": prefix,
        "Measurement": prefix,
        "Astronomy": prefix,
        "AirQuality": prefix,
        "Hazard": prefix,
        "MarineAlert": prefix,
        "Action": prefix,
    }
    return mapping.get(category, prefix)


def category_order_for_domain(domain: str) -> list[str]:
    if domain == "Weather":
        return WEATHER_CATEGORY_ORDER
    if domain == "Moon":
        return MOON_CATEGORY_ORDER
    if domain == "Compass":
        return COMPASS_CATEGORY_ORDER
    if domain == "Astronomy":
        return ASTRONOMY_CATEGORY_ORDER
    if domain in SINGLE_CATEGORY_DOMAINS:
        return [SINGLE_CATEGORY_DOMAINS[domain]]
    return []


def row_from_suffix(suffix: str, codepoint: int, weather_category: str | None = None) -> dict[str, str]:
    if weather_category is None:
        weather_category = categorize(suffix)
    domain = domain_for(suffix, weather_category)
    category = category_for_domain(domain, suffix, weather_category)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": nested_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": ascii_for(suffix),
    }
