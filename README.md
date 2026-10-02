# Ultra

An analog watchface for Pebble with eight complications you choose yourself. Four gauges curve around the corners, four subdials sit inside the dial, and every one is yours to swap.

- **Eight slots, your pick.** Steps, distance, heart rate, battery, calendar, temperature, rain, humidity, wind, air quality, UV index, elevation, sunrise and sunset, .beat time, current weather, your own text, or a value from any JSON API.
- **Five color schemes.** Black, White, two monochromes, or Accent: any background and accent from the watch's 64 colors.
- **Live preview.** The settings page draws the face as you change it.
- **No account, no API key.** Weather and air quality come from [Open-Meteo](https://open-meteo.com), refreshed every 30 minutes.

## Screenshots
### Pebble Time 2
![Emery 1](assets/emery_1.png)
![Emery 2](assets/emery_2.png)
![Emery 3](assets/emery_3.png)
![Emery 4](assets/emery_4.png)

### Pebble Time Round 2
![Gabbro 1](assets/gabbro_1.png)
![Gabbro 2](assets/gabbro_2.png)
![Gabbro 3](assets/gabbro_3.png)
![Gabbro 4](assets/gabbro_4.png)

## Store
[Rebble App Store](https://apps.rebble.io/en_US/application/6abf59c037b3780009304810)

[Pebble App Store](https://apps.repebble.com/6abf59c037b3780009304810)

### Settings
| Setting | What it does |
| --- | --- |
| Corners | The four arc gauges outside the dial: top left, top right, bottom left, bottom right. |
| Center | The four subdials inside the dial: top, left, right, bottom. |
| Text | Pick **Text** in any slot to show your own label: up to 12 characters in a corner, 4 in a subdial. |
| Custom complications | Show a value from any JSON API. Create up to 8, then pick them in any corner or subdial. See below. |
| Color scheme | Black, White, White on black, Black on white, or Accent. Accent adds a background and an accent color picker. |
| Units | °C, km and km/h, or °F, miles and mph. |
| Daily step goal | 1,000 to 30,000 steps. Fills the steps and distance gauges. |
| Seconds hand | Off by default. Uses more battery. |

Pick **None** to leave a slot empty.

### Custom complications
Each one reads a JSON API of your choice and shows a value from it.

| Field | What it does |
| --- | --- |
| URL | The endpoint. A plain `GET` that answers with JSON; an API key has to go in the URL. |
| Name | Up to 4 characters, shown with the value and in the slot pickers. |
| Type | **Text** shows the value, like heart rate. **Bar** fills from min to max, like chance of rain. **Gauge** marks the value between min and max, like temperature. |
| Text | What to show. `{{path}}` is replaced with that value from the response, for example `{{results.data.points[1].value}} kW`. Up to 12 characters once filled in. |
| Min, Max | Bar and gauge only. A number, or a `{{path}}` to one. The first `{{path}}` in Text is placed between them. |
| Refresh | Minutes between requests: 10 by default, 1 at least. |

The phone makes the requests, so the watch needs a connection to it. A value that can't be read shows as `--`.

Limits:
- Up to 8 custom complications.
- No request headers: the API has to work with a plain `GET`, any key in the URL.
- Numbers are rounded to 2 decimals. Text keeps only letters, digits, spaces and `% , - . / :`, so no `°` or accents.
- The bar and gauge use the accent color.
- On the round watch a bar with a 4-letter name and a long value leaves little room for the bar itself.
- A gauge in a subdial hides its min and max when either is longer than 3 characters.
- When a refresh fails the last value stays; `--` only shows if the first request fails.
- Two complications on the same URL make two requests.

### Good to know
- Weather, rain, humidity, wind, air quality, UV, elevation and sunrise/sunset need location access for the Pebble app and a connection to your phone.
- Steps, distance and heart rate come from Pebble Health, so it has to be turned on. Heart rate needs a watch with a sensor.

## Development
```sh
npm install
npm run emulator   # build and install on the emulator
npm run phone      # build and install on your watch
npm run config     # open the settings page in the emulator
npm test
```
The settings page is [src/pkjs/config.html](src/pkjs/config.html). `npm run build` inlines it into `src/pkjs/page.js`, so edit the HTML, not the generated file.


## Support
For issues, questions, or suggestions, please open an issue on GitHub.

## License
MIT License - feel free to modify and share!