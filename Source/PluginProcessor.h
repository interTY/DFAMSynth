#pragma once

#include <JuceHeader.h>
#include "DSP/Oscillator.h"
#include "DSP/Envelope.h"
#include "DSP/NoiseGenerator.h"
#include "DSP/LadderFilter.h"
#include "Sequencer/Sequencer.h"

class DFAMSynthAudioProcessor : public juce::AudioProcessor
{
public:
    DFAMSynthAudioProcessor();
    ~DFAMSynthAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

    // Preset management
    void savePreset(const juce::String& name);
    void loadPreset(const juce::File& presetFile);
    juce::StringArray getPresetList();
    juce::File getPresetsFolder();
    void refreshPresetList();
    juce::StringArray presetNames;
    juce::Array<juce::File> presetFiles;

    // For UI to display current step
    int getCurrentSequencerStep() const { return sequencer.getCurrentStep(); }
    bool isSequencerRunning() const { return sequencer.isRunning(); }

    // Manual trigger and advance (called from UI)
    void triggerManual() { manualTrigger.store(true); }
    void advanceManual() { manualAdvance.store(true); }

private:
    // DSP Components
    Oscillator vco1;
    Oscillator vco2;
    Oscillator subOsc;
    NoiseGenerator noise;
    LadderFilter filter;
    Envelope pitchEnv;   // VCO decay envelope
    Envelope filterEnv;
    Envelope vcaEnv;
    Sequencer sequencer;

    // Delay
    std::vector<float> delayBuffer;
    int delayWritePos = 0;
    int delayBufferSize = 0;
    double currentSampleRate = 44100.0;
    float delayFilterState = 0.0f;  // Simple one-pole lowpass state

    // Reverb
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    // Reverb pre-delay buffer
    std::vector<float> reverbPreDelayL;
    std::vector<float> reverbPreDelayR;
    int reverbPreDelayWritePos = 0;
    int reverbPreDelaySize = 0;

    // Ring modulator oscillator
    double ringModPhase = 0.0;
    double ringModPhaseInc = 0.0;

    // Manual trigger/advance flags
    std::atomic<bool> manualTrigger { false };
    std::atomic<bool> manualAdvance { false };

    // Parameter tree
    juce::AudioProcessorValueTreeState apvts;

    // Parameter layout creation
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Parameter pointers for efficient access
    // VCO section
    std::atomic<float>* vcoDecayParam = nullptr;
    std::atomic<float>* seqPitchModParam = nullptr;  // 0=VCO1&2, 1=OFF, 2=VCO2
    std::atomic<float>* vco1EgAmtParam = nullptr;    // bipolar -1 to +1
    std::atomic<float>* vco1FreqParam = nullptr;
    std::atomic<float>* vco1WaveParam = nullptr;
    std::atomic<float>* vco1LevelParam = nullptr;
    std::atomic<float>* subLevelParam = nullptr;
    std::atomic<float>* noiseLevelParam = nullptr;

    std::atomic<float>* fmAmountParam = nullptr;
    std::atomic<float>* hardSyncParam = nullptr;
    std::atomic<float>* vco2EgAmtParam = nullptr;    // bipolar -1 to +1
    std::atomic<float>* vco2FreqParam = nullptr;
    std::atomic<float>* vco2WaveParam = nullptr;
    std::atomic<float>* vco2LevelParam = nullptr;

    // Filter section
    std::atomic<float>* filterCutoffParam = nullptr;
    std::atomic<float>* filterModeParam = nullptr;   // 0=LP, 1=HP
    std::atomic<float>* filterResParam = nullptr;
    std::atomic<float>* vcaEgModeParam = nullptr;    // 0=fast, 1=slow
    std::atomic<float>* vcaLevelParam = nullptr;

    std::atomic<float>* filterDecayParam = nullptr;
    std::atomic<float>* filterEnvAmtParam = nullptr; // bipolar
    std::atomic<float>* noiseVcfModParam = nullptr;  // bipolar
    std::atomic<float>* vcaDecayParam = nullptr;

    // Sequencer section
    std::atomic<float>* tempoParam = nullptr;
    std::atomic<float>* tempoMultParam = nullptr;
    std::atomic<float>* seqRunParam = nullptr;
    std::atomic<float>* glideParam = nullptr;
    std::atomic<float>* droneParam = nullptr;

    // Glide/portamento state
    float currentGlidePitch = 0.0f;
    float targetGlidePitch = 0.0f;

    // Smoothed waveform for drone mode
    float smoothedWave1 = 0.33f;
    float smoothedWave2 = 0.33f;

    // MIDI state
    juce::MidiKeyboardState keyboardState;
    float midiNotePitch = 0.0f;  // Pitch offset from MIDI (semitones from C2)
    bool midiNoteActive = false;
    int lastMidiNote = -1;
    std::atomic<float>* midiHoldParam = nullptr;

    std::array<std::atomic<float>*, 8> seqPitchParams = {};
    std::array<std::atomic<float>*, 8> seqVelParams = {};
    std::array<std::atomic<float>*, 8> seqPanParams = {};
    std::array<std::atomic<float>*, 8> seqWaveParams = {};
    std::array<std::atomic<float>*, 8> seqRingModParams = {};
    std::array<std::atomic<float>*, 8> seqDelayPitchParams = {};

    // Delay parameters
    std::atomic<float>* delayTimeParam = nullptr;
    std::atomic<float>* delayFeedbackParam = nullptr;
    std::atomic<float>* delayFilterParam = nullptr;
    std::atomic<float>* delayMixParam = nullptr;

    // Reverb parameters
    std::atomic<float>* reverbDecayParam = nullptr;
    std::atomic<float>* reverbFilterParam = nullptr;
    std::atomic<float>* reverbMixParam = nullptr;

    // Reverb filter state
    float reverbFilterStateL = 0.0f;
    float reverbFilterStateR = 0.0f;

    // Sequencer swing/direction
    std::atomic<float>* swingParam = nullptr;
    std::atomic<float>* seqDirectionParam = nullptr;
    std::atomic<float>* hostSyncParam = nullptr;

    // Ring modulator parameters
    std::atomic<float>* ringModFreqParam = nullptr;
    std::atomic<float>* ringModMixParam = nullptr;

    // Scale quantization parameters
    std::atomic<float>* scaleTypeParam = nullptr;
    std::atomic<float>* scaleRootParam = nullptr;

    // Scale quantization helper
    float quantizePitchToScale(float pitchSemitones, int scaleType, int root);

    // === MOD MATRIX ===
    // LFO
    double lfoPhase = 0.0;
    std::atomic<float>* lfoRateParam = nullptr;
    std::atomic<float>* lfoWaveParam = nullptr;
    std::atomic<float>* lfoSyncParam = nullptr;  // tempo sync on/off

    // Mod slots (4 slots)
    static constexpr int NUM_MOD_SLOTS = 4;
    std::array<std::atomic<float>*, NUM_MOD_SLOTS> modSrcParams = {};
    std::array<std::atomic<float>*, NUM_MOD_SLOTS> modDstParams = {};
    std::array<std::atomic<float>*, NUM_MOD_SLOTS> modAmtParams = {};

    // Mod matrix helper
    float generateLFO(float waveform);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DFAMSynthAudioProcessor)
};
