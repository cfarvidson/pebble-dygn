# Dygn

24-hour one-hand watchface for Pebble Time 2 (emery, 200x228). Clone of
[WatchPebble](https://github.com/cy8aer/WatchPebble) by cy8aer, without the
minute hand. Midnight (24) at the top, noon (12) at the bottom, one red hand
that makes one rotation per day.

![](screenshots/emery.png)

## Settings

Phone app, gear icon on the watchface:

- 12-hour numerals (off by default)

## Build and install

```
pebble build
pebble install --emulator emery
pebble install --phone <phone ip>     # developer connection on in the Pebble app
```

The bundle is `build/dygn.pbw`; it can also be sideloaded from the phone.

## Store listing

The appstore description is in `DESCRIPTION.txt`.
