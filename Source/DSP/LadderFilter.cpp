#include "LadderFilter.h"
#include <cmath>
#include <algorithm>

LadderFilter::LadderFilter()
{
}

void LadderFilter::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
    updateCoefficients();
}

void LadderFilter::reset()
{
    for (int i = 0; i < 4; ++i)
        stage[i] = 0.0f;
    lastInput = 0.0f;
}

void LadderFilter::setCutoff(float frequencyHz)
{
    cutoff = std::clamp(frequencyHz, 20.0f, static_cast<float>(sampleRate * 0.45));
    updateCoefficients();
}

void LadderFilter::setResonance(float newResonance)
{
    resonance = std::clamp(newResonance, 0.0f, 1.0f);
    updateCoefficients();
}

void LadderFilter::setMode(Mode mode)
{
    filterMode = mode;
}

float LadderFilter::process(float input)
{
    // Feedback with resonance
    float feedback = stage[3] * k;

    // Input with feedback subtracted (inverted feedback for ladder topology)
    float u = input - feedback;

    // Soft clip the input to prevent runaway with high resonance
    u = std::tanh(u);

    // Four cascaded one-pole lowpass filters
    // Using TPT (topology-preserving transform) style integration
    for (int i = 0; i < 4; ++i)
    {
        float v = (u - stage[i]) * g;
        float y = v + stage[i];
        stage[i] = y + v;
        u = y;
    }

    float lpOutput = stage[3];

    if (filterMode == Mode::Highpass)
    {
        // Highpass = input - lowpass
        float hpOutput = input - lpOutput;
        return hpOutput;
    }

    return lpOutput;
}

void LadderFilter::updateCoefficients()
{
    // TPT coefficient for one-pole filter
    // g = tan(pi * fc / fs)
    float fc = cutoff / static_cast<float>(sampleRate);
    fc = std::min(fc, 0.45f);  // Limit to below Nyquist
    g = std::tan(juce::MathConstants<float>::pi * fc);
    g = g / (1.0f + g);  // Normalize for TPT integration

    // Resonance coefficient (k = 4 * resonance for self-oscillation at k=4)
    k = resonance * 4.0f;
}
