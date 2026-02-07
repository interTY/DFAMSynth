#include "NoiseGenerator.h"

NoiseGenerator::NoiseGenerator()
    : rng(std::random_device{}())
    , distribution(-1.0f, 1.0f)
{
}

void NoiseGenerator::prepare(double /*sampleRate*/)
{
    // Reseed for consistent behavior
    rng.seed(std::random_device{}());
}

float NoiseGenerator::process()
{
    return distribution(rng);
}
