#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts Font Awesome (i_fa.sh, v3.4.0)."""

from __future__ import annotations

from nerd_font_lib import kebab_case, pascal_case

FA_DOMAIN_ORDER = [
    "Navigation",
    "Shape",
    "Sign",
    "Emoji",
    "People",
    "Communication",
    "Media",
    "Music",
    "Device",
    "Transport",
    "Finance",
    "Health",
    "Education",
    "Game",
    "Sport",
    "Office",
    "Shopping",
    "Clothing",
    "Animal",
    "Building",
    "Science",
    "Religion",
    "FaWeather",
    "Geo",
    "SocialBrand",
    "Tool",
    "FaMisc",
]

FA_DOMAIN_PATH_PREFIX = {
    "Navigation": "fa-navigation",
    "Shape": "fa-shape",
    "Sign": "fa-sign",
    "Emoji": "fa-emoji",
    "People": "fa-people",
    "Communication": "fa-communication",
    "Media": "fa-media",
    "Music": "fa-music",
    "Device": "fa-device",
    "Transport": "fa-transport",
    "Finance": "fa-finance",
    "Health": "fa-health",
    "Education": "fa-education",
    "Game": "fa-game",
    "Sport": "fa-sport",
    "Office": "fa-office",
    "Shopping": "fa-shopping",
    "Clothing": "fa-clothing",
    "Animal": "fa-animal",
    "Building": "fa-building",
    "Science": "fa-science",
    "Religion": "fa-religion",
    "FaWeather": "fa-weather",
    "Geo": "fa-geo",
    "SocialBrand": "fa-social-brand",
    "Tool": "fa-tool",
    "FaMisc": "fa-misc",
}

FA_DOMAIN_CPP_STEM = {
    "Navigation": "fa_navigation_icon",
    "Shape": "fa_shape_icon",
    "Sign": "fa_sign_icon",
    "Emoji": "fa_emoji_icon",
    "People": "fa_people_icon",
    "Communication": "fa_communication_icon",
    "Media": "fa_media_icon",
    "Music": "fa_music_icon",
    "Device": "fa_device_icon",
    "Transport": "fa_transport_icon",
    "Finance": "fa_finance_icon",
    "Health": "fa_health_icon",
    "Education": "fa_education_icon",
    "Game": "fa_game_icon",
    "Sport": "fa_sport_icon",
    "Office": "fa_office_icon",
    "Shopping": "fa_shopping_icon",
    "Clothing": "fa_clothing_icon",
    "Animal": "fa_animal_icon",
    "Building": "fa_building_icon",
    "Science": "fa_science_icon",
    "Religion": "fa_religion_icon",
    "FaWeather": "fa_weather_icon",
    "Geo": "fa_geo_icon",
    "SocialBrand": "fa_social_brand_icon",
    "Tool": "fa_tool_icon",
    "FaMisc": "fa_misc_icon",
}

FA_DOMAIN_ENUM = {
    "Navigation": "FaNavigationIcon",
    "Shape": "FaShapeIcon",
    "Sign": "FaSignIcon",
    "Emoji": "FaEmojiIcon",
    "People": "FaPeopleIcon",
    "Communication": "FaCommunicationIcon",
    "Media": "FaMediaIcon",
    "Music": "FaMusicIcon",
    "Device": "FaDeviceIcon",
    "Transport": "FaTransportIcon",
    "Finance": "FaFinanceIcon",
    "Health": "FaHealthIcon",
    "Education": "FaEducationIcon",
    "Game": "FaGameIcon",
    "Sport": "FaSportIcon",
    "Office": "FaOfficeIcon",
    "Shopping": "FaShoppingIcon",
    "Clothing": "FaClothingIcon",
    "Animal": "FaAnimalIcon",
    "Building": "FaBuildingIcon",
    "Science": "FaScienceIcon",
    "Religion": "FaReligionIcon",
    "FaWeather": "FaWeatherIcon",
    "Geo": "FaGeoIcon",
    "SocialBrand": "FaSocialBrandIcon",
    "Tool": "FaToolIcon",
    "FaMisc": "FaMiscIcon",
}

FA_DOMAIN_CATEGORY_ENUM = {
    "Navigation": "FaNavigationIconCategory",
    "Shape": "FaShapeIconCategory",
    "Sign": "FaSignIconCategory",
    "Emoji": "FaEmojiIconCategory",
    "People": "FaPeopleIconCategory",
    "Communication": "FaCommunicationIconCategory",
    "Media": "FaMediaIconCategory",
    "Music": "FaMusicIconCategory",
    "Device": "FaDeviceIconCategory",
    "Transport": "FaTransportIconCategory",
    "Finance": "FaFinanceIconCategory",
    "Health": "FaHealthIconCategory",
    "Education": "FaEducationIconCategory",
    "Game": "FaGameIconCategory",
    "Sport": "FaSportIconCategory",
    "Office": "FaOfficeIconCategory",
    "Shopping": "FaShoppingIconCategory",
    "Clothing": "FaClothingIconCategory",
    "Animal": "FaAnimalIconCategory",
    "Building": "FaBuildingIconCategory",
    "Science": "FaScienceIconCategory",
    "Religion": "FaReligionIconCategory",
    "FaWeather": "FaWeatherIconCategory",
    "Geo": "FaGeoIconCategory",
    "SocialBrand": "FaSocialBrandIconCategory",
    "Tool": "FaToolIconCategory",
    "FaMisc": "FaMiscIconCategory",
}

