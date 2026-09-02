# Nerd Font Glyph Catalog (v3.4.0)

**10.386 Glyphen** in **126 semantische Domänen** — gleiches Prinzip wie die Weather-Aufteilung (Domäne → Kategorie → verschachtelter Pfad).

## Regenerieren

```bash
python3 scripts/export_nerd_glyph_catalog.py all   # alle Sets
python3 scripts/validate_nerd_glyph_catalog.py     # Zähler + Misc-Check
python3 scripts/export_nerd_glyph_catalog.py weather
python3 scripts/generate_nerd_icon_catalog.py      # nur Weather-Familie → C++
```

## Weather-Familie (`data/nerd_icons/`)

228 Icons → 10 Domänen: Weather, Moon, Clock, Compass, Measurement, Astronomy, AirQuality, Hazard, MarineAlert, Action.

## Utility (`data/nerd_glyphs/`)

| Domäne | Icons | Pfad |
|--------|------:|------|
| Powerline | 4 | `powerline-*` |
| PowerlineExtra | 36 | `powerline-extra-*` |
| PowerSymbol | 5 | `power-*` |
| Progress | 12 | `progress-*` |
| Pomodoro | 11 | `pomodoro-*` |

## Editor (Codicons + Octicons, gemeinsame Domänennamen)

`source-control-*`, `editor-debug-*`, `editor-edit-*`, `editor-format-*`, `editor-social-*`, `editor-communication-*`, …

## Dev / Dateien / Marken

| Router | Domänen (Auszug) |
|--------|------------------|
| `nerd_glyph_domains_dev.py` | Platform, TechLanguage, TechFramework, TechTool, TechDatabase, TechCloud, TechEditor, TechVcs, TechBrand |
| `nerd_glyph_domains_seti.py` | FileLanguage, FileMarkup, FileConfig, FileData, FileMedia, FileFolder, FileProject |
| `nerd_glyph_domains_logos.py` | LinuxDistro, DesktopEnvironment, TechSaas, TechCreative, TechCommunity |
| `nerd_glyph_domains_fae.py` | Food, Home, Body, Activity, Nature, Misc |

## Font Awesome & Material Design

| Router | Domänen |
|--------|---------|
| `nerd_glyph_domains_fa.py` | 27 Domänen (Navigation, SocialBrand, Office, Game, …; FaMisc ≤ 24) |
| `nerd_glyph_domains_md.py` | 35 Domänen (MdEditor, MdScience, MdGame, …; MdMisc ≤ 173) |

## Misc-Buckets (Reste)

| Domäne | Icons | Hinweis |
|--------|------:|---------|
| MdMisc | 173 | v. a. Marken/Logos ohne MDI-Präfix |
| EditorMisc | 113 | Cod+Oct Reste |
| Misc (FAE) | 55 | Font Awesome Extension Reste |
| DevMisc | 30 | GitHub-spezifisch |
| FaMisc | 24 | FA Essen/Trinken ohne Food-Domäne |

## TSV-Spalten

`name`, `domain`, `category`, `nerd_suffix`, `path`, `codepoint`, `ascii`, `source_set`

C++-Codegen existiert aktuell für die **Weather-Familie**; weitere Domänen können bei Bedarf nachgezogen werden.
