# Granula testing

Not tested on a unit yet. Everything below is emulator only; an emulator run never proves sound-chip code (sdk/machines/digitakt-ii/README.md, SHARC code rules).

## Emulator, 23 November 2026

Coupled ColdFire and sound-chip runs (interpreter, from a ready point saved with each image) of a private OS 1.17 image: the Digitakt II development base with linked voices (`base/dt2-linked-voices`, [#2005](https://github.com/repeat98/modwerk/pull/2005)) and Granula's package composed as the site does (`composeDt2MachineBuild`). The test card's one-second 48 kHz tone on track 1; Granula chosen through the menu (FUNC + SRC, DOWN to the end, YES twice); two taps of trig key 1 about half a second apart.

| Run | Result |
| --- | --- |
| First build | Silent. A trace showed the player reached and grains started, but each grain's window step was overwritten (on the sound chip `R1` and `F1` are one register), so every window stayed at 0; the same slip scrambled the grain's start. Fixed. |
| Default knobs (SIZE 0, DENS full, POS 0, SPRD 0, MODE FWD) | Both notes sound, peak 0.058 (SAMPLE: 0.063), RMS about 0.02. The 1 ms envelope shows a burst of about 5 ms every 9.2 ms: 256-sample grains, 16 a step at the test project's 102 BPM (7,058 samples a step / 16 = 441). The cloud ends when the sample would. |

| SIZE turned to full (knob C, three turns of 100) | First build: grains still about 5 ms and the level halved. A trace showed two faults: a new grain took over a slot whose grain still sounded (four slots, a start every 441 samples, 1 s grains), and the pair of voices a note plays on read different SIZE values (only the first voice's parameter record had the turn). Fixed: at most four grains overlap (the interval grows with SIZE), and both voices read the pair's first record. After the fix: the cloud swells in over about half a second and stays smooth (RMS 0.029 against 0.020 with SIZE 0), and the first note's cloud carries on into the second. |
| Linked voices, track 1 leading 3 tracks (base test flag) | The second note's cloud plays over the first: peak 0.088 against 0.058 for one cloud. Where they overlap the 1 ms envelope is still made of grain bursts, so the voice track plays Granula, not SAMPLE: its lead's machine type reached the sound chip. |

| MODE turned to REV (knob B fully down) | Every grain reversed (the record's own negative step); each burst's peak 0.056-0.058, as forward. |
| MODE at RAND (knob B down, then up in steps of 8: REV.L) | A trace shows each grain's step made positive, then negated when its random bit is set, so grains go both ways. First build: about two thirds of the bursts peaked at 0.033 instead of 0.056: the note's pair of voices, which sound together, chose directions apart, and a forward and a reversed grain partly cancelled. Fixed: the pair's second voice takes the first voice's random bits, so their grains match. After the fix every burst peaks at 0.052-0.058. |

| POS turned to its end, SPRD up (knobs E and G, small turns) | First build: the note's first block still held the previous sample's length (368 samples against 48,000), and a forward grain placed at the sample's very end played nothing; each cloud lasted a quarter second, as the guide ran from POS to the end. Fixed: the first grain waits one block, a forward grain starts early enough to fit, and the guide loops, so a cloud lasts until the amp envelope ends it. After the fix: grains start between words 21,320 and 23,871 of 24,000, scattered by SPRD; both notes play at full level (peak 0.064) and fade with the amp envelope over about 5 s. |
| Default knobs with the looping guide | The cloud fades with the amp envelope (peak 0.058 down to 0.0002 over 5 s) instead of stopping at the sample's end. No grain reversed (a trace of the negation found none), so the play mode reads right every block. |

## Still to test

- SIZE, DENS, POS and SPRD across their ranges; MODE REV and RAND (reversed grains).
- Linked voices: chords and overlapping notes on a lead with Granula.
- Trig locks, LFOs, save and reload, a kit with Granula on a build without it (expected: SAMPLE).
- Cost per voice on the sound chip with every Granula voice playing; on a unit, the real cycle count.
- On a unit: everything above, by ear.
