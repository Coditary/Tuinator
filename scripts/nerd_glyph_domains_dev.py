#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts Devicons (i_dev.sh, v3.4.0)."""

from __future__ import annotations

from nerd_font_lib import kebab_case, pascal_case

DEV_DOMAIN_ORDER = [
    "Platform",
    "TechLanguage",
    "TechFramework",
    "TechTool",
    "TechVcs",
    "TechDatabase",
    "TechCloud",
    "TechEditor",
    "TechBrand",
]

DEV_DOMAIN_PATH_PREFIX = {
    "Platform": "platform",
    "TechLanguage": "tech-language",
    "TechFramework": "tech-framework",
    "TechTool": "tech-tool",
    "TechVcs": "tech-vcs",
    "TechDatabase": "tech-database",
    "TechCloud": "tech-cloud",
    "TechEditor": "tech-editor",
    "TechBrand": "tech-brand",
}

DEV_DOMAIN_CPP_STEM = {
    "Platform": "platform_icon",
    "TechLanguage": "tech_language_icon",
    "TechFramework": "tech_framework_icon",
    "TechTool": "tech_tool_icon",
    "TechVcs": "tech_vcs_icon",
    "TechDatabase": "tech_database_icon",
    "TechCloud": "tech_cloud_icon",
    "TechEditor": "tech_editor_icon",
    "TechBrand": "tech_brand_icon",
}

DEV_DOMAIN_ENUM = {
    "Platform": "PlatformIcon",
    "TechLanguage": "TechLanguageIcon",
    "TechFramework": "TechFrameworkIcon",
    "TechTool": "TechToolIcon",
    "TechVcs": "TechVcsIcon",
    "TechDatabase": "TechDatabaseIcon",
    "TechCloud": "TechCloudIcon",
    "TechEditor": "TechEditorIcon",
    "TechBrand": "TechBrandIcon",
}

DEV_DOMAIN_CATEGORY_ENUM = {
    "Platform": "PlatformIconCategory",
    "TechLanguage": "TechLanguageIconCategory",
    "TechFramework": "TechFrameworkIconCategory",
    "TechTool": "TechToolIconCategory",
    "TechVcs": "TechVcsIconCategory",
    "TechDatabase": "TechDatabaseIconCategory",
    "TechCloud": "TechCloudIconCategory",
    "TechEditor": "TechEditorIconCategory",
    "TechBrand": "TechBrandIconCategory",
}

PLATFORM_OS = {
    "android",
    "apple",
    "archlinux",
    "centos",
    "debian",
    "fedora",
    "gentoo",
    "linux",
    "msdos",
    "nixos",
    "opensuse",
    "raspberry_pi",
    "redhat",
    "rockylinux",
    "ubuntu",
    "unix",
    "windows",
    "windows11",
}

PLATFORM_CONTAINER = {
    "docker",
    "k3os",
    "k3s",
    "kubernetes",
    "podman",
}

TECH_EDITOR = {
    "androidstudio",
    "atom",
    "clion",
    "codepen",
    "datagrip",
    "dataspell",
    "dbeaver",
    "dreamweaver",
    "eclipse",
    "emacs",
    "goland",
    "intellij",
    "jetbrains",
    "labview",
    "nano",
    "neovim",
    "ohmyzsh",
    "phpstorm",
    "putty",
    "pycharm",
    "qodana",
    "replit",
    "rider",
    "rstudio",
    "rubymine",
    "spyder",
    "ssh",
    "sublime",
    "terminal",
    "vim",
    "visualstudio",
    "vscode",
    "webstorm",
    "xcode",
}

TECH_DATABASE = {
    "azuresqldatabase",
    "cassandra",
    "cosmosdb",
    "couchbase",
    "couchdb",
    "database",
    "doctrine",
    "dynamodb",
    "ecto",
    "elasticsearch",
    "faunadb",
    "hibernate",
    "influxdb",
    "knexjs",
    "liquibase",
    "mariadb",
    "microsoftsqlserver",
    "mongodb",
    "mongoose",
    "mysql",
    "neo4j",
    "nhibernate",
    "postgresql",
    "prisma",
    "realm",
    "redis",
    "rocksdb",
    "sequelize",
    "sqlalchemy",
    "sqldeveloper",
    "sqlite",
    "vitess",
    "yugabytedb",
}

