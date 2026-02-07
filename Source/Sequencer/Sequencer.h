#pragma once

#include <JuceHeader.h>
#include <array>
#include <functional>

class Sequencer
{
public:
    static constexpr int NUM_STEPS = 8;

    Sequencer();

    void prepare(double sampleRate);
    void reset();

    // Set tempo in BPM
    void setTempo(float bpm);

    // Set pitch offset for a step (in semitones, -24 to +24)
    void setStepPitch(int step, float semitones);

    // Set velocity for a step (0 to 1)
    void setStepVelocity(int step, float velocity);

    // Set pan for a step (-1 to +1, where -1=left, 0=center, +1=right)
    void setStepPan(int step, float pan);

    // Set wave position for both VCOs per step (0-1)
    void setStepWave(int step, float wave);

    // Set ring mod frequency multiplier per step (0-1)
    void setStepRingMod(int step, float amount);

    // Set delay pitch per step (in semitones, for Karplus-Strong tuning)
    void setStepDelayPitch(int step, float semitones);

    // Start/stop the sequencer
    void setRunning(bool shouldRun);
    bool isRunning() const { return running; }

    // Set swing amount (0-1, where 0.5 = no swing)
    void setSwing(float amount);

    // Set direction: 0=forward, 1=backward, 2=pingpong
    void setDirection(int dir);

    // Get current step (0-7)
    int getCurrentStep() const { return currentStep; }

    // Manually advance to next step
    void advanceStep();

    // Process one sample. Returns true if a new step was triggered this sample.
    bool process();

    // Get the current step's pitch multiplier (for oscillator frequency)
    float getCurrentPitchMultiplier() const;

    // Get the current step's velocity
    float getCurrentVelocity() const;

    // Get the current step's pan (-1 to +1)
    float getCurrentPan() const;

    // Get the current step's wave position (0-1)
    float getCurrentWave() const;

    // Get the current step's ring mod amount (0-1)
    float getCurrentRingMod() const;

    // Get the current step's delay pitch (semitones)
    float getCurrentDelayPitch() const;

private:
    double sampleRate = 44100.0;
    float tempo = 120.0f;          // BPM
    double samplesPerStep = 0.0;
    double sampleCounter = 0.0;
    int currentStep = 0;
    bool running = false;
    float swing = 0.5f;            // 0-1, where 0.5 = no swing
    int direction = 0;             // 0=forward, 1=backward, 2=pingpong
    int pingPongDir = 1;           // 1=forward, -1=backward (for pingpong mode)

    std::array<float, NUM_STEPS> stepPitch = {};      // semitones
    std::array<float, NUM_STEPS> stepVelocity = {};   // 0-1
    std::array<float, NUM_STEPS> stepPan = {};        // -1 to +1
    std::array<float, NUM_STEPS> stepWave = {};       // 0-1 waveform position (both VCOs)
    std::array<float, NUM_STEPS> stepRingMod = {};    // 0-1 ring mod frequency multiplier
    std::array<float, NUM_STEPS> stepDelayPitch = {}; // semitones for Karplus-Strong delay tuning

    void updateTiming();
};
