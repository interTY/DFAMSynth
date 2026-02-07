#pragma once

#include <JuceHeader.h>

// Moog-style 4-pole (24dB/oct) ladder filter with HP/LP modes
class LadderFilter
{
public:
    enum class Mode { Lowpass, Highpass };

    LadderFilter();

    void prepare(double sampleRate);
    void reset();

    void setCutoff(float frequencyHz);
    void setResonance(float resonance);  // 0 to 1
    void setMode(Mode mode);

    float process(float input);

private:
    double sampleRate = 44100.0;
    float cutoff = 1000.0f;
    float resonance = 0.0f;
    Mode filterMode = Mode::Lowpass;

    // Filter state (4 cascaded one-pole filters)
    float stage[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // For highpass
    float lastInput = 0.0f;

    // Coefficients
    float g = 0.0f;  // cutoff coefficient
    float k = 0.0f;  // resonance coefficient

    void updateCoefficients();
};
