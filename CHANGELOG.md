# Changelog

## 1.6.0

- **Band color:** pick the band's color on the Sport and Chronograph rings. Ticks and numerals turn black or white to stay readable on it.
- **Roman ring:** new Roman ring, the Default dial with its hours in Roman numerals, turned along the dial.
- **Tachymeter ring:** new Tachymeter ring: ticks for the seconds around the edge, and inside them a colored band with 10 to 60 and a dot between each. The band follows the seconds color, or takes its own.
- **Compass ring:** new Compass ring, a compass bezel: N, E, S and W and the degrees between, 30 to 330, on a band, a tick every 5 degrees. It is fixed and does not turn to north. The band takes a color.
- **Weather refresh:** pick how often the weather refreshes, from 15 minutes to 3 hours.
- **Weather location:** search a city and pick it, for weather somewhere other than where the phone is. The phone's position is then not asked.
- **Lighter on the battery:** the face is drawn again only when something on it changed, not every minute, and only the hands move in between (on the round watch, with Line or no hands). Heart rate redraws on a change of 3 bpm, total calories every 10. The phone asks for weather, air quality and the place only when a complication showing them is on the face.
- **Sweeping seconds:** new switch under Seconds hand. The hand glides, 8 steps a second, instead of ticking. It drains the battery much faster.
- **Shake to hide hands:** new switch. Shake the watch twice and the hands step aside for 5 seconds, so the center complications show whole.
- **Seconds hand rests:** it stops during Quiet Time and at 20% battery or less, and comes back on its own.
- **Calories bar:** both calories complications now show a bar, in corners and subdials. Like distance, it fills with progress toward the daily step goal.
- **New complications:** digital time, time zones (one per slot, picked by abbreviation: PST, CET, IST), sleep last night, active minutes and moon phase. All in corners and subdials.
- **Date:** the Calendar complication is now Date, with a format per slot: Calendar (as before), Weekday and day, Month and day, Full date, Numeric, Week number, Day of year or Year. Calendar (plain) is now Date as Weekday and day; a face showing it keeps its look.
- **Week and Year progress:** two new complications. Week: seven sections, Monday first, lit up to today, with the weekday and the day. Year progress: a bar of how much of the year is gone.
- **Grouped pickers:** the complication dropdowns are sorted into Activity, Weather, Time and date, Sun and moon, Place, Watch and Custom.
- **Location:** new complication with the city your phone is in: its name in a corner, a short code (SEA, NY) in a subdial. Named by BigDataCloud, asked only while the complication is on the face.
- **No hands:** new None hand style hides the hour and minute hands, for a face that tells the time in a complication. The seconds hand keeps its own switch.

## 1.5.0

- **Ring styles:** new Ring setting with Default, Minimal (ticks only), Sport (white band, minute numerals, accent ring) and Chronograph (black-and-white band, hours 1 to 12).
- **Bigger center:** on Minimal, Sport and Chronograph the subdials grow to fill the larger center and the hands run longer.
- **More hand styles:** Pointer, Sword and Dauphine, next to Line, Bar and Outline.
- **Calories:** new complication, as a total (active and resting) or active only, in corners and subdials.
- **AQI and UV gauges:** air quality and UV index as a range gauge, as well as sections.
- **Smaller % sign** in subdial values, and in corner values on round watches.
- **Fixes:** sunrise and sunset subdial at night, weather icon on the round watch, hour hand length.
- New screenshots and banner.

## 1.3.0

- **Hand styles:** Line, Bar or Outline for the hour and minute hands.
- **Hand colors:** a color each for the hour, minute and seconds hands. The seconds color is also the 12, 3, 6 and 9 notches'.

## 1.2.0

- **Temperature gauge:** the arc is a gradient from blue to red, from today's low to its high.
- **AQI and UV:** drawn as a section per band, lit in each band's own color up to the current value.
- **Battery:** green over 70%, red under 15%, yellow between and while charging.
- **Calendar:** today is highlighted in light red.
- Bright yellow is darkened on light backgrounds so it stays readable.

## 1.1.0

- **Custom complications:** show a value from any JSON API as text, a bar or a gauge. Up to 8, in any corner or subdial.
- **New complications:** sunrise and sunset, wind, .beat time, and a plain calendar subdial.

## 1.0.0

- First release: an analog face for Pebble Time 2 (emery) and Pebble Time Round 2 (gabbro).
- **Eight slots:** four corner gauges and four subdials.
- **Complications:** steps, distance, heart rate, battery, calendar, temperature, chance of rain, humidity, air quality, UV index, elevation, current weather and your own text.
- **Color schemes:** Black, White, White on black, Black on white, or Accent with your own background and accent colors.
- **Options:** metric or imperial units, daily step goal, seconds hand.
