#pragma once

#include <JuceHeader.h>

class Oscillator
{
public:
    Oscillator();

    void prepare(double sampleRate);
    void setFrequency(float frequency);

    // Continuous waveform: 0=sine, 0.33=triangle, 0.66=square, 1.0=chaos
    void setWaveformPosition(float position);

    // Get next sample
    float process();

    // Process with FM input (frequency modulation)
    float process(float fmInput, float fmAmount);

    // Process with pitch modulation (for envelope/sequencer pitch mod)
    float processWithPitchMod(float pitchModSemitones);

    // Hard sync - reset phase when master oscillator completes cycle
    void sync();

    // Get current phase (for sync detection)
    double getPhase() const { return phase; }
    bool hasCompletedCycle() const { return completedCycle; }

private:
    double sampleRate = 44100.0;
    float frequency = 100.0f;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    float waveformPosition = 0.0f;  // 0=sine, 0.33=tri, 0.66=square, 1=chaos
    bool completedCycle = false;

    void updatePhaseIncrement();
    float generateSine() const;
    float generateTriangle() const;
    float generateSquare() const;
    float generateChaos() const;
    float generateMorphedWaveform() const;
};