TECH_CLOUD = {
    "amazonwebservices",
    "ansible",
    "apacheairflow",
    "apachekafka",
    "apachespark",
    "appwrite",
    "argocd",
    "azure",
    "azuredevops",
    "cloudflare",
    "cloudflareworkers",
    "consul",
    "digitalocean",
    "envoy",
    "firebase",
    "googlecloud",
    "grafana",
    "hadoop",
    "harvester",
    "helm",
    "heroku",
    "jaegertracing",
    "kibana",
    "logstash",
    "netlify",
    "nginx",
    "nomad",
    "oauth",
    "okta",
    "openstack",
    "opentelemetry",
    "oracle",
    "ory",
    "packer",
    "portainer",
    "prometheus",
    "pulsar",
    "pulumi",
    "rabbitmq",
    "railway",
    "rancher",
    "salesforce",
    "scalingo",
    "splunk",
    "supabase",
    "terraform",
    "tomcat",
    "traefikmesh",
    "traefikproxy",
    "uwsgi",
    "vagrant",
    "vault",
    "vercel",
    "vsphere",
}

TECH_VCS = {
    "bitbucket",
    "git",
    "git_branch",
    "git_commit",
    "git_compare",
    "git_merge",
    "git_pull_request",
    "gitbook",
    "github",
    "github_full",
    "githubactions",
    "githubcodespaces",
    "gitlab",
    "gitpod",
    "gitter",
    "sourcetree",
    "subversion",
    "tortoisegit",
    "towergit",
}

TECH_TOOL = {
    "algolia",
    "anaconda",
    "appcelerator",
    "babel",
    "bamboo",
    "beats",
    "bower",
    "browserstack",
    "canva",
    "carbon",
    "chrome",
    "circleci",
    "clarity",
    "cmake",
    "codeac",
    "codecov",
    "composer",
    "confluence",
    "cucumber",
    "cypressio",
    "discordjs",
    "dropbox",
    "eslint",
    "filezilla",
    "firefox",
    "gatling",
    "gcc",
    "gradle",
    "grunt",
    "gulp",
    "hardhat",
    "homebrew",
    "ie",
    "ifttt",
    "insomnia",
    "jasmine",
    "jenkins",
    "jest",
    "jira",
    "jiraalign",
    "junit",
    "k6",
    "kaggle",
    "karatelabs",
    "karma",
    "kdeneon",
    "llvm",
    "maven",
    "mocha",
    "mozilla",
    "notion",
    "npm",
    "nuget",
    "opera",
    "pfsense",
    "playwright",
    "pnpm",
    "poetry",
    "polygon",
    "postman",
    "protractor",
    "puppeteer",
    "pypi",
    "pyscript",
    "pytest",
    "readthedocs",
    "rollup",
    "rspec",
    "safari",
    "selenium",
    "sema",
    "sentry",
    "shotgrid",
    "slack",
    "smashing_magazine",
    "sonarqube",
    "spack",
    "thealgorithms",
    "travis",
    "trello",
    "unifiedmodelinglanguage",
    "vite",
    "vitejs",
    "vitest",
    "weblate",
    "webpack",
    "yarn",
    "yunohost",
    "zend",
}

