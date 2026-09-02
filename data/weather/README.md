# Weather Icons (Nerd Fonts v3.4.0)

Split TSV files for the 228 Weather Icons (`i_weather.sh`). Each file:

```
name	category	nerd_suffix	path	codepoint	ascii
Time1	Time	time_1	weather-time-1	E382	t
```

- `name` — PascalCase `WeatherIcon` enum member
- `category` — `WeatherIconCategory` value
- `nerd_suffix` — nerd-fonts `i_weather_*` key (`time_1`)
- `path` — nested cheat-sheet path (`weather-time-1`)
- `codepoint` — U+E300..E3E3 hex
- `ascii` — single-char fallback

Path rule: `weather-` + suffix with `_` replaced by `-`  
Examples: `day_sunny` → `weather-day-sunny`, `night_alt_rain` → `weather-night-alt-rain`

Merge order in `generate_weather_icons.py`: day → night_alt → night → moon → wind → storm → cloud → precipitation → time → direction → temperature → pressure → sun → beach → hazard → marine → astronomy → navigation → other
