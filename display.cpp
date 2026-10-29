#include "display.h"
#include "genome.h"
namespace seedsynth {
inline void display_reset(DisplayState& state) { state.unused = 0; }
inline void display(DisplayState&, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    char digits[8] = {};
    opxy::u32 seed = word(input.parameters[3]), count = 0;
    do { digits[count++] = static_cast<char>('0' + seed % 10); seed /= 10; } while (seed);
    for (opxy::u32 i = 0; i < count / 2; ++i) {
        const char digit = digits[i]; digits[i] = digits[count - 1 - i]; digits[count - 1 - i] = digit;
    }
    header(frame, 3, digits);
    const opxy::u32 selected = choice(input.parameters[0], 12);
    const char* const notes[12] = {"F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E"};
    // Use the audible genome: copying to all gives twelve identical branches.
    for (opxy::u32 i = 0; i < 12; ++i) {
        const opxy::u32 genome = effective(input.parameters, i);
        const opxy::i32 scale = 760 + static_cast<opxy::i32>((genome ^ (genome >> 7)) & 255u);
        const Turn angle = rotation(-900 + static_cast<opxy::i32>(i) * 300);
        const opxy::u32 shade = i == selected ? 15 : 6;
        place(frame, i, 240 * unit, 125 * unit, scale, shade, stroke);
        turn(frame, i, angle);
        place(frame, 12 + i, 240 * unit + 60 * scale * angle.cos / unit,
              125 * unit + 60 * scale * angle.sin / unit, unit, shade, fill_and_stroke);
    }
    place(frame, 24, 240 * unit, 125 * unit, unit, 15, stroke);
    text(frame, 0, 24 * unit, 173 * unit, unit, (word(input.parameters[3]) & 16384u) ? "UNIFORM" : "VARIED");
    text(frame, 1, 24 * unit, 135 * unit, unit, notes[selected]);
}
}