TECH_LANGUAGE = {
    "apl",
    "arduino",
    "awk",
    "ballerina",
    "bash",
    "bun",
    "c_lang",
    "cairo",
    "ceylon",
    "clojure",
    "clojure_alt",
    "clojurescript",
    "coffeescript",
    "corejs",
    "cplusplus",
    "crystal",
    "csharp",
    "css3",
    "css3_full",
    "dart",
    "denojs",
    "dlang",
    "elixir",
    "elm",
    "embeddedc",
    "erlang",
    "fortran",
    "fsharp",
    "go",
    "graphql",
    "groovy",
    "haskell",
    "haxe",
    "html5",
    "java",
    "javascript",
    "javascript_alt",
    "json",
    "jule",
    "julia",
    "kotlin",
    "latex",
    "less",
    "lua",
    "markdown",
    "matlab",
    "minitab",
    "nim",
    "nimble",
    "nodejs",
    "nodejs_small",
    "nodemon",
    "nodewebkit",
    "norg",
    "objectivec",
    "ocaml",
    "perl",
    "php",
    "powershell",
    "prolog",
    "purescript",
    "python",
    "r",
    "ruby",
    "ruby_rough",
    "rust",
    "sass",
    "scala",
    "solidity",
    "spss",
    "stata",
    "stylus",
    "swift",
    "tex",
    "typescript",
    "v8",
    "vala",
    "visualbasic",
    "vyper",
    "wasm",
    "xml",
    "yaml",
    "zig",
}

TECH_FRAMEWORK = {
    "adonisjs",
    "aftereffects",
    "akka",
    "alpinejs",
    "angular",
    "angularjs",
    "angularmaterial",
    "antdesign",
    "astro",
    "axios",
    "backbonejs",
    "blazor",
    "blender",
    "bootstrap",
    "bulma",
    "cakephp",
    "capacitor",
    "codeigniter",
    "contao",
    "d3js",
    "django",
    "djangorest",
    "dotnet",
    "dotnetcore",
    "dropwizard",
    "drupal",
    "electron",
    "eleventy",
    "ember",
    "express",
    "fastapi",
    "fastify",
    "feathersjs",
    "figma",
    "flask",
    "flutter",
    "foundation",
    "framermotion",
    "framework7",
    "gatsby",
    "gazebo",
    "ghost",
    "ghost_small",
    "gimp",
    "godot",
    "grails",
    "grpc",
    "handlebars",
    "hugo",
    "illustrator",
    "inkscape",
    "ionic",
    "jamstack",
    "jeet",
    "jekyll",
    "jetpackcompose",
    "jquery",
    "jupyter",
    "keras",
    "knockout",
    "krakenjs",
    "ktor",
    "laravel",
    "livewire",
    "lodash",
    "lumen",
    "magento",
    "materializecss",
    "materialui",
    "matplotlib",
    "maya",
    "meteor",
    "meteorfull",
    "mithril",
    "mobx",
    "modx",
    "moleculer",
    "moodle",
    "mootools_badge",
    "nestjs",
    "networkx",
    "nextjs",
    "ngrx",
    "numpy",
    "nuxtjs",
    "openal",
    "openapi",
    "opencl",
    "opencv",
    "opengl",
    "p5js",
    "pandas",
    "phalcon",
    "phoenix",
    "photonengine",
    "photoshop",
    "plotly",
    "postcss",
    "premierepro",
    "processing",
    "pytorch",
    "qt",
    "quarkus",
    "quasar",
    "qwik",
    "rails",
    "reach",
    "react",
    "reactbootstrap",
    "reactnavigation",
    "reactrouter",
    "rect",
    "redux",
    "renpy",
    "requirejs",
    "ros",
    "rxjs",
    "sanity",
    "scikitlearn",
    "sdl",
    "shopware",
    "sketch",
    "socketio",
    "solidjs",
    "spring",
    "storybook",
    "streamlit",
    "svelte",
    "swagger",
    "swiper",
    "symfony",
    "tailwindcss",
    "tauri",
    "tensorflow",
    "threedsmax",
    "threejs",
    "titaniumsdk",
    "trpc",
    "typo3",
    "unity",
    "unrealengine",
    "vertx",
    "vuejs",
    "vuestorefront",
    "vuetify",
    "webflow",
    "woocommerce",
    "wordpress",
    "xamarin",
    "xd",
    "yii",
}

TECH_BRAND = {
    "aarch64",
    "apache",
    "behance",
    "devicon",
    "facebook",
    "gnu",
    "google",
    "linkedin",
    "stackoverflow",
    "twitter",
}

