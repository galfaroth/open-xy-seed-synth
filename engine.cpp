#include "core.h"
#include "genome.h"
#include "tables.h"
#include <opxy/q31.hpp>
namespace seedsynth {
inline opxy::i32 sine(opxy::u32 phase) {
    const opxy::u32 i = phase >> 24;
    return sine_table[i] + opxy::q31::multiply(sine_table[i + 1] - sine_table[i],
        static_cast<opxy::i32>((phase & 0xffffffu) << 7));
}
inline void initialize(Voice& voice, Shared&, opxy::u32) { voice = Voice(); }
inline void start(Voice& voice, Shared&, opxy::u32, opxy::u32 key) {
    voice = Voice();
    voice.key = key;
    voice.noise = 0x93a7u;
    voice.envelope = 0x7fffffff;
}
inline void destroy(Voice&, Shared&, opxy::u32) {}
inline void render(Voice& voice, Shared&, opxy::u32, opxy::Output output,
                   opxy::Parameters parameters, opxy::Pitch pitch) {
    if (voice.key >= 128) {
        for (opxy::u32 i = 0; i < opxy::block_frames; ++i) output[i] = 0;
        return;
    }
    if (!voice.captured) {
        voice.genome = effective(parameters, slot(voice.key));
        voice.brightness = parameters[2] < 0 ? 0 : parameters[2];
        voice.captured = 1;
    }
    const opxy::u32 g = voice.genome;
    const opxy::u32 shape = g & 7, ratio = g >> 3 & 7, fm = g >> 6 & 7;
    const opxy::u32 texture = g >> 9 & 7, tone = g >> 12 & 7;
    const opxy::u32 ratios[8] = {2, 4, 6, 8, 1, 3, 5, 10}; // half units
    const opxy::u32 max_step = 0xe0000000u / ratios[ratio];
    const opxy::i32 noise_mix = static_cast<opxy::i32>(texture * 0x01800000u);
    const opxy::i32 harmonic = static_cast<opxy::i32>(shape * 0x08000000u);
    const opxy::i32 brightness = parameters[2] < 0 ? 0 : parameters[2];
    const opxy::i32 decay = 0x7fffffff - static_cast<opxy::i32>(texture * 5000u);
    for (opxy::u32 i = 0; i < opxy::block_frames; ++i) {
        const opxy::i32 modulator = sine(voice.phase_b);
        // Limit FM and harmonics as pitch rises; no oscillator exceeds Nyquist.
        const opxy::u32 step = pitch[i] > 0x70000000u ? 0x70000000u : pitch[i];
        const opxy::u32 attenuation = step < 0x08000000u ? 0 : step < 0x10000000u ? 1 : 31;
        const opxy::i32 deviation = opxy::q31::multiply(static_cast<opxy::i32>(step), modulator)
            * static_cast<opxy::i32>(fm >> attenuation);
        voice.phase_a += step + static_cast<opxy::u32>(deviation);
        opxy::u32 mod_step = step > max_step ? 0x70000000u : (step >> 1) * ratios[ratio];
        if (mod_step > 0x70000000u) mod_step = 0x70000000u;
        voice.phase_b += mod_step;
        voice.noise = voice.noise * 1664525u + 1013904223u;
        const opxy::i32 clean = sine(voice.phase_a);
        const opxy::i32 shaped = clean + opxy::q31::multiply(
            sine(voice.phase_a * 2u) - clean, harmonic >> attenuation);
        const opxy::i32 noise = static_cast<opxy::i32>(voice.noise >> 2) - 0x20000000;
        // Ratio mutations remain audible when FM depth is zero.
        const opxy::i32 paired = shaped + opxy::q31::multiply(
            sine(voice.phase_b) - shaped, 0x20000000 >> attenuation);
        const opxy::i32 sample = paired + opxy::q31::multiply(noise - paired, noise_mix);
        // Bright remains playable on held notes; smooth its target to avoid zipper noise.
        voice.brightness += opxy::q31::multiply(brightness - voice.brightness, 0x01000000);
        const opxy::i32 coefficient = 0x00800000 + static_cast<opxy::i32>(tone * 0x03000000u)
            + (voice.brightness >> 3);
        voice.filter += opxy::q31::multiply(sample - voice.filter, coefficient);
        voice.envelope = opxy::q31::multiply(voice.envelope, decay);
        output[i] = opxy::q31::multiply(voice.filter >> 2,
            0x20000000 + (voice.envelope >> 1));
    }
}
}
