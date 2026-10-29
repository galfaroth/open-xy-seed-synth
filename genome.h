#pragma once
#include <opxy/audio.hpp>
namespace seedsynth {
// Version 1: four three-bit genes and one two-bit gene. Keep this mapping stable for saved sounds.
inline opxy::u32 word(opxy::i32 value) { return value < 0 ? 0 : static_cast<opxy::u32>(value) >> 16; }
inline opxy::i32 saved(opxy::u32 value) { return static_cast<opxy::i32>((value & 32767u) * 65538u); }
inline opxy::u32 choice(opxy::i32 value, opxy::u32 count) { return word(value) * count >> 15; }
inline opxy::u32 slot(opxy::u32 key) { return (key + 7u) % 12u; }
inline opxy::u32 branch(opxy::u32 parent, opxy::u32 growth, opxy::u32 selected) {
    if (!growth) return parent;
    // Distinct masks: low growth flips only each gene's least significant bit.
    opxy::u32 mask = 0;
    for (opxy::u32 gene = 0; gene < 4; ++gene)
        if (selected & (1u << gene)) mask |= ((1u << growth) - 1u) << (gene * 3u);
    if (selected & 8u) mask |= 1u << 12;
    return parent ^ mask;
}
inline opxy::u32 effective(const opxy::i32* parameters, opxy::u32 selected) {
    const opxy::u32 state = word(parameters[3]);
    return branch(state & 16383u, state & 16384u ? 0 : 1 + choice(parameters[1], 3), selected);
}
inline void control_press(opxy::u32 encoder, opxy::i32* parameters) {
    if (encoder > 1) return;
    const opxy::u32 selected = choice(parameters[0], 12);
    // Mode and complete genome share one saved word: no half-published palette.
    parameters[3] = saved(effective(parameters, selected) | (encoder == 0 ? 16384u : 0));
    parameters[0] = saved(1365); // F is the unchanged parent of a fresh palette.
}
}
