# Morpha testing

Not tested on a unit yet. Everything below is emulator only; an emulator run never proves sound-chip code (sdk/machines/digitakt-ii/README.md, SHARC code rules).

## Emulator, 23 November 2026

Coupled ColdFire and sound-chip runs (interpreter, from a ready point saved with each image's own sound-chip code) of a private OS 1.17 image: the Digitakt II development base with linked voices (`base/dt2-linked-voices`, [#2005](https://github.com/repeat98/modwerk/pull/2005)) and Morpha's package composed as the site does (`composeDt2MachineBuild`). The test card's one-second 48 kHz 220 Hz tone in slot 1 on track 1; Morpha chosen through the menu (FUNC + SRC, DOWN to the end, YES twice); knobs turned on the SRC page with small encoder steps; two taps of trig key 1 about half a second apart.

| Run | Result |
| --- | --- |
| Menu and SRC page (panel probe) | MORPHA is the last row of MACHINE SEL; after YES the SRC page reads "Morpha" with its knob names. Track 1's type byte is 9. |
| First builds | A run restored from a ready point saved with the stock sound-chip code never entered the player (the stock code ran). With the image's own code: the pitch came out 0, as the assembler loads an `.equ` holding a float as an integer; the float is now written in the instruction. Then the waves read as whole 32-bit words: a dump of the reads showed the 220 Hz tone repeating every 109.5 words, so each word holds two 16-bit samples, the earlier one in the low half. Fixed in the reader and the built-in table. |
| Default knobs, the test tone as the table | Both notes sound. The tone's first 2,048 samples repeat at 261.6 Hz (middle C at the track's default note): the record's step counts quarter samples (0x40000000 = one sample a sample). |
| Level against SAMPLE | The same notes on SAMPLE peak at 0.063; Morpha's unscaled read of the same sample peaked at 0.157. Morpha scales by 0.4, SAMPLE's own level; with that, default knobs peak at 0.0629. |
| No sample (knob D turned down) | The built-in table plays: its first wave, a sine, peaks at 0.062, every other harmonic 56 dB or more down. First at 262.3 Hz, 5 cents sharp: the stock setup multiplies the step by the sample's rate (record word 0x61, read at `0x1c4c2e`), which an empty slot leaves over. With the rate divided out: 261.6 Hz. |
| The knob values (parameter record dump) | START (POS) and LENGTH arrive as floats 0-1 with or without a sample; a turn reaches the record gradually (smoothed over several blocks). LENGTH is full on a new track and the LOOP position 0, so MIX is on knob F (full: the wave alone) and DETN on knob G (0: one oscillator). |
| DETN (knob G up) | First build: one oscillator whatever the knob, as the unison flag was computed in `R0` after `F0` (one register) held DETN. Fixed. Now three oscillators: the partials spread (autocorrelation 0.77 against 1.00), RMS as one oscillator (0.042), peaks up to 0.108 where the three meet in phase. |
| MIX (knob F turned to 0) | The sub alone: 130.8 Hz, every other harmonic 90 dB or more down, peak 0.0615. |
| POS with the test tone (knob E up) | The later waves of the tone's table play (a different spectrum), same level. |
| Cost (entry and exit of the player traced over 9,546 calls) | With DETN, MIX in the middle, AMT and POS up: up to 14,977 emulator instructions per block for a sounding voice (mean 14,472); the track's second voice copies the first's block; a track with no note 174. A stock SAMPLE note takes about 7,000 (two voices). |
| Emulator stops | A build whose view routine began with a conditional return (`IF NE RTS`) stopped the coupled emulator there ("native-trap ... stop"), and a conditional compute after many forks ("fork"). Both forms are legal; the player now jumps to a plain return and uses MIN/MAX instead. Not an instrument result. |
| The view drawing (host) | `src/main.c` compiled on a PC with the view words a player posts (twelve table waves, then the sounding wave): the waterfall draws inside the waveform box only, the sounding wave bold at POS's depth, SYNC visible on it. Not seen in the coupled emulator (it saves no screen) nor on a unit. |

## Still to test

- POS through the built-in table; each WARP position with AMT on the built-in waves.
- TUNE and notes across the keyboard.
- The view on a real screen.
- Linked voices: chords on a lead with Morpha.
- Trig locks, LFOs on POS, save and reload, a kit with Morpha on a build without it (expected: SAMPLE).
- A real wavetable file (waves of 2,048 samples), mono and stereo.
- On a unit: everything above, by ear, and the cost with Morpha on several tracks.
