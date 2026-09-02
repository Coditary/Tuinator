#!/usr/bin/env python3
"""Generate scripts/nerd_glyph_domains_md.py from MD icon data and routing rules."""

from __future__ import annotations

import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from nerd_font_lib import kebab_case, load_set, pascal_case
from _md_routing_data import BRAND_FIRST, CROSS_OVERRIDES, MD_COMPOUND_RULES

OUT = ROOT / "scripts" / "nerd_glyph_domains_md.py"

MD_DOMAIN_ORDER = [
    "MdFile", "MdEditor", "MdNavigation", "MdMedia", "MdAction", "MdDevice",
    "MdCommunication", "MdHome", "MdTransport", "MdAccount", "MdTime", "MdFinance",
    "MdSecurity", "MdFood", "MdWeather", "MdShape", "MdAlert", "MdScience", "MdNature",
    "MdSport", "MdGame", "MdShopping", "MdDatabase", "MdMap", "MdHealth", "MdOffice",
    "MdBattery", "MdClothing", "MdTool", "MdEducation", "MdAnimal", "MdBuilding",
    "MdReligion", "MdEmoji", "MdMisc",
]

MD_ASCII = {
    "MdFile": "F", "MdEditor": "E", "MdNavigation": "^", "MdMedia": ">", "MdAction": "*",
    "MdDevice": "D", "MdCommunication": "@", "MdHome": "H", "MdTransport": "T", "MdAccount": "P",
    "MdTime": "C", "MdFinance": "$", "MdSecurity": "#", "MdFood": "f", "MdWeather": "W",
    "MdShape": "O", "MdAlert": "!", "MdScience": "S", "MdNature": "N", "MdSport": "r",
    "MdGame": "G", "MdShopping": "g", "MdDatabase": "b", "MdMap": "M", "MdHealth": "h",
    "MdOffice": "o", "MdBattery": "B", "MdClothing": "c", "MdTool": "t", "MdEducation": "e",
    "MdAnimal": "a", "MdBuilding": "u", "MdReligion": "R", "MdEmoji": ":", "MdMisc": ".",
}

MD_CATEGORY_ORDER = {
    "MdFile": ["Archive", "Attachment", "Book", "Clipboard", "Content", "Document", "Folder", "Library", "Note", "Package", "Page", "Source", "Misc"],
    "MdEditor": ["Format", "Text", "Code", "Draw", "Table", "Color", "Transform", "Source", "Symbol", "Numeric", "Misc"],
    "MdNavigation": ["Arrow", "Chevron", "Direction", "Location", "Map", "Transit", "Misc"],
    "MdMedia": ["Audio", "Video", "Image", "Playback", "Broadcast", "Misc"],
    "MdAction": ["Share", "Search", "Input", "System", "Misc"],
    "MdDevice": ["Computer", "Mobile", "Network", "Storage", "Power", "Peripheral", "Misc"],
    "MdCommunication": ["Message", "Mail", "Phone", "Social", "Web", "Misc"],
    "MdHome": ["Room", "Furniture", "Appliance", "Garden", "Utility", "Misc"],
    "MdTransport": ["Road", "Rail", "Air", "Water", "Misc"],
    "MdAccount": ["User", "Group", "Body", "Credential", "Misc"],
    "MdTime": ["Clock", "Calendar", "Timer", "Schedule", "Misc"],
    "MdFinance": ["Currency", "Payment", "Chart", "Business", "Misc"],
    "MdSecurity": ["Lock", "Shield", "Auth", "Privacy", "Misc"],
    "MdFood": ["Meal", "Drink", "Ingredient", "Restaurant", "Misc"],
    "MdWeather": ["Sun", "Cloud", "Precipitation", "Storm", "Temperature", "Misc"],
    "MdShape": ["Basic", "Polygon", "Solid", "Misc"],
    "MdAlert": ["Status", "Notification", "Progress", "Misc"],
    "MdScience": ["Chemistry", "Physics", "Math", "Biology", "Astronomy", "Misc"],
    "MdNature": ["Plant", "Animal", "Landscape", "Water", "Misc"],
    "MdSport": ["Ball", "Equipment", "Venue", "Award", "Misc"],
    "MdGame": ["Board", "Dice", "Video", "Toy", "Misc"],
    "MdShopping": ["Store", "Cart", "Bag", "Product", "Misc"],
    "MdDatabase": ["Database", "Misc"],
    "MdMap": ["Marker", "Map", "Misc"],
    "MdHealth": ["Medical", "Hospital", "Body", "Wellness", "Misc"],
    "MdOffice": ["Desk", "Document", "Business", "Misc"],
    "MdBattery": ["Battery", "Misc"],
    "MdClothing": ["Head", "Body", "Foot", "Accessory", "Misc"],
    "MdTool": ["Hand", "Power", "Measure", "Misc"],
    "MdEducation": ["School", "Book", "Learning", "Misc"],
    "MdAnimal": ["Mammal", "Bird", "Fish", "Insect", "Misc"],
    "MdBuilding": ["Residential", "Commercial", "Institutional", "Religious", "Misc"],
    "MdReligion": ["Symbol", "Place", "Text", "Misc"],
    "MdEmoji": ["Face", "Emotion", "Object", "Misc"],
    "MdMisc": ["Brand", "Symbol", "Misc"],
}

def _kebab_domain(name: str) -> str:
    return "md-" + name[2:].replace("_", "-").lower()

def _cpp_stem(name: str) -> str:
    return _kebab_domain(name).replace("-", "_") + "_icon"

