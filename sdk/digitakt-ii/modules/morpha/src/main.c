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
 *
 * The wavetable view: while the SRC page's second page (the sample view) shows a Morpha track, the page asks the base
 * for that track's machine view, where the player sends twelve waves of the table and the wave that sounds (warped and
 * morphed), 96 points each (src/player.sharc, the view). In place of the sample's waveform it draws them as a small
 * waterfall: the table's waves stacked from front to back, dotted, and the sounding wave bold at POS's place in the
 * stack.
 */
#include <stdint.h>
#include <mk2/core.h>
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

/* The player's view words (src/player.sharc). */
#define VIEW_MAGIC 0x4d525031u
enum {
    VIEW_TRACK = 1, VIEW_POS = 2, VIEW_SHAPE = 3, VIEW_WHICH = 4, VIEW_FILL = 5, VIEW_SERIAL = 6, VIEW_SLOT = 7,
    VIEW_POINTS = 8, POINT_WORDS = 24, POINTS = 96, SLOTS = 12,
};

/* The kit's sounds: track t's at 0x426532ec + 0x450 * t, its machine type byte at +0xa2. */
#define SOUND_TYPE(track) (*(const volatile uint8_t *)(0x426532ecu + 0x450u * (track) + 0xa2u))
#define PATTERN_TRACK_VIEW 0x30u   /* the current track, through fw_track_at */
#define TRACKS 16u
#define REDRAW 0x20u               /* ev_tick: ctrl + 0x20 = 1 asks for a redraw */

/* The sample view's waveform box (mk2/machines.h): x 25 to 121, rows 26-54. */
enum { BOX_LEFT = 25, BOX_RIGHT = 121, BOX_TOP = 26, BOX_BOTTOM = 54 };
/* The waterfall: the front wave's left end and centre row, each wave further back 2.6 columns right and 1.5 rows up,
 * 64 columns wide, 5 rows either way. */
enum { FALL_LEFT = 26, FALL_FRONT = 48, FALL_WIDTH = 64, FALL_AMP = 5 };

/* The screen controller's top view (ctrl + 0x18: the stack's top, its view at + 8) is the view holding the track's
 * pages, not a window over them: OS 1.17, from the panel probe (Mikrotakt), that view's first word is 0x40216f14. */
#define TRACK_PAGES_VIEW 0x40216f14u

static uint32_t views_seen;
static uint32_t shown_track = DT2_MACHINE_VIEW_NONE;
static uint32_t serial_seen;
static uint32_t table_waves;          /* the table's wave count the stored waves belong to */
static int8_t waves[SLOTS][POINTS];   /* the table's twelve waves, as the player sent them */
static uint16_t have;                 /* bit n: wave n stored */
static int8_t sounding[POINTS];       /* the wave that sounds */
static uint8_t have_sounding;

static int track_pages_on_top(const uint8_t *ctrl)
{
    const uint8_t *const top = *(const uint8_t *const *)(ctrl + 0x18);
    if (!top || top == ctrl + 0x14) {
        return 0;
    }
    const uint8_t *const view = *(const uint8_t *const *)(top + 8);
    return view && *(const uint32_t *)view == TRACK_PAGES_VIEW;
}

/* The frame bitmap counts rows from the bottom: row y from the top is 63 - y. */
static void fill(void *bitmap, int x0, int top, int x1, int bottom, int colour)
{
    if (top < BOX_TOP) {
        top = BOX_TOP;
    }
    if (bottom > BOX_BOTTOM) {
        bottom = BOX_BOTTOM;
    }
    if (top > bottom) {
        return;
    }
    fw_fillrect(bitmap, x0, 63 - bottom, x1, 63 - top, colour);
}

