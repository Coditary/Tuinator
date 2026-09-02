# Nerd Icon Domains (from Nerd Fonts i_weather.sh, v3.4.0)

All 228 weather glyphs are exported into semantic domains under `data/nerd_icons/`:

| Domain TSV | Icons | Path prefix | Enum |
|------------|------:|-------------|------|
| `weather.tsv` | 119 | `weather-` | `WeatherIcon` |
| `moon.tsv` | 56 | `moon-` | `MoonIcon` |
| `clock.tsv` | 12 | `clock-` | `ClockIcon` |
| `compass.tsv` | 8 | `compass-` | `CompassIcon` |
| `measurement.tsv` | 9 | `measurement-` | `MeasurementIcon` |
| `astronomy.tsv` | 11 | `astronomy-` | `AstronomyIcon` |
| `airquality.tsv` | 3 | `air-quality-` | `AirQualityIcon` |
| `hazard.tsv` | 6 | `hazard-` | `HazardIcon` |
| `marinealert.tsv` | 2 | `marine-` | `MarineAlertIcon` |
| `action.tsv` | 2 | `action-` | `ActionIcon` |

Regenerate:

```bash
python3 scripts/export_nerd_glyph_catalog.py weather
python3 scripts/generate_nerd_icon_catalog.py --verify
```

See `data/nerd_glyphs/README.md` for the full Nerd Fonts catalog.