def build_prefix_rules(icons: dict[str, int]) -> list[tuple[str, frozenset[str]]]:
    """Build prefix rules covering all non-brand first segments."""
    first_segments = {s.split("_")[0] for s in icons}
    assigned: dict[str, str] = {}

    groups: list[tuple[str, set[str]]] = [
        ("MdDatabase", {"database"}),
        ("MdBattery", {"battery"}),
        ("MdMap", {"map", "terrain", "latitude", "longitude", "geography", "geodesy"}),
        ("MdFile", {
            "file", "folder", "archive", "attachment", "paperclip", "clipboard", "notebook",
            "book", "bookshelf", "library", "package", "zip", "content", "document", "note",
            "post", "page", "tab", "source", "newspaper", "sticker", "bookmark", "inbox",
            "outbox", "draft", "sent", "tray", "mailbox", "briefcase",
        }),
        ("MdEditor", {
            "format", "text", "alpha", "numeric", "vector", "pencil", "code", "xml", "html",
            "css", "language", "translate", "spellcheck", "pen", "eraser", "marker", "draw",
            "bezier", "curve", "polygon", "rectangle", "rhombus", "hexagon", "octagon", "ellipse",
            "form", "input", "textarea", "select", "dropdown", "menu", "toolbar", "ruler", "grid",
            "table", "column", "row", "cell", "merge", "split", "wrap", "undo", "redo", "cut",
            "copy", "paste", "scissors", "crop", "resize", "rotate", "flip", "mirror", "layer",
            "layers", "group", "ungroup", "typeface", "typography", "letter", "character",
            "glyph", "symbol", "icon", "badge", "label", "tag", "application", "app", "program",
            "software", "window", "fullscreen", "minimize", "maximize", "collapse", "expand",
            "close", "check", "checkbox", "radiobox", "toggle", "switch", "slider", "dial",
            "knob", "button", "save", "disk", "floppy", "import", "export", "backup", "restore",
            "trash", "delete", "remove", "add", "create", "new", "edit", "modify", "update",
            "history", "version", "branch", "commit", "pull", "push", "fork", "clone",
            "repository", "git", "debug", "test", "build", "compile", "deploy", "release",
            "publish", "palette", "brush", "paint", "fill", "gradient", "opacity", "shadow",
            "blur", "contrast", "brightness", "saturation", "hue", "invert", "grayscale",
            "sepia", "vignette", "border", "outline", "stroke", "width", "height", "dimension",
            "measure", "size", "color", "list", "order", "sort", "counter", "abacus",
            "calculator", "protractor", "view", "relation", "tune", "adjust", "cog", "eyedropper",
            "script", "axis", "tooltip", "magnify", "selection", "set", "math", "decimal",
            "tally", "comma", "ampersand", "asterisk", "backspace", "checkerboard", "compare",
            "data", "animation", "augmented", "autorenew", "alphabetical", "alphabet", "ab",
            "incognito", "qrcode", "barcode", "api", "apps", "console", "cursor", "gesture",
            "hand", "lightbulb", "flash", "bolt", "power", "theme", "arrange", "distribute",
            "plus", "minus", "equal", "approximately", "percent", "division", "multiplication",
            "addition", "subtraction", "panorama", "filmstrip", "swap", "fast", "rewind",
            "skip", "motion", "dock", "details", "dots", "ev", "gas", "gift", "head", "ray",
            "seat", "smart", "ticket", "weight", "domain", "engine", "function", "lambda",
            "logic", "matrix", "modulo", "not", "omega", "phi", "psi", "rho", "theta",
            "variable", "xor", "zeta", "beta", "gamma", "delta", "epsilon", "infinity",
            "sigma", "pi", "fraction",
        }),
        ("MdNavigation", {
            "arrow", "chevron", "caret", "triangle", "navigation", "compass", "direction",
            "marker", "location", "pin", "gps", "sign", "milestone", "crosshairs", "target",
            "focus", "pan", "zoom", "fit", "route", "transit", "walk", "run", "hike", "ski",
            "dots", "more", "back", "forward", "up", "down", "left", "right", "flag",
        }),
        ("MdMedia", {
            "movie", "video", "image", "photo", "camera", "music", "audio", "sound", "volume",
            "speaker", "headphone", "headset", "microphone", "record", "play", "pause", "stop",
            "eject", "shuffle", "repeat", "loop", "random", "playlist", "queue", "radio",
            "podcast", "stream", "broadcast", "cast", "airplay", "film", "clapperboard",
            "vinyl", "disc", "album", "gallery", "picture", "multimedia", "television", "tv",
            "projector", "project", "equalizer", "waveform", "surround", "dolby", "dts", "hdmi",
            "headphones", "panorama", "filmstrip", "rewind",
        }),
        ("MdAction", {
            "share", "send", "receive", "download", "upload", "sync", "refresh", "reload",
            "login", "logout", "signin", "signup", "register", "search", "find", "replace",
            "filter", "drag", "drop", "move", "deselect", "highlight", "pin", "unpin", "open",
            "launch", "start", "resume", "scan", "print", "fax", "call", "dial", "answer",
            "hangup", "mute", "unmute", "restart", "reboot", "upgrade", "install", "uninstall",
            "tap", "click", "touch", "cached", "autorenew", "swap",
        }),
        ("MdDevice", {
            "wifi", "bluetooth", "nfc", "router", "modem", "server", "cloud", "harddisk",
            "ssd", "sim", "cpu", "gpu", "memory", "chip", "monitor", "display", "screen",
            "keyboard", "mouse", "touchpad", "trackpad", "gamepad", "joystick", "controller",
            "remote", "laptop", "desktop", "tablet", "smartphone", "watch", "wearable", "fitness",
            "printer", "scanner", "antenna", "satellite", "signal", "tower", "cell", "mobile",
            "plug", "outlet", "socket", "cable", "wire", "ethernet", "network", "lan", "wan",
            "vpn", "dns", "ip", "access", "hub", "rack", "nas", "ups", "charging", "solar",
            "fan", "thermostat", "light", "lamp", "bulb", "led", "flashlight", "torch",
            "earbuds", "webcam", "drone", "robot", "vacuum", "washer", "dryer", "fridge",
            "oven", "stove", "microwave", "toaster", "coffee", "kettle", "blender", "mixer",
            "iron", "sewing", "stereo", "amplifier", "receiver", "transmitter", "sensor",
            "detector", "meter", "gauge", "scale", "thermometer", "barometer", "hygrometer",
            "anemometer", "locator", "tracker", "beacon", "rfid", "qr", "zigbee", "zwave",
            "lora", "lte", "volte", "vowifi", "esim", "sd", "microsd", "usb", "thunderbolt",
            "serial", "parallel", "scsi", "sata", "nvme", "pcie", "ram", "rom", "flash",
            "fpga", "mcu", "dsp", "tpu", "npu", "apu", "core", "thread", "process", "task",
            "job", "queue", "stack", "heap", "pool", "cache", "buffer", "register", "interrupt",
            "dma", "i2c", "spi", "uart", "can", "lin", "flexray", "iot", "edge", "mesh",
            "gateway", "bridge", "repeater", "extender", "attenuator", "filter", "mixer",
            "oscillator", "synthesizer", "modulator", "demodulator", "codec", "adc", "dac",
            "pll", "vco", "lna", "pa", "rf", "microwave", "optical", "fiber", "copper",
            "coax", "connector", "adapter", "converter", "transceiver", "duplexer", "circulator",
            "isolator", "coupler", "splitter", "combiner", "multiplexer", "demultiplexer",
            "matrix", "crossbar", "bus", "backplane", "midplane", "chassis", "enclosure",
            "cabinet", "shelf", "blade", "module", "board", "pcb", "assembly", "subsystem",
            "system", "platform", "driver", "kernel", "hypervisor", "container", "vm",
            "virtual", "cellphone", "smart", "dock", "ev", "gas", "headphones", "qrcode",
            "barcode", "domain", "console",
        }),
        ("MdCommunication", {
            "message", "chat", "comment", "forum", "discussion", "reply", "email", "mail",
            "envelope", "inbox", "outbox", "conference", "meeting", "event", "reminder",
            "notification", "rss", "feed", "web", "internet", "browser", "website", "url",
            "link", "hyperlink", "anchor", "favorite", "like", "dislike", "thumb", "rate",
            "review", "feedback", "survey", "poll", "vote", "ballot", "phone", "bullhorn",
            "mailbox", "send", "at",
        }),
        ("MdHome", {
            "home", "house", "door", "garage", "gate", "fence", "wall", "roof", "chimney",
            "balcony", "terrace", "patio", "deck", "porch", "veranda", "garden", "lawn", "yard",
            "pool", "spa", "sauna", "jacuzzi", "bathtub", "shower", "toilet", "sink", "faucet",
            "tap", "pipe", "plumbing", "heating", "cooling", "hvac", "ventilation", "insulation",
            "lighting", "chandelier", "sconce", "fixture", "furniture", "sofa", "chair", "bed",
            "mattress", "pillow", "blanket", "sheet", "curtain", "blind", "rug", "carpet",
            "floor", "tile", "wood", "laminate", "vinyl", "concrete", "brick", "stone", "marble",
            "granite", "quartz", "ceramic", "porcelain", "glass", "mirror", "shelf", "cabinet",
            "drawer", "closet", "wardrobe", "dresser", "nightstand", "desk", "entertainment",
            "center", "stand", "mount", "bracket", "hook", "hanger", "organizer", "storage",
            "bin", "basket", "container", "jar", "can", "bottle", "cup", "mug", "plate", "bowl",
            "utensil", "knife", "fork", "spoon", "chopstick", "grater", "peeler", "opener",
            "corkscrew", "tongs", "whisk", "spatula", "ladle", "colander", "strainer", "sieve",
            "funnel", "measuring", "laundry", "mop", "broom", "dustpan", "sponge", "cloth",
            "towel", "soap", "detergent", "bleach", "disinfectant", "sanitizer", "recycle",
            "compost", "hamper", "lint", "roller", "stain", "remover", "fabric", "softener",
            "steamer", "press", "needle", "thread", "button", "zipper", "snap", "loop", "velcro",
            "elastic", "ribbon", "lace", "trim", "hem", "seam", "dart", "pleat", "gather",
            "ruffle", "frill", "pocket", "collar", "cuff", "sleeve", "bodice", "skirt", "pants",
            "shorts", "dress", "suit", "jacket", "coat", "vest", "sweater", "cardigan", "hoodie",
            "tshirt", "shirt", "blouse", "tank", "top", "underwear", "bra", "panties", "boxers",
            "briefs", "socks", "stockings", "tights", "leggings", "shoes", "boots", "sandals",
            "slippers", "sneakers", "heels", "flats", "loafers", "oxfords", "brogues", "derbies",
            "monk", "straps", "mules", "clogs", "espadrilles", "flip", "flops", "crocs",
            "wellies", "galoshes", "hiking", "running", "training", "basketball", "football",
            "soccer", "tennis", "golf", "baseball", "hockey", "cricket", "rugby", "volleyball",
            "badminton", "ping", "pong", "squash", "racquetball", "handball", "lacrosse", "field",
            "ice", "inline", "skate", "snowboard", "sled", "kayak", "canoe", "raft", "sail",
            "row", "paddle", "kite", "windsurf", "wakeboard", "parasail", "paraglide", "hang",
            "glide", "skydive", "bungee", "climb", "boulder", "mountaineer", "trek", "camp",
            "backpack", "tent", "sleeping", "pad", "mat", "lantern", "headlamp", "telescope",
            "magnifier", "multitool", "axe", "hatchet", "saw", "shovel", "pick", "mallet",
            "crowbar", "pliers", "drill", "sander", "grinder", "router", "planer", "jointer",
            "lathe", "mill", "welder", "solder", "glue", "gun", "staple", "nail", "screw",
            "bolt", "nut", "washer", "rivet", "clip", "clamp", "vise", "anvil", "forge", "kiln",
            "furnace", "boiler", "duct", "vent", "register", "grille", "diffuser", "damper",
            "humidifier", "dehumidifier", "purifier", "ionizer", "ozone", "generator", "uv",
            "softener", "distiller", "dispenser", "fountain", "irrigation", "sprinkler", "drip",
            "hose", "nozzle", "controller", "attic", "basement", "crawl", "utility", "room",
            "mudroom", "pantry", "linen", "coat", "shoe", "workshop", "studio", "office", "den",
            "living", "dining", "kitchen", "bathroom", "bedroom", "nursery", "playroom", "game",
            "theater", "gym", "gazebo", "pergola", "arbor", "trellis", "driveway", "walkway",
            "path", "sidewalk", "curb", "gutter", "downspout", "drain", "sewer", "septic",
            "well", "pump", "tank", "heater", "ac", "heat", "blinds", "ceiling", "air", "pail",
            "greenhouse", "beach", "barn", "barrel", "dock", "mailbox", "motion", "seat", "smart",
            "ticket", "weight", "zodiac", "advertisements", "all", "allergy", "angle", "aspect",
            "assistant", "at", "auto", "av", "backburger", "cached", "ceiling", "decimal",
            "details", "dots", "ev", "gift", "head", "incognito", "motion", "pail", "ray",
            "seat", "selection", "smart", "swap", "ticket", "tooltip", "tray", "weight", "zodiac",
        }),
        ("MdTransport", {
            "car", "bus", "truck", "van", "taxi", "train", "tram", "subway", "metro", "rail",
            "plane", "airplane", "helicopter", "rocket", "ship", "boat", "ferry", "anchor",
            "bicycle", "bike", "scooter", "motorcycle", "moped", "skateboard", "roller",
            "caravan", "trailer", "rv", "ambulance", "tow", "garbage", "delivery", "forklift",
            "crane", "excavator", "bulldozer", "loader", "grader", "compactor", "paver", "dump",
            "tanker", "flatbed", "pickup", "suv", "sedan", "coupe", "convertible", "hatchback",
            "wagon", "minivan", "limousine", "atv", "utv", "snowmobile", "jet", "glider",
            "balloon", "blimp", "uav", "space", "shuttle", "orbit", "launch", "landing",
            "runway", "taxiway", "hangar", "baggage", "luggage", "boarding", "cockpit", "cabin",
            "deck", "hull", "bow", "stern", "mast", "rudder", "propeller", "wheel", "tire",
            "brake", "steering", "headlight", "taillight", "wiper", "bumper", "fender", "hood",
            "trunk", "ignition", "fuel", "diesel", "hybrid", "mileage", "odometer", "speedometer",
            "tachometer", "congestion", "accident", "construction", "detour", "road", "highway",
            "freeway", "expressway", "parkway", "turnpike", "toll", "tunnel", "intersection",
            "roundabout", "crosswalk", "lane", "parking", "valet", "carpool", "rideshare", "hov",
            "platform", "track", "crossing", "barrier", "fare", "transit", "commute", "travel",
            "trip", "journey", "voyage", "cruise", "expedition", "sail", "kayak", "canoe",
            "raft", "submarine", "yacht", "speedboat", "hovercraft", "segway", "unicycle",
            "monorail", "funicular", "gondola", "cable", "chairlift", "escalator", "elevator",
            "airport", "airballoon", "airbag", "ev", "seat",
        }),
        ("MdAccount", {
            "account", "user", "profile", "avatar", "human", "person", "people", "face", "head",
            "body", "finger", "foot", "leg", "arm", "eye", "ear", "nose", "mouth", "child",
            "baby", "elder", "gender", "male", "female", "transgender", "nonbinary", "pregnancy",
            "breastfeeding", "wheelchair", "blind", "deaf", "id", "passport", "license",
            "credential", "certificate", "diploma", "degree", "graduation", "school", "university",
            "college", "student", "teacher", "professor", "staff", "employee", "employer",
            "manager", "supervisor", "director", "executive", "ceo", "cfo", "cto", "coo", "cio",
            "cmo", "chro", "cpo", "cso", "clo", "cdo", "cao", "cbo", "cco", "cro", "cgo", "cko",
            "cvo", "cxo",
        }),
        ("MdTime", {
            "clock", "calendar", "timer", "stopwatch", "hourglass", "alarm", "schedule", "date",
            "time", "timeline",
        }),
        ("MdFinance", {
            "credit", "bank", "cash", "money", "currency", "dollar", "euro", "pound", "yen",
            "rupee", "bitcoin", "ethereum", "crypto", "wallet", "piggy", "chart", "graph",
            "trending", "stock", "bond", "fund", "portfolio", "investment", "savings", "loan",
            "mortgage", "insurance", "tax", "invoice", "receipt", "payment", "transaction",
            "transfer", "withdraw", "deposit", "atm", "pos", "briefcase", "business", "corporate",
            "company", "organization", "enterprise", "startup", "venture", "capital", "angel",
            "seed", "series", "round", "divestiture", "spinoff", "dividend", "yield", "interest",
            "apr", "apy", "roi", "irr", "npv", "inflation", "deflation", "stagflation", "recession",
            "depression", "expansion", "boom", "bust", "cycle", "trend", "seasonality",
            "volatility", "risk", "return", "sharpe", "sortino", "treynor", "jensen", "beta",
            "gamma", "delta", "theta", "vega", "rho",
        }),
        ("MdSecurity", {
            "shield", "lock", "unlock", "key", "password", "security", "privacy", "vpn",
            "firewall", "antivirus", "scan", "fingerprint", "recognition", "authentication",
            "authorization", "certificate", "token", "jwt", "oauth", "saml", "ldap", "kerberos",
            "radius", "tacacs", "snmp", "encryption", "decryption", "hash", "salt", "pepper",
            "nonce", "rsa", "dsa", "ecdsa", "eddsa", "aes", "des", "blowfish", "twofish",
            "serpent", "camellia", "chacha20", "salsa20", "rc4", "idea", "cast", "aria", "sm4",
            "quantum", "lattice", "nist", "pqc", "fips", "soc", "pci", "dss", "hipaa", "gdpr",
            "ccpa", "sox", "glba", "ferpa", "coppa", "pipeda", "lgpd", "pdpa", "appi", "popia",
            "kvkk", "pdpl", "dpa", "dpo", "dsar", "rtbf", "portability", "rectification",
            "erasure", "restriction", "objection", "profiling", "consent", "controller",
            "processor", "subprocessor", "representative", "supervisory", "authority", "dpia",
            "pia", "tra", "tia", "lia", "bcr", "scc", "idta", "addendum", "adequacy",
            "derogation", "exception", "safeguard", "measure", "pseudonymization", "anonymization",
            "minimization", "limitation", "accuracy", "integrity", "confidentiality", "availability",
            "accountability", "transparency", "fairness", "lawfulness", "breach", "notification",
        }),
        ("MdFood", {
            "food", "restaurant", "menu", "silverware", "chopsticks", "pot", "pan", "skillet",
            "wok", "grill", "stove", "tea", "beer", "wine", "cocktail", "bar", "bakery", "cake",
            "cookie", "candy", "chocolate", "cream", "pizza", "burger", "dog", "taco", "burrito",
            "sushi", "noodles", "rice", "bread", "cheese", "egg", "meat", "fish", "seafood",
            "fruit", "vegetable", "apple", "banana", "orange", "lemon", "grape", "strawberry",
            "cherry", "peach", "pear", "pineapple", "watermelon", "melon", "coconut", "avocado",
            "tomato", "potato", "carrot", "corn", "pepper", "onion", "garlic", "mushroom",
            "broccoli", "cabbage", "lettuce", "spinach", "kale", "celery", "cucumber", "zucchini",
            "eggplant", "pumpkin", "squash", "bean", "pea", "lentil", "nut", "almond", "walnut",
            "peanut", "hazelnut", "cashew", "pistachio", "seed", "grain", "wheat", "oat", "barley",
            "rye", "quinoa", "millet", "sorghum", "buckwheat", "amaranth", "teff", "spelt",
            "kamut", "farro", "bulgur", "couscous", "pasta", "noodle", "ramen", "udon", "soba",
            "vermicelli", "spaghetti", "linguine", "fettuccine", "penne", "rigatoni", "macaroni",
            "lasagna", "ravioli", "tortellini", "gnocchi", "dumpling", "wonton", "gyoza",
            "potsticker", "spring", "roll", "samosas", "pakora", "bhaji", "falafel", "hummus",
            "tahini", "tabbouleh", "fattoush", "shawarma", "kebab", "gyro", "doner", "souvlaki",
            "satay", "teriyaki", "tempura", "katsu", "tonkatsu", "yakitori", "okonomiyaki",
            "takoyaki", "onigiri", "maki", "nigiri", "sashimi", "poke", "ceviche", "burek",
            "banitsa", "sarma", "goulash", "paprikash", "schnitzel", "bratwurst", "sauerkraut",
            "pretzel", "strudel", "sacher", "torte", "cheesecake", "tiramisu", "panna", "cotta",
            "gelato", "sorbet", "granita", "parfait", "mousse", "souffle", "creme", "brulee",
            "flan", "pudding", "custard", "jelly", "jam", "marmalade", "preserve", "compote",
            "chutney", "relish", "pickle", "kimchi", "miso", "soy", "sauce", "worcestershire",
            "tabasco", "sriracha", "ketchup", "mustard", "mayonnaise", "aioli", "ranch",
            "vinaigrette", "dressing", "marinade", "rub", "spice", "herb", "salt", "sugar",
            "honey", "syrup", "molasses", "vinegar", "oil", "butter", "milk", "yogurt", "kefir",
            "buttermilk", "sour", "fraiche", "mascarpone", "ricotta", "cottage", "feta",
            "mozzarella", "parmesan", "cheddar", "gouda", "brie", "camembert", "roquefort",
            "gorgonzola", "stilton", "swiss", "provolone", "monterey", "jack", "colby", "havarti",
            "edam", "gruyere", "emmental", "jarlsberg", "limburger", "munster", "taleggio",
            "fontina", "asiago", "pecorino", "manchego", "halloumi", "paneer", "queso", "fresco",
            "cotija", "oaxaca", "chihuahua", "idiazabal", "roncal", "zamorano", "cabrales",
            "mahon", "tetilla", "gamoneu", "afuega", "pit", "beyos", "pria", "penamellera",
            "bable", "casin", "chili", "hamburger", "baguette", "candycane", "carrot", "coffee",
            "cookie", "fruit", "kettle", "pan", "pail",
        }),
        ("MdWeather", {
            "weather", "cloud", "rain", "snow", "wind", "sun", "moon", "star", "thunder",
            "lightning", "tornado", "hurricane", "fog", "mist", "hail", "sleet", "temperature",
            "thermometer", "umbrella", "water", "wave", "tsunami", "flood", "drought", "fire",
            "smoke", "ash", "dust", "sand", "storm", "cyclone", "typhoon", "blizzard", "avalanche",
            "earthquake", "volcano", "aurora", "rainbow", "sunset", "sunrise", "partly", "mostly",
            "clear", "overcast", "hazy", "humid", "dry", "hot", "cold", "warm", "cool", "freezing",
            "boiling", "dew", "frost", "ice", "glaze", "rime", "hoar", "snowflake", "smoke",
        }),
        ("MdShape", {
            "square", "circle", "triangle", "star", "heart", "diamond", "rhombus", "hexagon",
            "octagon", "ellipse", "oval", "rectangle", "polygon", "shape", "dot", "ring", "cube",
            "sphere", "cylinder", "cone", "pyramid", "prism", "torus", "emoticon",
        }),
        ("MdAlert", {
            "alert", "warning", "error", "info", "help", "question", "success", "fail", "cancel",
            "bell", "badge", "flag", "exclamation", "information", "progress", "loading", "spinner",
        }),
        ("MdScience", {
            "flask", "beaker", "atom", "molecule", "microscope", "telescope", "dna", "virus",
            "bacteria", "chemistry", "physics", "math", "formula", "function", "sigma", "pi",
            "infinity", "percent", "decimal", "fraction", "abacus", "calculator", "protractor",
            "divider", "ruler", "magnify", "periodic", "orbit", "planet", "solar", "lunar",
            "eclipse", "magnet", "prism", "lens", "wave", "frequency", "resistor", "capacitor",
            "inductor", "transistor", "diode", "circuit", "oscilloscope", "voltmeter", "ammeter",
            "ohmmeter", "multimeter", "pipette", "burette", "funnel", "petri", "centrifuge",
            "incubator", "autoclave", "spectrometer", "chromatograph", "electrophoresis", "biotech",
            "genome", "helix", "rna", "protein", "enzyme", "lab", "experiment", "hypothesis",
            "theory", "quantum", "relativity", "neutron", "proton", "electron", "photon", "quark",
            "boson", "fermion", "isotope", "radiation", "radioactive", "decay", "fusion", "fission",
            "reactor", "collider", "accelerator", "particle", "nanotech", "biochemistry",
            "microbiology", "astronomy", "astrophysics", "cosmology", "nebula", "galaxy",
            "constellation", "asteroid", "comet", "meteor", "altimeter", "barometer", "hygrometer",
            "anemometer", "seismograph", "geiger", "caliper", "micrometer", "vernier", "tachometer",
            "speedometer", "odometer", "ab", "axis", "tally", "comma", "ampersand", "asterisk",
            "backspace", "checkerboard", "compare", "data", "engine", "lambda", "logic", "matrix",
            "modulo", "not", "omega", "phi", "psi", "rho", "theta", "variable", "xor", "zeta",
            "alpha", "beta", "gamma", "delta", "epsilon", "zodiac", "numeric", "relation",
            "selection", "set", "tooltip", "decimal", "math", "domain", "bio", "biohazard",
            "radiation", "nuclear", "orbit", "planet", "solar", "lunar", "eclipse", "magnet",
            "prism", "lens", "wave", "frequency", "resistor", "capacitor", "inductor",
            "transistor", "diode", "circuit", "oscilloscope", "voltmeter", "ammeter", "ohmmeter",
            "multimeter", "pipette", "burette", "funnel", "petri", "centrifuge", "incubator",
            "autoclave", "spectrometer", "chromatograph", "electrophoresis", "biotech", "genome",
            "helix", "rna", "protein", "enzyme", "lab", "experiment", "hypothesis", "theory",
            "quantum", "relativity", "neutron", "proton", "electron", "photon", "quark", "boson",
            "fermion", "isotope", "radiation", "radioactive", "decay", "fusion", "fission",
            "reactor", "collider", "accelerator", "particle", "nanotech", "biochemistry",
            "microbiology", "astronomy", "astrophysics", "cosmology", "nebula", "galaxy",
            "constellation", "asteroid", "comet", "meteor", "altimeter", "barometer", "hygrometer",
            "anemometer", "seismograph", "geiger", "caliper", "micrometer", "vernier", "tachometer",
            "speedometer", "odometer",
        }),
        ("MdNature", {
            "tree", "flower", "leaf", "plant", "grass", "bush", "shrub", "fern", "moss", "algae",
            "cactus", "palm", "pine", "oak", "maple", "willow", "birch", "cedar", "sequoia",
            "bonsai", "seedling", "sprout", "root", "branch", "trunk", "bark", "pollen", "nectar",
            "nature", "forest", "jungle", "meadow", "prairie", "savanna", "tundra", "wetland",
            "marsh", "swamp", "pond", "lake", "river", "stream", "waterfall", "spring", "cave",
            "mountain", "hill", "valley", "canyon", "cliff", "rock", "stone", "pebble", "sand",
            "soil", "earth", "globe", "world", "continent", "island", "beach", "coast", "ocean",
            "sea", "reef", "coral", "seaweed", "kelp", "bamboo", "ivy", "vine", "rose", "tulip",
            "daisy", "sunflower", "lotus", "orchid", "lily", "poppy", "dandelion", "clover",
            "mushroom", "fungus", "lichen", "snowflake", "icicle", "frost", "dew", "raindrop",
            "campfire", "bonfire", "flame", "ember", "smoke", "ash", "charcoal", "wood", "log",
            "stump", "acorn", "pinecone", "nut", "berry", "hay", "straw", "haystack", "barn",
            "silo", "windmill", "watermill", "well", "fountain", "watering", "sprinkler", "hose",
            "nozzle", "rake", "hoe", "shovel", "spade", "pitchfork", "sickle", "scythe", "pruning",
            "shears", "lawn", "mower", "trimmer", "hedge", "topiary", "greenhouse", "garden",
            "flowerbed", "compost", "mulch", "fertilizer", "pesticide", "herbicide", "insecticide",
            "fungicide", "seed", "sapling", "nursery", "orchard", "vineyard", "farm", "field",
            "crop", "harvest", "tractor", "plow", "cultivator", "irrigation", "drainage",
            "erosion", "conservation", "sustainability", "eco", "organic", "biodegradable",
            "compostable", "renewable", "biomass", "tidal", "carbon", "footprint", "emission",
            "pollution", "contamination", "toxic", "hazardous", "waste", "landfill", "incinerator",
            "sewage", "treatment", "filtration", "purification", "desalination", "reservoir", "dam",
            "levee", "dike", "canal", "aqueduct", "pipeline", "pump", "valve", "gauge", "meter",
            "sensor", "monitor", "climate", "temperature", "humidity", "pressure", "wind", "rain",
            "snow", "hail", "sleet", "storm", "thunder", "lightning", "tornado", "hurricane",
            "typhoon", "cyclone", "blizzard", "avalanche", "flood", "drought", "land", "terrain",
            "geology", "mineral", "crystal", "gem", "diamond", "emerald", "ruby", "sapphire",
            "amethyst", "quartz", "marble", "granite", "limestone", "volcano", "lava", "magma",
            "geyser", "oasis", "desert", "dune", "water", "fire", "smoke", "chili", "fruit",
            "vegetable", "carrot", "potato", "tomato", "onion", "garlic", "pepper", "cucumber",
            "lettuce", "spinach", "kale", "broccoli", "cauliflower", "cabbage", "pumpkin", "squash",
            "melon", "watermelon", "grape", "pear", "peach", "cherry", "strawberry", "blueberry",
            "raspberry", "blackberry", "cranberry", "lemon", "lime", "orange", "banana", "pineapple",
            "mango", "coconut", "avocado", "olive", "fig", "date", "plum", "apricot", "nectarine",
            "kiwi", "papaya", "guava", "passion", "dragon", "lychee", "rambutan", "durian",
            "jackfruit", "breadfruit", "plantain", "yam", "taro", "cassava", "beet", "radish",
            "turnip", "parsnip", "celery", "asparagus", "artichoke", "eggplant", "zucchini", "okra",
            "bean", "pea", "corn", "wheat", "rice", "barley", "oat", "rye", "millet", "sorghum",
            "quinoa", "buckwheat", "amaranth", "teff", "spelt", "kamut", "farro", "bulgur", "couscous",
        }),
        ("MdSport", {
            "soccer", "football", "basketball", "baseball", "tennis", "golf", "hockey", "cricket",
            "rugby", "volleyball", "badminton", "squash", "racquetball", "handball", "lacrosse",
            "polo", "curling", "bowling", "billiards", "pool", "snooker", "darts", "archery",
            "fencing", "boxing", "wrestling", "judo", "karate", "taekwondo", "kungfu", "aikido",
            "jiujitsu", "muaythai", "kickboxing", "mma", "ufc", "wwe", "sumo", "weightlifting",
            "powerlifting", "bodybuilding", "crossfit", "gymnastics", "athletics", "track",
            "marathon", "sprint", "hurdle", "relay", "triathlon", "biathlon", "pentathlon",
            "decathlon", "heptathlon", "swimming", "diving", "surfing", "windsurfing", "kitesurfing",
            "paddleboarding", "kayaking", "canoeing", "rowing", "sailing", "yachting", "boating",
            "fishing", "hunting", "shooting", "skiing", "snowboarding", "skating", "bobsled", "luge",
            "skeleton", "figure", "speed", "shorttrack", "longtrack", "nordic", "alpine", "freestyle",
            "mogul", "aerial", "halfpipe", "slopestyle", "medal", "trophy", "podium", "champion",
            "winner", "loser", "score", "goal", "point", "match", "round", "inning", "quarter",
            "half", "period", "overtime", "penalty", "foul", "referee", "umpire", "coach", "trainer",
            "team", "player", "captain", "substitute", "bench", "locker", "shower", "sauna",
            "massage", "therapy", "rehabilitation", "injury", "bandage", "crutch", "wheelchair",
            "prosthetic", "orthotic", "helmet", "pads", "guard", "glove", "shoe", "cleat", "spike",
            "blade", "ski", "board", "stick", "bat", "racket", "club", "ball", "puck", "disc",
            "frisbee", "javelin", "discus", "shotput", "hammer", "pole", "vault", "high", "long",
            "triple", "steeplechase", "race", "walk", "ultra", "trail", "stadium", "arena", "court",
            "pitch", "rink", "course", "range", "lane", "net", "hoop", "post", "marker", "line",
            "boundary", "fair", "safe", "strike", "hit", "miss", "catch", "throw", "pass", "kick",
            "punch", "block", "dodge", "parry", "counter", "combo", "knockout", "submission", "pin",
            "tap", "surrender", "forfeit", "disqualification", "ejection", "suspension", "ban",
            "fine", "warning", "caution", "whistle", "horn", "gun", "lap", "split", "pace",
            "distance", "time", "record", "personal", "best", "world", "olympic", "national",
            "international", "professional", "amateur", "collegiate", "highschool", "youth", "junior",
            "senior", "veteran", "masters", "recreational", "competitive", "elite", "league",
            "tournament", "championship", "cup", "ribbon", "certificate", "diploma", "degree",
            "rank", "belt", "dan", "kyu", "grade", "level", "division", "weight", "class",
            "category", "age", "gender", "mixed", "doubles", "singles", "sport", "diving", "weight",
            "boom", "ski", "hockey", "bowling", "archery", "fencing", "boxing", "wrestling",
            "karate", "judo", "swimming", "surfing", "sailing", "rowing", "climbing", "hiking",
            "camping", "skateboard", "bicycle", "motorbike", "racing", "stadium", "arena", "court",
            "field", "pool", "track", "goal", "net", "hoop", "bat", "racket", "club", "stick",
            "puck", "disc", "frisbee", "javelin", "discus", "shotput", "hammer", "pole", "vault",
            "hurdle", "marathon", "sprint", "relay", "triathlon", "pentathlon", "decathlon", "biathlon",
            "curling", "bobsled", "luge", "skeleton", "skiing", "snowboarding", "skating",
            "diving", "swimming", "surfing", "sailing", "rowing", "canoeing", "kayaking",
            "climbing", "bouldering", "mountaineering", "hiking", "trekking", "camping",
            "backpacking", "orienteering", "geocaching", "parkour", "freerunning", "skateboarding",
            "rollerblading", "scootering", "bmx", "motocross", "rally", "racing", "formula",
            "nascar", "indycar", "drag", "drift", "rallycross", "autocross", "karting",
            "motogp", "superbike", "enduro", "trials", "speedway", "flat", "track", "dirt",
            "track", "oval", "road", "course", "circuit", "paddock", "pit", "garage",
            "mechanic", "engineer", "strategist", "spotter", "crew", "chief", "driver",
            "rider", "pilot", "navigator", "co", "driver", "passenger", "spectator", "fan",
            "ticket", "seat", "stand", "box", "suite", "hospitality", "merchandise", "souvenir",
            "autograph", "photo", "selfie", "broadcast", "stream", "highlight", "replay",
            "slow", "motion", "instant", "review", "var", "challenge", "appeal", "protest",
        }),
        ("MdGame", {
            "game", "gamepad", "joystick", "controller", "console", "dice", "cards", "card",
            "chess", "puzzle", "toy", "lego", "brick", "block", "domino", "mahjong", "poker",
            "blackjack", "roulette", "slot", "bingo", "lottery", "scratch", "ticket", "prize",
            "jackpot", "casino", "bet", "wager", "gamble", "chips", "token", "crown", "scepter",
            "throne", "castle", "knight", "pawn", "rook", "bishop", "queen", "king", "checkmate",
            "stalemate", "draw", "resign", "promotion", "capture", "move", "turn", "clock",
            "timer", "score", "rating", "elo", "rank", "level", "xp", "experience", "skill",
            "ability", "power", "mana", "health", "stamina", "energy", "shield", "armor", "weapon",
            "sword", "axe", "bow", "arrow", "spear", "dagger", "mace", "hammer", "staff", "wand",
            "spell", "magic", "potion", "elixir", "scroll", "treasure", "chest", "loot", "drop",
            "spawn", "respawn", "checkpoint", "save", "load", "pause", "resume", "quit", "exit",
            "menu", "settings", "options", "controls", "keybind", "hotkey", "macro", "script",
            "mod", "plugin", "addon", "expansion", "dlc", "patch", "update", "version", "beta",
            "alpha", "demo", "trial", "full", "premium", "deluxe", "ultimate", "collector",
            "edition", "bundle", "pack", "season", "pass", "subscription", "membership", "vip",
            "freemium", "microtransaction", "iap", "market", "auction", "trade", "exchange",
            "gift", "reward", "achievement", "badge", "ribbon", "title", "leaderboard",
            "scoreboard", "stats", "profile", "avatar", "character", "class", "race", "faction",
            "guild", "clan", "party", "group", "raid", "dungeon", "quest", "mission", "objective",
            "challenge", "collectible", "hidden", "secret", "easter", "egg", "cheat", "hack",
            "exploit", "glitch", "bug", "crash", "lag", "ping", "fps", "graphics", "resolution",
            "fullscreen", "windowed", "borderless", "vsync", "aa", "texture", "shadow", "lighting",
            "shader", "particle", "effect", "animation", "physics", "collision", "hitbox",
            "hurtbox", "damage", "critical", "miss", "dodge", "block", "parry", "counter", "combo",
            "chain", "streak", "multiplier", "bonus", "penalty", "debuff", "buff", "status",
            "condition", "poison", "burn", "freeze", "stun", "silence", "root", "snare", "slow",
            "haste", "invisible", "stealth", "detect", "reveal", "scan", "mark", "target", "focus",
            "aggro", "threat", "tank", "heal", "support", "dps", "carry", "jungler", "mid", "top",
            "bot", "adc", "roam", "gank", "push", "farm", "last", "hit", "deny", "creep", "wave",
            "lane", "tower", "inhibitor", "nexus", "ancient", "roshan", "baron", "dragon", "herald",
            "objective", "camp", "jungle", "river", "brush", "fog", "war", "vision", "ward",
            "trinket", "item", "equipment", "gear", "inventory", "stash", "bank", "vendor",
            "merchant", "npc", "mob", "boss", "minion", "add", "trash", "elite", "rare", "epic",
            "legendary", "mythic", "artifact", "relic", "unique", "set", "socket", "gem", "enchant",
            "upgrade", "craft", "forge", "smith", "alchemy", "cooking", "mining", "herbalism",
            "skinning", "engineering", "jewelcrafting", "inscription", "archaeology", "first",
            "aid", "profession", "recipe", "blueprint", "schematic", "plan", "design", "pattern",
            "template", "mold", "cast", "melt", "smelt", "refine", "purify", "transmute",
            "combine", "merge", "fuse", "split", "disassemble", "salvage", "scrap", "recycle",
            "repair", "durability", "quality", "rarity", "tier", "enchantment", "affix", "prefix",
            "suffix", "stat", "attribute", "requirement", "restriction", "limitation", "cooldown",
            "charge", "stack", "duration", "radius", "area", "cone", "line", "beam", "projectile",
            "aoe", "dot", "hot", "cc", "crowd", "control", "interrupt", "disarm", "fear", "charm",
            "taunt", "knockback", "pull", "push", "blink", "dash", "leap", "jump", "fly", "swim",
            "climb", "crouch", "sneak", "sprint", "walk", "run", "idle", "emote", "dance", "wave",
            "bow", "salute", "laugh", "cry", "angry", "happy", "sad", "surprised", "confused",
            "bored", "tired", "sleep", "eat", "drink", "sit", "stand", "lie", "kneel", "crawl",
            "roll", "attack", "defend", "cast", "channel", "aim", "shoot", "reload", "switch",
            "melee", "ranged", "physical", "elemental", "ice", "lightning", "earth", "light", "dark",
            "holy", "shadow", "arcane", "bleed", "disease", "curse", "blessing", "summon",
            "transform", "polymorph", "metamorphosis", "evolve", "devolve", "ascend", "descend",
            "rebirth", "resurrection", "revive", "ghost", "spirit", "soul", "undead", "zombie",
            "skeleton", "vampire", "werewolf", "demon", "angel", "dragon", "phoenix", "griffin",
            "unicorn", "pegasus", "centaur", "minotaur", "medusa", "hydra", "chimera", "kraken",
            "leviathan", "behemoth", "golem", "elemental", "construct", "automaton", "robot",
            "android", "cyborg", "alien", "ufo", "spaceship", "planet", "star", "galaxy",
            "universe", "dimension", "portal", "gate", "wormhole", "teleport", "warp",
            "hyperspace", "subspace", "astral", "ethereal", "plane", "realm", "world", "zone",
            "region", "ruins", "temple", "fortress", "village", "town", "city", "capital",
            "kingdom", "empire", "nation", "alliance", "horde", "republic", "democracy", "monarchy",
            "dictatorship", "theocracy", "anarchy", "revolution", "rebellion", "uprising", "coup",
            "invasion", "war", "battle", "siege", "skirmish", "ambush", "patrol", "scout", "spy",
            "assassin", "thief", "rogue", "ranger", "hunter", "warrior", "paladin", "crusader",
            "templar", "berserker", "barbarian", "monk", "priest", "cleric", "druid", "shaman",
            "mage", "wizard", "sorcerer", "warlock", "necromancer", "bard", "dancer", "singer",
            "musician", "artist", "craftsman", "blacksmith", "armorer", "weaponsmith", "enchanter",
            "alchemist", "herbalist", "cook", "fisher", "miner", "lumberjack", "farmer", "merchant",
            "trader", "banker", "noble", "peasant", "slave", "prisoner", "criminal", "outlaw",
            "bandit", "pirate", "mercenary", "soldier", "guard", "captain", "general", "commander",
            "admiral", "marshal", "prince", "princess", "duke", "duchess", "earl", "count", "baron",
            "lord", "lady", "sir", "dame", "squire", "page", "herald", "ambassador", "diplomat",
            "dice", "cards", "card", "chess", "puzzle", "toy", "gamepad", "joystick", "console",
            "lego", "brick", "block", "domino", "mahjong", "poker", "blackjack", "roulette",
            "slot", "bingo", "lottery", "scratch", "ticket", "prize", "jackpot", "casino",
            "bet", "wager", "gamble", "chips", "token", "crown", "scepter", "throne",
            "knight", "pawn", "rook", "bishop", "queen", "king", "checkmate", "stalemate",
            "draw", "resign", "promotion", "capture", "move", "turn", "clock",
            "timer", "score", "rating", "elo", "rank", "level", "xp", "experience", "skill",
            "ability", "power", "mana", "health", "stamina", "energy", "shield", "armor",
            "weapon", "sword", "axe", "bow", "arrow", "spear", "dagger", "mace", "hammer",
            "staff", "wand", "spell", "magic", "potion", "elixir", "scroll", "treasure",
            "chest", "loot", "drop", "spawn", "respawn", "checkpoint", "save", "load",
            "pause", "resume", "quit", "exit", "menu", "settings", "options", "controls",
            "keybind", "hotkey", "macro", "script", "mod", "plugin", "addon", "expansion",
            "dlc", "patch", "update", "version", "beta", "alpha", "demo", "trial", "full",
            "premium", "deluxe", "ultimate", "collector", "edition", "bundle", "pack",
            "season", "pass", "subscription", "membership", "vip", "premium", "free", "play",
            "freemium", "microtransaction", "iap", "store", "shop", "market", "auction",
            "trade", "exchange", "gift", "reward", "achievement", "badge", "medal",
            "ribbon", "title", "rank", "leaderboard", "scoreboard", "stats", "profile",
            "avatar", "character", "class", "race", "faction", "guild", "clan", "team",
            "party", "group", "raid", "dungeon", "quest", "mission", "objective", "challenge",
            "achievement", "collectible", "hidden", "secret", "easter", "egg", "cheat", "hack",
            "exploit", "glitch", "bug", "crash", "lag", "ping", "fps", "graphics", "resolution",
            "fullscreen", "windowed", "borderless", "vsync", "aa", "texture", "shadow",
            "lighting", "shader", "particle", "effect", "animation", "physics", "collision",
            "hitbox", "hurtbox", "damage", "critical", "miss", "dodge", "block", "parry",
            "counter", "combo", "chain", "streak", "multiplier", "bonus", "penalty", "debuff",
            "buff", "status", "condition", "poison", "burn", "freeze", "stun", "silence",
            "root", "snare", "slow", "haste", "invisible", "stealth", "detect", "reveal",
            "scan", "mark", "target", "focus", "aggro", "threat", "tank", "heal", "support",
            "dps", "carry", "jungler", "mid", "top", "bot", "adc", "support", "roam", "gank",
            "push", "farm", "last", "hit", "deny", "creep", "wave", "lane", "tower",
            "inhibitor", "nexus", "ancient", "roshan", "baron", "dragon", "herald", "objective",
            "buff", "camp", "jungle", "river", "brush", "fog", "war", "vision", "ward",
            "trinket", "item", "equipment", "gear", "inventory", "stash", "bank", "vendor",
            "merchant", "npc", "mob", "boss", "minion", "add", "trash", "elite", "rare",
            "epic", "legendary", "mythic", "artifact", "relic", "unique", "set", "bonus",
            "socket", "gem", "enchant", "upgrade", "craft", "forge", "smith", "alchemy",
            "cooking", "fishing", "mining", "herbalism", "skinning", "engineering",
            "jewelcrafting", "inscription", "archaeology", "first", "aid", "profession",
            "skill", "recipe", "blueprint", "schematic", "plan", "design", "pattern",
            "template", "mold", "cast", "melt", "smelt", "refine", "purify", "transmute",
            "combine", "merge", "fuse", "split", "disassemble", "salvage", "scrap", "recycle",
            "repair", "durability", "condition", "quality", "rarity", "tier", "grade",
            "level", "enchantment", "affix", "prefix", "suffix", "stat", "attribute", "bonus",
            "penalty", "requirement", "restriction", "limitation", "cooldown", "charge",
            "stack", "duration", "range", "radius", "area", "cone", "line", "beam",
            "projectile", "aoe", "dot", "hot", "buff", "debuff", "cc", "crowd", "control",
            "interrupt", "silence", "disarm", "root", "snare", "slow", "stun", "fear",
            "charm", "taunt", "knockback", "pull", "push", "blink", "dash", "leap", "jump",
            "fly", "swim", "climb", "crouch", "sneak", "sprint", "walk", "run", "idle",
            "emote", "dance", "wave", "bow", "salute", "laugh", "cry", "angry", "happy",
            "sad", "surprised", "confused", "bored", "tired", "sleep", "eat", "drink",
            "sit", "stand", "lie", "kneel", "crawl", "roll", "attack", "defend", "cast",
            "channel", "aim", "shoot", "reload", "switch", "weapon", "melee", "ranged",
            "magic", "physical", "elemental", "fire", "ice", "lightning", "earth", "wind",
            "water", "light", "dark", "holy", "shadow", "arcane", "nature", "poison",
            "bleed", "disease", "curse", "blessing", "summon", "transform", "polymorph",
            "metamorphosis", "evolve", "devolve", "ascend", "descend", "rebirth",
            "resurrection", "revive", "respawn", "ghost", "spirit", "soul", "undead",
            "zombie", "skeleton", "vampire", "werewolf", "demon", "angel", "dragon",
            "phoenix", "griffin", "unicorn", "pegasus", "centaur", "minotaur", "medusa",
            "hydra", "chimera", "kraken", "leviathan", "behemoth", "golem", "elemental",
            "construct", "automaton", "robot", "android", "cyborg", "alien", "ufo",
            "spaceship", "planet", "star", "galaxy", "universe", "dimension", "portal",
            "gate", "wormhole", "teleport", "warp", "hyperspace", "subspace", "astral",
            "ethereal", "plane", "realm", "world", "zone", "area", "region", "continent",
            "island", "dungeon", "cave", "forest", "desert", "mountain", "ocean", "river",
            "lake", "swamp", "jungle", "tundra", "volcano", "ruins", "temple", "castle",
            "fortress", "tower", "village", "town", "city", "capital", "kingdom", "empire",
            "nation", "faction", "alliance", "horde", "republic", "democracy", "monarchy",
            "dictatorship", "theocracy", "anarchy", "revolution", "rebellion", "uprising",
            "coup", "invasion", "war", "battle", "siege", "skirmish", "ambush", "raid",
            "patrol", "scout", "spy", "assassin", "thief", "rogue", "ranger", "hunter",
            "warrior", "paladin", "knight", "crusader", "templar", "berserker", "barbarian",
            "monk", "priest", "cleric", "druid", "shaman", "mage", "wizard", "sorcerer",
            "warlock", "necromancer", "bard", "dancer", "singer", "musician", "artist",
            "craftsman", "blacksmith", "armorer", "weaponsmith", "enchanter", "alchemist",
            "herbalist", "cook", "fisher", "miner", "lumberjack", "farmer", "merchant",
            "trader", "banker", "noble", "peasant", "slave", "prisoner", "criminal",
            "outlaw", "bandit", "pirate", "mercenary", "soldier", "guard", "captain",
            "general", "commander", "admiral", "marshal", "king", "queen", "prince",
            "princess", "duke", "duchess", "earl", "count", "baron", "lord", "lady",
            "sir", "dame", "knight", "squire", "page", "herald", "ambassador", "diplomat",
        }),
        ("MdShopping", {"store", "cart", "bag", "shopping", "shop", "warehouse", "market", "mall", "basket", "pail"}),
        ("MdHealth", {
            "hospital", "medical", "medicine", "pharmacy", "doctor", "nurse", "patient", "clinic",
            "ambulance", "emergency", "surgery", "stethoscope", "pill", "bandage", "cross", "bio",
            "biohazard", "skull", "allergy", "wheelchair", "crutch", "cane", "brace", "splint",
            "cast", "syringe", "needle", "mask", "glove", "gown", "stretcher", "gurney",
        }),
        ("MdOffice", {
            "briefcase", "desk", "office", "filing", "cabinet", "drawer", "stapler", "paperclip",
            "binder", "folder", "envelope", "stamp", "fax", "copier", "shredder", "calculator",
            "calendar", "clock", "timer", "alarm", "schedule", "meeting", "conference", "presentation",
            "projector", "whiteboard", "blackboard", "chalkboard", "marker", "pen", "pencil",
            "eraser", "ruler", "scissors", "tape", "glue", "staple", "clip", "pin", "tack",
            "magnet", "board", "cork", "bulletin", "notice", "memo", "note", "pad", "notebook",
            "journal", "diary", "planner", "agenda", "organizer", "tray", "inbox", "outbox",
            "filing", "archive", "record", "document", "report", "invoice", "receipt", "quote",
            "estimate", "proposal", "contract", "agreement", "license", "permit", "certificate",
            "diploma", "degree", "award", "trophy", "medal", "ribbon", "badge", "pin",
            "nameplate", "sign", "label", "tag", "barcode", "qrcode", "newspaper",
        }),
        ("MdClothing", {
            "shirt", "pants", "dress", "skirt", "jacket", "coat", "suit", "tie", "hat", "shoe",
            "boot", "sandal", "slipper", "sneaker", "heel", "flat", "loafer", "oxford", "brogue",
            "derby", "monk", "strap", "mule", "clog", "espadrille", "flip", "flop", "croc",
            "wellie", "galosh", "hiking", "running", "training", "basketball", "football", "soccer",
            "tennis", "golf", "baseball", "hockey", "cricket", "rugby", "volleyball", "badminton",
            "ping", "pong", "squash", "racquetball", "handball", "lacrosse", "field", "ice",
            "inline", "skate", "snowboard", "sled", "kayak", "canoe", "raft", "sail", "row",
            "paddle", "kite", "windsurf", "wakeboard", "parasail", "paraglide", "hang", "glide",
            "skydive", "bungee", "climb", "boulder", "mountaineer", "trek", "camp", "backpack",
            "tent", "sleeping", "pad", "mat", "lantern", "headlamp", "telescope", "magnifier",
            "multitool", "axe", "hatchet", "saw", "shovel", "pick", "mallet", "crowbar", "pliers",
            "drill", "sander", "grinder", "router", "planer", "jointer", "lathe", "mill", "welder",
            "solder", "glue", "gun", "staple", "nail", "screw", "bolt", "nut", "washer", "rivet",
            "clip", "clamp", "vise", "anvil", "forge", "kiln", "furnace", "boiler", "duct", "vent",
            "register", "grille", "diffuser", "damper", "humidifier", "dehumidifier", "purifier",
            "ionizer", "ozone", "generator", "uv", "softener", "distiller", "dispenser", "fountain",
            "irrigation", "sprinkler", "drip", "hose", "nozzle", "controller", "attic", "basement",
            "crawl", "utility", "room", "mudroom", "pantry", "linen", "coat", "shoe", "workshop",
            "studio", "office", "den", "living", "dining", "kitchen", "bathroom", "bedroom",
            "nursery", "playroom", "game", "theater", "gym", "gazebo", "pergola", "arbor", "trellis",
            "driveway", "walkway", "path", "sidewalk", "curb", "gutter", "downspout", "drain",
            "sewer", "septic", "well", "pump", "tank", "heater", "ac", "heat", "blinds", "ceiling",
            "air", "pail", "greenhouse", "beach", "barn", "barrel", "dock", "mailbox", "motion",
            "seat", "smart", "ticket", "weight", "zodiac", "advertisements", "all", "allergy",
            "angle", "aspect", "assistant", "at", "auto", "av", "backburger", "cached", "ceiling",
            "decimal", "details", "dots", "ev", "gift", "head", "incognito", "motion", "pail",
            "ray", "seat", "selection", "smart", "swap", "ticket", "tooltip", "tray", "weight",
            "zodiac",
        }),
        ("MdTool", {
            "wrench", "hammer", "screwdriver", "pliers", "drill", "saw", "axe", "hatchet",
            "shovel", "pick", "mallet", "crowbar", "clamp", "vise", "anvil", "forge", "kiln",
            "furnace", "boiler", "duct", "vent", "register", "grille", "diffuser", "damper",
            "humidifier", "dehumidifier", "purifier", "ionizer", "ozone", "generator", "uv",
            "softener", "distiller", "dispenser", "fountain", "irrigation", "sprinkler", "drip",
            "hose", "nozzle", "controller", "attic", "basement", "crawl", "utility", "room",
            "mudroom", "pantry", "linen", "coat", "shoe", "workshop", "studio", "office", "den",
            "living", "dining", "kitchen", "bathroom", "bedroom", "nursery", "playroom", "game",
            "theater", "gym", "gazebo", "pergola", "arbor", "trellis", "driveway", "walkway",
            "path", "sidewalk", "curb", "gutter", "downspout", "drain", "sewer", "septic", "well",
            "pump", "tank", "heater", "ac", "heat", "blinds", "ceiling", "air", "pail", "greenhouse",
            "beach", "barn", "barrel", "dock", "mailbox", "motion", "seat", "smart", "ticket",
            "weight", "zodiac", "advertisements", "all", "allergy", "angle", "aspect", "assistant",
            "at", "auto", "av", "backburger", "cached", "ceiling", "decimal", "details", "dots",
            "ev", "gift", "head", "incognito", "motion", "pail", "ray", "seat", "selection",
            "smart", "swap", "ticket", "tooltip", "tray", "weight", "zodiac", "multitool",
            "solder", "glue", "gun", "staple", "nail", "screw", "bolt", "nut", "washer", "rivet",
            "clip", "clamp", "vise", "anvil", "forge", "kiln", "furnace", "boiler", "duct", "vent",
            "register", "grille", "diffuser", "damper", "humidifier", "dehumidifier", "purifier",
            "ionizer", "ozone", "generator", "uv", "softener", "distiller", "dispenser", "fountain",
            "irrigation", "sprinkler", "drip", "hose", "nozzle", "controller", "attic", "basement",
            "crawl", "utility", "room", "mudroom", "pantry", "linen", "coat", "shoe", "workshop",
            "studio", "office", "den", "living", "dining", "kitchen", "bathroom", "bedroom",
            "nursery", "playroom", "game", "theater", "gym", "gazebo", "pergola", "arbor", "trellis",
            "driveway", "walkway", "path", "sidewalk", "curb", "gutter", "downspout", "drain",
            "sewer", "septic", "well", "pump", "tank", "heater", "ac", "heat", "blinds", "ceiling",
            "air", "pail", "greenhouse", "beach", "barn", "barrel", "dock", "mailbox", "motion",
            "seat", "smart", "ticket", "weight", "zodiac", "advertisements", "all", "allergy",
            "angle", "aspect", "assistant", "at", "auto", "av", "backburger", "cached", "ceiling",
            "decimal", "details", "dots", "ev", "gift", "head", "incognito", "motion", "pail",
            "ray", "seat", "selection", "smart", "swap", "ticket", "tooltip", "tray", "weight",
            "zodiac",
        }),
        ("MdEducation", {
            "school", "university", "college", "student", "teacher", "professor", "staff",
            "graduation", "diploma", "degree", "certificate", "award", "trophy", "medal", "ribbon",
            "badge", "pin", "nameplate", "sign", "label", "tag", "barcode", "qrcode", "book",
            "notebook", "journal", "diary", "planner", "agenda", "organizer", "tray", "inbox",
            "outbox", "filing", "archive", "record", "document", "report", "invoice", "receipt",
            "quote", "estimate", "proposal", "contract", "agreement", "license", "permit",
        }),
        ("MdAnimal", {
            "cat", "dog", "bird", "fish", "rabbit", "cow", "pig", "horse", "spider", "bee",
            "butterfly", "snail", "turtle", "elephant", "panda", "penguin", "owl", "bat", "shark",
            "whale", "dolphin", "kangaroo", "koala", "monkey", "snake", "ladybug", "alien",
            "ghost", "skull", "bat", "cat", "dog", "bird", "fish", "rabbit", "cow", "pig",
            "horse", "spider", "bee", "butterfly", "snail", "turtle", "elephant", "panda",
            "penguin", "owl", "bat", "shark", "whale", "dolphin", "kangaroo", "koala", "monkey",
            "snake", "ladybug",
        }),
        ("MdBuilding", {
            "building", "house", "home", "office", "factory", "warehouse", "store", "shop",
            "mall", "market", "hospital", "school", "university", "college", "church", "mosque",
            "synagogue", "temple", "castle", "fortress", "tower", "bridge", "tunnel", "dam",
            "lighthouse", "windmill", "watermill", "barn", "silo", "greenhouse", "gazebo", "pergola",
            "arbor", "trellis", "fence", "gate", "door", "window", "wall", "roof", "chimney",
            "balcony", "terrace", "patio", "deck", "porch", "veranda", "garage", "shed", "cabin",
            "cottage", "mansion", "villa", "apartment", "condo", "townhouse", "duplex", "triplex",
            "skyscraper", "highrise", "lowrise", "midrise", "tower", "spire", "dome", "arch",
            "column", "pillar", "beam", "truss", "frame", "foundation", "basement", "attic",
            "loft", "penthouse", "studio", "loft", "flat", "unit", "suite", "room", "hall",
            "lobby", "foyer", "vestibule", "corridor", "hallway", "staircase", "stair", "step",
            "ramp", "elevator", "escalator", "lift", "platform", "stage", "arena", "stadium",
            "auditorium", "theater", "cinema", "museum", "gallery", "library", "archive",
            "laboratory", "clinic", "pharmacy", "restaurant", "cafe", "bar", "pub", "club",
            "hotel", "motel", "inn", "resort", "spa", "gym", "fitness", "pool", "sauna",
            "jacuzzi", "bathhouse", "laundry", "drycleaner", "salon", "barbershop", "beauty",
            "nail", "tattoo", "piercing", "jewelry", "watch", "clock", "repair", "shoe", "tailor",
            "drycleaner", "laundromat", "carwash", "gas", "station", "parking", "garage", "lot",
            "deck", "ramp", "dock", "pier", "wharf", "marina", "harbor", "port", "airport",
            "terminal", "hangar", "runway", "taxiway", "control", "tower", "radar", "beacon",
            "lighthouse", "buoy", "mooring", "anchor", "chain", "rope", "cable", "wire",
            "pole", "post", "sign", "billboard", "banner", "flag", "pennant", "bunting",
            "garland", "wreath", "ornament", "decoration", "light", "lamp", "fixture", "chandelier",
            "sconce", "spotlight", "floodlight", "streetlight", "traffic", "signal", "crosswalk",
            "stop", "yield", "speed", "limit", "bumper", "barrier", "guardrail", "median",
            "shoulder", "lane", "marking", "sign", "post", "mile", "marker", "exit", "ramp",
            "overpass", "underpass", "interchange", "roundabout", "rotary", "traffic", "circle",
            "plaza", "square", "park", "garden", "playground", "field", "court", "pitch",
            "track", "course", "trail", "path", "walkway", "sidewalk", "bike", "lane", "bus",
            "stop", "shelter", "bench", "fountain", "statue", "monument", "memorial", "plaque",
            "grave", "tomb", "mausoleum", "cemetery", "churchyard", "graveyard", "necropolis",
            "catacomb", "crypt", "tomb", "sarcophagus", "coffin", "urn", "ashes", "cremation",
            "burial", "funeral", "wake", "memorial", "service", "ceremony", "ritual", "tradition",
            "custom", "culture", "heritage", "history", "legacy", "ancestry", "genealogy", "family",
            "tree", "lineage", "descent", "origin", "roots", "homeland", "motherland", "fatherland",
            "nation", "country", "state", "province", "region", "district", "county", "city",
            "town", "village", "hamlet", "settlement", "colony", "outpost", "frontier", "border",
            "boundary", "frontier", "territory", "domain", "realm", "kingdom", "empire", "republic",
            "democracy", "monarchy", "dictatorship", "theocracy", "anarchy", "revolution",
            "rebellion", "uprising", "coup", "invasion", "war", "battle", "siege", "skirmish",
            "ambush", "raid", "patrol", "scout", "spy", "assassin", "thief", "rogue", "ranger",
            "hunter", "warrior", "paladin", "knight", "crusader", "templar", "berserker",
            "barbarian", "monk", "priest", "cleric", "druid", "shaman", "mage", "wizard",
            "sorcerer", "warlock", "necromancer", "bard", "dancer", "singer", "musician",
            "artist", "craftsman", "blacksmith", "armorer", "weaponsmith", "enchanter",
            "alchemist", "herbalist", "cook", "fisher", "miner", "lumberjack", "farmer",
            "merchant", "trader", "banker", "noble", "peasant", "slave", "prisoner", "criminal",
            "outlaw", "bandit", "pirate", "mercenary", "soldier", "guard", "captain", "general",
            "commander", "admiral", "marshal", "king", "queen", "prince", "princess", "duke",
            "duchess", "earl", "count", "baron", "lord", "lady", "sir", "dame", "knight",
            "squire", "page", "herald", "ambassador", "diplomat",
        }),
        ("MdReligion", {
            "cross", "church", "mosque", "synagogue", "temple", "star", "david", "crescent",
            "om", "yin", "yang", "ankh", "dharma", "wheel", "lotus", "prayer", "beads", "rosary",
            "bible", "quran", "torah", "tanakh", "gospel", "psalm", "hymn", "chant", "meditation",
            "zen", "karma", "nirvana", "enlightenment", "soul", "spirit", "angel", "demon",
            "heaven", "hell", "purgatory", "limbo", "paradise", "eden", "garden", "ark", "covenant",
            "commandment", "prophet", "saint", "martyr", "pilgrim", "pilgrimage", "shrine",
            "altar", "pulpit", "pew", "baptism", "communion", "eucharist", "confession", "penance",
            "absolution", "blessing", "anointing", "ordination", "consecration", "canonization",
            "beatification", "relic", "icon", "fresco", "mosaic", "stained", "glass", "bell",
            "tower", "dome", "minaret", "spire", "steeple", "nave", "apse", "transept", "choir",
            "vestry", "sacristy", "cloister", "monastery", "convent", "abbey", "cathedral",
            "basilica", "chapel", "oratory", "tabernacle", "sanctuary", "holy", "sacred",
            "divine", "blessed", "sacred", "holy", "divine", "blessed", "sacred", "holy",
        }),
        ("MdEmoji", {
            "emoticon", "face", "smile", "frown", "laugh", "cry", "angry", "happy", "sad",
            "surprised", "confused", "bored", "tired", "sleep", "eat", "drink", "sit", "stand",
            "lie", "kneel", "crawl", "roll", "dodge", "block", "parry", "counter", "attack",
            "defend", "heal", "buff", "debuff", "cast", "channel", "charge", "aim", "shoot",
            "reload", "switch", "weapon", "melee", "ranged", "magic", "physical", "elemental",
            "fire", "ice", "lightning", "earth", "wind", "water", "light", "dark", "holy",
            "shadow", "arcane", "nature", "poison", "bleed", "disease", "curse", "blessing",
            "summon", "transform", "polymorph", "metamorphosis", "evolve", "devolve", "ascend",
            "descend", "rebirth", "resurrection", "revive", "respawn", "ghost", "spirit", "soul",
            "undead", "zombie", "skeleton", "vampire", "werewolf", "demon", "angel", "dragon",
            "phoenix", "griffin", "unicorn", "pegasus", "centaur", "minotaur", "medusa", "hydra",
            "chimera", "kraken", "leviathan", "behemoth", "golem", "elemental", "construct",
            "automaton", "robot", "android", "cyborg", "alien", "ufo", "spaceship", "planet",
            "star", "galaxy", "universe", "dimension", "portal", "gate", "wormhole", "teleport",
            "warp", "hyperspace", "subspace", "astral", "ethereal", "plane", "realm", "world",
            "zone", "area", "region", "continent", "island", "dungeon", "cave", "forest",
            "desert", "mountain", "ocean", "river", "lake", "swamp", "jungle", "tundra",
            "volcano", "ruins", "temple", "castle", "fortress", "tower", "village", "town",
            "city", "capital", "kingdom", "empire", "nation", "faction", "alliance", "horde",
            "republic", "democracy", "monarchy", "dictatorship", "theocracy", "anarchy",
            "revolution", "rebellion", "uprising", "coup", "invasion", "war", "battle", "siege",
            "skirmish", "ambush", "raid", "patrol", "scout", "spy", "assassin", "thief", "rogue",
            "ranger", "hunter", "warrior", "paladin", "knight", "crusader", "templar", "berserker",
            "barbarian", "monk", "priest", "cleric", "druid", "shaman", "mage", "wizard",
            "sorcerer", "warlock", "necromancer", "bard", "dancer", "singer", "musician", "artist",
            "craftsman", "blacksmith", "armorer", "weaponsmith", "enchanter", "alchemist",
            "herbalist", "cook", "fisher", "miner", "lumberjack", "farmer", "merchant", "trader",
            "banker", "noble", "peasant", "slave", "prisoner", "criminal", "outlaw", "bandit",
            "pirate", "mercenary", "soldier", "guard", "captain", "general", "commander", "admiral",
            "marshal", "king", "queen", "prince", "princess", "duke", "duchess", "earl", "count",
            "baron", "lord", "lady", "sir", "dame", "knight", "squire", "page", "herald",
            "ambassador", "diplomat", "heart", "skull", "ghost", "alien",
        }),
    ]

    for domain, segments in groups:
        for seg in segments:
            if seg not in BRAND_FIRST:
                assigned.setdefault(seg, domain)

    # Supplemental assignments from misc analysis
    supplemental: dict[str, str] = {
        "square": "MdShape", "align": "MdEditor", "roman": "MdEditor", "guitar": "MdMedia",
        "paw": "MdAnimal", "smoking": "MdFood", "stairs": "MdBuilding", "unfold": "MdEditor",
        "waves": "MdWeather", "transmission": "MdTransport", "signature": "MdEditor",
        "syllabary": "MdEditor", "lotion": "MdHealth", "contactless": "MdDevice",
        "paper": "MdOffice", "radiator": "MdHome", "washing": "MdHome", "tumble": "MdHome",
        "contain": "MdAction", "dishwasher": "MdHome", "white": "MdShape", "black": "MdShape",
        "numeric": "MdEditor", "decimal": "MdScience", "abjad": "MdEditor", "abugida": "MdEditor",
        "syllabary": "MdEditor", "hieroglyph": "MdEditor", "braille": "MdEditor",
        "currency": "MdFinance", "bitcoin": "MdFinance", "ethereum": "MdFinance",
        "ethereum": "MdFinance", "litecoin": "MdFinance", "dogecoin": "MdFinance",
        "ripple": "MdFinance", "cardano": "MdFinance", "polkadot": "MdFinance",
        "solana": "MdFinance", "avalanche": "MdFinance", "polygon": "MdFinance",
        "uniswap": "MdFinance", "opensea": "MdFinance", "metamask": "MdFinance",
        "ledger": "MdFinance", "trezor": "MdFinance", "nvidia": "MdMisc", "amd": "MdMisc",
        "intel": "MdMisc", "qualcomm": "MdMisc", "broadcom": "MdMisc", "cisco": "MdMisc",
        "juniper": "MdMisc", "arista": "MdMisc", "paloalto": "MdMisc", "fortinet": "MdMisc",
        "checkpoint": "MdMisc", "crowdstrike": "MdMisc", "sentinelone": "MdMisc",
        "splunk": "MdMisc", "datadog": "MdMisc", "newrelic": "MdMisc", "grafana": "MdMisc",
        "prometheus": "MdMisc", "elastic": "MdMisc", "mongodb": "MdMisc", "redis": "MdMisc",
        "postgresql": "MdMisc", "mysql": "MdMisc", "oracle": "MdMisc", "ibm": "MdMisc",
        "hp": "MdMisc", "dell": "MdMisc", "lenovo": "MdMisc", "asus": "MdMisc", "acer": "MdMisc",
        "msi": "MdMisc", "gigabyte": "MdMisc", "razer": "MdMisc", "logitech": "MdMisc",
        "corsair": "MdMisc", "steelseries": "MdMisc", "hyperx": "MdMisc", "roccat": "MdMisc",
        "benq": "MdMisc", "lg": "MdMisc", "samsung": "MdMisc", "sony": "MdMisc",
        "panasonic": "MdMisc", "philips": "MdMisc", "siemens": "MdMisc", "bosch": "MdMisc",
        "ge": "MdMisc", "honeywell": "MdMisc", "schneider": "MdMisc", "abb": "MdMisc",
        "emerson": "MdMisc", "rockwell": "MdMisc", "omron": "MdMisc", "fanuc": "MdMisc",
        "kuka": "MdMisc", "tesla": "MdMisc", "ford": "MdMisc", "gm": "MdMisc", "toyota": "MdMisc",
        "honda": "MdMisc", "nissan": "MdMisc", "bmw": "MdMisc", "mercedes": "MdMisc",
        "audi": "MdMisc", "volkswagen": "MdMisc", "porsche": "MdMisc", "ferrari": "MdMisc",
        "lamborghini": "MdMisc", "mclaren": "MdMisc", "bugatti": "MdMisc", "rollsroyce": "MdMisc",
        "bentley": "MdMisc", "astonmartin": "MdMisc", "jaguar": "MdMisc", "landrover": "MdMisc",
        "volvo": "MdMisc", "saab": "MdMisc", "subaru": "MdMisc", "mazda": "MdMisc",
        "hyundai": "MdMisc", "kia": "MdMisc", "genesis": "MdMisc", "rivian": "MdMisc",
        "lucid": "MdMisc", "nio": "MdMisc", "xpeng": "MdMisc", "byd": "MdMisc", "geely": "MdMisc",
        "greatwall": "MdMisc", "chery": "MdMisc", "haval": "MdMisc", "mg": "MdMisc",
        "rover": "MdMisc", "mini": "MdMisc", "fiat": "MdMisc", "alfa": "MdMisc", "lancia": "MdMisc",
        "maserati": "MdMisc", "pagani": "MdMisc", "koenigsegg": "MdMisc", "rimac": "MdMisc",
        "pininfarina": "MdMisc", "italdesign": "MdMisc", "bertone": "MdMisc", "ghia": "MdMisc",
        "zagato": "MdMisc", "touringsuperleggera": "MdMisc", "carrozzeria": "MdMisc",
        "pinin": "MdMisc", "farina": "MdMisc", "nintendo": "MdMisc", "playstation": "MdMisc",
        "xbox": "MdMisc", "steam": "MdMisc", "epic": "MdMisc", "unity": "MdMisc",
        "unreal": "MdMisc", "blender": "MdMisc", "autodesk": "MdMisc", "sketchup": "MdMisc",
        "solidworks": "MdMisc", "matlab": "MdMisc", "wolfram": "MdMisc", "mathworks": "MdMisc",
        "ansys": "MdMisc", "cadence": "MdMisc", "synopsys": "MdMisc", "xilinx": "MdMisc",
        "altera": "MdMisc", "arm": "MdMisc", "qualcomm": "MdMisc", "broadcom": "MdMisc",
        "cisco": "MdMisc", "juniper": "MdMisc", "oracle": "MdMisc", "sap": "MdMisc",
        "vmware": "MdMisc", "citrix": "MdMisc", "redhat": "MdMisc", "centos": "MdMisc",
        "archlinux": "MdMisc", "opensuse": "MdMisc", "gentoo": "MdMisc", "freebsd": "MdMisc",
        "netbsd": "MdMisc", "openbsd": "MdMisc", "slackware": "MdMisc", "alpinelinux": "MdMisc",
        "nixos": "MdMisc", "manjaro": "MdMisc", "elementary": "MdMisc", "popos": "MdMisc",
        "zorin": "MdMisc", "mint": "MdMisc", "kde": "MdMisc", "gnome": "MdMisc", "xfce": "MdMisc",
        "lxde": "MdMisc", "mate": "MdMisc", "cinnamon": "MdMisc", "budgie": "MdMisc",
        "pantheon": "MdMisc", "deepin": "MdMisc", "solus": "MdMisc", "void": "MdMisc",
        "artix": "MdMisc", "endeavouros": "MdMisc", "garuda": "MdMisc", "arcolinux": "MdMisc",
        "endeavour": "MdMisc", "manjaro": "MdMisc", "endeavour": "MdMisc", "endeavour": "MdMisc",
        "endeavour": "MdMisc", "binoculars": "MdMisc", "bio": "MdScience", "biohazard": "MdHealth",
        "bowling": "MdSport", "broom": "MdTool", "castle": "MdBuilding", "cat": "MdAnimal",
        "church": "MdReligion", "counter": "MdEditor", "deskphone": "MdDevice", "details": "MdEditor",
        "elevator": "MdTransport", "escalator": "MdTransport", "feather": "MdEditor",
        "feather": "MdEditor", "filmstrip": "MdMedia", "fire": "MdNature", "fireplace": "MdHome",
        "firework": "MdNature", "fishbowl": "MdAnimal", "flash": "MdAction", "fountain": "MdNature",
        "fountain": "MdNature", "fuse": "MdDevice", "gavel": "MdOffice", "gender": "MdAccount",
        "gesture": "MdAction", "gift": "MdShopping", "glass": "MdHome", "glasses": "MdClothing",
        "guitar": "MdMedia", "hanger": "MdClothing", "hazard": "MdAlert", "headphones": "MdMedia",
        "heart": "MdEmoji", "help": "MdAlert", "hexagon": "MdShape", "hockey": "MdSport",
        "home": "MdHome", "horse": "MdAnimal", "hospital": "MdHealth", "hourglass": "MdTime",
        "human": "MdAccount", "ice": "MdWeather", "image": "MdMedia", "incognito": "MdSecurity",
        "infinity": "MdScience", "information": "MdAlert", "iron": "MdHome", "key": "MdSecurity",
        "keyboard": "MdDevice", "kite": "MdSport", "knife": "MdHome", "label": "MdEditor",
        "ladder": "MdTool", "lamp": "MdHome", "lantern": "MdHome", "laptop": "MdDevice",
        "laser": "MdScience", "leaf": "MdNature", "leak": "MdAlert", "library": "MdFile",
        "lightbulb": "MdAction", "lightning": "MdWeather", "link": "MdCommunication",
        "lock": "MdSecurity", "magnify": "MdEditor", "mailbox": "MdCommunication", "map": "MdMap",
        "marker": "MdEditor", "medal": "MdSport", "medical": "MdHealth", "memory": "MdDevice",
        "menu": "MdEditor", "message": "MdCommunication", "microphone": "MdMedia", "microscope": "MdScience",
        "minus": "MdEditor", "mirror": "MdHome", "monitor": "MdDevice", "moon": "MdWeather",
        "mosque": "MdReligion", "motion": "MdAction", "motorcycle": "MdTransport", "mouse": "MdDevice",
        "movie": "MdMedia", "music": "MdMedia", "nail": "MdTool", "nature": "MdNature",
        "navigation": "MdNavigation", "network": "MdDevice", "newspaper": "MdOffice", "nfc": "MdDevice",
        "note": "MdFile", "notebook": "MdFile", "notification": "MdCommunication", "numeric": "MdEditor",
        "octagon": "MdShape", "office": "MdOffice", "oil": "MdFood", "olive": "MdFood",
        "omega": "MdScience", "open": "MdAction", "orbit": "MdScience", "order": "MdEditor",
        "outlet": "MdDevice", "package": "MdFile", "page": "MdFile", "paint": "MdEditor",
        "palette": "MdEditor", "pan": "MdFood", "paperclip": "MdFile", "parachute": "MdSport",
        "park": "MdNature", "parking": "MdTransport", "party": "MdEmoji", "passport": "MdAccount",
        "pause": "MdMedia", "paw": "MdAnimal", "pen": "MdEditor", "pencil": "MdEditor",
        "percent": "MdScience", "phone": "MdCommunication", "photo": "MdMedia", "picture": "MdMedia",
        "pill": "MdHealth", "pin": "MdNavigation", "pipe": "MdHome", "pizza": "MdFood",
        "plane": "MdTransport", "planet": "MdScience", "play": "MdMedia", "playlist": "MdMedia",
        "plug": "MdDevice", "plus": "MdEditor", "podcast": "MdMedia", "poke": "MdFood",
        "police": "MdSecurity", "poll": "MdCommunication", "pool": "MdHome", "post": "MdFile",
        "pot": "MdFood", "power": "MdAction", "presentation": "MdOffice", "printer": "MdDevice",
        "progress": "MdAlert", "projector": "MdMedia", "puzzle": "MdGame", "qrcode": "MdDevice",
        "question": "MdAlert", "radio": "MdMedia", "rainbow": "MdWeather", "rake": "MdNature",
        "receipt": "MdFinance", "record": "MdMedia", "recycle": "MdNature", "refresh": "MdAction",
        "relation": "MdEditor", "remote": "MdDevice", "repeat": "MdMedia", "reply": "MdCommunication",
        "report": "MdOffice", "resize": "MdEditor", "restaurant": "MdFood", "rewind": "MdMedia",
        "ribbon": "MdSport", "ring": "MdShape", "robot": "MdDevice", "rocket": "MdTransport",
        "rotate": "MdEditor", "router": "MdDevice", "rss": "MdCommunication", "ruler": "MdEditor",
        "run": "MdSport", "sail": "MdTransport", "satellite": "MdDevice", "save": "MdEditor",
        "scale": "MdDevice", "scanner": "MdDevice", "school": "MdEducation", "scissors": "MdEditor",
        "scooter": "MdTransport", "scoreboard": "MdGame", "screw": "MdTool", "script": "MdEditor",
        "scroll": "MdFile", "search": "MdAction", "seat": "MdTransport", "security": "MdSecurity",
        "select": "MdEditor", "selection": "MdEditor", "send": "MdAction", "server": "MdDevice",
        "set": "MdEditor", "settings": "MdEditor", "shape": "MdShape", "share": "MdAction",
        "shield": "MdSecurity", "ship": "MdTransport", "shirt": "MdClothing", "shoe": "MdClothing",
        "shop": "MdShopping", "shopping": "MdShopping", "shovel": "MdTool", "shower": "MdHome",
        "shuffle": "MdMedia", "sign": "MdNavigation", "signal": "MdDevice", "signature": "MdEditor",
        "sim": "MdDevice", "sine": "MdScience", "sitemap": "MdOffice", "skate": "MdSport",
        "skateboard": "MdSport", "ski": "MdSport", "skip": "MdMedia", "skull": "MdEmoji",
        "sleep": "MdEmoji", "slide": "MdOffice", "smoke": "MdWeather", "smoking": "MdFood",
        "snowflake": "MdNature", "soccer": "MdSport", "socket": "MdDevice", "sofa": "MdHome",
        "solar": "MdDevice", "sort": "MdEditor", "sound": "MdMedia", "source": "MdFile",
        "spa": "MdHome", "sparkles": "MdEmoji", "speaker": "MdMedia", "speedometer": "MdTransport",
        "spellcheck": "MdEditor", "spider": "MdAnimal", "spirit": "MdEmoji", "spoon": "MdHome",
        "sport": "MdSport", "spotify": "MdMisc", "spray": "MdHome", "sprout": "MdNature",
        "square": "MdShape", "stadium": "MdSport", "stage": "MdBuilding", "stairs": "MdBuilding",
        "stamp": "MdOffice", "star": "MdShape", "station": "MdTransport", "stethoscope": "MdHealth",
        "sticker": "MdFile", "stop": "MdMedia", "stopwatch": "MdTime", "store": "MdShopping",
        "stove": "MdHome", "strategy": "MdGame", "stream": "MdMedia", "stroller": "MdTransport",
        "subway": "MdTransport", "sun": "MdWeather", "sunglasses": "MdClothing", "surfing": "MdSport",
        "surround": "MdMedia", "swap": "MdAction", "swim": "MdSport", "switch": "MdEditor",
        "sword": "MdGame", "sync": "MdAction", "syringe": "MdHealth", "table": "MdEditor",
        "tablet": "MdDevice", "tag": "MdEditor", "tank": "MdTransport", "tap": "MdAction",
        "tape": "MdTool", "target": "MdNavigation", "taxi": "MdTransport", "tea": "MdFood",
        "teach": "MdEducation", "telescope": "MdScience", "television": "MdMedia", "temperature": "MdWeather",
        "tennis": "MdSport", "tent": "MdNature", "terminal": "MdDevice", "terrain": "MdMap",
        "test": "MdEditor", "text": "MdEditor", "theater": "MdBuilding", "thermometer": "MdWeather",
        "thought": "MdEmoji", "thumb": "MdCommunication", "ticket": "MdTransport", "tie": "MdClothing",
        "timer": "MdTime", "tire": "MdTransport", "toaster": "MdHome", "toggle": "MdEditor",
        "toilet": "MdHome", "tool": "MdTool", "toolbox": "MdTool", "tooth": "MdHealth",
        "top": "MdNavigation", "torch": "MdDevice", "tornado": "MdWeather", "tower": "MdBuilding",
        "toy": "MdGame", "track": "MdSport", "tractor": "MdNature", "traffic": "MdTransport",
        "train": "MdTransport", "tram": "MdTransport", "transfer": "MdFinance", "translate": "MdEditor",
        "trash": "MdEditor", "travel": "MdTransport", "tree": "MdNature", "trending": "MdFinance",
        "triangle": "MdShape", "trophy": "MdSport", "truck": "MdTransport", "trumpet": "MdMedia",
        "tshirt": "MdClothing", "tune": "MdEditor", "tunnel": "MdTransport", "turkey": "MdFood",
        "turn": "MdNavigation", "tv": "MdMedia", "typewriter": "MdOffice", "umbrella": "MdWeather",
        "undo": "MdEditor", "unfold": "MdEditor", "university": "MdEducation", "unlock": "MdSecurity",
        "upload": "MdAction", "usb": "MdDevice", "vacuum": "MdHome", "van": "MdTransport",
        "vault": "MdSecurity", "vector": "MdEditor", "video": "MdMedia", "view": "MdEditor",
        "violin": "MdMedia", "virus": "MdScience", "visibility": "MdAction", "voice": "MdCommunication",
        "volcano": "MdNature", "volleyball": "MdSport", "volume": "MdMedia", "vote": "MdCommunication",
        "vpn": "MdSecurity", "walk": "MdSport", "wall": "MdHome", "wallet": "MdFinance",
        "warehouse": "MdShopping", "warning": "MdAlert", "wash": "MdHome", "washing": "MdHome",
        "watch": "MdDevice", "water": "MdNature", "waves": "MdWeather", "weather": "MdWeather",
        "web": "MdCommunication", "webcam": "MdDevice", "weight": "MdSport", "wheel": "MdTransport",
        "wheelchair": "MdAccount", "whistle": "MdSport", "white": "MdShape", "wifi": "MdDevice",
        "wind": "MdWeather", "window": "MdEditor", "wine": "MdFood", "wing": "MdTransport",
        "wrench": "MdTool", "wrist": "MdDevice", "write": "MdEditor", "xray": "MdHealth",
        "yoga": "MdSport", "youtube": "MdMisc", "zodiac": "MdScience", "zone": "MdMap",
        "zoom": "MdNavigation",
    }
    for seg, domain in supplemental.items():
        if seg not in BRAND_FIRST:
            assigned[seg] = domain

    def classify_fallback(seg: str) -> str:
        """Keyword fallback for unassigned non-brand segments."""
        rules = [
            ("MdShape", ("square", "circle", "triangle", "pentagon", "octagram", "decagram", "hexagon", "diameter", "tilde")),
            ("MdScience", ("zodiac", "exponent", "greater", "less", "math", "formula", "atom", "dna", "virus")),
            ("MdEditor", ("align", "roman", "drawing", "transcribe", "reorder", "subdirectory", "reflect", "closed", "material", "widgets", "directions", "routes", "elevation")),
            ("MdDevice", ("bluetooth", "camcorder", "vibrate", "cctv", "midi", "current", "transition")),
            ("MdMedia", ("piano", "metronome", "magazine")),
            ("MdFood", ("sausage", "cannabis", "mortar")),
            ("MdHealth", ("bone", "toothbrush", "death")),
            ("MdSport", ("rollerblade", "bullseye")),
            ("MdHome", ("doorbell", "bunk", "storefront", "vanish")),
            ("MdShopping", ("sale", "storefront")),
            ("MdSecurity", ("seal", "cctv")),
            ("MdTransport", ("routes", "directions")),
            ("MdGame", ("bomb", "dice")),
            ("MdAlert", ("priority", "bullseye")),
            ("MdCommunication", ("contacts", "thumbs", "skype")),
            ("MdAnimal", ("paw", "bone")),
            ("MdBuilding", ("elevation", "storefront", "bunk")),
            ("MdClothing", ("rollerblade",)),
            ("MdTool", ("mortar", "drawing")),
            ("MdOffice", ("magazine", "drawing")),
            ("MdNature", ("cannabis",)),
            ("MdEmoji", ("death", "bomb")),
            ("MdAction", ("reorder", "reflect", "vanish", "transition")),
            ("MdNavigation", ("directions", "routes", "elevation", "bullseye")),
            ("MdFinance", ("sale",)),
            ("MdReligion", ("death",)),
        ]
        for domain, keywords in rules:
            if seg in keywords or any(k in seg for k in keywords):
                return domain
        # Substring heuristics
        if any(x in seg for x in ("food", "eat", "drink", "meal", "cook", "bake", "grill")):
            return "MdFood"
        if any(x in seg for x in ("car", "bus", "train", "plane", "ship", "bike", "road", "drive")):
            return "MdTransport"
        if any(x in seg for x in ("medical", "health", "hospital", "doctor", "pill", "drug")):
            return "MdHealth"
        if any(x in seg for x in ("game", "play", "dice", "chess", "puzzle", "toy")):
            return "MdGame"
        if any(x in seg for x in ("shop", "cart", "store", "buy", "sell", "price")):
            return "MdShopping"
        if any(x in seg for x in ("school", "learn", "teach", "book", "study")):
            return "MdEducation"
        if any(x in seg for x in ("cat", "dog", "bird", "fish", "animal", "paw")):
            return "MdAnimal"
        if any(x in seg for x in ("build", "house", "home", "office", "tower", "castle")):
            return "MdBuilding"
        if any(x in seg for x in ("cross", "church", "mosque", "temple", "pray")):
            return "MdReligion"
        if any(x in seg for x in ("face", "smile", "emoticon", "emoji", "happy", "sad")):
            return "MdEmoji"
        if any(x in seg for x in ("tree", "flower", "leaf", "plant", "nature", "forest")):
            return "MdNature"
        if any(x in seg for x in ("sport", "ball", "golf", "tennis", "soccer", "hockey")):
            return "MdSport"
        if any(x in seg for x in ("tool", "wrench", "hammer", "drill", "saw", "screw")):
            return "MdTool"
        if any(x in seg for x in ("weather", "cloud", "rain", "snow", "sun", "wind")):
            return "MdWeather"
        if any(x in seg for x in ("lock", "key", "shield", "secure", "password")):
            return "MdSecurity"
        if any(x in seg for x in ("money", "cash", "bank", "credit", "pay", "coin")):
            return "MdFinance"
        if any(x in seg for x in ("clock", "time", "calendar", "timer", "alarm")):
            return "MdTime"
        if any(x in seg for x in ("user", "account", "person", "people", "human", "face")):
            return "MdAccount"
        if any(x in seg for x in ("message", "chat", "mail", "phone", "call", "web")):
            return "MdCommunication"
        if any(x in seg for x in ("video", "music", "audio", "film", "camera", "image")):
            return "MdMedia"
        if any(x in seg for x in ("arrow", "nav", "map", "location", "pin", "compass")):
            return "MdNavigation"
        if any(x in seg for x in ("file", "folder", "document", "archive", "book")):
            return "MdFile"
        if any(x in seg for x in ("edit", "format", "text", "font", "code", "draw")):
            return "MdEditor"
        if any(x in seg for x in ("device", "computer", "phone", "tablet", "wifi", "network")):
            return "MdDevice"
        if any(x in seg for x in ("home", "house", "room", "kitchen", "bath", "bed")):
            return "MdHome"
        if any(x in seg for x in ("alert", "warn", "error", "info", "bell", "notif")):
            return "MdAlert"
        if any(x in seg for x in ("battery", "charge", "power")):
            return "MdBattery"
        if any(x in seg for x in ("database", "sql", "data")):
            return "MdDatabase"
        if any(x in seg for x in ("shirt", "pant", "dress", "shoe", "hat", "cloth")):
            return "MdClothing"
        if any(x in seg for x in ("flask", "beaker", "science", "lab", "atom", "molecule")):
            return "MdScience"
        return "MdOffice"  # sensible default for unclassified office/misc icons

    # Assign any remaining non-brand segments via fallback
    for seg in first_segments:
        if seg not in BRAND_FIRST and seg not in assigned:
            assigned[seg] = classify_fallback(seg)

    # Final safety: anything still unassigned (shouldn't happen) -> MdMisc
    for seg in first_segments:
        if seg not in BRAND_FIRST and seg not in assigned:
            assigned[seg] = "MdMisc"

    # Build prefix rules in domain order
    by_domain: dict[str, set[str]] = defaultdict(set)
    for seg, domain in assigned.items():
        by_domain[domain].add(seg)

    rules: list[tuple[str, frozenset[str]]] = []
    for domain in MD_DOMAIN_ORDER:
        if domain in by_domain and by_domain[domain]:
            rules.append((domain, frozenset(sorted(by_domain[domain]))))
    return rules