/* Keep a whole wave the player has posted. */
static void take_wave(const volatile uint32_t *view)
{
    if (view[VIEW_FILL] < POINT_WORDS) {
        return;
    }
    const uint32_t which = view[VIEW_WHICH], slot = view[VIEW_SLOT];
    int8_t *to;
    if (which == UINT32_MAX) {
        to = sounding;
        have_sounding = 1;
    } else if (slot < SLOTS) {
        to = waves[slot];
        have |= (uint16_t)(1u << slot);
    } else {
        return;
    }
    for (unsigned int w = 0; w < POINT_WORDS; ++w) {
        const uint32_t word = view[VIEW_POINTS + w];
        for (unsigned int b = 0; b < 4; ++b) {
            to[4 * w + b] = (int8_t)(word >> (8 * b));
        }
    }
}

/* One wave of the waterfall at depth d (in tenths of a step: 0 the front, 110 the back): the columns under it are
 * cleared so the waves behind are hidden, then it is drawn, every other column when dotted, two rows thick when bold. */
static void draw_wave(void *bitmap, const int8_t *points, int depth, int dotted, int bold)
{
    const int left = FALL_LEFT + depth * 26 / 100, centre = FALL_FRONT - depth * 15 / 100;
    int last = centre - points[0] * FALL_AMP / 127;
    for (int c = 0; c < FALL_WIDTH; ++c) {
        const int y = centre - points[c * 3 / 2] * FALL_AMP / 127, x = left + c;
        const int lo = y < last ? y : last, hi = y < last ? last : y;
        fill(bitmap, x, hi + 1, x, centre + FALL_AMP + 1, 0);
        if (!dotted || !(c & 1)) {
            fill(bitmap, x, lo, x, hi + (bold ? 1 : 0), 1);
        }
        last = y;
    }
}

void morpha_draw(void *bitmap, void *ctrl)
{
    const uint32_t views = core_machine_sample_views;
    const int sample_view = views != views_seen;
    views_seen = views;
    unsigned int track = DT2_MACHINE_VIEW_NONE;
    const uint8_t *const pattern_ui = fw_pattern_ui;
    if (sample_view && pattern_ui && ctrl && track_pages_on_top(ctrl)) {
        const unsigned int current = fw_track_at(pattern_ui + PATTERN_TRACK_VIEW);
        if (current < TRACKS && SOUND_TYPE(current) == morpha_machine.type) {
            track = current;
        }
    }
    if (track == DT2_MACHINE_VIEW_NONE) {
        if (shown_track != DT2_MACHINE_VIEW_NONE && core_machine_view_track == shown_track) {
            core_machine_view_track = DT2_MACHINE_VIEW_NONE;
        }
        shown_track = DT2_MACHINE_VIEW_NONE;
        return;
    }
    const volatile uint32_t *const view = core_machine_view;
    if (track != shown_track) {
        have = 0;
        have_sounding = 0;
    }
    shown_track = track;
    core_machine_view_track = track;
    if (view[0] != VIEW_MAGIC || (view[VIEW_TRACK] & 0xffu) != track) {
        return; /* the stock waveform stays until the player has posted */
    }
    const uint32_t count = view[VIEW_SHAPE] >> 16;
    if (count != table_waves) {
        table_waves = count;
        have = 0;
        have_sounding = 0;
    }
    if (view[VIEW_SERIAL] != serial_seen) {
        serial_seen = view[VIEW_SERIAL];
        take_wave(view);
    }
    fill(bitmap, BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM, 0);
    for (int n = SLOTS - 1; n >= 0; --n) {
        if (have & (1u << n)) {
            draw_wave(bitmap, waves[n], n * 10, 1, 0);
        }
    }
    if (have_sounding) {
        /* POS's place in the stack: 0-65535 over the twelve depths. */
        const int depth = (int)((view[VIEW_POS] & 0xffffu) * 110u / 65535u);
        draw_wave(bitmap, sounding, depth, 0, 1);
    }
}

/* While the view shows, redraw every tick so it follows POS and the warp. */
void morpha_tick(uint8_t *ctrl)
{
    if (shown_track != DT2_MACHINE_VIEW_NONE) {
        ctrl[REDRAW] = 1;
    }
}
