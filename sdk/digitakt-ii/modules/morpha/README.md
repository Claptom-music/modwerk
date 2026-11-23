# Morpha

A Digitakt II machine by @Claptom-music. Version 0.1.0-experimental. OS 1.17, core 1.1, machine type 9.

## Overview

Morpha is a wavetable synth machine. A wavetable is a row of single waves; Morpha plays one of them over and over at the pitch of the note, and POS chooses where in the row. Between two waves it morphs smoothly, so turning POS, locking it on trigs or sweeping it with an LFO changes the tone while the note plays.

The table is the track's sample, cut into waves of 2,048 samples each, the usual size of wavetable files: load a wavetable as a sample and choose it with knob D. A sample shorter than 2,048 samples plays as one wave. With no sample, Morpha plays its built-in table of 16 waves: sine, triangle, saw, square, a narrow pulse and two vowel-like waves, with gradual steps between them.

WARP bends each wave in one of four ways, as far as AMT says: BEND pushes the wave towards its ends, SYNC restarts it up to four times a cycle for the bright, tearing sound of a synced oscillator, MIRR plays it forward and then backward, and FOLD folds its level back on itself. DETN adds two copies detuned up and down for a wide unison, and MIX blends in a sine an octave below.

The track's filter, amp envelope, LFOs, sends and level shape Morpha as they shape any machine. For chords, link tracks as voices with the Digitakt II base's VOICES setting: each note of a chord then plays on its own track.

## Controls

The SRC page, while a track plays Morpha:

| Knob | Name | What it does |
| --- | --- | --- |
| A | TUNE | The pitch. At the track's default note Morpha plays middle C. |
| B | WARP | BEND, SYNC, MIRR or FOLD. The knob has four positions, as SAMPLE's play mode: FOLD, MIRR, SYNC, BEND. |
| C | AMT | How much WARP bends the wave. At 0 the wave plays as it is. |
| D | SAMP | The wavetable sample, or none for the built-in table. |
| E | POS | Where in the table: the first wave at 0, the last at full. |
| F | MIX | The wave against a sine one octave below. Full (where a new track starts): the wave alone. Turned down, the sine comes in; at the middle both at full; at 0 the sine alone. |
| G | DETN | Unison. At 0 (where a new track starts) one oscillator; turned up, two more, detuned up and down by up to half a semitone and louder as you turn. |
| H | LEV | The track level. |

The SRC page's second page (press SRC again) shows the wavetable in place of the sample's waveform: twelve of the table's waves stacked from front (the first) to back (the last), dotted, and the wave you hear drawn bold at POS's place in the stack, warped as WARP and AMT bend it. It shows the table with or without a note playing; a new sample's waves appear over about a second and a half.

Morpha's knobs are SAMPLE's own parameters under new names, so trig locks, live recording, MIDI and LFOs work on them as on SAMPLE. Every knob acts at once, while the note plays.

## Usage

1. Select a track, hold FUNC and press SRC, scroll down past the stock machines to MORPHA and press YES.
2. On the SRC page turn knob D down to no sample: you hear the built-in table's sine.
3. Turn POS slowly: the sine turns into a triangle, a saw, a square, a pulse and the vowels.
4. Press SRC again to see the table as a stack of waves, then turn AMT up and try each WARP position on knob B: the bold wave shows what it does.
5. Assign an LFO to POS (the START destination) for a tone that keeps moving.
6. Turn DETN up for a wide unison, and MIX down a little for weight from the sub.
7. Load a wavetable file (a WAV made of single waves of 2,048 samples) with Elektron Transfer and choose it with knob D.
8. For chords: open SETTINGS on the track, set VOICES to 3 or 4, and play chords from the keys or over MIDI.

## Compatibility and limitations

- Needs the Digitakt II base with added machines (core 1.1). Polyphony needs the base's linked voices (SETTINGS > VOICES).
- Machine type 9, claimed as `machine:9`: kits save it, and a kit with Morpha loads as SAMPLE where Morpha is not installed.
- Morpha's sound-chip memory is L2 `0x200f0000`-`0x200fa000` (board claim `dsp-region digitakt-ii/morpha-l2`).
- The waves are read without band-limiting: bright waves played high can alias. The built-in table keeps its harmonics low enough for most of the keyboard.
- A stereo sample is read as if it were mono. Use mono wavetables.
- The wavetable view replaces the sample view's waveform on a Morpha track; its play markers are not drawn there.
- Cost on the sound chip: see [TESTING.md](TESTING.md). Not measured on a unit yet, where reading the sample memory may cost more.

## How it works

The base sends Morpha's voices to its sound-chip player (`src/player.sharc`). Each block of 32 samples the player reads the note's pitch from the voice's record (TUNE, the note, pitch locks and LFOs, as SAMPLE would play a sample), works out the two waves around POS and how far between them, and for each sample advances up to three oscillators' phases, bends each phase as WARP and AMT say, reads the two waves at that phase (two neighbouring samples each, linearly) and mixes them by POS. The sub reads the built-in sine at half the speed. The second of a track's two voices copies the first's block, so the two never drift apart and a note costs one voice's work.

For the screen, the player sends the base's machine view one word a block: four points of a wave, 96 points a wave, the sounding wave and one of twelve table waves in turn; `src/main.c` keeps each whole wave and draws the stack over the sample view's waveform box, which the base counts as it draws.

The built-in table is made by `tools/builtin-table.mjs` from formulas: each wave a sum of up to 48 harmonics, the waves between the key shapes their spectra in between.

## Tests and measurements

See [TESTING.md](TESTING.md). Not tested on a unit yet.

## Authorship and licences

Original work by Claptom-music, GPL-3.0-or-later. No stock code, tables or samples are copied, and the built-in waves are computed from formulas.
