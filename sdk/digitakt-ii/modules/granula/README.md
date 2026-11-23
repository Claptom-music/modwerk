# Granula

A Digitakt II machine by @Claptom-music. Version 0.1.0-experimental. OS 1.17, core 1.1, machine type 8.

## Overview

Granula is a granular synth machine. Choose it in a track's machine list and every note starts a cloud of short grains cut from the track's sample. POS picks where in the sample the grains come from and SPRD scatters them around it, SIZE sets how long each grain lasts and DENS how many start per step of the track, so the cloud breathes with the tempo and the track's speed. MODE plays every grain forward, every grain reversed, or each one either way at random.

Each grain plays through the Digitakt's own sample player, so TUNE, the note you play and the sample's rate set its pitch, and the track's filter, amp envelope, LFOs, sends and level shape the cloud as they shape any machine.

For chords and clouds that overlap, link tracks as voices with the Digitakt II base's VOICES setting: each note then plays on its own track, as its own cloud.

## Controls

The SRC page, while a track plays Granula:

| Knob | Name | What it does |
| --- | --- | --- |
| A | TUNE | The grains' pitch. |
| B | MODE | FWD: every grain forward. REV: every grain reversed. RAND: each grain either way, at random. The knob has four positions, as SAMPLE's play mode: REV, RAND, RAND, FWD. |
| C | SIZE | How long each grain lasts, from about 5 ms to about 1 s. |
| D | SAMP | The sample the grains come from. |
| E | POS | Where in the sample the grains come from. |
| F | DENS | How many grains start per step of the track, from 1 to 16. |
| G | SPRD | How far each grain's start scatters around POS, up to half the sample either way. |
| H | LEV | The track level. |

Granula's knobs are SAMPLE's own parameters under new names, so trig locks, live recording, MIDI and LFOs work on them as on SAMPLE. Every knob acts on the next grain: a turn changes the cloud as it plays.

A cloud lasts as long as the sample does at its own speed, wherever POS is: a 4 s pad gives 4 s clouds. The amp envelope shapes it and can end it sooner, as on a synth. For longer clouds, use a longer sample.

## Usage

1. Select a track, hold FUNC and press SRC, scroll past MIDI to GRANULA and press YES.
2. On the SRC page choose a long sample with knob D: a pad, a field recording or a vocal.
3. Play a note. Turn SIZE up for smooth, long grains and down for a buzzing, rhythmic texture.
4. Turn DENS up for a dense cloud; change the track's speed (or the tempo) to hear the grains follow it.
5. Set MODE to RAND and SPRD to about half for a shimmering, shifting cloud; sweep POS slowly through the sample.
6. For chords: open SETTINGS on the track, set VOICES to 3 or 4, and play chords from the keys or over MIDI.

## Compatibility and limitations

- Needs the Digitakt II base with added machines (core 1.1). Polyphony needs the base's linked voices (SETTINGS > VOICES).
- Machine type 8, claimed as `machine:8`: kits save it, and a kit with Granula loads as SAMPLE where Granula is not installed.
- Granula's sound-chip memory is L2 `0x200d0000`-`0x200f0000` (board claim `dsp-region digitakt-ii/granula-l2`).
- B's two loop positions both play RAND.
- Cost on the sound chip (emulator): a sounding note takes up to about 34,000 instructions of a block's 666,667, about 5% (two voices of four grains each at full SIZE; a stock SAMPLE note about 7,000). A few Granula notes at once are fine; Granula on every track with long grains is not, and is not measured on a unit yet.
- Up to four grains sound at once per voice. With a long SIZE, DENS starts no more grains than that: a 1 s grain allows one start every quarter second.

## How it works

The base sends Granula's voices to its sound-chip player (`src/player.sharc`). Each block of 32 samples the player plays the voice's own record, looping over the whole sample, into a block nobody hears (the guide, which tells it a note is playing), starts a grain when the countdown from DENS runs out, plays each sounding grain through the stock sample player from its own copy of the record, and adds it into the voice's output through a smooth window. The window's peak gain falls as grains overlap more, so a dense cloud is about as loud as a sparse one.

## Tests and measurements

See [TESTING.md](TESTING.md). Not tested on a unit yet.

## Authorship and licences

Original work by Claptom-music, GPL-3.0-or-later. No stock code, tables or samples are copied: the player calls the OS's own sample player at its address.
