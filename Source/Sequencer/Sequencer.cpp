#include "Sequencer.h"
#include <cmath>

Sequencer::Sequencer()
{
    // Initialize all steps with default values
    stepPitch.fill(0.0f);
    stepVelocity.fill(0.8f);
    stepPan.fill(0.0f);       // center
    stepWave.fill(0.33f);     // triangle
    stepRingMod.fill(0.5f);   // middle frequency
    stepDelayPitch.fill(0.0f); // C2 base pitch for Karplus-Strong
}

void Sequencer::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    updateTiming();
    reset();
}

void Sequencer::reset()
{
    currentStep = 0;
    sampleCounter = 0.0;
}

void Sequencer::setTempo(float bpm)
{
    tempo = std::clamp(bpm, 30.0f, 300.0f);
    updateTiming();
}

void Sequencer::setStepPitch(int step, float semitones)
{
    if (step >= 0 && step < NUM_STEPS)
        stepPitch[step] = std::clamp(semitones, -24.0f, 24.0f);
}

void Sequencer::setStepVelocity(int step, float velocity)
{
    if (step >= 0 && step < NUM_STEPS)
        stepVelocity[step] = std::clamp(velocity, 0.0f, 1.0f);
}

void Sequencer::setStepPan(int step, float pan)
{
    if (step >= 0 && step < NUM_STEPS)
        stepPan[step] = std::clamp(pan, -1.0f, 1.0f);
}

void Sequencer::setStepWave(int step, float wave)
{
    if (step >= 0 && step < NUM_STEPS)
        stepWave[step] = std::clamp(wave, 0.0f, 1.0f);
}

void Sequencer::setStepRingMod(int step, float amount)
{
    if (step >= 0 && step < NUM_STEPS)
        stepRingMod[step] = std::clamp(amount, 0.0f, 1.0f);
}

void Sequencer::setStepDelayPitch(int step, float semitones)
{
    if (step >= 0 && step < NUM_STEPS)
        stepDelayPitch[step] = std::clamp(semitones, -24.0f, 24.0f);
}

void Sequencer::setRunning(bool shouldRun)
{
    if (shouldRun && !running)
    {
        // Starting - reset to beginning
        reset();
    }
    running = shouldRun;
}

void Sequencer::setSwing(float amount)
{
    swing = std::clamp(amount, 0.0f, 1.0f);
}

void Sequencer::setDirection(int dir)
{
    direction = std::clamp(dir, 0, 2);
}

void Sequencer::advanceStep()
{
    if (direction == 0)  // Forward
    {
        currentStep = (currentStep + 1) % NUM_STEPS;
    }
    else if (direction == 1)  // Backward
    {
        currentStep = (currentStep - 1 + NUM_STEPS) % NUM_STEPS;
    }
    else  // Pingpong
    {
        currentStep += pingPongDir;
        if (currentStep >= NUM_STEPS - 1)
        {
            currentStep = NUM_STEPS - 1;
            pingPongDir = -1;
        }
        else if (currentStep <= 0)
        {
            currentStep = 0;
            pingPongDir = 1;
        }
    }
    sampleCounter = 0.0;
}

bool Sequencer::process()
{
    if (!running)
        return false;

    bool triggered = false;

    // Check if we need to advance to the next step
    sampleCounter += 1.0;

    // Calculate swing-adjusted step length
    // Swing delays odd steps (off-beats): swing=0.5 means no swing
    // swing=0.75 means off-beats are delayed by 50% of step length
    double currentStepLength = samplesPerStep;
    bool isOffBeat = (currentStep % 2) == 1;

    if (isOffBeat)
    {
        // Off-beat: adjust timing based on swing
        // swing 0.5 = normal, swing 1.0 = maximum delay (triplet feel)
        double swingOffset = (swing - 0.5) * 2.0;  // -1 to +1
        currentStepLength = samplesPerStep * (1.0 + swingOffset * 0.5);
    }
    else
    {
        // On-beat: compensate for swing
        double swingOffset = (swing - 0.5) * 2.0;
        currentStepLength = samplesPerStep * (1.0 - swingOffset * 0.5);
    }

    if (sampleCounter >= currentStepLength)
    {
        sampleCounter -= currentStepLength;

        // Advance step based on direction
        if (direction == 0)  // Forward
        {
            currentStep = (currentStep + 1) % NUM_STEPS;
        }
        else if (direction == 1)  // Backward
        {
            currentStep = (currentStep - 1 + NUM_STEPS) % NUM_STEPS;
        }
        else  // Pingpong
        {
            currentStep += pingPongDir;
            if (currentStep >= NUM_STEPS - 1)
            {
                currentStep = NUM_STEPS - 1;
                pingPongDir = -1;
            }
            else if (currentStep <= 0)
            {
                currentStep = 0;
                pingPongDir = 1;
            }
        }
        triggered = true;
    }

    return triggered;
}

float Sequencer::getCurrentPitchMultiplier() const
{
    // Convert semitones to frequency multiplier
    // multiplier = 2^(semitones/12)
    float semitones = stepPitch[currentStep];
    return std::pow(2.0f, semitones / 12.0f);
}

float Sequencer::getCurrentVelocity() const
{
    return stepVelocity[currentStep];
}

float Sequencer::getCurrentPan() const
{
    return stepPan[currentStep];
}

float Sequencer::getCurrentWave() const
{
    return stepWave[currentStep];
}

float Sequencer::getCurrentRingMod() const
{
    return stepRingMod[currentStep];
}

float Sequencer::getCurrentDelayPitch() const
{
    return stepDelayPitch[currentStep];
}

void Sequencer::updateTiming()
{
    // Calculate samples per step
    // At 120 BPM with 16th notes: 120 beats/min = 2 beats/sec = 8 16th notes/sec
    // For DFAM-style: we'll use 8th notes (2 steps per beat)
    float beatsPerSecond = tempo / 60.0f;
    float stepsPerSecond = beatsPerSecond * 2.0f;  // 2 steps per beat (8th notes)
    samplesPerStep = sampleRate / stepsPerSecond;
}