PLATFORM_CATEGORY_ORDER = ["OS", "Container"]
TECH_LANGUAGE_CATEGORY_ORDER = ["Language", "Markup", "Query"]
TECH_FRAMEWORK_CATEGORY_ORDER = [
    "WebFrontend", "WebUI", "WebBackend", "Mobile", "Desktop",
    "ML", "Game", "CMS", "Graphics", "Library", "Framework",
]
TECH_TOOL_CATEGORY_ORDER = [
    "PackageManager", "Build", "CI", "Test", "Browser",
    "Monitor", "API", "Collaboration", "Infrastructure", "Misc",
]
TECH_VCS_CATEGORY_ORDER = ["Git", "Hosting", "Client", "Misc"]
TECH_DATABASE_CATEGORY_ORDER = ["SQL", "NoSQL", "Cache", "Search", "ORM"]
TECH_CLOUD_CATEGORY_ORDER = [
    "Provider", "Orchestration", "DataPlatform", "Observability",
    "WebServer", "BaaS", "Identity", "Platform",
]
TECH_EDITOR_CATEGORY_ORDER = ["IDE", "Editor", "Terminal"]
TECH_BRAND_CATEGORY_ORDER = ["Social", "Organization", "Brand"]

def dev_domain_for(suffix: str) -> str:
    if suffix in PLATFORM_OS or suffix in PLATFORM_CONTAINER:
        return "Platform"
    if suffix in TECH_EDITOR:
        return "TechEditor"
    if suffix in TECH_DATABASE:
        return "TechDatabase"
    if suffix in TECH_CLOUD:
        return "TechCloud"
    if suffix in TECH_TOOL:
        return "TechTool"
    if suffix in TECH_VCS:
        return "TechVcs"
    if suffix in TECH_LANGUAGE:
        return "TechLanguage"
    if suffix in TECH_FRAMEWORK:
        return "TechFramework"
    return "TechBrand"


