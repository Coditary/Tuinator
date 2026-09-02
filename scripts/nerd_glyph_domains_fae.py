#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts FAE extension (i_fae.sh, 170 icons)."""

from __future__ import annotations

from nerd_font_lib import kebab_case, pascal_case

FAE_DOMAIN_ORDER = ["Food", "Home", "Body", "Activity", "Nature", "Misc"]

FAE_DOMAIN_PATH_PREFIX = {
    "Food": "food",
    "Home": "home",
    "Body": "body",
    "Activity": "activity",
    "Nature": "nature",
    "Misc": "misc",
}

FAE_DOMAIN_CPP_STEM = {
    "Food": "food_icon",
    "Home": "home_icon",
    "Body": "body_icon",
    "Activity": "activity_icon",
    "Nature": "nature_icon",
    "Misc": "misc_icon",
}

FAE_DOMAIN_ENUM = {
    "Food": "FoodIcon",
    "Home": "HomeIcon",
    "Body": "BodyIcon",
    "Activity": "ActivityIcon",
    "Nature": "NatureIcon",
    "Misc": "MiscIcon",
}

FAE_DOMAIN_CATEGORY_ENUM = {
    "Food": "FoodIconCategory",
    "Home": "HomeIconCategory",
    "Body": "BodyIconCategory",
    "Activity": "ActivityIconCategory",
    "Nature": "NatureIconCategory",
    "Misc": "MiscIconCategory",
}

FOOD = {
    "apple_fruit",
    "banana",
    "bread",
    "carot",
    "cheese",
    "cherry",
    "chicken_thigh",
    "chilli",
    "coffe_beans",
    "cup_coffe",
    "donut",
    "hamburger",
    "hotdog",
    "ice_cream",
    "lollipop",
    "meat",
    "milk_bottle",
    "mushroom",
    "orange",
    "peach",
    "pear",
    "pizza",
    "popcorn",
    "popsicle",
    "soda",
    "soup",
    "sushi",
    "tacos",
}

HOME = {
    "bath",
    "bed",
    "comb",
    "crown",
    "dice",
    "disco",
    "envelope_open",
    "envelope_open_o",
    "galery",
    "gift_card",
    "glass",
    "hat",
    "loyalty_card",
    "refrigerator",
    "ring",
    "ruler",
    "sofa",
    "thin_close",
    "toilet",
    "tools",
    "triangle_ruler",
    "umbrella",
    "wallet",
}

BODY = {
    "bones",
    "brain",
    "dress",
    "drop",
    "fingerprint",
    "footprint",
    "gut",
    "halter",
    "high_heel",
    "injection",
    "lips",
    "lipstick",
    "liver",
    "lung",
    "makeup_brushes",
    "medicine",
    "mustache",
    "pulse",
    "real_heart",
    "shirt",
    "spermatozoon",
    "stomach",
    "thermometer",
    "thermometer_high",
    "thermometer_low",
    "tooth",
    "uterus",
    "virus",
}

ACTIVITY = {
    "chess_bishop",
    "chess_horse",
    "chess_king",
    "chess_pawn",
    "chess_queen",
    "chess_tower",
    "cicling",
    "guitar",
    "nintendo",
    "playstation",
    "spin_double",
    "walking",
    "xbox",
}

NATURE = {
    "bacteria",
    "biohazard",
    "butterfly",
    "cloud",
    "cockroach",
    "comet",
    "dna",
    "galaxy",
    "microscope",
    "molecule",
    "moon_cloud",
    "mountains",
    "planet",
    "plant",
    "poison",
    "radioactive",
    "raining",
    "snowing",
    "storm",
    "sun_cloud",
    "telescope",
    "tree",
    "wind",
}

FOOD_CATEGORY_ORDER = ["Food"]
HOME_CATEGORY_ORDER = ["Home"]
BODY_CATEGORY_ORDER = ["Body"]
ACTIVITY_CATEGORY_ORDER = ["Activity"]
NATURE_CATEGORY_ORDER = ["Nature"]
MISC_CATEGORY_ORDER = ["Misc"]


def fae_domain_for(suffix: str) -> str:
    if suffix in FOOD:
        return "Food"
    if suffix in HOME:
        return "Home"
    if suffix in BODY:
        return "Body"
    if suffix in ACTIVITY:
        return "Activity"
    if suffix in NATURE:
        return "Nature"
    return "Misc"


def fae_category_for(domain: str, suffix: str) -> str:
    return {
        "Food": "Food",
        "Home": "Home",
        "Body": "Body",
        "Activity": "Activity",
        "Nature": "Nature",
        "Misc": "Misc",
    }[domain]


def fae_path(domain: str, suffix: str) -> str:
    return f"{FAE_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"


def fae_ascii(domain: str, suffix: str) -> str:
    return {
        "Food": "F",
        "Home": "H",
        "Body": "B",
        "Activity": "A",
        "Nature": "N",
        "Misc": ".",
    }[domain]


def fae_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = fae_domain_for(suffix)
    category = fae_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": fae_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": fae_ascii(domain, suffix),
        "source_set": "fae",
    }


def fae_category_order_for(domain: str) -> list[str]:
    return {
        "Food": FOOD_CATEGORY_ORDER,
        "Home": HOME_CATEGORY_ORDER,
        "Body": BODY_CATEGORY_ORDER,
        "Activity": ACTIVITY_CATEGORY_ORDER,
        "Nature": NATURE_CATEGORY_ORDER,
        "Misc": MISC_CATEGORY_ORDER,
    }.get(domain, [])


def fae_category_path_for(domain: str, category: str) -> str:
    return FAE_DOMAIN_PATH_PREFIX[domain]


FAE_REGISTRY = {
    "order": FAE_DOMAIN_ORDER,
    "path_prefix": FAE_DOMAIN_PATH_PREFIX,
    "cpp_stem": FAE_DOMAIN_CPP_STEM,
    "enum": FAE_DOMAIN_ENUM,
    "category_enum": FAE_DOMAIN_CATEGORY_ENUM,
    "category_order": fae_category_order_for,
    "category_path": fae_category_path_for,
}