COMPOUND_LOOKUP: dict[tuple[str, str], str] = {}
for first, second, domain in MD_COMPOUND_RULES:
    COMPOUND_LOOKUP[(first, second)] = domain


def domain_for_suffix(suffix: str, prefix_rules: list[tuple[str, frozenset[str]]]) -> str:
    parts = suffix.split("_")
    first = parts[0]
    if first in BRAND_FIRST:
        return "MdMisc"
    for key, domain in CROSS_OVERRIDES.items():
        if first == key:
            return domain
    if len(parts) >= 2:
        compound = COMPOUND_LOOKUP.get((first, parts[1]))
        if compound:
            return compound
    for domain, prefixes in prefix_rules:
        if first in prefixes:
            return domain
    return "MdMisc"


def category_for(domain: str, suffix: str) -> str:
    parts = suffix.split("_")
    first = parts[0]
    second = parts[1] if len(parts) > 1 else ""

    if domain == "MdFile":
        if first in {"archive"}: return "Archive"
        if first in {"attachment", "paperclip"}: return "Attachment"
        if first in {"book", "bookshelf"}: return "Book"
        if first in {"clipboard"}: return "Clipboard"
        if first in {"content", "document"}: return "Document"
        if first in {"file"}: return "File"
        if first in {"folder"}: return "Folder"
        if first in {"library"}: return "Library"
        if first in {"note", "notebook"}: return "Note"
        if first in {"package", "zip"}: return "Package"
        if first in {"page", "tab", "post"}: return "Page"
        if first in {"source"}: return "Source"
        return "Misc"
    if domain == "MdEditor":
        if first in {"format", "text", "alpha", "numeric"}: return "Format"
        if first in {"code", "xml", "html", "css", "script"}: return "Code"
        if first in {"draw", "vector", "bezier", "curve", "polygon"}: return "Draw"
        if first in {"table", "grid", "column", "row", "cell"}: return "Table"
        if first in {"color", "palette", "brush", "paint", "fill", "gradient"}: return "Color"
        if first in {"rotate", "flip", "mirror", "crop", "resize"}: return "Transform"
        if first in {"relation", "axis", "math", "decimal"}: return "Numeric"
        if first in {"source", "git", "branch", "commit"}: return "Source"
        return "Misc"
    if domain == "MdNavigation":
        if first in {"arrow"}: return "Arrow"
        if first in {"chevron", "caret"}: return "Chevron"
        if first in {"compass", "direction", "navigation"}: return "Direction"
        if first in {"marker", "location", "pin", "gps"}: return "Location"
        if first in {"map"}: return "Map"
        if first in {"transit", "route", "walk", "run"}: return "Transit"
        return "Misc"
    if domain == "MdMedia":
        if first in {"music", "audio", "sound", "volume", "speaker", "headphone"}: return "Audio"
        if first in {"movie", "video", "film", "clapperboard"}: return "Video"
        if first in {"image", "photo", "camera", "gallery", "picture"}: return "Image"
        if first in {"play", "pause", "stop", "record", "skip", "shuffle", "repeat"}: return "Playback"
        if first in {"broadcast", "cast", "stream", "podcast", "radio"}: return "Broadcast"
        return "Misc"
    if domain == "MdAction":
        if first in {"share", "send", "receive"}: return "Share"
        if first in {"search", "find", "replace", "filter"}: return "Search"
        if first in {"download", "upload", "sync", "refresh", "reload"}: return "Input"
        if first in {"login", "logout", "install", "upgrade", "restart"}: return "System"
        return "Misc"
    if domain == "MdDevice":
        if first in {"laptop", "desktop", "monitor", "display", "screen"}: return "Computer"
        if first in {"smartphone", "tablet", "cellphone", "mobile", "watch"}: return "Mobile"
        if first in {"wifi", "bluetooth", "network", "router", "server", "cloud"}: return "Network"
        if first in {"harddisk", "ssd", "sd", "usb", "database"}: return "Storage"
        if first in {"battery", "charging", "plug", "outlet"}: return "Power"
        if first in {"printer", "scanner", "keyboard", "mouse", "webcam"}: return "Peripheral"
        return "Misc"
    if domain == "MdCommunication":
        if first in {"message", "chat", "comment", "forum"}: return "Message"
        if first in {"email", "mail", "envelope", "inbox", "outbox"}: return "Mail"
        if first in {"phone", "call", "dial"}: return "Phone"
        if first in {"web", "internet", "browser", "link", "url"}: return "Web"
        if first in {"like", "thumb", "favorite", "poll", "vote"}: return "Social"
        return "Misc"
    if domain == "MdHome":
        if first in {"kitchen", "bathroom", "bedroom", "living", "dining"}: return "Room"
        if first in {"sofa", "chair", "bed", "desk", "table"}: return "Furniture"
        if first in {"fridge", "oven", "washer", "dryer", "microwave"}: return "Appliance"
        if first in {"garden", "lawn", "plant", "flower", "tree"}: return "Garden"
        return "Misc"
    if domain == "MdTransport":
        if first in {"car", "bus", "truck", "van", "taxi", "bike", "bicycle", "scooter"}: return "Road"
        if first in {"train", "tram", "subway", "metro", "rail"}: return "Rail"
        if first in {"plane", "airplane", "helicopter", "rocket", "jet"}: return "Air"
        if first in {"ship", "boat", "ferry", "anchor", "sail"}: return "Water"
        return "Misc"
    if domain == "MdAccount":
        if first in {"account", "user", "profile", "avatar"}: return "User"
        if first in {"human", "person", "people", "face", "head", "body"}: return "Body"
        if first in {"group", "multiple", "team"}: return "Group"
        if first in {"id", "passport", "license", "credential", "certificate"}: return "Credential"
        return "Misc"
    if domain == "MdTime":
        if first in {"clock"}: return "Clock"
        if first in {"calendar"}: return "Calendar"
        if first in {"timer", "stopwatch", "hourglass"}: return "Timer"
        if first in {"schedule", "timeline", "alarm"}: return "Schedule"
        return "Misc"
    if domain == "MdFinance":
        if first in {"cash", "money", "currency", "dollar", "euro", "pound", "yen", "bitcoin"}: return "Currency"
        if first in {"credit", "bank", "wallet", "payment", "invoice", "receipt"}: return "Payment"
        if first in {"chart", "graph", "trending", "stock"}: return "Chart"
        if first in {"briefcase", "business", "company"}: return "Business"
        return "Misc"
    if domain == "MdSecurity":
        if first in {"lock", "unlock", "key", "password"}: return "Lock"
        if first in {"shield", "security"}: return "Shield"
        if first in {"fingerprint", "authentication", "certificate", "token"}: return "Auth"
        if first in {"privacy", "vpn", "firewall"}: return "Privacy"
        return "Misc"
    if domain == "MdFood":
        if first in {"food", "restaurant", "menu", "meal"}: return "Meal"
        if first in {"beer", "wine", "cocktail", "tea", "coffee"}: return "Drink"
        if first in {"fruit", "vegetable", "meat", "fish", "bread", "cheese"}: return "Ingredient"
        if first in {"bakery", "bar", "grill"}: return "Restaurant"
        return "Misc"
    if domain == "MdWeather":
        if first in {"sun", "moon", "star"}: return "Sun"
        if first in {"cloud", "fog", "mist", "haze"}: return "Cloud"
        if first in {"rain", "snow", "hail", "sleet"}: return "Precipitation"
        if first in {"thunder", "lightning", "tornado", "hurricane", "storm"}: return "Storm"
        if first in {"temperature", "thermometer", "hot", "cold"}: return "Temperature"
        return "Misc"
    if domain == "MdShape":
        if first in {"square", "circle", "triangle", "dot", "ring"}: return "Basic"
        if first in {"hexagon", "octagon", "polygon", "rhombus"}: return "Polygon"
        if first in {"cube", "sphere", "cylinder", "cone", "pyramid"}: return "Solid"
        return "Misc"
    if domain == "MdAlert":
        if first in {"alert", "warning", "error", "info", "success", "fail"}: return "Status"
        if first in {"bell", "notification"}: return "Notification"
        if first in {"progress", "loading", "spinner"}: return "Progress"
        return "Misc"
    if domain == "MdScience":
        if first in {"flask", "beaker", "pipette", "burette"}: return "Chemistry"
        if first in {"atom", "molecule", "electron", "proton", "neutron"}: return "Physics"
        if first in {"math", "alpha", "beta", "numeric", "decimal", "axis"}: return "Math"
        if first in {"dna", "virus", "bacteria", "microscope"}: return "Biology"
        if first in {"telescope", "planet", "orbit", "galaxy"}: return "Astronomy"
        return "Misc"
    if domain == "MdNature":
        if first in {"tree", "flower", "leaf", "plant", "grass", "bush"}: return "Plant"
        if first in {"cat", "dog", "bird", "fish", "bee", "butterfly"}: return "Animal"
        if first in {"mountain", "hill", "valley", "canyon", "beach", "island"}: return "Landscape"
        if first in {"water", "river", "lake", "ocean", "pond"}: return "Water"
        return "Misc"
    if domain == "MdSport":
        if first in {"soccer", "football", "basketball", "baseball", "tennis", "golf", "hockey"}: return "Ball"
        if first in {"medal", "trophy", "podium"}: return "Award"
        if first in {"stadium", "arena", "court", "field", "pool"}: return "Venue"
        if first in {"diving", "ski", "swimming", "weight"}: return "Equipment"
        return "Misc"
    if domain == "MdGame":
        if first in {"chess", "cards", "card", "domino", "mahjong"}: return "Board"
        if first in {"dice"}: return "Dice"
        if first in {"gamepad", "joystick", "controller", "console"}: return "Video"
        if first in {"toy", "brick", "block", "puzzle"}: return "Toy"
        return "Misc"
    if domain == "MdShopping":
        if first in {"store", "shop", "warehouse", "market", "mall"}: return "Store"
        if first in {"cart", "basket"}: return "Cart"
        if first in {"bag", "pail"}: return "Bag"
        return "Product"
    if domain == "MdDatabase":
        return "Database"
    if domain == "MdMap":
        if first in {"map"}: return "Map"
        if first in {"marker", "pin", "location"}: return "Marker"
        return "Misc"
    if domain == "MdHealth":
        if first in {"hospital", "clinic", "ambulance"}: return "Hospital"
        if first in {"medical", "medicine", "pharmacy", "pill", "syringe"}: return "Medical"
        if first in {"heart", "pulse", "bandage", "wheelchair"}: return "Body"
        return "Wellness"
    if domain == "MdOffice":
        if first in {"desk", "briefcase"}: return "Desk"
        if first in {"newspaper", "document", "report"}: return "Document"
        if first in {"office", "business"}: return "Business"
        return "Misc"
    if domain == "MdBattery":
        return "Battery"
    if domain == "MdClothing":
        if first in {"hat", "cap", "helmet"}: return "Head"
        if first in {"shirt", "pants", "dress", "jacket", "coat"}: return "Body"
        if first in {"shoe", "boot", "sandal", "sneaker"}: return "Foot"
        return "Accessory"
    if domain == "MdTool":
        if first in {"wrench", "hammer", "screwdriver", "pliers", "drill", "saw"}: return "Hand"
        if first in {"multitool", "axe", "shovel", "pick"}: return "Power"
        if first in {"ruler", "protractor", "caliper"}: return "Measure"
        return "Misc"
    if domain == "MdEducation":
        if first in {"school", "university", "college"}: return "School"
        if first in {"book", "notebook"}: return "Book"
        if first in {"graduation", "diploma", "degree"}: return "Learning"
        return "Misc"
    if domain == "MdAnimal":
        if first in {"cat", "dog", "rabbit", "cow", "pig", "horse", "elephant", "panda"}: return "Mammal"
        if first in {"bird", "owl", "penguin"}: return "Bird"
        if first in {"fish", "shark", "whale", "dolphin"}: return "Fish"
        if first in {"bee", "butterfly", "spider", "ladybug", "snail"}: return "Insect"
        return "Misc"
    if domain == "MdBuilding":
        if first in {"home", "house", "apartment", "condo"}: return "Residential"
        if first in {"office", "factory", "warehouse", "store", "mall"}: return "Commercial"
        if first in {"school", "hospital", "library", "museum"}: return "Institutional"
        if first in {"church", "mosque", "synagogue", "temple"}: return "Religious"
        return "Misc"
    if domain == "MdReligion":
        if first in {"cross", "star", "om", "ankh"}: return "Symbol"
        if first in {"church", "mosque", "synagogue", "temple"}: return "Place"
        if first in {"bible", "quran", "torah"}: return "Text"
        return "Misc"
    if domain == "MdEmoji":
        if first in {"emoticon", "face"}: return "Face"
        if first in {"heart", "skull", "ghost"}: return "Emotion"
        return "Object"
    if domain == "MdMisc":
        if first in BRAND_FIRST: return "Brand"
        return "Misc"
    return pascal_case(first)