def dev_category_for(domain: str, suffix: str) -> str:
    if domain == "Platform":
        return "Container" if suffix in PLATFORM_CONTAINER else "OS"
    if domain == "TechEditor":
        if suffix in {"terminal", "putty", "ssh", "ohmyzsh"}:
            return "Terminal"
        if suffix in {"vim", "neovim", "emacs", "sublime", "atom", "nano"}:
            return "Editor"
        return "IDE"
    if domain == "TechDatabase":
        if suffix == "redis":
            return "Cache"
        if suffix == "elasticsearch":
            return "Search"
        if suffix in {
            "doctrine", "hibernate", "nhibernate", "sequelize", "knexjs", "prisma",
            "ecto", "sqlalchemy", "mongoose", "realm",
        }:
            return "ORM"
        if suffix in {
            "mongodb", "couchdb", "couchbase", "dynamodb", "cosmosdb", "neo4j",
            "faunadb", "rocksdb", "influxdb",
        }:
            return "NoSQL"
        return "SQL"
    if domain == "TechCloud":
        if suffix in {
            "amazonwebservices", "azure", "googlecloud", "digitalocean", "heroku",
            "netlify", "vercel", "railway", "scalingo", "firebase", "cloudflare",
            "openstack", "oracle", "salesforce",
        }:
            return "Provider"
        if suffix in {
            "terraform", "pulumi", "ansible", "packer", "vagrant", "helm", "argocd",
            "nomad", "consul", "vault", "harvester", "rancher", "portainer",
            "traefikmesh", "traefikproxy", "envoy", "vsphere",
        }:
            return "Orchestration"
        if suffix in {"apachekafka", "apachespark", "apacheairflow", "hadoop", "rabbitmq", "pulsar"}:
            return "DataPlatform"
        if suffix in {
            "grafana", "prometheus", "kibana", "logstash", "splunk", "jaegertracing",
            "opentelemetry",
        }:
            return "Observability"
        if suffix in {"nginx", "tomcat", "uwsgi"}:
            return "WebServer"
        if suffix in {"supabase", "appwrite", "cloudflareworkers"}:
            return "BaaS"
        if suffix in {"okta", "oauth", "ory", "azuredevops"}:
            return "Identity"
        return "Platform"
    if domain == "TechTool":
        if suffix in {"npm", "yarn", "pnpm", "bower", "composer", "nuget", "poetry", "pypi", "pyscript", "homebrew"}:
            return "PackageManager"
        if suffix in {
            "webpack", "vite", "vitejs", "rollup", "gulp", "grunt", "gradle", "maven", "cmake",
            "gcc", "llvm", "babel", "spack", "eslint", "hardhat",
        }:
            return "Build"
        if suffix in {"jenkins", "travis", "circleci", "bamboo", "codecov", "sonarqube"}:
            return "CI"
        if suffix in {
            "jest", "mocha", "jasmine", "karma", "pytest", "vitest", "cypressio",
            "playwright", "protractor", "selenium", "rspec", "cucumber", "gatling",
            "karatelabs", "junit", "puppeteer",
        }:
            return "Test"
        if suffix in {"chrome", "firefox", "safari", "opera", "ie", "mozilla"}:
            return "Browser"
        if suffix in {"sentry"}:
            return "Monitor"
        if suffix in {"postman", "insomnia", "browserstack", "codeac", "algolia"}:
            return "API"
        if suffix in {"slack", "trello", "notion", "confluence", "jira", "jiraalign", "discordjs"}:
            return "Collaboration"
        if suffix in {"dropbox", "filezilla", "pfsense"}:
            return "Infrastructure"
        return "Misc"
    if domain == "TechVcs":
        if suffix in {"git", "git_branch", "git_commit", "git_compare", "git_merge", "git_pull_request"}:
            return "Git"
        if suffix in {
            "github", "github_full", "githubactions", "githubcodespaces", "gitlab",
            "bitbucket", "gitpod", "gitter", "gitbook",
        }:
            return "Hosting"
        if suffix in {"sourcetree", "subversion", "tortoisegit", "towergit"}:
            return "Client"
        return "Misc"
    if domain == "TechLanguage":
        if suffix in {"html5", "css3", "css3_full", "xml", "yaml", "json", "markdown", "latex", "tex"}:
            return "Markup"
        if suffix == "graphql":
            return "Query"
        return "Language"
    if domain == "TechFramework":
        if suffix in {
            "react", "reactbootstrap", "reactnavigation", "reactrouter", "reach", "redux",
            "rxjs", "vuejs", "vuetify", "vuestorefront", "angular", "angularjs",
            "angularmaterial", "svelte", "ember", "backbonejs", "alpinejs", "jquery",
            "knockout", "mithril", "mobx", "ngrx", "solidjs", "qwik", "astro", "nextjs",
            "nuxtjs", "gatsby",
        }:
            return "WebFrontend"
        if suffix in {
            "bootstrap", "bulma", "foundation", "tailwindcss", "materialui",
            "materializecss", "antdesign", "framermotion", "swiper",
        }:
            return "WebUI"
        if suffix in {
            "django", "djangorest", "flask", "fastapi", "fastify", "express", "nestjs",
            "rails", "laravel", "livewire", "spring", "grails", "phoenix", "adonisjs",
            "feathersjs", "ktor", "akka", "vertx", "quarkus", "dropwizard", "moleculer",
            "lumen", "yii", "cakephp", "codeigniter", "phalcon", "symfony", "dotnet",
            "dotnetcore",
        }:
            return "WebBackend"
        if suffix in {"flutter", "ionic", "capacitor", "xamarin"}:
            return "Mobile"
        if suffix in {"electron", "tauri", "blazor"}:
            return "Desktop"
        if suffix in {
            "tensorflow", "pytorch", "keras", "scikitlearn", "pandas", "numpy",
            "matplotlib", "plotly", "networkx", "jupyter",
        }:
            return "ML"
        if suffix in {"unity", "unrealengine", "godot", "gazebo", "ros", "photonengine", "titaniumsdk"}:
            return "Game"
        if suffix in {
            "wordpress", "woocommerce", "drupal", "magento", "shopware", "typo3",
            "contao", "modx", "moodle", "ghost", "ghost_small", "hugo", "jekyll",
            "eleventy", "sanity",
        }:
            return "CMS"
        if suffix in {
            "threejs", "threedsmax", "maya", "blender", "p5js", "d3js", "opencv",
            "openal", "opencl", "opengl", "sdl", "qt", "processing", "renpy",
        }:
            return "Graphics"
        if suffix in {
            "grpc", "trpc", "openapi", "swagger", "socketio", "axios", "handlebars",
            "postcss",
        }:
            return "Library"
        return "Framework"
    if domain == "TechBrand":
        if suffix in {"facebook", "twitter", "linkedin", "behance", "stackoverflow"}:
            return "Social"
        if suffix in {"google", "gnu", "apache", "aarch64"}:
            return "Organization"
        return "Brand"
    return "Brand"