# First-segment prefixes checked in priority order (SocialBrand handled separately).
FA_PREFIX_RULES: list[tuple[str, frozenset[str]]] = [
    (
        "Emoji",
        frozenset(
            {
                "face",
                "smile",
                "grin",
                "laugh",
                "kiss",
                "sad",
                "angry",
                "meh",
                "frown",
                "dizzy",
                "flushed",
                "grimace",
                "rolling",
                "tired",
                "surprise",
                "poo",
                "poop",
            }
        ),
    ),
    (
        "Sign",
        frozenset(
            {
                "asterisk",
                "ban",
                "check",
                "info",
                "question",
                "minus",
                "plus",
                "slash",
                "xmark",
                "greater",
                "not",
                "ok",
                "remove",
                "registered",
                "trademark",
                "recycle",
                "universal",
                "braille",
                "border",
                "rectangle",
                "object",
                "vector",
                "layer",
                "spell",
                "infinity",
                "thumbtack",
                "wave",
                "bullseye",
            }
        ),
    ),
    (
        "Navigation",
        frozenset(
            {
                "arrow",
                "angle",
                "angles",
                "chevron",
                "caret",
                "location",
                "compass",
                "route",
                "signs",
                "street",
                "expand",
                "compress",
                "maximize",
                "minimize",
                "arrows",
                "turn",
                "share",
                "reply",
                "forward",
                "backward",
                "step",
                "skip",
                "fast",
                "undo",
                "redo",
                "rotate",
                "sync",
                "refresh",
                "repeat",
                "shuffle",
                "random",
                "sort",
                "filter",
                "search",
                "magnifying",
                "ellipsis",
                "grip",
                "bars",
                "th",
                "up",
                "down",
                "left",
                "right",
                "list",
                "delete",
                "window",
            }
        ),
    ),
    (
        "Shape",
        frozenset(
            {
                "square",
                "circle",
                "triangle",
                "star",
                "diamond",
                "dot",
                "ring",
                "shapes",
                "cube",
                "cubes",
                "hexagon",
                "octagon",
            }
        ),
    ),
    (
        "People",
        frozenset(
            {
                "user",
                "users",
                "person",
                "people",
                "child",
                "baby",
                "hand",
                "hands",
                "handshake",
                "wheelchair",
                "blind",
                "deaf",
                "gender",
                "mars",
                "venus",
                "transgender",
                "neuter",
                "mercury",
                "group",
                "head",
                "body",
                "mask",
                "id",
                "ear",
                "thumbs",
            }
        ),
    ),
    (
        "Communication",
        frozenset(
            {
                "envelope",
                "envelopes",
                "mail",
                "comment",
                "comments",
                "message",
                "sms",
                "inbox",
                "paper",
                "phone",
                "fax",
                "tty",
                "rss",
                "bullhorn",
                "megaphone",
                "microphone",
                "headphones",
                "headset",
                "volume",
                "bell",
                "hashtag",
                "quote",
                "language",
                "translate",
                "keyboard",
                "blog",
                "newspaper",
                "pen",
                "pencil",
                "eraser",
                "feather",
                "signature",
                "voicemail",
                "tower",
                "broadcast",
                "walkie",
                "satellite",
                "podcast",
                "radio",
                "bluetooth",
                "nfc",
                "signal",
                "wifi",
                "ethernet",
                "network",
                "globe",
                "at",
                "pager",
            }
        ),
    ),
    (
        "Media",
        frozenset(
            {
                "play",
                "pause",
                "stop",
                "eject",
                "film",
                "video",
                "camera",
                "photo",
                "image",
                "images",
                "picture",
                "clapperboard",
                "tv",
                "display",
                "music",
                "note",
                "notes",
                "guitar",
                "drum",
                "compact",
                "disc",
                "record",
                "vinyl",
                "cassette",
                "closed",
                "palette",
                "paint",
                "brush",
                "fill",
                "droplet",
                "eye",
                "crop",
                "scissors",
                "clone",
                "copy",
                "paste",
                "cut",
                "font",
                "bold",
                "italic",
                "underline",
                "strikethrough",
                "subscript",
                "superscript",
                "align",
                "indent",
                "outdent",
                "paragraph",
                "heading",
                "text",
                "link",
                "unlink",
                "paperclip",
                "attach",
                "icons",
                "wand",
                "magic",
            }
        ),
    ),
    (
        "Music",
        frozenset(
            {
                "masks",
                "audio",
            }
        ),
    ),
    (
        "Device",
        frozenset(
            {
                "mobile",
                "tablet",
                "laptop",
                "desktop",
                "computer",
                "mouse",
                "hard",
                "drive",
                "hdd",
                "ssd",
                "server",
                "database",
                "memory",
                "microchip",
                "cpu",
                "sim",
                "sd",
                "usb",
                "plug",
                "battery",
                "power",
                "charging",
                "solar",
                "fan",
                "lightbulb",
                "lamp",
                "toggle",
                "switch",
                "slider",
                "dial",
                "gauge",
                "meter",
                "print",
                "printer",
                "scanner",
                "router",
                "antenna",
                "upload",
                "download",
                "outbox",
                "archive",
                "floppy",
                "vr",
            }
        ),
    ),
    (
        "Transport",
        frozenset(
            {
                "car",
                "truck",
                "bus",
                "taxi",
                "plane",
                "helicopter",
                "rocket",
                "ship",
                "boat",
                "anchor",
                "bicycle",
                "motorcycle",
                "train",
                "subway",
                "tram",
                "ferry",
                "shuttle",
                "van",
                "ambulance",
                "road",
                "highway",
                "bridge",
                "gas",
                "parking",
                "traffic",
                "sailboat",
                "jet",
                "fighter",
                "caravan",
                "trailer",
                "sleigh",
                "cable",
                "tractor",
            }
        ),
    ),
    (
        "Finance",
        frozenset(
            {
                "money",
                "dollar",
                "yen",
                "euro",
                "pound",
                "rupee",
                "ruble",
                "won",
                "shekel",
                "cent",
                "coins",
                "coin",
                "piggy",
                "bank",
                "credit",
                "card",
                "chart",
                "graph",
                "receipt",
                "invoice",
                "cash",
                "register",
                "percent",
                "calculator",
                "sack",
                "wallet",
                "bill",
                "lira",
                "hryvnia",
                "sterling",
                "tenge",
            }
        ),
    ),
    (
        "Health",
        frozenset(
            {
                "heartbeat",
                "hospital",
                "medkit",
                "pills",
                "prescription",
                "syringe",
                "stethoscope",
                "thermometer",
                "xray",
                "dna",
                "virus",
                "bacteria",
                "disease",
                "bandage",
                "crutch",
                "accessibility",
                "accessible",
                "lungs",
                "brain",
                "bone",
                "tooth",
                "teeth",
                "kit",
                "medical",
                "staff",
                "snake",
                "capsules",
                "tablets",
                "vial",
                "vials",
                "bed",
                "shower",
                "spa",
                "bath",
                "mortar",
            }
        ),
    ),
    (
        "Education",
        frozenset(
            {
                "graduation",
                "school",
                "chalkboard",
                "student",
                "teacher",
                "university",
                "marker",
                "highlighter",
                "award",
            }
        ),
    ),
    (
        "Game",
        frozenset(
            {
                "gamepad",
                "dice",
                "puzzle",
                "chess",
            }
        ),
    ),
    (
        "Sport",
        frozenset(
            {
                "baseball",
                "basketball",
                "football",
                "futbol",
                "golf",
                "hockey",
                "volleyball",
                "bowling",
                "dumbbell",
                "swimmer",
                "tennis",
                "life",
            }
        ),
    ),
    (
        "Office",
        frozenset(
            {
                "briefcase",
                "calendar",
                "clipboard",
                "clock",
                "hourglass",
                "stopwatch",
                "file",
                "folder",
                "table",
                "stamp",
                "passport",
                "sitemap",
                "diagram",
                "industry",
                "gavel",
                "business",
                "sticky",
                "tachograph",
                "trash",
                "suitcase",
                "certificate",
                "ticket",
                "book",
                "scroll",
                "crown",
                "chair",
            }
        ),
    ),
    (
        "Shopping",
        frozenset(
            {
                "shop",
                "store",
                "cart",
                "bag",
                "basket",
                "gem",
                "warehouse",
            }
        ),
    ),
    (
        "Clothing",
        frozenset(
            {
                "shirt",
                "shoe",
                "socks",
                "hat",
                "tie",
                "glasses",
                "helmet",
                "vest",
                "swatchbook",
            }
        ),
    ),
    (
        "Animal",
        frozenset(
            {
                "bug",
                "spider",
                "fish",
                "frog",
                "horse",
                "dog",
                "cat",
                "crow",
                "dove",
                "kiwi",
                "otter",
                "paw",
                "hippo",
                "locust",
                "mosquito",
                "worm",
                "dragon",
                "shrimp",
                "dinosaur",
                "bird",
                "duck",
                "feather",
                "drumstick",
                "turkey",
                "otter",
            }
        ),
    ),
    (
        "Building",
        frozenset(
            {
                "building",
                "house",
                "home",
                "hotel",
                "city",
                "landmark",
                "monument",
                "campground",
                "tent",
                "archway",
                "door",
                "torii",
                "church",
                "synagogue",
                "vihara",
                "factory",
            }
        ),
    ),
    (
        "Science",
        frozenset(
            {
                "flask",
                "microscope",
                "robot",
                "bezier",
                "radiation",
                "biohazard",
                "atom",
                "draw",
                "oil",
                "binoculars",
                "splotch",
                "spray",
            }
        ),
    ),
    (
        "Religion",
        frozenset(
            {
                "bahai",
                "khanda",
                "menorah",
                "om",
                "peace",
                "yin",
                "holly",
                "worship",
                "spaghetti",
                "place",
            }
        ),
    ),
    (
        "FaWeather",
        frozenset(
            {
                "cloud",
                "sun",
                "moon",
                "rain",
                "snow",
                "wind",
                "tornado",
                "umbrella",
                "temperature",
                "smog",
                "rainbow",
                "meteor",
                "volcano",
                "fire",
                "snowflake",
                "snowman",
                "dumpster",
            }
        ),
    ),
    (
        "Geo",
        frozenset(
            {
                "earth",
                "map",
                "mountain",
                "tree",
                "leaf",
                "water",
                "seedling",
                "plant",
                "flower",
                "clover",
                "island",
            }
        ),
    ),
    (
        "Tool",
        frozenset(
            {
                "wrench",
                "screwdriver",
                "hammer",
                "toolbox",
                "tools",
                "gear",
                "gears",
                "cog",
                "cogs",
                "magnet",
                "ruler",
                "abacus",
                "scale",
                "weight",
                "balance",
                "broom",
                "lock",
                "unlock",
                "key",
                "shield",
                "code",
                "terminal",
                "command",
                "fingerprint",
                "qrcode",
                "barcode",
                "tag",
                "tags",
                "flag",
                "bookmark",
                "gift",
                "cake",
                "birthday",
                "party",
                "confetti",
                "sparkles",
                "ghost",
                "skull",
                "crossbones",
                "bomb",
                "bolt",
                "box",
                "pallet",
                "dolly",
                "blender",
                "tape",
                "wine",
                "couch",
                "parachute",
                "smoking",
                "medal",
                "ribbon",
                "trophy",
                "sign",
                "hot",
            }
        ),
    ),
]