def fmt_frozenset(items: frozenset[str], indent: int = 8) -> str:
    pad = " " * indent
    inner = ",\n".join(f'{pad}    "{x}"' for x in sorted(items))
    return f"frozenset(\n{pad}{{\n{inner},\n{pad}}}\n{pad[:-4]})"


def fmt_compound_rules() -> str:
    lines = ["MD_COMPOUND_RULES: list[tuple[str, str, str]] = ["]
    seen: set[tuple[str, str, str]] = set()
    for first, second, domain in MD_COMPOUND_RULES:
        key = (first, second, domain)
        if key in seen:
            continue
        seen.add(key)
        lines.append(f'    ("{first}", "{second}", "{domain}"),')
    lines.append("]")
    return "\n".join(lines)


def generate_output(prefix_rules: list[tuple[str, frozenset[str]]]) -> str:
    lines = [
        '#!/usr/bin/env python3',
        '"""Semantic domain routing for Nerd Fonts Material Design Icons (i_md.sh, v3.4.0)."""',
        "",
        "from __future__ import annotations",
        "",
        "from nerd_font_lib import kebab_case, pascal_case",
        "",
        "MD_DOMAIN_ORDER = [",
    ]
    for d in MD_DOMAIN_ORDER:
        lines.append(f'    "{d}",')
    lines.append("]")
    lines.append("")

    for name, mapping in [
        ("MD_DOMAIN_PATH_PREFIX", {d: _kebab_domain(d) for d in MD_DOMAIN_ORDER}),
        ("MD_DOMAIN_CPP_STEM", {d: _cpp_stem(d) for d in MD_DOMAIN_ORDER}),
        ("MD_DOMAIN_ENUM", {d: d + "Icon" for d in MD_DOMAIN_ORDER}),
        ("MD_DOMAIN_CATEGORY_ENUM", {d: d + "IconCategory" for d in MD_DOMAIN_ORDER}),
    ]:
        lines.append(f"{name} = {{")
        for d in MD_DOMAIN_ORDER:
            lines.append(f'    "{d}": "{mapping[d]}",')
        lines.append("}")
        lines.append("")

    lines.append("BRAND_FIRST = frozenset(")
    lines.append("    {")
    for b in sorted(BRAND_FIRST):
        lines.append(f'        "{b}",')
    lines.append("    }")
    lines.append(")")
    lines.append("")
    lines.append(fmt_compound_rules())
    lines.append("")
    lines.append("MD_PREFIX_RULES: list[tuple[str, frozenset[str]]] = [")
    for domain, prefixes in prefix_rules:
        lines.append(f'    ("{domain}", {fmt_frozenset(prefixes)}),')
    lines.append("]")
    lines.append("")

    lines.extend([
        "def md_domain_for(suffix: str) -> str:",
        '    parts = suffix.split("_")',
        "    first = parts[0]",
        "    if first in BRAND_FIRST:",
        '        return "MdMisc"',
    ])
    for key, domain in CROSS_OVERRIDES.items():
        lines.append(f'    if first == "{key}":')
        lines.append(f'        return "{domain}"')
    lines.extend([
        "    if len(parts) >= 2:",
        "        for first_c, second_c, domain_c in MD_COMPOUND_RULES:",
        "            if parts[0] == first_c and parts[1] == second_c:",
        "                return domain_c",
        "    for domain, prefixes in MD_PREFIX_RULES:",
        "        if first in prefixes:",
        "            return domain",
        '    return "MdMisc"',
        "",
    ])

    # category_for as generated lookup from our function
    lines.append("def md_category_for(domain: str, suffix: str) -> str:")
    lines.append('    parts = suffix.split("_")')
    lines.append("    first = parts[0]")
    lines.append("    # domain-specific category routing")
    # We'll embed the category logic inline - use a simplified version that calls same logic
    lines.append("    return _md_category_lookup(domain, suffix)")
    lines.append("")
    lines.append("def _md_category_lookup(domain: str, suffix: str) -> str:")
    lines.append('    """Generated category lookup."""')
    lines.append("    pass  # replaced below")
    lines.append("")

    # Generate category lookup by sampling all icons
    icons = load_set("md")
    cat_map: dict[tuple[str, str], str] = {}
    for s in icons:
        d = domain_for_suffix(s, prefix_rules)
        cat_map[(d, s)] = category_for(d, s)

    lines[-2] = "    key = (domain, suffix)"
    lines[-1] = "    return _MD_CATEGORY_MAP.get(key, pascal_case(suffix.split('_')[0]))"
    lines.append("")
    lines.append("_MD_CATEGORY_MAP: dict[tuple[str, str], str] = {")
    for (d, s), cat in sorted(cat_map.items()):
        lines.append(f'    ("{d}", "{s}"): "{cat}",')
    lines.append("}")
    lines.append("")

    ascii_entries = ",\n".join(f'        "{d}": "{MD_ASCII[d]}"' for d in MD_DOMAIN_ORDER)
    lines.extend([
        "def md_path(domain: str, suffix: str) -> str:",
        '    return f"{MD_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"',
        "",
        "def md_ascii(suffix: str) -> str:",
        "    domain = md_domain_for(suffix)",
        "    return {",
        ascii_entries + ",",
        '    }.get(domain, ".")',
        "",
        "def md_row(suffix: str, codepoint: int) -> dict[str, str]:",
        "    domain = md_domain_for(suffix)",
        "    category = md_category_for(domain, suffix)",
        "    return {",
        '        "name": pascal_case(suffix),',
        '        "domain": domain,',
        '        "category": category,',
        '        "nerd_suffix": suffix,',
        '        "path": md_path(domain, suffix),',
        '        "codepoint": f"{codepoint:04X}",',
        '        "ascii": md_ascii(suffix),',
        '        "source_set": "md",',
        "    }",
        "",
        "def md_category_order_for(domain: str) -> list[str]:",
        "    return _MD_CATEGORY_ORDER.get(domain, [])",
        "",
        "def md_category_path_for(domain: str, category: str) -> str:",
        "    prefix = MD_DOMAIN_PATH_PREFIX[domain]",
        "    if not category:",
        "        return prefix",
        '    return f"{prefix}-{kebab_case(category)}"',
        "",
        "MD_REGISTRY = {",
        '    "order": MD_DOMAIN_ORDER,',
        '    "path_prefix": MD_DOMAIN_PATH_PREFIX,',
        '    "cpp_stem": MD_DOMAIN_CPP_STEM,',
        '    "enum": MD_DOMAIN_ENUM,',
        '    "category_enum": MD_DOMAIN_CATEGORY_ENUM,',
        '    "category_order": md_category_order_for,',
        '    "category_path": md_category_path_for,',
        "}",
        "",
    ])

    lines.append("_MD_CATEGORY_ORDER: dict[str, list[str]] = {")
    for d, cats in MD_CATEGORY_ORDER.items():
        cat_str = ", ".join(f'"{c}"' for c in cats)
        lines.append(f'    "{d}": [{cat_str}],')
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


