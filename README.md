# SwissCorners - Swiss Railways Watch Face for Pebble

Inspired by the Official Swiss Railways (SBB / Mondaine) watch.

**Author:** Lawrence Hon

This project is based on **[Maartje Eyskens’s ModernSwiss](https://github.com/meyskens/ModernSwiss)**, which brought the classic “Swiss Square” face forward for reborn Pebble hardware and the modern SDK. Earlier lineage: Johnny Lim’s Mondaine watchface, reworked by Leo Lin.

SwissCorners adds configurable corner complications and a battery-friendly second-hand mode.

## Features

- Swiss railway dial (white / black / no-logo / round face on square watches)
- Quartz or Stop2Go second hand, or off
- **On backlight** second hand (default): shows seconds only while the backlight is on, then drops to once-a-minute updates to save battery
- Optional date window on the dial
- Hourly vibration and Bluetooth connect/disconnect alerts
- Clay settings page for appearance and complications

## Complications

Four corner complications (rectangular watches only — not shown on Pebble Time Round / Round 2). Each corner has its own type and color.

| Corner | Default |
|--------|---------|
| Top left | Temperature |
| Top right | Day (Mon, Tue…) |
| Bottom left | Steps (`1.2k`) |
| Bottom right | Date (1–31) |

Also available: day + date (Mo/Tu… over 1–31), heart rate, battery % / battery icon, sunrise, sunset, weather icon, or off. Temperature unit (°C / °F) is configurable.

Weather, sunrise, and sunset come from Open-Meteo via the phone (location permission). Last successful reading is kept on the watch for up to 3 hours if the phone is offline. Steps and heart rate use Pebble Health where available. The watch only accesses location or health data while a matching complication is enabled.

## Building

Manifest v2 permissions require `pebble-tool` 5.1.0 or later and SDK 4.44 or later.

```bash
npm install
pebble build --sdk 4.44
pebble install --emulator basalt   # or chalk, gabbro, emery, etc.
```

## License

MIT — see [LICENSE](LICENSE). Based on ModernSwiss (MIT) by Maartje Eyskens.