FA_SOCIAL_BRAND_PREFIXES = frozenset(
    {
        "500px",
        "accusoft",
        "adn",
        "adversal",
        "affiliatetheme",
        "airbnb",
        "algolia",
        "alipay",
        "amazon",
        "amilia",
        "android",
        "angellist",
        "angrycreative",
        "angular",
        "apper",
        "apple",
        "artstation",
        "asymmetrik",
        "atlassian",
        "audible",
        "autoprefixer",
        "avianex",
        "aviato",
        "aws",
        "bandcamp",
        "behance",
        "bimobject",
        "bitbucket",
        "bitcoin",
        "bity",
        "blackberry",
        "blogger",
        "bluesky",
        "bootstrap",
        "buromobelexperte",
        "buysellads",
        "canadian",
        "centercode",
        "centos",
        "chrome",
        "chromecast",
        "cloudflare",
        "cloudscale",
        "cloudsmith",
        "cloudversify",
        "codepen",
        "codiepie",
        "confluence",
        "connectdevelop",
        "contao",
        "cotton",
        "cpanel",
        "creative",
        "critical",
        "css3",
        "cuttlefish",
        "dailymotion",
        "dashcube",
        "deezer",
        "delicious",
        "deploydog",
        "deskpro",
        "dev",
        "deviantart",
        "dhl",
        "diaspora",
        "digg",
        "digital",
        "discord",
        "discourse",
        "dochub",
        "docker",
        "draft2digital",
        "dribbble",
        "dropbox",
        "drupal",
        "dyalog",
        "earlybirds",
        "ebay",
        "edge",
        "elementor",
        "ello",
        "ember",
        "empire",
        "envira",
        "erlang",
        "ethereum",
        "etsy",
        "evernote",
        "expeditedssl",
        "facebook",
        "fantasy",
        "fedex",
        "fedora",
        "figma",
        "firefox",
        "first",
        "flickr",
        "flipboard",
        "fly",
        "font",
        "fonticons",
        "fort",
        "forumbee",
        "foursquare",
        "free",
        "freebsd",
        "fulcrum",
        "galactic",
        "get",
        "gg",
        "git",
        "github",
        "gitkraken",
        "gitlab",
        "gitter",
        "glide",
        "gofore",
        "golang",
        "goodreads",
        "google",
        "gratipay",
        "grav",
        "gripfire",
        "grunt",
        "guilded",
        "gulp",
        "hacker",
        "hackerrank",
        "hashnode",
        "hips",
        "hire",
        "hive",
        "hooli",
        "hornbill",
        "hotjar",
        "houzz",
        "html5",
        "hubspot",
        "ideal",
        "imdb",
        "instagram",
        "instalod",
        "intercom",
        "internet",
        "invision",
        "ioxhost",
        "itch",
        "itunes",
        "java",
        "jedi",
        "jenkins",
        "jira",
        "joget",
        "joomla",
        "js",
        "jsfiddle",
        "jxl",
        "kaggle",
        "keybase",
        "kickstarter",
        "korvue",
        "laravel",
        "lastfm",
        "leanpub",
        "less",
        "letterboxd",
        "line",
        "linkedin",
        "linode",
        "linux",
        "lyft",
        "magento",
        "mailchimp",
        "mandalorian",
        "mastodon",
        "maxcdn",
        "mdb",
        "medapps",
        "medium",
        "medrt",
        "meetup",
        "megaport",
        "mendeley",
        "meta",
        "microblog",
        "microsoft",
        "mix",
        "mixcloud",
        "mixer",
        "mizuni",
        "modx",
        "monero",
        "napster",
        "neos",
        "nimblr",
        "node",
        "npm",
        "ns8",
        "nutritionix",
        "octopus",
        "odnoklassniki",
        "odysee",
        "old",
        "opencart",
        "openid",
        "opera",
        "optin",
        "orcid",
        "osi",
        "padlet",
        "page4",
        "pagelines",
        "palfed",
        "patreon",
        "paypal",
        "perbyte",
        "periscope",
        "phabricator",
        "phoenix",
        "php",
        "pied",
        "pinterest",
        "pix",
        "pixiv",
        "playstation",
        "product",
        "pushed",
        "python",
        "qq",
        "quinscape",
        "quora",
        "r",
        "raspberry",
        "ravelry",
        "react",
        "reacteurope",
        "readme",
        "rebel",
        "red",
        "reddit",
        "redhat",
        "renren",
        "replyd",
        "researchgate",
        "resolving",
        "rev",
        "rocketchat",
        "rockrms",
        "rust",
        "safari",
        "salesforce",
        "sass",
        "schlix",
        "scribd",
        "searchengin",
        "sellcast",
        "sellsy",
        "servicestack",
        "shirtsinbulk",
        "shopify",
        "shopware",
        "simplybuilt",
        "sistrix",
        "sith",
        "sitrox",
        "sketch",
        "skyatlas",
        "skype",
        "slack",
        "slideshare",
        "snapchat",
        "soundcloud",
        "sourcetree",
        "space",
        "speakap",
        "speaker",
        "spotify",
        "square",
        "squarespace",
        "stack",
        "staylinked",
        "steam",
        "sticker",
        "strava",
        "stripe",
        "stubber",
        "studiovinari",
        "stumbleupon",
        "superpowers",
        "supple",
        "suse",
        "swift",
        "symfony",
        "teamspeak",
        "telegram",
        "tencent",
        "the",
        "themeco",
        "themeisle",
        "think",
        "threads",
        "tiktok",
        "trade",
        "trello",
        "tumblr",
        "twitch",
        "twitter",
        "typo3",
        "uber",
        "ubuntu",
        "uikit",
        "umbraco",
        "uncharted",
        "uniregistry",
        "unity",
        "unsplash",
        "untappd",
        "ups",
        "upwork",
        "usps",
        "ussunnah",
        "vaadin",
        "viacoin",
        "viadeo",
        "viber",
        "vimeo",
        "vine",
        "vk",
        "vnv",
        "vuejs",
        "watchman",
        "waze",
        "weebly",
        "weibo",
        "weixin",
        "whatsapp",
        "whmcs",
        "wikipedia",
        "windows",
        "wirsindhandwerk",
        "wix",
        "wizards",
        "wodu",
        "wolf",
        "wordpress",
        "wpbeginner",
        "wpexplorer",
        "wpforms",
        "wpressr",
        "x",
        "xbox",
        "xing",
        "y",
        "yahoo",
        "yammer",
        "yandex",
        "yarn",
        "yelp",
        "yoast",
        "youtube",
        "zhihu",
        "cc",
        "nfc",
        "fab",
        "far",
        "fas",
        "fal",
        "btc",
        "buffer",
        "buromobelexperte",
    }
)

