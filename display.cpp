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
    for (opxy::u32 i = 0; i < 12; ++i)
        place(frame, i, (18 + i * 39) * unit, 85 * unit, unit, i == selected ? 15 : 5, fill_and_stroke);
    text(frame, 0, 18 * unit, 65 * unit, unit, (word(input.parameters[3]) & 16384u) ? "UNIFORM" : "VARIED");
    text(frame, 1, 18 * unit, 190 * unit, unit, "SELECT COPY / GROW NEW");
}
}
