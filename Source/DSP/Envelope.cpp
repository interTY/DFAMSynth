#include "Envelope.h"
#include <cmath>

Envelope::Envelope()
{
}

void Envelope::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    currentValue = 0.0f;
    active = false;
    updateCoefficients();
}

void Envelope::setDecayTime(float decayMs)
{
    decayTime = decayMs / 1000.0f;  // convert to seconds
    updateCoefficients();
}

void Envelope::trigger(float velocity)
{
    targetVelocity = velocity;
    currentValue = velocity;
    active = true;
}

float Envelope::process()
{
    if (!active)
        return 0.0f;

    float output = currentValue;

    // Exponential decay
    currentValue *= decayCoeff;

    // Stop when below threshold
    if (currentValue < 0.0001f)
    {
        currentValue = 0.0f;
        active = false;
    }

    return output;
}

void Envelope::updateCoefficients()
{
    // Calculate decay coefficient for exponential decay
    // We want to reach ~0.001 (-60dB) in decayTime seconds
    if (decayTime > 0.0f && sampleRate > 0.0)
    {
        float numSamples = static_cast<float>(decayTime * sampleRate);
        decayCoeff = std::pow(0.001f, 1.0f / numSamples);
    }
    else
    {
        decayCoeff = 0.0f;
    }
}