FA_SOCIAL_SQUARE_MARKERS = (
    "facebook",
    "twitter",
    "instagram",
    "github",
    "gitlab",
    "reddit",
    "youtube",
    "whatsapp",
    "snapchat",
    "pinterest",
    "dribbble",
    "behance",
    "vimeo",
    "xing",
    "tumblr",
    "steam",
    "lastfm",
    "letterboxd",
    "odnoklassniki",
    "pied",
    "hacker",
    "google",
    "font",
    "web",
    "threads",
    "upwork",
    "viadeo",
    "x_twitter",
    "js",
)

FA_NON_BRAND_SINGLE = frozenset(
    {
        "anchor",
        "asterisk",
        "at",
        "ban",
        "bars",
        "bed",
        "beer",
        "bell",
        "bolt",
        "bomb",
        "book",
        "bug",
        "bus",
        "car",
        "cat",
        "code",
        "cog",
        "copy",
        "crop",
        "cube",
        "cut",
        "dna",
        "dog",
        "dot",
        "edit",
        "eye",
        "fan",
        "fax",
        "fire",
        "fish",
        "flag",
        "flask",
        "frog",
        "gem",
        "gift",
        "globe",
        "golf",
        "graduation_cap",
        "hdd",
        "heart",
        "home",
        "horse",
        "hotel",
        "image",
        "inbox",
        "info",
        "key",
        "leaf",
        "link",
        "lock",
        "magnet",
        "map",
        "medal",
        "music",
        "paperclip",
        "paste",
        "paw",
        "pen",
        "phone",
        "plane",
        "plug",
        "plus",
        "print",
        "qrcode",
        "quote",
        "rss",
        "save",
        "seedling",
        "server",
        "shield",
        "ship",
        "shower",
        "snowflake",
        "spider",
        "star",
        "sun",
        "tag",
        "taxi",
        "th",
        "train",
        "trash",
        "tree",
        "truck",
        "tv",
        "umbrella",
        "unlock",
        "upload",
        "user",
        "video",
        "wifi",
        "wrench",
        "xmark",
        "baby",
        "bacon",
        "bahai",
        "bandage",
        "barcode",
        "baseball",
        "basketball",
        "bath",
        "bicycle",
        "binoculars",
        "biohazard",
        "blender",
        "bold",
        "bone",
        "bong",
        "bookmark",
        "box",
        "braille",
        "brain",
        "briefcase",
        "broom",
        "brush",
        "building",
        "bullhorn",
        "bullseye",
        "burger",
        "calculator",
        "calendar",
        "camera",
        "campground",
        "candy",
        "cannabis",
        "capsules",
        "caravan",
        "caret",
        "cart",
        "certificate",
        "chair",
        "chart",
        "check",
        "chess",
        "child",
        "church",
        "circle",
        "city",
        "clipboard",
        "clock",
        "cloud",
        "cocktail",
        "coffee",
        "cog",
        "coins",
        "comment",
        "compact",
        "compass",
        "compress",
        "cookie",
        "couch",
        "credit",
        "crop",
        "crown",
        "cube",
        "database",
        "desktop",
        "dice",
        "display",
        "dollar",
        "dolly",
        "download",
        "dumbbell",
        "envelope",
        "euro",
        "expand",
        "explosion",
        "eye",
        "face",
        "feather",
        "file",
        "filter",
        "fire",
        "fish",
        "flask",
        "folder",
        "football",
        "forward",
        "frown",
        "futbol",
        "gamepad",
        "gauge",
        "gavel",
        "gear",
        "ghost",
        "gift",
        "glasses",
        "globe",
        "golf",
        "graduation_cap",
        "grip",
        "guitar",
        "hammer",
        "hand",
        "hashtag",
        "headphones",
        "heart",
        "helicopter",
        "helmet",
        "highlighter",
        "hospital",
        "hourglass",
        "house",
        "ice",
        "icons",
        "id",
        "image",
        "industry",
        "infinity",
        "info",
        "italic",
        "jet",
        "joint",
        "key",
        "keyboard",
        "khanda",
        "kiss",
        "kit",
        "landmark",
        "language",
        "laptop",
        "laugh",
        "leaf",
        "lemon",
        "life",
        "lightbulb",
        "link",
        "list",
        "location",
        "lock",
        "lungs",
        "magic",
        "magnet",
        "map",
        "marker",
        "mars",
        "mask",
        "medal",
        "meh",
        "memory",
        "menorah",
        "mercury",
        "meteor",
        "microchip",
        "microphone",
        "microscope",
        "minus",
        "mobile",
        "money",
        "monument",
        "moon",
        "mortar",
        "motorcycle",
        "mountain",
        "mug",
        "music",
        "network",
        "newspaper",
        "notdef",
        "note",
        "object",
        "om",
        "otter",
        "outdent",
        "pager",
        "paint",
        "palette",
        "paperclip",
        "paragraph",
        "parking",
        "passport",
        "paste",
        "pause",
        "paw",
        "peace",
        "pen",
        "pencil",
        "people",
        "pepper",
        "percent",
        "person",
        "phone",
        "photo",
        "pills",
        "plane",
        "play",
        "plug",
        "plus",
        "podcast",
        "poop",
        "power",
        "prescription",
        "print",
        "pump",
        "puzzle",
        "qrcode",
        "question",
        "quote",
        "radiation",
        "radio",
        "rainbow",
        "receipt",
        "record",
        "recycle",
        "registered",
        "remove",
        "repeat",
        "reply",
        "ribbon",
        "right",
        "ring",
        "road",
        "robot",
        "rocket",
        "rotate",
        "route",
        "rss",
        "ruble",
        "rug",
        "ruler",
        "rupee",
        "sack",
        "sad",
        "sailboat",
        "satellite",
        "save",
        "scale",
        "school",
        "scissors",
        "screwdriver",
        "scroll",
        "sd",
        "search",
        "seedling",
        "server",
        "shapes",
        "share",
        "sheet",
        "shield",
        "ship",
        "shirt",
        "shoe",
        "shop",
        "shower",
        "shuffle",
        "shuttle",
        "sign",
        "signal",
        "signature",
        "sitemap",
        "skull",
        "slash",
        "sleigh",
        "smog",
        "smoking",
        "snowflake",
        "snowman",
        "soap",
        "socks",
        "solar",
        "sort",
        "spa",
        "space",
        "spaghetti",
        "spell",
        "spider",
        "spinner",
        "splotch",
        "spoon",
        "spray",
        "square",
        "stamp",
        "star",
        "stop",
        "stopwatch",
        "store",
        "stream",
        "street",
        "strikethrough",
        "stroopwafel",
        "subscript",
        "subway",
        "suitcase",
        "sun",
        "superscript",
        "surprise",
        "swatchbook",
        "swimmer",
        "synagogue",
        "sync",
        "syringe",
        "table",
        "tablet",
        "tachometer",
        "tag",
        "tags",
        "tape",
        "taxi",
        "teeth",
        "temperature",
        "tenge",
        "terminal",
        "text",
        "thermometer",
        "thumbs",
        "thumbtack",
        "ticket",
        "timeline",
        "toggle",
        "toilet",
        "toolbox",
        "tooth",
        "tornado",
        "tower",
        "tractor",
        "trademark",
        "traffic",
        "trailer",
        "train",
        "tram",
        "trash",
        "tree",
        "triangle",
        "trophy",
        "truck",
        "tty",
        "tugrik",
        "turkey",
        "turn",
        "tv",
        "umbrella",
        "underline",
        "undo",
        "universal",
        "unlock",
        "up",
        "upload",
        "user",
        "users",
        "utensils",
        "van",
        "vault",
        "venus",
        "vest",
        "vial",
        "video",
        "vihara",
        "virus",
        "voicemail",
        "volcano",
        "volleyball",
        "volume",
        "vr",
        "walkie",
        "wallet",
        "wand",
        "warehouse",
        "water",
        "wave",
        "weight",
        "wheelchair",
        "whiskey",
        "wifi",
        "wind",
        "window",
        "wine",
        "won",
        "worm",
        "wrench",
        "xmark",
        "xray",
        "yen",
        "yin",
        "zap",
    }
)


