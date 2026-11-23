// SPDX-License-Identifier: GPL-3.0-or-later
// Writes Morpha's built-in wavetable into src/player.sharc, between its BEGIN BUILTIN and END BUILTIN lines.
//
// 16 waves of 256 samples, each a sum of up to 48 harmonics (band-limited, so a wave read at middle C has nothing
// above about 12.5 kHz), from formulas only: sine (wave 1), triangle (4), saw (7), square (10), a 15% pulse (12) and
// two vowel-like waves (14, 16) made of a falling slope of harmonics with two resonant bumps. The waves between are
// the spectra in between, so POS sweeps through gradual changes of tone rather than crossfades. Each wave peaks at
// 28,000 of 32,767, the level of a well-recorded sample.
//
// Run: node sdk/digitakt-ii/modules/morpha/tools/builtin-table.mjs
import { readFileSync, writeFileSync } from 'node:fs'
import { fileURLToPath } from 'node:url'

const N = 256, H = 48, WAVES = 16, PEAK = 28000
const player = fileURLToPath(new URL('../src/player.sharc', import.meta.url))

// A spectrum: [sine part, cosine part] for harmonics 1..H.
const spectrum = f => Array.from({ length: H }, (_, i) => f(i + 1))
const sine = spectrum(n => [n === 1 ? 1 : 0, 0])
const triangle = spectrum(n => [n % 2 ? (((n - 1) / 2) % 2 ? -1 : 1) / (n * n) : 0, 0])
const saw = spectrum(n => [1 / n, 0])
const square = spectrum(n => [n % 2 ? 1 / n : 0, 0])
const pulse = spectrum(n => [0, Math.sin(Math.PI * n * 0.15) / n])
const bump = (n, centre, width) => Math.exp(-0.5 * ((n - centre) / width) ** 2)
const vowel = (f1, f2) => spectrum(n => [0.35 / n + bump(n, f1, 1.2) + 0.7 * bump(n, f2, 1.6), 0])
const vowelA = vowel(6.6, 10)
const vowelO = vowel(4.4, 7.6)

const keys = [[0, sine], [3, triangle], [6, saw], [9, square], [11, pulse], [13, vowelA], [15, vowelO]]

function waveAt(index) {
  let k = 0
  while (k < keys.length - 2 && keys[k + 1][0] <= index) k++
  const [i0, a] = keys[k], [i1, b] = keys[k + 1]
  const t = Math.min(1, Math.max(0, (index - i0) / (i1 - i0)))
  const samples = new Float64Array(N)
  for (let h = 0; h < H; h++) {
    const s = a[h][0] * (1 - t) + b[h][0] * t, c = a[h][1] * (1 - t) + b[h][1] * t
    if (!s && !c) continue
    const n = h + 1, rolloff = 0.5 + 0.5 * Math.cos(Math.PI * h / H) // smooth top so the highest harmonics fade
    for (let x = 0; x < N; x++) {
      const w = 2 * Math.PI * n * x / N
      samples[x] += rolloff * (s * Math.sin(w) + c * Math.cos(w))
    }
  }
  let mean = 0
  for (const v of samples) mean += v / N
  let peak = 1e-9
  for (let x = 0; x < N; x++) { samples[x] -= mean; peak = Math.max(peak, Math.abs(samples[x])) }
  return Array.from(samples, v => Math.round(v / peak * PEAK))
}

// Packed as the sound chip's sample memory: two 16-bit samples a word, the earlier one in the low half.
const hex = v => '0x' + (v >>> 0).toString(16).padStart(8, '0')
const lines = []
for (let w = 0; w < WAVES; w++) {
  const values = waveAt(w)
  const words = []
  for (let x = 0; x < N; x += 2) words.push(hex((values[x] & 0xffff) | ((values[x + 1] & 0xffff) << 16)))
  lines.push(`; wave ${w + 1}`)
  for (let x = 0; x < words.length; x += 8) lines.push(`.word ${words.slice(x, x + 8).join(', ')}`)
}
const source = readFileSync(player, 'utf8')
const begin = source.indexOf('; BEGIN BUILTIN'), end = source.indexOf('; END BUILTIN')
if (begin < 0 || end < begin) throw new Error('player.sharc has no BEGIN BUILTIN / END BUILTIN lines')
const head = source.slice(0, source.indexOf('\n', begin) + 1)
writeFileSync(player, head + lines.join('\n') + '\n' + source.slice(end))
console.log(`wrote ${WAVES} waves of ${N} samples to ${player}`)
