#pragma once

#include <JuceHeader.h>

class Envelope
{
public:
    Envelope();

    void prepare(double sampleRate);
    void setDecayTime(float decayMs);
    void trigger(float velocity = 1.0f);

    // Get next envelope value (0 to 1)
    float process();

    bool isActive() const { return active; }

private:
    double sampleRate = 44100.0;
    float decayTime = 0.2f;   // in seconds
    float decayCoeff = 0.0f;
    float currentValue = 0.0f;
    float targetVelocity = 1.0f;
    bool active = false;

    void updateCoefficients();
};
