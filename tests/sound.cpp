#include "engine.cpp"
#include "display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <initializer_list>
using namespace seedsynth;
void require(bool ok, const char* message) { if (!ok) { std::fprintf(stderr, "%s\n", message); std::exit(1); } }
int main() {
    for (unsigned seed : {0u, 9234u, 16383u, 16384u, 32767u}) {
        for (unsigned selected = 0; selected < 12; ++selected) {
            opxy::draw::Input input = {};
            input.parameters[0] = saved((selected * 32768u + 16384u) / 12u);
            input.parameters[1] = saved(32767);
            input.parameters[3] = saved(seed);
            DisplayState state = {}; Frame frame = {};
            display_reset(state); display(state, input, frame);
            require(opxy::draw::valid(frame), "Valid flower frame");
            for (unsigned i = 0; i < 12; ++i) {
                require(frame.nodes[i].shade == (i == selected ? 15u : 6u), "Flower selection");
                require(frame.nodes[12+i].y > 38 * opxy::draw::unit && frame.nodes[12+i].y < 205 * opxy::draw::unit, "Flower below header");
                if (seed & 16384u) require(frame.nodes[i].scale == frame.nodes[0].scale, "Uniform flower");
            }
        }
    }
    require(slot(53) == 0 && slot(65) == 0 && slot(77) == 0 && slot(60) == 7, "F-first mapping");
    for (unsigned n = 0; n < 116; ++n) require(slot(n) == slot(n + 12), "Octave identity");
    opxy::i32 parameters[8] = {saved(1365), saved(12288), saved(16384), saved(9234)};
    for (unsigned seed = 0; seed < 32768; ++seed) {
        const unsigned stored = static_cast<unsigned>(std::llround(saved(seed) * (32767.0 / INT_MAX)));
        require(stored == seed && word(saved(stored)) == seed, "Q15 save round trip");
        for (unsigned growth = 1; growth < 4; ++growth) {
            for (unsigned a = 0; a < 12; ++a) {
                const unsigned target = branch(seed, growth, a);
                require(target < 32768, "Genome range");
                for (unsigned b = 0; b < a; ++b)
                    require(target != branch(seed, growth, b), "Distinct palette branches");
            }
        }
    }
    for (unsigned selected = 0; selected < 12; ++selected) {
        parameters[0] = saved((selected * 32768 + 16384) / 12);
        parameters[1] = saved(20480); parameters[3] = saved(9234);
        const unsigned sound = effective(parameters, selected);
        control_press(0, parameters);
        for (unsigned n = 0; n < 128; ++n) require(effective(parameters, slot(n)) == sound, "Exact promotion");
        control_press(1, parameters);
        require(effective(parameters, 0) == sound && choice(parameters[1], 3) == 1, "Grow from copied sound");
    }
    // Complete audio, not only the genome ID, survives promotion.
    Shared shared;
    for (unsigned selected = 0; selected < 12; ++selected) {
        opxy::i32 params[8] = {saved((selected * 32768 + 16384) / 12), saved(20480), saved(16384), saved(9234)};
        Voice before = {}, after = {};
        opxy::i32 a[64], b[64]; opxy::u32 tune[64];
        for (auto& p : tune) p = 42852281;
        start(before, shared, 1, 53 + selected);
        render(before, shared, 1, a, params, tune);
        control_press(0, params);
        start(after, shared, 1, 53 + selected);
        render(after, shared, 1, b, params, tune);
        for (unsigned i = 0; i < 64; ++i) require(a[i] == b[i], "Promoted sound changed");
    }
    Voice held = {}, fresh = {};
    opxy::i32 output[64]; opxy::u32 pitch[64];
    for (auto& p : pitch) p = 42852281;
    start(held, shared, 1, 65); render(held, shared, 1, output, parameters, pitch);
    const unsigned captured = held.genome;
    parameters[3] = saved(32767); control_press(1, parameters);
    for (auto& p : pitch) p = 48000000;
    render(held, shared, 1, output, parameters, pitch);
    require(held.genome == captured && held.key == 65, "Held voice / bend identity");
    // Different genome IDs alone are insufficient: compare actual audio at one pitch.
    opxy::i32 palette[12][2048];
    opxy::i32 sprout[8] = {saved(1365), saved(5461), saved(16384), saved(9234)};
    for (auto& p : pitch) p = 42852281;
    for (unsigned key = 0; key < 12; ++key) {
        start(fresh, shared, 1, 65 + key);
        for (unsigned block = 0; block < 32; ++block)
            render(fresh, shared, 1, palette[key] + block * 64, sprout, pitch);
        for (unsigned previous = 0; previous < key; ++previous) {
            long long difference = 0, level = 0;
            for (unsigned i = 0; i < 2048; ++i) {
                difference += std::abs(static_cast<long long>(palette[key][i]) - palette[previous][i]);
                level += std::abs(static_cast<long long>(palette[previous][i]));
            }
            require(difference > level / 100, "Sprout branches must differ in audio, not only genome IDs");
        }
    }
    // Bright must change an already sounding voice, without changing its genome.
    parameters[2] = 0;
    start(fresh, shared, 1, 65); render(fresh, shared, 1, output, parameters, pitch);
    Voice dark = fresh;
    opxy::i32 dark_output[64];
    render(dark, shared, 1, dark_output, parameters, pitch);
    parameters[2] = 0x7fffffff;
    render(fresh, shared, 1, output, parameters, pitch);
    long long changed = 0;
    for (unsigned i = 0; i < 64; ++i) changed += std::abs(static_cast<long long>(output[i]) - dark_output[i]);
    require(changed > 1000000 && fresh.brightness > dark.brightness, "Bright responds on held notes");
    require(fresh.genome == dark.genome, "Bright preserves genome");
    long long energy = 0;
    for (unsigned seed = 0; seed < 32768; seed += 127) {
        parameters[3] = saved(seed);
        for (unsigned note : {0u, 53u, 65u, 96u, 127u}) {
            for (auto& p : pitch) p = static_cast<unsigned>(std::llround(440.0 * std::pow(2.0, (int(note)-69)/12.0) / 44100 * 4294967296.0));
            start(fresh, shared, 1, note);
            for (unsigned block = 0; block < 32; ++block) {
                render(fresh, shared, 1, output, parameters, pitch);
                for (int sample : output) { require(sample <= 268435456 && sample >= -268435456, "Peak bound"); energy += std::abs(sample); }
            }
        }
    }
    require(energy > 1000000, "Audible engine");
    std::puts("PASS: palette, Q15 recall, exact copy/grow, held notes, bend identity, audio bounds");
}
