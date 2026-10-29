#pragma once
#include <opxy/audio.hpp>
namespace seedsynth {
struct Voice {
    opxy::u32 phase_a, phase_b, noise, key, genome, captured;
    opxy::i32 filter, envelope, brightness;
};
struct Shared {};
}
