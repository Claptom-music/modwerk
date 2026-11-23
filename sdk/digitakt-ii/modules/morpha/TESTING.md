# Morpha testing

Not tested on a unit yet. Everything below is emulator only; an emulator run never proves sound-chip code (sdk/machines/digitakt-ii/README.md, SHARC code rules).

## Emulator, 23 November 2026

Coupled ColdFire and sound-chip runs (interpreter, from a ready point saved with each image's own sound-chip code) of a private OS 1.17 image: the Digitakt II development base with linked voices (`base/dt2-linked-voices`, [#2005](https://github.com/repeat98/modwerk/pull/2005)) and Morpha's package composed as the site does (`composeDt2MachineBuild`). The test card's one-second 48 kHz 220 Hz tone in slot 1 on track 1; Morpha chosen through the menu (FUNC + SRC, DOWN to the end, YES twice); knobs turned on the SRC page with small encoder steps; two taps of trig key 1 about half a second apart.

| Run | Result |
| --- | --- |
| Menu and SRC page (panel probe) | MORPHA is the last row of MACHINE SEL; after YES the SRC page reads "Morpha" with POS, DETN and SUB under knobs E-G. Track 1's type byte is 9. |
| First builds | A run restored from a ready point saved with the stock sound-chip code never entered the player (the stock code ran). With the image's own code: the pitch came out 0, as the assembler loads an `.equ` holding a float as an integer; the float is now written in the instruction. Then the waves read as whole 32-bit words: a dump of the reads showed the 220 Hz tone repeating every 109.5 words, so each word holds two 16-bit samples, the earlier one in the low half. Fixed in the reader and the built-in table. |
| Default knobs, the test tone as the table | Both notes sound. The tone's first 2,048 samples repeat at 261.6 Hz (middle C at the track's default note): the record's step counts quarter samples (0x40000000 = one sample a sample). |
| Level against SAMPLE | The same notes on SAMPLE peak at 0.063; Morpha's unscaled read of the same sample peaked at 0.157. Morpha now scales by 0.4, SAMPLE's own level. |
| No sample (knob D turned down) | The built-in table plays: its first wave, a sine, peaks at 0.062 (SAMPLE: 0.063), every other harmonic 56 dB or more down. The pitch was 262.3 Hz, 5 cents sharp: the stock setup multiplies the step by the sample's rate (record word 0x61), which an empty slot leaves over. Morpha now divides the rate out (not yet re-run). |
| Cost (entry and exit of the player traced over 9,544 calls) | With DETN, SUB, AMT and POS up: 6,380 emulator instructions per block for a sounding voice; the track's second voice, which copies the first, 152; a track with no note, 171. A stock SAMPLE note takes about 7,000 (two voices). |

## Still to test

- The knobs one by one: POS through the built-in table, each WARP with AMT, DETN, SUB.
- The pitch with no sample after the rate fix; TUNE and notes across the keyboard.
- The view words on the ColdFire (`core_machine_view`), and the screen view once chosen.
- Linked voices: chords on a lead with Morpha.
- Trig locks, LFOs on POS, save and reload, a kit with Morpha on a build without it (expected: SAMPLE).
- A real wavetable file (waves of 2,048 samples), mono and stereo.
- On a unit: everything above, by ear, and the cost with Morpha on several tracks.
