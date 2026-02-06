# weatherapp (CLI) 🛰️

Simple command-line weather app written in C. Uses libcurl to fetch current weather from OpenWeatherMap and cJSON to parse the JSON response.

## Requirements
- libcurl (development headers)
- cJSON (development headers)
- A C compiler (gcc / clang / MSVC)
- An OpenWeatherMap API key (set via environment variable `OPENWEATHER_API_KEY`)

## Build (Linux / WSL / MSYS2)
- Debian/Ubuntu: `sudo apt install libcurl4-openssl-dev libcjson-dev build-essential`
- MSYS2 (Windows): `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-curl mingw-w64-x86_64-cjson`

Then run:

```sh
make
```

This produces an executable named `weather`.

## Usage

Set your API key in the environment:

Windows (PowerShell):
```
$env:OPENWEATHER_API_KEY = "your_api_key_here"
```

Linux / macOS:
```
export OPENWEATHER_API_KEY=your_api_key_here
```

Then run:

```
./weather "New York"
./weather "Los Angeles" -f   # show Fahrenheit
```

You can also run the provided PowerShell example (Windows PowerShell):

```
./run_example.ps1
```

Output example:

```
New York
Temperature: 5.2 C
Humidity: 72%
Conditions: light rain
```

## Notes
- If you don't want to install cJSON as a system lib, you can vendor its single-file implementation into the project. I kept the project simple and external-only for now.
- Consider checking API rate limits on OpenWeatherMap.