# Explicit suffix overrides (checked before prefix rules).
FA_SUFFIX_DOMAIN: dict[str, str] = {
  # Education
    "graduation_cap": "Education",
    "school": "Education",
    "chalkboard_user": "Education",
    # Game / roleplay
    "d_and_d": "Game",
    "d_and_d_beyond": "Game",
    "battle_net": "Game",
    "puzzle_piece": "Game",
    # Sport
    "broom_ball": "Sport",
    "table_tennis_paddle_ball": "Sport",
    "life_ring": "Sport",
    # Office
    "business_time": "Office",
    "diagram_project": "Office",
    "boxes_stacked": "Office",
    "i_cursor": "Sign",
    # Shopping
    "bag_shopping": "Shopping",
    "basket_shopping": "Shopping",
    "cart_arrow_down": "Shopping",
    "cart_flatbed": "Shopping",
    "cart_flatbed_suitcase": "Shopping",
    "cart_plus": "Shopping",
    "cart_shopping": "Shopping",
    # Clothing
    "black_tie": "Clothing",
    "hat_cowboy": "Clothing",
    "hat_cowboy_side": "Clothing",
    "hat_wizard": "Clothing",
    "helmet_safety": "Clothing",
    "masks_theater": "Music",
    # Religion
    "place_of_worship": "Religion",
    "scroll_torah": "Religion",
    "yin_yang": "Religion",
    "spaghetti_monster_flying": "Religion",
    "book_bible": "Religion",
    "book_quran": "Religion",
    "book_tanakh": "Religion",
    # Science
    "draw_polygon": "Science",
    "vector_square": "Science",
    "layer_group": "Science",
    # Building
    "torii_gate": "Building",
    # Weather
    "hot_tub_person": "Health",
    "umbrella_beach": "Geo",
    "fire_extinguisher": "Tool",
    "water_ladder": "Sport",
    # Finance currency signs
    "hryvnia_sign": "Finance",
    "lira_sign": "Finance",
    "sterling_sign": "Finance",
    "tenge_sign": "Finance",
    # Brands that slipped through
    "app_store": "SocialBrand",
    "app_store_ios": "SocialBrand",
    "buy_n_large": "SocialBrand",
    "heard_o": "SocialBrand",
    "spinner": "Sign",
}

