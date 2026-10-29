# Seed Synth for OP-XY

Original native two-oscillator synth inspired by sound exploration, not a
Synplant port. No Synplant preset compatibility or Genopatch AI.

Requires the companion Open-XY SDK branch [`feat/seed-synth-sdk`](https://github.com/galfaroth/open-xy/tree/feat/seed-synth-sdk) (SDK 1.7),
pinned by the CI template to `967363780625c1ca3cb7d19c3659d2d2b9f82ed7`.
The source is a separate machine repository. Existing arranger work is unchanged.

## Play and grow

- Turn **Select** to choose F, F sharp, G, ... E. The bright circle marks the
  explicit copy target. Selection does not follow the last played note.
- Turn **Growth**: Sprout, Grow and Wild set mutation depth in a varied palette.
  Copy switches to Uniform; Grow switches back to Varied. The palette repeats in every octave.
- Turn **Bright** for the next notes' filter brightness.
- Turn **Seed** through state codes 0–32767: 0–16383 are varied palettes;
  16384–32767 are their uniform versions.
- Press **Select** to copy the selected complete effective genome to every key.
- Press **Growth** to promote the selected sound and create a fresh
  palette at the current Growth depth. F retains that exact parent. Pressing Bright or Seed does nothing.

Sharps are abbreviated Fs/Gs/etc. in the stock enum header; the palette shows
F#/G#/etc. Pitch bend does not switch the captured seed. Stock legato updates
without a new onset retain the original voice's seed. The stock amp envelope,
filter and FX pages provide performance shaping. Held voices retain their
captured genome and brightness, including during Copy/Grow.

## Sound and recall

Version 1 has four three-bit genes and one two-bit gene for oscillator shape, B:A ratio, true bounded
frequency modulation, noise/decay and filter tone. Sine interpolation, limited
second harmonics, a one-pole filter, a decaying amplitude contour and a fixed
output gain keep mutation bounded. FM/harmonic depth reduces at high pitches.
This is a deliberately compact 16384-genome instrument, without deep genome
editing, resonance, history or audio-to-patch search.

The complete palette is a pure function of four saved parameters. The parent
genome and Uniform/Varied mode occupy one Q15-safe word; Growth selects a stable mutation mask for
each F-first pitch class. Promotion stores the resulting genome, not an RNG
history. Brightness remains unchanged. Repeated promotions therefore have no
history-size limit. Version-1 gene/mutation meanings must remain stable; future
incompatible mappings require a new machine identity or an explicit migration.

The machine captures its timbre on its first audio render after note start.
Copy/Grow changes the complete genome and mode in one saved word, so
publication cannot expose a half-promoted palette. Parameter locks/LFO values
are not copied: actions operate on the saved patch. Hardware validation must
cover live publication, legato, project/preset reload and power cycling.

## Develop

From this repository:

```sh
python3 ../open-xy-seed-sdk/sdk.py test --machine . --cxx <wasm-capable-clang++>
python3 ../open-xy-seed-sdk/sdk.py preview --machine . --out ../open-xy-seed-sdk/local/seed-preview-001 --cxx <wasm-capable-clang++>
python3 ../open-xy-seed-sdk/tools/serve_harness.py --preview ../open-xy-seed-sdk/local/seed-preview-001
```

Preview buttons provide the same encoder actions. Patch export/import stores
the resulting four parameters. Sprout, Glass, Dust and Pure are example palettes.
Host tests exhaust all parent genomes for Q15 round trips and distinct branches,
then check exact promotion, octave mapping, held notes, bend identity and audio
bounds. Host timing is not a hardware CPU measurement.

Firmware, stock assets and packages stay outside every checkout. A development
collection includes the user's existing 22 machines plus Seed Synth, preserving
its layout numbers. This isolated SDK build does not include the separate,
uncommitted linear-arranger extension. Do not replace an arranger device image
with it expecting arranger features. Device installation and profiling require
separate authorization. Release-host verification and hardware acceptance are
separate from a successful local compile.

MIT; see LICENSE. DSP and mutation code are original. The sine table samples
sin(2*pi*i/256), scaled by 0x3fffffff; no proprietary instrument data is included.

The ready-to-use CI template is `ci/machine.yml`. It is inactive because the
current GitHub OAuth token cannot create workflows. Enable it by moving it to
`.github/workflows/machine.yml` with an authorized GitHub account; no broader
account permissions were requested. Local checks use the commands above.
