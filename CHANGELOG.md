# Changelog

## 2.2.0

- **Temperature kinds:** Temperature gets a picker beside it, like Weather's. Today is the temperature on today's range, as before. Now shows the temperature alone, large ("21°"). Feels like shows what it feels like, large, and the temperature smaller and dimmer beside it ("19° 21°"); in a subdial the temperature goes under it. Corners and subdials.

## 2.1.0

- **Settings on Android:** tapping a row on the settings page opens its page again, and the phone's back button still returns to the list.

## 2.0.0

- **Older watches:** Ultra now runs on the Pebble Time, Pebble Time Round, Pebble 2 and Pebble 2 Duo. They have no corner complications: the dial fills the screen, with the four subdials, every ring and every hand style, in the same smooth text as on the Time 2 and Round 2.
- **Black and white watches:** on the Pebble 2 and Pebble 2 Duo the color scheme is White on black or Black on white, and it starts as White on black.
- **New settings page:** one short list, each row showing what is set now (the corners' complications, the scheme, the ring, the hands, the weather). Tap a row to change it; the phone's back button returns to the list. The preview, what is saved and the Save button stay at the top on every page. Units, the step goal and the switches are right on the list.
- **Only what your watch shows:** the settings page hides what the watch can't show: the corners on the older watches, and the accent, background, hand and band colors on black and white ones. The preview draws your watch's own dial.
- **Rectangular ring:** on the Pebble Time, Pebble 2 and Pebble 2 Duo, a new switch under Ring makes the ring follow the screen's edge: every ring's ticks, numerals and band on a rounded rectangle, the numerals upright. The subdials stay round.
- **Joined corners:** two corners can now be one complication, its bar or gauge running on through the gap between them. On the Time 2, join the top two, the bottom two, or both. On the Round 2, join any neighbors: the top, bottom, left or right pair, two pairs, three corners, or all four as one closed ring, its labels at 6 o'clock. Switches at the top of the Corners page; a joined corner's picker goes, and the run shows the complication picked in the corner at its left end (the bottom one for a side). On the Time 2 its labels and icon stay in that corner; on the Round 2 text centers on the joined arc, and side bars fill from the bottom up.
- **Wider corners on the Round 2:** each corner complication now spans 65 degrees, centered on its corner, up from 45.
- **Outlined hands:** every hand style but Outline, the seconds hand and the pin have a thin outline in the background color, so they stand out over the dial, the band, the subdials and each other.
- **Weather in the corners:** the Weather complication now fits a corner too: its icon toward the corner and the conditions curved along the arc ("CLOUDY", "PT CLOUDY"). On the Round 2 the icon leads them.
- **Weather with temperature:** Weather gets a second kind, Icon + temp. In a corner the temperature goes before the conditions ("21° CLOUDY"); in a subdial the icon is smaller and the temperature sits under it.
- **Temperature gauge on the Round 2:** the Temperature corner is a gauge again, now that the corners have room: the temperature leads, then today's min, the colored arc and its thumb, and the max, min and max dimmer. Custom gauges get the same layout. The icon-and-temperature look lives on as Weather, Icon + temp.
- **Languages:** the face and the settings page in English, French, German, Dutch, Italian or Spanish. A new Language picker under Data; Auto, the default, follows the watch's language. On the face: weekdays and months, the weather's conditions, the Date heads (week, day), the Active and Sleep captions, the wind's and the compass ring's points. The city search answers in it too.
- **Accented letters:** the watch font has them now (É, Ä, Ñ, Ü, Ã, Ø, Å, Œ…), so the Text complication keeps them, and so do city names: "Zürich", not "Zrich", and "São Paulo", "København". A letter it still lacks loses its accent, or becomes its base letter: "Łódź" is "Lódz".
- **Hourly rain:** Chance of rain gets a second kind, Hourly: six sections from the hour before this one to four hours on, a darker blue the likelier rain is, none for no rain. This hour's section is ringed in the accent and its chance shown beside it. Corners and subdials. The sections move on with the hour between fetches.
- **Days until / since:** a new complication counts the days to a date, or since it once it has passed: "64 DAYS LEFT", "3 DAYS AGO". Pick the date per corner or subdial, and name it with the text field beside it ("XMAS"); the name goes beside the arc in a corner, above the count in a subdial.
- **Month, week and day progress:** Year progress is now Progress, with four kinds: Year, Month, Week and Day. Month shows how much of the month is gone under its name, Week how much of the week (from the day it starts on, see below), Day how much of the day under the weekday.
- **Week starts on:** a new setting under Data, Monday, Sunday or Saturday. The Week complication's sections and Week progress start on that day. Monday keeps ISO week numbers; from Sunday or Saturday, week 1 is the one 1 January is in.
- **Pressure:** a new weather complication, sea-level pressure. In a corner the value along the arc and the unit beside it ("1013 hPa" on one line on the Round 2); in a subdial on a barometer's scale from 970 to 1050 hPa, blue for low to yellow for high. In inHg with imperial units.

## 1.9.0

- **Minimal ring:** the hour hand is 15% shorter, so it no longer reads like the minute hand.

## 1.8.0

- **Battery saver:** the hands keep the right time while the face sleeps, instead of falling up to an hour behind. Walking now wakes the face even when the watch still thinks you are asleep.

## 1.7.0

- **Settings on Android:** the settings page no longer jitters or sticks at the top when you scroll past the preview.

## 1.6.0

### Rings

- **Band color:** pick the band's color on the Sport, Chronograph, Tachymeter and Compass rings. Ticks and numerals turn black or white to stay readable on it.
- **Roman ring:** new Roman ring, the Default dial with its hours in Roman numerals, turned along the dial.
- **Tachymeter ring:** new Tachymeter ring: ticks for the seconds around the edge, and inside them a colored band with 10 to 60 and a dot between each. The band follows the seconds color, or takes its own.
- **Compass ring:** new Compass ring, a compass bezel: N, E, S and W and the degrees between, 30 to 330, on a band, a tick every 5 degrees. It is fixed and does not turn to north. The band takes a color.

### Hands

- **No hands:** new None hand style hides the hour and minute hands, for a face that tells the time in a complication. The seconds hand keeps its own switch.
- **Sweeping seconds:** new switch under Seconds hand. The hand glides, 8 steps a second, instead of ticking. It drains the battery much faster.
- **Seconds hand rests:** it stops during Quiet Time and at 20% battery or less, and comes back on its own.
- **Shake to hide hands:** new switch, under Experimental. Shake the watch twice and the hands step aside for 5 seconds, so the center complications show whole.

### Complications

- **New complications:** digital time, time zones (one per slot, picked by abbreviation: PST, CET, IST), sleep last night, active minutes and moon phase. All in corners and subdials.
- **Week and Year progress:** two new complications. Week: seven sections, Monday first, lit up to today, with the weekday and the day. Year progress: a bar of how much of the year is gone.
- **Location:** new complication with the city your phone is in: its name in a corner, a short code (SEA, NY) in a subdial. Named by BigDataCloud, asked only while the complication is on the face.
- **Date:** the Calendar complication is now Date, with a format per slot: Calendar (as before), Weekday and day, Month and day, Full date, Numeric, Week number, Day of year or Year. Calendar (plain) is now Date as Weekday and day; a face showing it keeps its look.
- **Calories bar:** both calories complications now show a bar, in corners and subdials. Like distance, it fills with progress toward the daily step goal.
- **Fewer picks, same complications:** Air quality, UV index and Calories are one pick each. A second picker beside it sets the kind: Sections or Gauge, Total or Active. Saved faces keep theirs.
- **Grouped pickers:** the complication dropdowns are sorted into Activity, Weather, Time and date, Sun and moon, Place, Watch and Custom.
- **Preview fills in:** pick a complication the phone has not fetched for yet (air quality, location, wind) and the settings preview draws it with sample values, not `--`.

### Weather

- **Weather refresh:** pick how often the weather refreshes, from 15 minutes to 3 hours.
- **Weather location:** search a city and pick it, for weather somewhere other than where the phone is. The phone's position is then not asked.

### Battery

- **Lighter on the battery:** the face is drawn again only when something on it changed, not every minute, and only the hands move in between (on the round watch, with Line or no hands). Heart rate redraws on a change of 3 bpm, total calories every 10. The phone asks for weather, air quality and the place only when a complication showing them is on the face.
- **Battery saver:** new switch, under Experimental. After an hour with no steps and no pulse, the face sleeps: one redraw an hour, no seconds hand, weather and custom complications once an hour. A step, a pulse or a shake wakes it.

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
