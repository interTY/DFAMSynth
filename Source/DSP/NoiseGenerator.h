#pragma once

#include <JuceHeader.h>
#include <random>

class NoiseGenerator
{
public:
    NoiseGenerator();

    void prepare(double sampleRate);

    // Get next noise sample (-1 to 1)
    float process();

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> distribution;
};