# Substring heuristics applied when prefix rules miss (domain, markers).
FA_SUBSTRING_RULES: list[tuple[str, tuple[str, ...]]] = [
    ("Shopping", ("_shopping", "_shop", "_store", "cart_")),
    ("Office", ("calendar_", "hourglass", "clipboard_", "briefcase", "sticky_note", "tachograph", "trash_", "suitcase")),
    ("Education", ("_school", "_student", "_teacher", "_university", "graduation", "chalkboard")),
    ("Game", ("_dice", "gamepad", "puzzle_", "chess_", "d_and_d")),
    ("Sport", ("_ball", "dumbbell", "futbol", "swimmer", "hockey", "volleyball", "bowling", "golf", "tennis")),
    ("Clothing", ("_shirt", "_shoe", "_socks", "_hat", "_tie", "_helmet", "_vest", "swatchbook")),
    ("Animal", ("_bird", "_cat", "_dog", "_fish", "_frog", "_horse", "_spider", "_bug", "_paw", "_worm", "drumstick")),
    ("Building", ("_building", "_house", "_hotel", "_door", "campground", "monument", "landmark", "archway", "torii", "church", "synagogue", "vihara")),
    ("Science", ("microscope", "flask", "robot", "bezier", "radiation", "biohazard", "draw_polygon", "vector_", "infinity", "splotch", "spray_can")),
    ("Religion", ("_worship", "menorah", "synagogue", "torah", "yin_yang", "bahai", "khanda", "holly_berry", "spaghetti_monster", "book_bible", "book_quran", "book_tanakh")),
    ("FaWeather", ("cloud_", "temperature_", "snowflake", "snowman", "smog", "rainbow", "meteor", "umbrella", "tornado", "wind")),
    ("Sign", ("_sign", "check_", "border_", "rectangle_", "object_", "spell_check", "not_equal", "greater_than", "universal_access", "braille", "recycle", "trademark", "registered")),
    ("Music", ("audio_description", "masks_theater")),
    ("Health", ("_medical", "mortar_pestle", "bed_pulse", "hot_tub", "toilet", "shower", "spa", "bath")),
    ("Food", ("bacon", "burger", "bread_", "candy_", "cannabis", "champagne_", "cookie", "ice_cream", "joint", "lemon", "martini_", "mug_", "oil_can", "pepper_", "pizza_", "spoon", "stroopwafel", "utensils", "whiskey_", "beer_", "wine_")),
]

FA_CATEGORY_ORDER: dict[str, list[str]] = {
    "Sign": ["Status", "Math", "Border", "Layout", "Legal", "Access", "Misc"],
    "Education": ["School", "University", "Learning", "Book", "Award"],
    "Game": ["Board", "Dice", "Video", "Roleplay", "Puzzle"],
    "Sport": ["Ball", "Team", "Individual", "Equipment", "Water"],
    "Office": ["Calendar", "Clock", "Document", "File", "Folder", "Table", "Travel", "Legal", "Misc"],
    "Shopping": ["Store", "Cart", "Bag", "Product"],
    "Clothing": ["Head", "Body", "Foot", "Accessory", "Costume"],
    "Animal": ["Mammal", "Bird", "Fish", "Amphibian", "Insect", "Arachnid", "Generic"],
    "Building": ["Residential", "Commercial", "Institutional", "Religious", "Landmark", "Door"],
    "Science": ["Chemistry", "Biology", "Physics", "Math", "Technology"],
    "Religion": ["Symbol", "Place", "Text", "Holiday"],
    "FaWeather": ["Sun", "Moon", "Cloud", "Precipitation", "Temperature", "Wind", "Storm", "Other"],
    "Music": ["Performance", "Audio"],
    "Navigation": ["Arrow", "Chevron", "Direction", "Window", "Control", "Misc"],
    "People": ["User", "Group", "Body", "Accessibility", "Gender", "Misc"],
    "Communication": ["Mail", "Phone", "Audio", "Network", "Writing", "Misc"],
    "Media": ["Playback", "Visual", "Audio", "Text", "Edit", "Misc"],
    "Device": ["Computer", "Mobile", "Storage", "Power", "Peripheral", "Misc"],
    "Transport": ["Road", "Air", "Water", "Rail", "Misc"],
    "Finance": ["Currency", "Payment", "Chart", "Misc"],
    "Health": ["Medical", "Body", "Facility", "Wellness", "Misc"],
    "Geo": ["Earth", "Map", "Nature", "Water", "Misc"],
    "Tool": ["Hardware", "Security", "Code", "Award", "Hazard", "Household", "Misc"],
    "Emoji": ["Face", "Emotion", "Misc"],
    "Shape": ["Basic", "Polygon", "Misc"],
    "SocialBrand": ["Brand"],
    "FaMisc": ["Misc"],
}

FA_RELIGIOUS_BOOKS = frozenset({"book_bible", "book_quran", "book_tanakh", "book_journal_whills"})
FA_EDUCATION_BOOKS = frozenset(
    {
        "book",
        "book_open",
        "book_open_reader",
        "book_atlas",
        "book_medical",
        "book_skull",
    }
)


def fa_domain_for(suffix: str) -> str:
    first = suffix.split("_")[0]

    if first in FA_SOCIAL_BRAND_PREFIXES:
        return "SocialBrand"
    if suffix.startswith("cc_") or suffix.startswith("creative_commons"):
        return "SocialBrand"
    if suffix.startswith("square_") and any(marker in suffix for marker in FA_SOCIAL_SQUARE_MARKERS):
        return "SocialBrand"
    if "_" not in suffix and suffix not in FA_NON_BRAND_SINGLE:
        return "SocialBrand"

    if suffix in FA_SUFFIX_DOMAIN:
        return FA_SUFFIX_DOMAIN[suffix]

    if suffix.startswith("address_book") or suffix.startswith("address_card"):
        return "People"
    if first == "heart" and ("pulse" in suffix or suffix == "heart"):
        return "Health"
    if first == "heart" and suffix == "heart_crack":
        return "Emoji"
    if first == "box" and suffix in {"box_open", "boxes_stacked", "box_archive"}:
        return "Office" if suffix == "boxes_stacked" else "Tool"
    if first == "landmark":
        return "Finance" if suffix == "landmark" else "Building"
    if suffix in FA_RELIGIOUS_BOOKS:
        return "Religion"
    if suffix in FA_EDUCATION_BOOKS or (first == "book" and suffix not in FA_RELIGIOUS_BOOKS):
        return "Education"
    if first == "broom" and suffix != "broom_ball":
        return "Tool"
    if first == "sign" and suffix == "sign_hanging":
        return "Tool"
    if first == "feather" and suffix == "feather":
        return "Communication"
    if first == "fire" and suffix.startswith("fire_flame"):
        return "FaWeather"
    if first == "water" and suffix == "water_ladder":
        return "Sport"

    for domain, prefixes in FA_PREFIX_RULES:
        if first in prefixes:
            return domain

    for domain, markers in FA_SUBSTRING_RULES:
        if domain == "Food":
            continue
        if any(marker in suffix for marker in markers):
            return domain

    for domain, markers in FA_SUBSTRING_RULES:
        if domain != "Food":
            continue
        if any(marker in suffix for marker in markers):
            return "FaMisc"

    return "FaMisc"


