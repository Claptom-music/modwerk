# Morpha testing

## Unit, 23 November 2026, test build `063eea9a` (reported by the author)

Digitakt II OS 1.17 with the development base and linked voices, Morpha on track 1, loaded over USB with Transfer.

| Check | Reported |
| --- | --- |
| Pitch, TUNE and the keys | Pass. |
| POS through the built-in table | Moves, but in steps rather than smoothly. |
| WARP with AMT | Changes the tone, but AMT jumps from the plain wave to full bend with nothing between. |
| DETN, MIX | Pass. The sub should not go below the audible range when TUNE goes down. |
| A wavetable file, POS | Pass, but in steps. |
| The view | Moves, not smoothly; most visible with WARP. |
| Chords with VOICES | Each new note and each key release moves to another track, each track sounding a different wave; letting go of the second key of a two-key chord leaves a note sounding that jumps to the first key's pitch. Only on Morpha. |
| TRIG page NOTE turned while a note sounds | The pitch does not follow (not yet compared with SAMPLE). |
| Save, load another project, come back | Pass. |
| Morpha on 4 or more tracks | Pass: no crackles, slowdowns or freezes. |

## Unit, 23 November 2026, test build `629ee977` (reported by the author)

| Check | Reported |
| --- | --- |
| WARP at FOLD | Plays nothing and the track stops answering notes; switched to FOLD while a note plays, a high note starts and the unit needs a power cycle. |
| AMT (knob C) | Morphs very well on the note playing; a following note (C, then E) did not seem to keep the change. |
| POS (knob E) | Moves smoothly, but sounds like a blend of waves rather than one wave changing shape; more noticeable with a sample than with the built-in table. |
| The second SRC page, FUNC held | The view fills up, then disappears when FUNC is released. |
| TRIG page NOTE turned while a note plays | Morpha's pitch does not follow, with or without a sample. |

## Emulator, 23 November 2026 (after the second unit test)

| Run | Result |
| --- | --- |
| WARP at FOLD (build `629ee977`) | The coupled emulator's sound chip stopped at the player's `IF NE RTS` in the oscillator, the one conditional return that FOLD runs past instead of taking. The assembler encodes a conditional RTS in a form that only works when it returns; the player now jumps to a plain RTS. After: FOLD plays (the default note, peak 0.061), no stop. |
| AMT and POS on the next note, one track | AMT turned up during a first note: the second note on the same track keeps it (same spectrum). POS likewise. |
| AMT on the next note, track 1 leading 3 (test-only build flag) | AMT turned during note 1 on track 1; note 2 on track 2 sounds bent as well. The unit report is not reproduced here. |
| Built-in table of 48 waves | Assembled: the table fills Morpha's L2 range to `0x200fa000`. |
| The view and FUNC (host build of `src/main.c`) | Compiles and draws as before; the page now keeps asking for redraws for 90 draws after the sample view stops being drawn. Not seen on a screen. |

## Emulator, 23 November 2026 (after the first unit test)

Same coupled runs as below, on development base builds of `base/dt2-linked-voices` with the changes noted.

| Run | Result |
| --- | --- |
| Why AMT jumped (trace of the knob value at the player while knob C turned during a held note) | The parameter record's word `0x28` (SAMPLE's crossfade) read 0, then 1.0 at the first detent, and nothing else: the sound chip gets that knob as off or on. Granula's SIZE reads the same word. |
| The base sends knobs C and E at full resolution (one wire word a track) | A probe of the ColdFire's parameter targets while knob C turned showed slot 27 at `0x0a00`, `0x1400`, `0x3200`, `0x6400`, `0x7f00` (the scripted encoder accelerates), START at slot 31. A first version with two words a track (C, E, F, G) garbled every track's sound; one word a track (C, E) leaves the default note as before (peak 0.0629, the same harmonics). |
| AMT turned during a held note | The tone moves through in-between spectra over about 0.16 s (the scripted turn's speed) to the full BEND, instead of jumping. |
| POS turned during a held note | The built-in waves pass one after another (sine, then mixed spectra, to the vowel waves) instead of jumping. |
| Glide | Every knob is glided (time constant about 10 ms), so a knob's steps become a sweep. |
| The sub with TUNE far down and MIX at 0 | The sub stays at or above C1 (65.5 Hz measured at TUNE's lowest with the default note). |
| Linked voices, track 1 leading 3 (test-only build flag), two taps then a tap of trig key 2 | The second note plays on track 2 with Morpha over the first; track 2's own tap is silent while it is linked. |

## Emulator before the first unit test

Everything below is emulator only; an emulator run never proves sound-chip code (sdk/machines/digitakt-ii/README.md, SHARC code rules).

### 23 November 2026

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