def dev_path(domain: str, suffix: str) -> str:
    return f"{DEV_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"


def dev_ascii(domain: str, suffix: str) -> str:
    return {
        "Platform": "P",
        "TechLanguage": "L",
        "TechFramework": "F",
        "TechTool": "T",
        "TechVcs": "V",
        "TechDatabase": "D",
        "TechCloud": "C",
        "TechEditor": "E",
        "TechBrand": "B",
    }[domain]


def dev_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = dev_domain_for(suffix)
    category = dev_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": dev_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": dev_ascii(domain, suffix),
        "source_set": "dev",
    }


def dev_category_order_for(domain: str) -> list[str]:
    return {
        "Platform": PLATFORM_CATEGORY_ORDER,
        "TechLanguage": TECH_LANGUAGE_CATEGORY_ORDER,
        "TechFramework": TECH_FRAMEWORK_CATEGORY_ORDER,
        "TechTool": TECH_TOOL_CATEGORY_ORDER,
        "TechVcs": TECH_VCS_CATEGORY_ORDER,
        "TechDatabase": TECH_DATABASE_CATEGORY_ORDER,
        "TechCloud": TECH_CLOUD_CATEGORY_ORDER,
        "TechEditor": TECH_EDITOR_CATEGORY_ORDER,
        "TechBrand": TECH_BRAND_CATEGORY_ORDER,
    }.get(domain, [])


def dev_category_path_for(domain: str, category: str) -> str:
    prefix = DEV_DOMAIN_PATH_PREFIX[domain]
    flat = {
        "OS", "Container", "Language", "IDE", "Editor", "Terminal", "Brand",
        "SQL", "NoSQL", "Cache", "Search", "ORM", "Provider", "Platform",
        "VCS", "Git", "Hosting", "Client", "PackageManager", "Build", "CI", "Browser", "Monitor", "API",
        "Collaboration", "Infrastructure", "Misc", "Markup", "Query",
        "Mobile", "Desktop", "ML", "Game", "CMS", "Graphics", "Library",
        "Framework", "Orchestration", "DataPlatform", "Observability",
        "WebServer", "BaaS", "Identity", "Social", "Organization",
    }
    prefix_only = {
        "IDE", "Editor", "Terminal", "Brand", "Language", "Framework",
        "Platform", "Provider", "Misc", "Library", "Graphics", "Git", "Client", "Hosting",
    }
    if category in flat:
        if category in prefix_only:
            return prefix
        return f"{prefix}-{kebab_case(category)}"
    return prefix


DEV_REGISTRY = {
    "order": DEV_DOMAIN_ORDER,
    "path_prefix": DEV_DOMAIN_PATH_PREFIX,
    "cpp_stem": DEV_DOMAIN_CPP_STEM,
    "enum": DEV_DOMAIN_ENUM,
    "category_enum": DEV_DOMAIN_CATEGORY_ENUM,
    "category_order": dev_category_order_for,
    "category_path": dev_category_path_for,
}
