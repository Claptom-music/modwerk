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

## Still to test

- SIZE, DENS, POS and SPRD across their ranges; MODE REV and RAND (reversed grains).
- Linked voices: chords and overlapping notes on a lead with Granula.
- Trig locks, LFOs, save and reload, a kit with Granula on a build without it (expected: SAMPLE).
- Cost per voice on the sound chip with every Granula voice playing; on a unit, the real cycle count.
- On a unit: everything above, by ear.