def fa_category_for(domain: str, suffix: str) -> str:
    first = suffix.split("_")[0]

    if domain == "Sign":
        if first in {"check", "xmark", "ban", "slash", "ok", "remove", "spinner"} or "check_" in suffix:
            return "Status"
        if first in {"greater", "not", "minus", "plus", "infinity"} or suffix == "not_equal":
            return "Math"
        if first == "border" or suffix.startswith("rectangle_"):
            return "Border"
        if first in {"object", "vector", "layer"} or suffix == "i_cursor":
            return "Layout"
        if first in {"registered", "trademark", "recycle"}:
            return "Legal"
        if first in {"universal", "braille"}:
            return "Access"
        return "Misc"

    if domain == "Education":
        if suffix in {"school", "graduation_cap"} or "school" in suffix:
            return "School"
        if "university" in suffix or "student" in suffix:
            return "University"
        if first == "book" or suffix == "chalkboard_user":
            return "Book"
        if first in {"marker", "highlighter"}:
            return "Learning"
        if first in {"award", "certificate"}:
            return "Award"
        return "Learning"

    if domain == "Game":
        if suffix.startswith("chess"):
            return "Board"
        if first == "dice" or "dice_" in suffix:
            return "Dice"
        if first == "gamepad":
            return "Video"
        if suffix in {"d_and_d", "d_and_d_beyond", "battle_net"}:
            return "Roleplay"
        if first == "puzzle":
            return "Puzzle"
        return "Misc"

    if domain == "Sport":
        if any(part in suffix for part in ("football", "baseball", "basketball", "volleyball", "bowling", "golf", "hockey", "futbol", "tennis")):
            return "Ball"
        if first == "dumbbell" or suffix == "broom_ball":
            return "Equipment"
        if first == "swimmer" or suffix == "water_ladder":
            return "Water"
        if suffix == "life_ring":
            return "Water"
        return "Team"

    if domain == "Office":
        if first == "calendar":
            return "Calendar"
        if first in {"clock", "hourglass", "stopwatch"}:
            return "Clock"
        if first in {"file", "scroll", "sticky"}:
            return "Document"
        if first == "file":
            return "File"
        if first == "folder":
            return "Folder"
        if first == "table":
            return "Table"
        if first in {"briefcase", "suitcase", "passport"}:
            return "Travel"
        if first in {"gavel", "stamp", "certificate"}:
            return "Legal"
        if first in {"clipboard", "diagram", "sitemap", "industry", "business", "tachograph", "trash", "crown"}:
            return "Misc"
        return "Misc"

    if domain == "Shopping":
        if first in {"shop", "store", "warehouse"}:
            return "Store"
        if first == "cart" or "cart_" in suffix:
            return "Cart"
        if first == "bag" or "bag_" in suffix or first == "basket":
            return "Bag"
        if first == "gem":
            return "Product"
        return "Store"

    if domain == "Clothing":
        if first == "hat" or suffix == "helmet_safety":
            return "Head"
        if first == "shirt" or suffix == "black_tie":
            return "Body"
        if first == "shoe":
            return "Foot"
        if first in {"glasses", "socks", "swatchbook"}:
            return "Accessory"
        if suffix == "masks_theater":
            return "Costume"
        return "Body"

    if domain == "Animal":
        if first in {"cat", "dog", "horse", "otter", "hippo", "paw"}:
            return "Mammal"
        if first in {"crow", "dove", "kiwi", "duck", "turkey"}:
            return "Bird"
        if first == "fish":
            return "Fish"
        if first == "frog":
            return "Amphibian"
        if first in {"bug", "locust", "mosquito", "worm"}:
            return "Insect"
        if first == "spider":
            return "Arachnid"
        return "Generic"

    if domain == "Building":
        if first in {"house", "home", "hotel", "campground", "tent"}:
            return "Residential"
        if first in {"building", "city", "industry", "warehouse", "store", "shop"}:
            return "Commercial"
        if first in {"landmark", "monument", "archway"}:
            return "Landmark"
        if first in {"church", "synagogue", "vihara"} or suffix in {"place_of_worship", "torii_gate"}:
            return "Religious"
        if first == "door":
            return "Door"
        return "Commercial"

    if domain == "Science":
        if first in {"flask", "vial"} or "mortar" in suffix:
            return "Chemistry"
        if first in {"microscope", "dna"} or suffix == "biohazard":
            return "Biology"
        if first in {"radiation", "atom"} or suffix == "infinity":
            return "Physics"
        if first in {"bezier", "draw", "vector"}:
            return "Math"
        if first == "robot":
            return "Technology"
        return "Technology"

    if domain == "Religion":
        if suffix in {"om", "yin_yang", "peace", "khanda", "bahai", "menorah"}:
            return "Symbol"
        if suffix in {"place_of_worship", "synagogue", "vihara", "torii_gate", "church"} or first == "church":
            return "Place"
        if suffix in FA_RELIGIOUS_BOOKS or suffix == "scroll_torah":
            return "Text"
        if suffix in {"holly_berry", "spaghetti_monster_flying"}:
            return "Holiday"
        return "Symbol"

    if domain == "FaWeather":
        if first == "sun":
            return "Sun"
        if first == "moon" or "cloud_moon" in suffix:
            return "Moon"
        if first == "cloud" or suffix.startswith("cloud_"):
            return "Cloud"
        if first in {"rain", "snow", "snowflake", "snowman"} or "rain" in suffix:
            return "Precipitation"
        if first == "temperature" or suffix.startswith("temperature_"):
            return "Temperature"
        if first == "wind" or suffix == "tornado":
            return "Wind"
        if first in {"fire", "volcano", "meteor", "dumpster"} or suffix == "smog":
            return "Storm"
        if first == "umbrella":
            return "Other"
        if suffix == "rainbow":
            return "Other"
        return "Other"

    if domain == "Music":
        if suffix == "masks_theater":
            return "Performance"
        return "Audio"

    if domain == "Navigation":
        if first in {"arrow", "arrows", "angle", "angles", "chevron", "caret"}:
            return "Arrow" if first in {"arrow", "arrows"} else "Chevron"
        if first in {"location", "compass", "route", "street", "signs"}:
            return "Direction"
        if first == "window":
            return "Window"
        if first in {"expand", "compress", "maximize", "minimize", "delete"}:
            return "Control"
        return "Misc"

    if domain == "People":
        if first in {"user", "users", "person", "people", "child", "baby"}:
            return "User" if first in {"user", "person"} else "Group"
        if first in {"hand", "hands", "head", "body", "thumbs"}:
            return "Body"
        if first in {"wheelchair", "blind", "deaf", "ear"}:
            return "Accessibility"
        if first in {"gender", "mars", "venus", "transgender", "neuter", "mercury"}:
            return "Gender"
        return "Misc"

    if domain == "Communication":
        if first in {"envelope", "envelopes", "mail", "inbox", "outbox"}:
            return "Mail"
        if first in {"phone", "fax", "tty", "pager", "walkie", "voicemail"}:
            return "Phone"
        if first in {"microphone", "headphones", "headset", "volume", "podcast", "radio"}:
            return "Audio"
        if first in {"wifi", "ethernet", "network", "signal", "satellite", "bluetooth", "nfc", "rss"}:
            return "Network"
        if first in {"pen", "pencil", "feather", "keyboard", "blog", "newspaper", "comment", "comments", "message", "sms"}:
            return "Writing"
        return "Misc"

    if domain == "Media":
        if first in {"play", "pause", "stop", "eject"}:
            return "Playback"
        if first in {"film", "video", "camera", "photo", "image", "images", "picture", "tv", "display", "clapperboard"}:
            return "Visual"
        if first in {"music", "note", "notes", "guitar", "drum", "compact", "disc", "record", "vinyl", "cassette"}:
            return "Audio"
        if first in {"font", "bold", "italic", "underline", "strikethrough", "subscript", "superscript", "align", "indent", "outdent", "paragraph", "heading", "text"}:
            return "Text"
        if first in {"crop", "scissors", "clone", "copy", "paste", "cut", "paint", "brush", "palette", "fill", "wand", "magic"}:
            return "Edit"
        return "Misc"

    if domain == "Device":
        if first in {"desktop", "laptop", "computer", "server", "database", "microchip", "cpu", "memory"}:
            return "Computer"
        if first in {"mobile", "tablet"}:
            return "Mobile"
        if first in {"hard", "drive", "hdd", "ssd", "sd", "sim", "floppy", "archive"}:
            return "Storage"
        if first in {"battery", "power", "charging", "solar", "plug"}:
            return "Power"
        if first in {"mouse", "print", "printer", "scanner", "router", "antenna", "usb", "vr"}:
            return "Peripheral"
        return "Misc"

    if domain == "Transport":
        if first in {"car", "truck", "bus", "taxi", "van", "motorcycle", "bicycle", "road", "highway", "parking", "traffic", "tractor", "cable"}:
            return "Road"
        if first in {"plane", "helicopter", "rocket", "jet", "fighter"}:
            return "Air"
        if first in {"ship", "boat", "anchor", "ferry", "sailboat"}:
            return "Water"
        if first in {"train", "subway", "tram"}:
            return "Rail"
        return "Misc"

    if domain == "Finance":
        if suffix.endswith("_sign") or first in {"dollar", "yen", "euro", "pound", "rupee", "ruble", "won", "shekel", "lira", "hryvnia", "sterling", "tenge"}:
            return "Currency"
        if first in {"credit", "card", "wallet", "cash", "coins", "coin", "money", "piggy", "sack", "bill"}:
            return "Payment"
        if first in {"chart", "graph", "receipt", "invoice", "percent", "calculator"}:
            return "Chart"
        return "Misc"

    if domain == "Health":
        if first in {"pills", "prescription", "syringe", "stethoscope", "medkit", "capsules", "tablets", "vial", "vials", "bandage", "crutch", "mortar"}:
            return "Medical"
        if first in {"brain", "bone", "tooth", "teeth", "lungs", "dna", "heartbeat"}:
            return "Body"
        if first == "hospital":
            return "Facility"
        if first in {"shower", "spa", "bath", "bed", "toilet"}:
            return "Wellness"
        return "Misc"

    if domain == "Geo":
        if first == "earth":
            return "Earth"
        if first == "map":
            return "Map"
        if first in {"tree", "leaf", "seedling", "plant", "flower", "clover", "mountain", "island"}:
            return "Nature"
        if first == "water":
            return "Water"
        return "Misc"

    if domain == "Tool":
        if first in {"wrench", "screwdriver", "hammer", "toolbox", "tools", "gear", "gears", "cog", "cogs", "magnet", "ruler", "tape", "blender"}:
            return "Hardware"
        if first in {"lock", "unlock", "key", "shield", "fingerprint"}:
            return "Security"
        if first in {"code", "terminal", "command"}:
            return "Code"
        if first in {"trophy", "medal", "ribbon", "certificate", "award"}:
            return "Award"
        if first in {"biohazard", "radiation", "bomb", "skull", "ghost"}:
            return "Hazard"
        if first in {"wine", "couch", "smoking", "gift", "cake", "party"}:
            return "Household"
        return "Misc"

    if domain == "Emoji":
        if first == "face" or "face_" in suffix:
            return "Face"
        return "Emotion"

    if domain == "Shape":
        if first in {"square", "circle", "triangle", "star", "diamond", "dot", "ring"}:
            return "Basic"
        if first in {"hexagon", "octagon", "cube", "cubes"}:
            return "Polygon"
        return "Misc"

    if domain == "SocialBrand":
        return "Brand"

    return pascal_case(first)


