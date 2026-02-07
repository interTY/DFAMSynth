#include "Oscillator.h"
#include <cmath>

Oscillator::Oscillator()
{
}

void Oscillator::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    phase = 0.0;
    completedCycle = false;
    updatePhaseIncrement();
}

void Oscillator::setFrequency(float newFrequency)
{
    frequency = newFrequency;
    updatePhaseIncrement();
}

void Oscillator::setWaveformPosition(float position)
{
    waveformPosition = std::clamp(position, 0.0f, 1.0f);
}

float Oscillator::process()
{
    completedCycle = false;
    float output = generateMorphedWaveform();

    phase += phaseIncrement;
    if (phase >= 1.0)
    {
        phase -= 1.0;
        completedCycle = true;
    }

    return output;
}

float Oscillator::process(float fmInput, float fmAmount)
{
    completedCycle = false;

    // Apply FM: modulate frequency by fmInput
    // fmAmount controls the depth of modulation
    double modulatedIncrement = phaseIncrement * (1.0 + fmInput * fmAmount * 4.0);

    float output = generateMorphedWaveform();

    phase += modulatedIncrement;
    while (phase >= 1.0)
    {
        phase -= 1.0;
        completedCycle = true;
    }
    while (phase < 0.0)
        phase += 1.0;

    return output;
}

float Oscillator::processWithPitchMod(float pitchModSemitones)
{
    completedCycle = false;

    // Apply pitch modulation in semitones
    float pitchMult = std::pow(2.0f, pitchModSemitones / 12.0f);
    double modulatedIncrement = phaseIncrement * pitchMult;

    float output = generateMorphedWaveform();

    phase += modulatedIncrement;
    while (phase >= 1.0)
    {
        phase -= 1.0;
        completedCycle = true;
    }
    while (phase < 0.0)
        phase += 1.0;

    return output;
}

void Oscillator::sync()
{
    phase = 0.0;
}

void Oscillator::updatePhaseIncrement()
{
    phaseIncrement = frequency / sampleRate;
}

float Oscillator::generateSine() const
{
    return static_cast<float>(std::sin(phase * 2.0 * 3.14159265358979323846));
}

float Oscillator::generateTriangle() const
{
    // Triangle wave: ramp up from -1 to 1 in first half, down from 1 to -1 in second half
    if (phase < 0.5)
        return static_cast<float>(4.0 * phase - 1.0);
    else
        return static_cast<float>(3.0 - 4.0 * phase);
}

float Oscillator::generateSquare() const
{
    return (phase < 0.5) ? 1.0f : -1.0f;
}

float Oscillator::generateChaos() const
{
    // Aggressive wavetable-style waveform with wave folding and bit crushing feel
    double p = phase * 2.0 * 3.14159265358979323846;

    // Sawtooth base
    float saw = static_cast<float>(1.0 - 2.0 * phase);

    // Add many harmonics for rich texture
    float h2 = static_cast<float>(std::sin(p * 2.0) * 0.7);
    float h3 = static_cast<float>(std::sin(p * 3.0) * 0.6);
    float h4 = static_cast<float>(std::sin(p * 4.0) * 0.5);
    float h5 = static_cast<float>(std::sin(p * 5.0) * 0.45);
    float h7 = static_cast<float>(std::sin(p * 7.0) * 0.35);
    float h9 = static_cast<float>(std::sin(p * 9.0) * 0.25);
    float h11 = static_cast<float>(std::sin(p * 11.0) * 0.2);

    float complex = saw + h2 + h3 + h4 + h5 + h7 + h9 + h11;

    // Aggressive wave folding - multiple passes
    complex = std::sin(complex * 2.5f);
    complex = std::tanh(complex * 3.0f);

    // Add some edge with soft clipping
    if (complex > 0.8f) complex = 0.8f + (complex - 0.8f) * 0.3f;
    if (complex < -0.8f) complex = -0.8f + (complex + 0.8f) * 0.3f;

    return std::clamp(complex, -1.0f, 1.0f);
}

float Oscillator::generateMorphedWaveform() const
{
    // Morph through: sine (0) -> triangle (0.33) -> square (0.66) -> chaos (1.0)
    const float pos = waveformPosition;

    if (pos <= 0.33f)
    {
        // Morph from sine to triangle
        float blend = pos / 0.33f;
        return generateSine() * (1.0f - blend) + generateTriangle() * blend;
    }
    else if (pos <= 0.66f)
    {
        // Morph from triangle to square
        float blend = (pos - 0.33f) / 0.33f;
        return generateTriangle() * (1.0f - blend) + generateSquare() * blend;
    }
    else
    {
        // Morph from square to chaos
        float blend = (pos - 0.66f) / 0.34f;
        return generateSquare() * (1.0f - blend) + generateChaos() * blend;
    }
}