def _kebab_domain(name: str) -> str:
    return "md-" + name[2:].lower()


def _cpp_stem(name: str) -> str:
    return _kebab_domain(name).replace("-", "_") + "_icon"


def main() -> None:
    icons = load_set("md")
    prefix_rules = build_prefix_rules(icons)
    output = generate_output(prefix_rules)
    OUT.write_text(output + "\n", encoding="utf-8")
    print(f"Wrote {OUT}")

    # Validate
    sys.path.insert(0, str(ROOT / "scripts"))
    import importlib
    import nerd_glyph_domains_md as md_mod
    importlib.reload(md_mod)
    from nerd_glyph_domains_md import md_row

    counts = Counter(md_row(s, cp)["domain"] for s, cp in icons.items())
    misc = counts.get("MdMisc", 0)
    print(f"total {len(icons)} domains {len(counts)} mdmisc {misc}")
    print("top 15:", counts.most_common(15))
    if misc >= 200:
        print("WARNING: MdMisc >= 200, need more rules")
        misc_segs = Counter()
        for s in icons:
            if md_mod.md_domain_for(s) == "MdMisc":
                misc_segs[s.split("_")[0]] += 1
        print("top misc first segments:", misc_segs.most_common(30))
        raise SystemExit(1)


if __name__ == "__main__":
    main()