def fa_path(domain: str, suffix: str) -> str:
    return f"{FA_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"


def fa_ascii(suffix: str) -> str:
    first = suffix.split("_")[0]
    domain = fa_domain_for(suffix)
    return {
        "Navigation": "^",
        "Shape": "O",
        "Sign": "!",
        "People": "P",
        "Communication": "@",
        "Media": ">",
        "Music": "~",
        "Device": "D",
        "Transport": "T",
        "Finance": "$",
        "Health": "+",
        "Education": "E",
        "Game": "G",
        "Sport": "S",
        "Office": "F",
        "Shopping": "C",
        "Clothing": "W",
        "Animal": "A",
        "Building": "B",
        "Science": "R",
        "Religion": "&",
        "FaWeather": "W",
        "Geo": "M",
        "Emoji": ":",
        "SocialBrand": "b",
        "Tool": "*",
    }.get(domain, "." if first not in {"check", "xmark", "ban"} else ("+" if first == "check" else "x"))


def fa_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = fa_domain_for(suffix)
    category = fa_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": fa_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": fa_ascii(suffix),
        "source_set": "fa",
    }


def fa_category_order_for(domain: str) -> list[str]:
    return FA_CATEGORY_ORDER.get(domain, [])


def fa_category_path_for(domain: str, category: str) -> str:
    prefix = FA_DOMAIN_PATH_PREFIX[domain]
    if not category:
        return prefix
    return f"{prefix}-{kebab_case(category)}"


FA_REGISTRY = {
    "order": FA_DOMAIN_ORDER,
    "path_prefix": FA_DOMAIN_PATH_PREFIX,
    "cpp_stem": FA_DOMAIN_CPP_STEM,
    "enum": FA_DOMAIN_ENUM,
    "category_enum": FA_DOMAIN_CATEGORY_ENUM,
    "category_order": fa_category_order_for,
    "category_path": fa_category_path_for,
}
