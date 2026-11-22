/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Granula for the Digitakt II: a granular synth machine (machine type 8, standing for SAMPLE).
 *
 * The base lists Granula in the track's machine list and plays its voices through Granula's sound-chip player
 * (src/player.sharc). Its knobs are SAMPLE's own parameters under Granula's names, so saving, trig locks, live
 * recording, MIDI and LFOs work as on SAMPLE:
 *
 *   A TUNE   the grains' pitch (SAMPLE's TUNE)
 *   B MODE   SAMPLE's play mode: FWD plays every grain forward, REV every grain backward, and both loop positions
 *            (REV.L and FWD.L, values 1 and 2) RAND, where each grain picks its direction at random
 *   C SIZE   grain length (SAMPLE's crossfade, which SAMPLE itself does not show)
 *   D SAMP   the sample the grains come from
 *   E POS    where in the sample the grains start (SAMPLE's START)
 *   F DENS   how many grains start per step of the track (SAMPLE's LENGTH)
 *   G SPRD   how far each grain's start scatters around POS (SAMPLE's LOOP position)
 *   H LEV    the track level
 *
 * Polyphony comes from the base's linked voices (SETTINGS > VOICES): the voice tracks play a lead's notes with the
 * lead's machine, so each note becomes its own grain cloud on its own track.
 */
#include <stdint.h>
#include <mk2/machines.h>

/* Indexed by SAMPLE's play-mode value: 0 REV, 1 REV.L, 2 FWD.L, 3 FWD. */
static const char *const modes[] = { "REV", "RAND", "RAND", "FWD", 0 };

const struct dt2_machine granula_machine = {
    .type = 8,
    .stands_for = 0, /* SAMPLE */
    .strings = { "Granula", "GRN", 0 },
    .knobs = {
        { "Grain Mode", "MODE", modes },
        { "Grain Size", "SIZE", 0 },
        { "Position", "POS", 0 },
        { "Density", "DENS", 0 },
        { "Spread", "SPRD", 0 },
    },
};
