/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Morpha for the Digitakt II: a wavetable synth machine (machine type 9, standing for SAMPLE).
 *
 * The base lists Morpha in the track's machine list and plays its voices through Morpha's sound-chip player
 * (src/player.sharc). Its knobs are SAMPLE's own parameters under Morpha's names, so saving, trig locks, live
 * recording, MIDI and LFOs work as on SAMPLE:
 *
 *   A TUNE   the pitch (SAMPLE's TUNE)
 *   B WARP   how the wave is bent (SAMPLE's play mode): BEND, SYNC, MIRR or FOLD
 *   C AMT    how much warp (SAMPLE's crossfade, which SAMPLE itself does not show)
 *   D SAMP   the wavetable: a sample cut into waves of 2,048 samples, or no sample for the built-in table
 *   E POS    which wave of the table plays, morphing smoothly between neighbours (SAMPLE's START)
 *   F DETN   unison: one oscillator at 0, three detuned ones turned up (SAMPLE's LENGTH)
 *   G SUB    a sine one octave below (SAMPLE's LOOP position)
 *   H LEV    the track level
 *
 * Polyphony comes from the base's linked voices (SETTINGS > VOICES): the voice tracks play a lead's notes with the
 * lead's machine, so each note of a chord plays on its own track.
 */
#include <stdint.h>
#include <mk2/machines.h>

/* Indexed by SAMPLE's play-mode value: 0 REV, 1 REV.L, 2 FWD.L, 3 FWD. */
static const char *const warps[] = { "FOLD", "MIRR", "SYNC", "BEND", 0 };

const struct dt2_machine morpha_machine = {
    .type = 9,
    .stands_for = 0, /* SAMPLE */
    .strings = { "Morpha", "MRP", 0 },
    .knobs = {
        { "Warp Mode", "WARP", warps },
        { "Warp Amount", "AMT", 0 },
        { "Position", "POS", 0 },
        { "Detune", "DETN", 0 },
        { "Sub Osc", "SUB", 0 },
    },
};
