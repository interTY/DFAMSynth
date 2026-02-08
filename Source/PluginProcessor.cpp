#include "PluginProcessor.h"
#include "PluginEditor.h"

DFAMSynthAudioProcessor::DFAMSynthAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache parameter pointers - VCO section
    vcoDecayParam = apvts.getRawParameterValue("vcoDecay");
    seqPitchModParam = apvts.getRawParameterValue("seqPitchMod");
    vco1EgAmtParam = apvts.getRawParameterValue("vco1EgAmt");
    vco1FreqParam = apvts.getRawParameterValue("vco1Freq");
    vco1WaveParam = apvts.getRawParameterValue("vco1Wave");
    vco1LevelParam = apvts.getRawParameterValue("vco1Level");
    subLevelParam = apvts.getRawParameterValue("subLevel");
    noiseLevelParam = apvts.getRawParameterValue("noiseLevel");

    fmAmountParam = apvts.getRawParameterValue("fmAmount");
    hardSyncParam = apvts.getRawParameterValue("hardSync");
    vco2EgAmtParam = apvts.getRawParameterValue("vco2EgAmt");
    vco2FreqParam = apvts.getRawParameterValue("vco2Freq");
    vco2WaveParam = apvts.getRawParameterValue("vco2Wave");
    vco2LevelParam = apvts.getRawParameterValue("vco2Level");

    // Filter section
    filterCutoffParam = apvts.getRawParameterValue("filterCutoff");
    filterModeParam = apvts.getRawParameterValue("filterMode");
    filterResParam = apvts.getRawParameterValue("filterRes");
    vcaEgModeParam = apvts.getRawParameterValue("vcaEgMode");
    vcaLevelParam = apvts.getRawParameterValue("vcaLevel");

    filterDecayParam = apvts.getRawParameterValue("filterDecay");
    filterEnvAmtParam = apvts.getRawParameterValue("filterEnvAmt");
    noiseVcfModParam = apvts.getRawParameterValue("noiseVcfMod");
    vcaDecayParam = apvts.getRawParameterValue("vcaDecay");

    // Sequencer
    tempoParam = apvts.getRawParameterValue("tempo");
    tempoMultParam = apvts.getRawParameterValue("tempoMult");
    seqRunParam = apvts.getRawParameterValue("seqRun");
    glideParam = apvts.getRawParameterValue("glide");
    droneParam = apvts.getRawParameterValue("drone");
    midiHoldParam = apvts.getRawParameterValue("midiHold");

    for (int i = 0; i < 8; ++i)
    {
        seqPitchParams[i] = apvts.getRawParameterValue("seqPitch" + juce::String(i + 1));
        seqVelParams[i] = apvts.getRawParameterValue("seqVel" + juce::String(i + 1));
        seqPanParams[i] = apvts.getRawParameterValue("seqPan" + juce::String(i + 1));
        seqWaveParams[i] = apvts.getRawParameterValue("seqWave_" + juce::String(i + 1));
        seqRingModParams[i] = apvts.getRawParameterValue("seqRingMod_" + juce::String(i + 1));
        seqDelayPitchParams[i] = apvts.getRawParameterValue("seqDelayPitch_" + juce::String(i + 1));
    }

    // Delay
    delayTimeParam = apvts.getRawParameterValue("delayTime");
    delayFeedbackParam = apvts.getRawParameterValue("delayFeedback");
    delayFilterParam = apvts.getRawParameterValue("delayFilter");
    delayMixParam = apvts.getRawParameterValue("delayMix");

    // Reverb
    reverbDecayParam = apvts.getRawParameterValue("reverbDecay");
    reverbFilterParam = apvts.getRawParameterValue("reverbFilter");
    reverbMixParam = apvts.getRawParameterValue("reverbMix");

    // Sequencer swing/direction
    swingParam = apvts.getRawParameterValue("swing");
    seqDirectionParam = apvts.getRawParameterValue("seqDirection");
    hostSyncParam = apvts.getRawParameterValue("hostSync");

    // Ring modulator
    ringModFreqParam = apvts.getRawParameterValue("ringModFreq");
    ringModMixParam = apvts.getRawParameterValue("ringModMix");

    // Scale quantization
    scaleTypeParam = apvts.getRawParameterValue("scaleType");
    scaleRootParam = apvts.getRawParameterValue("scaleRoot");

    // Mod matrix
    lfoRateParam = apvts.getRawParameterValue("lfoRate");
    lfoWaveParam = apvts.getRawParameterValue("lfoWave");
    lfoSyncParam = apvts.getRawParameterValue("lfoSync");

    for (int i = 0; i < NUM_MOD_SLOTS; ++i)
    {
        juce::String num = juce::String(i + 1);
        modSrcParams[i] = apvts.getRawParameterValue("modSrc" + num);
        modDstParams[i] = apvts.getRawParameterValue("modDst" + num);
        modAmtParams[i] = apvts.getRawParameterValue("modAmt" + num);
    }
}

DFAMSynthAudioProcessor::~DFAMSynthAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DFAMSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ===== ROW 1: VCO DECAY, SEQ PITCH MOD, VCO1 EG AMT, VCO1 FREQ, VCO1 WAVE, VCO1 LEVEL, NOISE LEVEL, CUTOFF, RESONANCE, VCA EG, VOLUME =====

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vcoDecay", 1), "VCO Decay",
        juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.4f), 200.0f, "ms"));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("seqPitchMod", 1), "Seq Pitch Mod",
        juce::StringArray("VCO 1&2", "OFF", "VCO 2"), 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco1EgAmt", 1), "VCO1 EG Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco1Freq", 1), "VCO1 Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "st")); // C2 = 0, +/-2 octaves

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco1Wave", 1), "VCO1 Wave",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.33f)); // 0=sine, 0.33=tri, 0.66=sq, 1=chaos

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco1Level", 1), "VCO1 Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("subLevel", 1), "Sub Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("noiseLevel", 1), "Noise Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("filterCutoff", 1), "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f), 1000.0f, "Hz"));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("filterMode", 1), "Filter HP/LP", false)); // false=LP, true=HP

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("filterRes", 1), "Filter Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.2f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("vcaEgMode", 1), "VCA EG Fast/Slow", false)); // false=fast, true=slow

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vcaLevel", 1), "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    // ===== ROW 2: FM AMOUNT, HARD SYNC, VCO2 EG AMT, VCO2 FREQ, VCO2 WAVE, VCO2 LEVEL, VCF DECAY, VCF EG AMT, NOISE/VCF MOD, VCA DECAY =====

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("fmAmount", 1), "1-2 FM Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("hardSync", 1), "Hard Sync", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco2EgAmt", 1), "VCO2 EG Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco2Freq", 1), "VCO2 Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "st")); // C2 = 0, +/-2 octaves

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco2Wave", 1), "VCO2 Wave",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.33f)); // 0=sine, 0.33=tri, 0.66=sq, 1=chaos

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vco2Level", 1), "VCO2 Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("filterDecay", 1), "VCF Decay",
        juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.4f), 200.0f, "ms"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("filterEnvAmt", 1), "VCF EG Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("noiseVcfMod", 1), "Noise/VCF Mod",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vcaDecay", 1), "VCA Decay",
        juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.4f), 200.0f, "ms"));

    // ===== DELAY =====
    // Very short minimum for Karplus-Strong (0.0001s = 0.1ms = 10kHz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayTime", 1), "Delay Time",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.0001f, 0.25f), 0.0f, "s"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayFeedback", 1), "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayFilter", 1), "Delay Filter",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.3f), 8000.0f, "Hz"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayMix", 1), "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // ===== REVERB =====
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbDecay", 1), "Reverb Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbFilter", 1), "Reverb Filter",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.3f), 8000.0f, "Hz"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbMix", 1), "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // ===== RING MODULATOR =====
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("ringModFreq", 1), "Ring Mod Freq",
        juce::NormalisableRange<float>(1.0f, 5000.0f, 1.0f, 0.3f), 440.0f, "Hz"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("ringModMix", 1), "Ring Mod Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // ===== SEQUENCER =====

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("tempo", 1), "Tempo",
        juce::NormalisableRange<float>(30.0f, 300.0f, 0.1f), 120.0f, "BPM"));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("tempoMult", 1), "Tempo Multiplier",
        juce::StringArray("1/4x", "1/2x", "1x", "2x", "4x"), 2));  // Default: 1x

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("swing", 1), "Swing",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("seqDirection", 1), "Seq Direction",
        juce::StringArray("Forward", "Backward", "PingPong"), 0));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("hostSync", 1), "Host Sync", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("seqRun", 1), "Seq Run", false));

    // Glide/Portamento: 0 = instant (glide), 1 = very slow (drone)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("glide", 1), "Glide",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    // Drone mode: when ON, envelopes don't retrigger on each step
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("drone", 1), "Drone Mode", false));

    // MIDI Hold: when ON, note-offs are ignored (sustain all notes)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("midiHold", 1), "MIDI Hold", false));

    // Sequencer step parameters
    for (int i = 0; i < 8; ++i)
    {
        juce::String num = juce::String(i + 1);

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("seqPitch" + num, 1), "Step " + num + " Pitch",
            juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "st"));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("seqVel" + num, 1), "Step " + num + " Velocity",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("seqPan" + num, 1), "Step " + num + " Pan",
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("seqWave_" + num, 1), "Step " + num + " Wave",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.33f)); // default triangle

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("seqRingMod_" + num, 1), "Step " + num + " Ring Mod",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f)); // default middle

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("seqDelayPitch_" + num, 1), "Step " + num + " Delay Pitch",
            juce::NormalisableRange<float>(-24.0f, 48.0f, 0.1f), 0.0f, "st")); // Karplus-Strong tuning (extended for faster delays)
    }

    // ===== SCALE QUANTIZATION =====
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("scaleType", 1), "Scale Type",
        juce::StringArray("OFF", "Major", "Minor", "Harmonic Min", "Pent Major", "Pent Minor",
                          "Blues", "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian", "Whole Tone"), 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("scaleRoot", 1), "Scale Root",
        juce::StringArray("C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"), 0));

    // ===== MOD MATRIX =====
    // LFO controls
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoRate", 1), "LFO Rate",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f, 0.4f), 1.0f, "Hz"));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lfoWave", 1), "LFO Wave",
        juce::StringArray("Sine", "Triangle", "Square", "Saw Up", "Saw Down", "S&H"), 0));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("lfoSync", 1), "LFO Tempo Sync", false));

    // Mod slots (4 slots, each with source, destination, amount)
    // Sources: OFF, LFO, Pitch Env, Filter Env, VCA Env, Velocity, Random
    // Destinations: OFF, Flt Cut, Flt Res, VCO1 Pitch, VCO2 Pitch, Ring Freq, Pan, VCO1 Level, VCO2 Level
    for (int i = 0; i < 4; ++i)
    {
        juce::String num = juce::String(i + 1);

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID("modSrc" + num, 1), "Mod Slot " + num + " Source",
            juce::StringArray("OFF", "LFO", "Pitch Env", "Filter Env", "VCA Env", "Velocity", "Random"), 0));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID("modDst" + num, 1), "Mod Slot " + num + " Dest",
            juce::StringArray("OFF", "Flt Cut", "Flt Res", "VCO1 Pitch", "VCO2 Pitch", "Ring Freq", "Pan", "VCO1 Lvl", "VCO2 Lvl", "VCA Decay", "Noise VCF", "VCF Decay", "FM Amt"), 0));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("modAmt" + num, 1), "Mod Slot " + num + " Amount",
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    }

    return { params.begin(), params.end() };
}

const juce::String DFAMSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DFAMSynthAudioProcessor::acceptsMidi() const { return true; }
bool DFAMSynthAudioProcessor::producesMidi() const { return false; }
bool DFAMSynthAudioProcessor::isMidiEffect() const { return false; }
double DFAMSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int DFAMSynthAudioProcessor::getNumPrograms() { return 1; }
int DFAMSynthAudioProcessor::getCurrentProgram() { return 0; }
void DFAMSynthAudioProcessor::setCurrentProgram(int) {}
const juce::String DFAMSynthAudioProcessor::getProgramName(int) { return {}; }
void DFAMSynthAudioProcessor::changeProgramName(int, const juce::String&) {}

void DFAMSynthAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;

    vco1.prepare(sampleRate);
    vco2.prepare(sampleRate);
    subOsc.prepare(sampleRate);
    noise.prepare(sampleRate);
    filter.prepare(sampleRate);
    pitchEnv.prepare(sampleRate);
    filterEnv.prepare(sampleRate);
    vcaEnv.prepare(sampleRate);
    sequencer.prepare(sampleRate);

    // Initialize delay buffer (max 2 seconds)
    delayBufferSize = static_cast<int>(sampleRate * 2.0);
    delayBuffer.resize(delayBufferSize, 0.0f);
    delayWritePos = 0;

    // Initialize reverb
    reverb.setSampleRate(sampleRate);

    // Initialize reverb pre-delay buffer (max 100ms)
    reverbPreDelaySize = static_cast<int>(sampleRate * 0.1);
    reverbPreDelayL.resize(reverbPreDelaySize, 0.0f);
    reverbPreDelayR.resize(reverbPreDelaySize, 0.0f);
    reverbPreDelayWritePos = 0;
}

void DFAMSynthAudioProcessor::releaseResources()
{
}

bool DFAMSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

float DFAMSynthAudioProcessor::generateLFO(float waveform)
{
    // waveform: 0=Sine, 1=Triangle, 2=Square, 3=Saw Up, 4=Saw Down, 5=S&H
    int wave = static_cast<int>(waveform);
    float phase = static_cast<float>(lfoPhase);

    switch (wave)
    {
        case 0:  // Sine
            return std::sin(phase * 2.0f * juce::MathConstants<float>::pi);

        case 1:  // Triangle
            if (phase < 0.25f)
                return phase * 4.0f;
            else if (phase < 0.75f)
                return 1.0f - (phase - 0.25f) * 4.0f;
            else
                return -1.0f + (phase - 0.75f) * 4.0f;

        case 2:  // Square
            return phase < 0.5f ? 1.0f : -1.0f;

        case 3:  // Saw Up
            return phase * 2.0f - 1.0f;

        case 4:  // Saw Down
            return 1.0f - phase * 2.0f;

        case 5:  // Sample & Hold (random value held until next cycle)
        {
            static float shValue = 0.0f;
            static float lastPhase = 0.0f;
            if (phase < lastPhase)  // Phase wrapped, new random value
                shValue = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            lastPhase = phase;
            return shValue;
        }

        default:
            return 0.0f;
    }
}

float DFAMSynthAudioProcessor::quantizePitchToScale(float pitchSemitones, int scaleType, int root)
{
    // Scale type 0 = OFF (chromatic), return unchanged
    if (scaleType == 0)
        return pitchSemitones;

    // Scale intervals (semitones from root)
    static const std::vector<std::vector<int>> scales = {
        {},                                    // 0: OFF (not used)
        {0, 2, 4, 5, 7, 9, 11},                // 1: Major
        {0, 2, 3, 5, 7, 8, 10},                // 2: Natural Minor
        {0, 2, 3, 5, 7, 8, 11},                // 3: Harmonic Minor
        {0, 2, 4, 7, 9},                       // 4: Pentatonic Major
        {0, 3, 5, 7, 10},                      // 5: Pentatonic Minor
        {0, 3, 5, 6, 7, 10},                   // 6: Blues
        {0, 2, 3, 5, 7, 9, 10},                // 7: Dorian
        {0, 1, 3, 5, 7, 8, 10},                // 8: Phrygian
        {0, 2, 4, 6, 7, 9, 11},                // 9: Lydian
        {0, 2, 4, 5, 7, 9, 10},                // 10: Mixolydian
        {0, 1, 3, 5, 6, 8, 10},                // 11: Locrian
        {0, 2, 4, 6, 8, 10}                    // 12: Whole Tone
    };

    if (scaleType < 1 || scaleType >= static_cast<int>(scales.size()))
        return pitchSemitones;

    const auto& scale = scales[scaleType];

    // Shift pitch relative to root (root = 0 means C, etc.)
    float shiftedPitch = pitchSemitones - static_cast<float>(root);

    // Find octave and position within octave
    int octave = static_cast<int>(std::floor(shiftedPitch / 12.0f));
    float noteInOctave = shiftedPitch - (octave * 12.0f);

    // Make sure noteInOctave is positive (0-11.99)
    if (noteInOctave < 0.0f)
    {
        noteInOctave += 12.0f;
        octave -= 1;
    }

    // Find the closest note in the scale
    float minDistance = 100.0f;
    int closestNote = 0;

    for (int note : scale)
    {
        float distance = std::abs(noteInOctave - static_cast<float>(note));
        // Also check wrapping (e.g., 11.5 should snap to 0 of next octave)
        float wrapDistance = std::abs(noteInOctave - (static_cast<float>(note) + 12.0f));
        float wrapDistanceNeg = std::abs(noteInOctave - (static_cast<float>(note) - 12.0f));

        if (distance < minDistance)
        {
            minDistance = distance;
            closestNote = note;
        }
        if (wrapDistance < minDistance)
        {
            minDistance = wrapDistance;
            closestNote = note + 12;
        }
        if (wrapDistanceNeg < minDistance)
        {
            minDistance = wrapDistanceNeg;
            closestNote = note - 12;
        }
    }

    // Calculate quantized pitch
    float quantizedPitch = static_cast<float>(octave * 12 + closestNote + root);

    return quantizedPitch;
}

void DFAMSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear buffer
    for (auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Process MIDI messages
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            lastMidiNote = message.getNoteNumber();
            midiNoteActive = true;
            // Convert MIDI note to pitch offset from C2 (MIDI note 36)
            // C2 = 36, so offset = noteNumber - 36
            midiNotePitch = static_cast<float>(lastMidiNote - 36);
        }
        else if (message.isNoteOff())
        {
            // When MIDI hold is active, ignore note-offs
            bool holdActive = midiHoldParam->load() > 0.5f;
            if (!holdActive && message.getNoteNumber() == lastMidiNote)
            {
                midiNoteActive = false;
            }
        }
    }

    // Read all parameters
    float vcoDecay = vcoDecayParam->load();
    int seqPitchMod = static_cast<int>(seqPitchModParam->load());  // 0=VCO1&2, 1=OFF, 2=VCO2
    float vco1EgAmt = vco1EgAmtParam->load();
    float vco1Freq = vco1FreqParam->load();
    float vco1Wave = vco1WaveParam->load();
    float vco1Level = vco1LevelParam->load();
    float subLevel = subLevelParam->load();
    float noiseLevel = noiseLevelParam->load();

    float fmAmount = fmAmountParam->load();
    bool hardSync = hardSyncParam->load() > 0.5f;
    float vco2EgAmt = vco2EgAmtParam->load();
    float vco2Freq = vco2FreqParam->load();
    float vco2Wave = vco2WaveParam->load();
    float vco2Level = vco2LevelParam->load();

    float filterCutoff = filterCutoffParam->load();
    bool filterModeHP = filterModeParam->load() > 0.5f;
    float filterRes = filterResParam->load();
    bool vcaEgSlow = vcaEgModeParam->load() > 0.5f;
    float vcaLevel = vcaLevelParam->load();

    float filterDecay = filterDecayParam->load();
    float filterEnvAmt = filterEnvAmtParam->load();
    float noiseVcfMod = noiseVcfModParam->load();
    float vcaDecay = vcaDecayParam->load();

    float tempo = tempoParam->load();
    int tempoMultIdx = static_cast<int>(tempoMultParam->load());
    const float tempoMultipliers[] = { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f };
    float swing = swingParam->load();
    int seqDirection = static_cast<int>(seqDirectionParam->load());
    bool seqRun = seqRunParam->load() > 0.5f;
    bool hostSync = hostSyncParam->load() > 0.5f;

    // Host sync - override tempo and transport with DAW values
    if (hostSync)
    {
        if (auto* playHead = getPlayHead())
        {
            if (auto posInfo = playHead->getPosition())
            {
                // Sync tempo to host
                if (posInfo->getBpm().hasValue())
                    tempo = static_cast<float>(*posInfo->getBpm());

                // Sync transport to host (play/stop)
                if (posInfo->getIsPlaying())
                    seqRun = true;
                else
                    seqRun = false;
            }
        }
    }

    // Apply tempo multiplier AFTER host sync so it works in both modes
    tempo *= tempoMultipliers[tempoMultIdx];

    // Ring modulator parameters
    float ringModFreq = ringModFreqParam->load();
    float ringModMix = ringModMixParam->load();
    ringModPhaseInc = ringModFreq / currentSampleRate;

    // Delay parameters
    float delayTimeSeconds = delayTimeParam->load();
    float delayFeedback = delayFeedbackParam->load();
    float delayFilterCutoff = delayFilterParam->load();
    float delayMix = delayMixParam->load();

    // Reverb parameters
    float reverbDecay = reverbDecayParam->load();
    float reverbFilterCutoff = reverbFilterParam->load();
    float reverbMix = reverbMixParam->load();

    // Calculate reverb filter coefficient
    float reverbFilterCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * reverbFilterCutoff / static_cast<float>(currentSampleRate));

    // Scale quantization parameters
    int scaleType = static_cast<int>(scaleTypeParam->load());
    int scaleRoot = static_cast<int>(scaleRootParam->load());

    // Mod matrix parameters
    float lfoRate = lfoRateParam->load();
    int lfoWave = static_cast<int>(lfoWaveParam->load());
    bool lfoSync = lfoSyncParam->load() > 0.5f;

    // If tempo-synced, convert rate to divisions of tempo
    // Rate 1.0 = 1 bar, 2.0 = 1/2 note, 4.0 = 1/4 note, etc.
    double lfoPhaseInc;
    if (lfoSync)
    {
        // Sync rate: map 0.1-20 to tempo divisions
        // We'll make rate represent "cycles per beat"
        float beatsPerSecond = tempo / 60.0f;
        lfoPhaseInc = (lfoRate * beatsPerSecond) / currentSampleRate;
    }
    else
    {
        lfoPhaseInc = lfoRate / currentSampleRate;
    }

    // Read mod slot parameters
    std::array<int, NUM_MOD_SLOTS> modSrc, modDst;
    std::array<float, NUM_MOD_SLOTS> modAmt;
    for (int i = 0; i < NUM_MOD_SLOTS; ++i)
    {
        modSrc[i] = static_cast<int>(modSrcParams[i]->load());
        modDst[i] = static_cast<int>(modDstParams[i]->load());
        modAmt[i] = modAmtParams[i]->load();
    }

    // Base delay time from slider (will be combined with sequencer pitch in the loop)

    // Calculate lowpass filter coefficient for delay feedback
    float delayFilterCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * delayFilterCutoff / static_cast<float>(currentSampleRate));

    // Update reverb parameters
    // Improved reverb settings for warmer sound
    reverbParams.roomSize = 0.2f + reverbDecay * 0.5f;  // 0.2 to 0.7 range (less extreme)
    reverbParams.damping = 0.7f + reverbDecay * 0.25f;  // 0.7 to 0.95 - warmer, less metallic
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverbParams.width = 0.6f + reverbDecay * 0.3f;     // Tighter stereo image
    reverb.setParameters(reverbParams);

    // Update sequencer step parameters (with scale quantization)
    for (int i = 0; i < 8; ++i)
    {
        float rawPitch = seqPitchParams[i]->load();
        float quantizedPitch = quantizePitchToScale(rawPitch, scaleType, scaleRoot);
        sequencer.setStepPitch(i, quantizedPitch);
        sequencer.setStepVelocity(i, seqVelParams[i]->load());
        sequencer.setStepPan(i, seqPanParams[i]->load());
        sequencer.setStepWave(i, seqWaveParams[i]->load());
        sequencer.setStepRingMod(i, seqRingModParams[i]->load());
        // Also quantize delay pitch to scale
        float rawDelayPitch = seqDelayPitchParams[i]->load();
        float quantizedDelayPitch = quantizePitchToScale(rawDelayPitch, scaleType, scaleRoot);
        sequencer.setStepDelayPitch(i, quantizedDelayPitch);
    }

    // Convert semitone pitch to Hz (C2 = 65.41 Hz as base)
    const float c2Hz = 65.41f;
    // Add MIDI pitch offset to VCO frequencies (when MIDI note is active)
    float midiPitchOffset = midiNoteActive ? midiNotePitch : 0.0f;
    float vco1FreqHz = c2Hz * std::pow(2.0f, (vco1Freq + midiPitchOffset) / 12.0f);
    float vco2FreqHz = c2Hz * std::pow(2.0f, (vco2Freq + midiPitchOffset) / 12.0f);

    // Set up oscillators (waveform set per-step in the loop)
    vco1.setFrequency(vco1FreqHz);
    vco1.setWaveformPosition(vco1Wave);  // default, will be overridden per-step
    vco2.setFrequency(vco2FreqHz);
    vco2.setWaveformPosition(vco2Wave);  // default, will be overridden per-step
    subOsc.setFrequency(vco1FreqHz * 0.5f);  // 1 octave below VCO1
    subOsc.setWaveformPosition(0.66f);       // Fixed square wave

    // Set up filter
    filter.setResonance(filterRes);
    filter.setMode(filterModeHP ? LadderFilter::Mode::Highpass : LadderFilter::Mode::Lowpass);

    // Set up envelopes
    pitchEnv.setDecayTime(vcoDecay);
    filterEnv.setDecayTime(filterDecay);

    // VCA envelope - fast or slow mode (slow = 4x longer)
    float vcaDecayActual = vcaEgSlow ? vcaDecay * 4.0f : vcaDecay;
    vcaEnv.setDecayTime(vcaDecayActual);

    // Set up sequencer
    sequencer.setTempo(tempo);
    sequencer.setSwing(swing);
    sequencer.setDirection(seqDirection);
    sequencer.setRunning(seqRun);

    // Check for manual trigger/advance
    bool doManualTrigger = manualTrigger.exchange(false);
    bool doManualAdvance = manualAdvance.exchange(false);

    // Process audio
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = totalNumOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Process sequencer
        bool stepTrigger = sequencer.process();

        // Handle manual advance (only on first sample of block)
        if (sample == 0 && doManualAdvance)
        {
            sequencer.advanceStep();
            stepTrigger = true;
        }

        // Handle manual trigger
        if (sample == 0 && doManualTrigger)
        {
            stepTrigger = true;
        }

        if (stepTrigger)
        {
            // In drone mode, don't retrigger envelopes - sound continues smoothly
            bool droneMode = droneParam->load() > 0.5f;
            if (!droneMode)
            {
                // Trigger all envelopes
                float velocity = sequencer.getCurrentVelocity();
                pitchEnv.trigger(velocity);
                filterEnv.trigger(velocity);
                vcaEnv.trigger(velocity);
            }
        }

        // Get envelope values
        float pitchEnvValue = pitchEnv.process();
        float filterEnvValue = filterEnv.process();
        float vcaEnvValue = vcaEnv.process();

        // Generate LFO value
        float lfoValue = generateLFO(static_cast<float>(lfoWave));
        lfoPhase += lfoPhaseInc;
        if (lfoPhase >= 1.0)
            lfoPhase -= 1.0;

        // Process mod matrix - initialize modulation accumulators
        float filterCutoffMod = 0.0f;
        float filterResMod = 0.0f;
        float vco1PitchModMatrix = 0.0f;
        float vco2PitchModMatrix = 0.0f;
        float ringFreqMod = 0.0f;
        float panMod = 0.0f;
        float vco1LevelMod = 0.0f;
        float vco2LevelMod = 0.0f;
        float vcaDecayMod = 0.0f;
        float noiseVcfModMod = 0.0f;
        float vcfDecayMod = 0.0f;
        float fmAmountMod = 0.0f;

        // Current velocity from sequencer (for velocity mod source)
        float currentVelocity = sequencer.getCurrentVelocity();

        // Random value for random mod source (regenerated per-sample for variation)
        static float randomModValue = 0.0f;
        static int randomCounter = 0;
        if (++randomCounter > static_cast<int>(currentSampleRate / 50.0))  // ~50Hz update
        {
            randomModValue = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            randomCounter = 0;
        }

        for (int slot = 0; slot < NUM_MOD_SLOTS; ++slot)
        {
            if (modSrc[slot] == 0 || modDst[slot] == 0)
                continue;  // Skip if source or dest is OFF

            // Get modulation source value (-1 to +1)
            float srcValue = 0.0f;
            switch (modSrc[slot])
            {
                case 1: srcValue = lfoValue; break;           // LFO
                case 2: srcValue = pitchEnvValue * 2.0f - 1.0f; break;  // Pitch Env (0-1 -> -1 to +1)
                case 3: srcValue = filterEnvValue * 2.0f - 1.0f; break; // Filter Env
                case 4: srcValue = vcaEnvValue * 2.0f - 1.0f; break;    // VCA Env
                case 5: srcValue = currentVelocity * 2.0f - 1.0f; break; // Velocity
                case 6: srcValue = randomModValue; break;      // Random
            }

            // Apply amount
            float modValue = srcValue * modAmt[slot];

            // Route to destination
            switch (modDst[slot])
            {
                case 1: filterCutoffMod += modValue; break;    // Filter Cutoff
                case 2: filterResMod += modValue; break;       // Filter Resonance
                case 3: vco1PitchModMatrix += modValue * 12.0f; break;  // VCO1 Pitch (±12 semitones)
                case 4: vco2PitchModMatrix += modValue * 12.0f; break;  // VCO2 Pitch
                case 5: ringFreqMod += modValue; break;        // Ring Freq
                case 6: panMod += modValue; break;             // Pan
                case 7: vco1LevelMod += modValue * 0.5f; break; // VCO1 Level
                case 8: vco2LevelMod += modValue * 0.5f; break; // VCO2 Level
                case 9: vcaDecayMod += modValue; break;        // VCA Decay
                case 10: noiseVcfModMod += modValue; break;    // Noise VCF Mod
                case 11: vcfDecayMod += modValue; break;       // VCF Decay
                case 12: fmAmountMod += modValue * 0.5f; break; // FM Amount
            }
        }

        // Calculate pitch modulation from sequencer with glide/portamento
        float seqPitchSemitones = 0.0f;
        if (seqPitchMod != 1)  // Not OFF
        {
            // Get target pitch from sequencer
            targetGlidePitch = std::log2(sequencer.getCurrentPitchMultiplier()) * 12.0f;

            // Apply glide (portamento)
            // glide 0 = instant, glide 1 = very slow (drone-like)
            float glideAmount = glideParam->load();
            bool droneMode = droneParam->load() > 0.5f;

            // In drone mode, force very slow crossfade glide
            if (droneMode)
            {
                glideAmount = std::max(glideAmount, 0.85f);  // Minimum 85% glide in drone mode
            }

            if (glideAmount < 0.01f)
            {
                // No glide - instant pitch change
                currentGlidePitch = targetGlidePitch;
            }
            else
            {
                // Glide: smoothly move toward target
                // Higher glide value = slower transition
                // Map glide 0-1 to time constant (fast to very slow)
                float glideSpeed = 1.0f - glideAmount;  // 1 = fast, 0 = frozen
                glideSpeed = glideSpeed * glideSpeed;  // Quadratic curve - less aggressive at low values

                // In drone mode, make transitions even smoother
                float baseSpeed = droneMode ? 5.0f : 20.0f;
                float glideCoeff = 1.0f - std::exp(-glideSpeed * baseSpeed / static_cast<float>(currentSampleRate));

                currentGlidePitch += (targetGlidePitch - currentGlidePitch) * glideCoeff;
            }

            seqPitchSemitones = currentGlidePitch;
        }

        // Calculate pitch envelope modulation (in semitones, scaled by amount)
        // Add mod matrix pitch modulation
        float vco1PitchMod = pitchEnvValue * vco1EgAmt * 24.0f + vco1PitchModMatrix;
        float vco2PitchMod = pitchEnvValue * vco2EgAmt * 24.0f + vco2PitchModMatrix;

        // Add sequencer pitch to appropriate oscillators
        if (seqPitchMod == 0)  // VCO 1&2
        {
            vco1PitchMod += seqPitchSemitones;
            vco2PitchMod += seqPitchSemitones;
        }
        else if (seqPitchMod == 2)  // VCO 2 only
        {
            vco2PitchMod += seqPitchSemitones;
        }

        // Combine VCO wave knobs with sequencer wave modulation
        // Sequencer wave (0-1) adds modulation to the base wave position
        float seqWaveMod = (sequencer.getCurrentWave() - 0.5f) * 0.5f;  // -0.25 to +0.25 modulation
        float vco1WaveTarget = std::clamp(vco1Wave + seqWaveMod, 0.0f, 1.0f);
        float vco2WaveTarget = std::clamp(vco2Wave + seqWaveMod, 0.0f, 1.0f);

        // In drone mode, smooth waveform transitions to avoid clicks
        bool droneModeWave = droneParam->load() > 0.5f;
        if (droneModeWave)
        {
            float waveSmooth = 1.0f - std::exp(-5.0f / static_cast<float>(currentSampleRate));
            smoothedWave1 += (vco1WaveTarget - smoothedWave1) * waveSmooth;
            smoothedWave2 += (vco2WaveTarget - smoothedWave2) * waveSmooth;
            vco1.setWaveformPosition(smoothedWave1);
            vco2.setWaveformPosition(smoothedWave2);
        }
        else
        {
            vco1.setWaveformPosition(vco1WaveTarget);
            vco2.setWaveformPosition(vco2WaveTarget);
        }

        // Generate VCO2 first (needed for FM and sync)
        float vco2Sample = vco2.processWithPitchMod(vco2PitchMod);

        // Hard sync: reset VCO1 phase when VCO2 completes a cycle
        if (hardSync && vco2.hasCompletedCycle())
        {
            vco1.sync();
        }

        // Generate VCO1 with FM from VCO2 and pitch modulation
        // Apply mod matrix to FM amount
        float modulatedFmAmount = std::clamp(fmAmount + fmAmountMod, 0.0f, 1.0f);
        float vco1PitchWithFM = vco1PitchMod + (vco2Sample * modulatedFmAmount * 24.0f);
        float vco1Sample = vco1.processWithPitchMod(vco1PitchWithFM);

        // Generate sub oscillator (follows VCO1 pitch modulation, 1 octave below)
        float subSample = 0.0f;
        if (subLevel > 0.0f)
        {
            subSample = subOsc.processWithPitchMod(vco1PitchMod);
        }

        // Generate noise
        float noiseSample = noise.process();

        // Apply mod matrix level modulation (clamped to 0-1)
        float vco1LevelModulated = std::clamp(vco1Level + vco1LevelMod, 0.0f, 1.0f);
        float vco2LevelModulated = std::clamp(vco2Level + vco2LevelMod, 0.0f, 1.0f);

        // Mix all oscillators
        float mixed = vco1Sample * vco1LevelModulated + vco2Sample * vco2LevelModulated + noiseSample * noiseLevel;
        mixed += subSample * subLevel;

        // Calculate filter cutoff modulation
        // Apply mod matrix to noise VCF mod (noiseVcfModMod adds ±1 to the -1 to +1 range)
        float modulatedNoiseVcfMod = std::clamp(noiseVcfMod + noiseVcfModMod, -1.0f, 1.0f);
        // Apply mod matrix to filter envelope amount (vcfDecayMod scales the env amount)
        float modulatedFilterEnvAmt = std::clamp(filterEnvAmt + vcfDecayMod, -1.0f, 1.0f);
        float cutoffMod = filterEnvValue * modulatedFilterEnvAmt * 10.0f;
        // Directional noise modulation: positive = brighten, negative = darken
        float noiseVcfValue = (modulatedNoiseVcfMod >= 0.0f)
            ? std::abs(noiseSample) * modulatedNoiseVcfMod * 2.0f
            : -std::abs(noiseSample) * modulatedNoiseVcfMod * 2.0f;
        cutoffMod += noiseVcfValue;
        cutoffMod += filterCutoffMod * 5.0f;  // Mod matrix: ±5 octaves

        float modulatedCutoff = filterCutoff * std::pow(2.0f, cutoffMod);
        modulatedCutoff = std::clamp(modulatedCutoff, 20.0f, 20000.0f);

        // Apply resonance modulation
        float modulatedRes = std::clamp(filterRes + filterResMod * 0.5f, 0.0f, 1.0f);

        filter.setCutoff(modulatedCutoff);
        filter.setResonance(modulatedRes);
        float filtered = filter.process(mixed);

        // Apply VCA envelope (in drone mode, keep VCA open)
        // VCA Decay mod affects the envelope curve (positive = longer sustain, negative = faster decay)
        bool droneMode = droneParam->load() > 0.5f;
        float modulatedVcaEnvValue = vcaEnvValue;
        if (vcaDecayMod > 0.0f)
            modulatedVcaEnvValue = std::pow(vcaEnvValue, 1.0f - vcaDecayMod * 0.8f);  // Slower decay
        else if (vcaDecayMod < 0.0f)
            modulatedVcaEnvValue = std::pow(vcaEnvValue, 1.0f - vcaDecayMod * 2.0f);  // Faster decay
        float vcaValue = droneMode ? 1.0f : modulatedVcaEnvValue;
        float output = filtered * vcaValue * vcaLevel;

        // === FX ORDER: Delay (with filter) -> Ring Mod -> Reverb ===

        // 1. Calculate Karplus-Strong tuned delay time from sequencer pitch
        // Base frequency C2 = 65.41 Hz, pitch in semitones offsets this
        float delayPitch = sequencer.getCurrentDelayPitch();
        const float ksBaseFreq = 65.41f; // C2
        float ksFreq = ksBaseFreq * std::pow(2.0f, delayPitch / 12.0f);
        float ksDelayTimeSeconds = 1.0f / ksFreq;

        // Combine Karplus-Strong pitch-based delay with base delay time slider
        // Base slider adds offset for fine-tuning or longer echo effects
        float totalDelayTime = ksDelayTimeSeconds + delayTimeSeconds;
        int delaySamples = static_cast<int>(totalDelayTime * currentSampleRate);
        delaySamples = std::clamp(delaySamples, 1, delayBufferSize - 1);

        // Apply delay with lowpass filter in feedback
        int readPos = (delayWritePos - delaySamples + delayBufferSize) % delayBufferSize;
        float delayedSample = delayBuffer[readPos];

        // Apply lowpass filter to feedback (one-pole filter)
        delayFilterState = delayFilterState * delayFilterCoeff + delayedSample * (1.0f - delayFilterCoeff);
        float filteredFeedback = delayFilterState;

        delayBuffer[delayWritePos] = output + filteredFeedback * delayFeedback;
        delayWritePos = (delayWritePos + 1) % delayBufferSize;
        output = output + delayedSample * delayMix;

        // 2. Apply ring modulator with sequencer modulation
        if (ringModMix > 0.0f)
        {
            // Sequencer modulates ring mod frequency (0-1 maps to 0.25x to 4x base freq)
            float seqRingMod = sequencer.getCurrentRingMod();
            float freqMult = 0.25f + seqRingMod * 3.75f;  // 0.25x to 4x
            // Apply mod matrix ring freq modulation (±2 octaves)
            freqMult *= std::pow(2.0f, ringFreqMod * 2.0f);
            double modulatedRingInc = ringModPhaseInc * freqMult;

            float ringModSignal = static_cast<float>(std::sin(ringModPhase * 2.0 * juce::MathConstants<double>::pi));
            ringModPhase += modulatedRingInc;
            if (ringModPhase >= 1.0)
                ringModPhase -= 1.0;

            float ringModOutput = output * ringModSignal;
            output = output * (1.0f - ringModMix) + ringModOutput * ringModMix;
        }

        // Apply per-step panning with mod matrix modulation
        float pan = std::clamp(sequencer.getCurrentPan() + panMod, -1.0f, 1.0f);
        float panAngle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        float leftGain = std::cos(panAngle);
        float rightGain = std::sin(panAngle);

        // Output (store for reverb processing)
        leftChannel[sample] = output * leftGain;
        if (rightChannel != nullptr)
            rightChannel[sample] = output * rightGain;
    }

    // 3. Apply reverb (final stage, post-delay, post-ring)
    if (reverbMix > 0.0f)
    {
        // Create temp buffer for wet signal
        std::vector<float> wetLeft(buffer.getNumSamples());
        std::vector<float> wetRight(buffer.getNumSamples());

        // Pre-delay: 30ms gives depth without being noticeable
        int preDelaySamples = static_cast<int>(currentSampleRate * 0.03);
        preDelaySamples = std::min(preDelaySamples, reverbPreDelaySize - 1);

        // Apply pre-delay to reverb input
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            // Read from pre-delay buffer
            int readPos = (reverbPreDelayWritePos - preDelaySamples + reverbPreDelaySize) % reverbPreDelaySize;
            wetLeft[i] = reverbPreDelayL[readPos];
            wetRight[i] = reverbPreDelayR[readPos];

            // Write current sample to pre-delay buffer
            reverbPreDelayL[reverbPreDelayWritePos] = leftChannel[i];
            reverbPreDelayR[reverbPreDelayWritePos] = rightChannel != nullptr ? rightChannel[i] : leftChannel[i];
            reverbPreDelayWritePos = (reverbPreDelayWritePos + 1) % reverbPreDelaySize;
        }

        // Process reverb in stereo
        reverb.processStereo(wetLeft.data(), wetRight.data(), buffer.getNumSamples());

        // Apply lowpass filter to reverb output and blend dry/wet
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            // Filter the wet signal (one-pole lowpass) - softens harsh highs
            reverbFilterStateL = reverbFilterStateL * reverbFilterCoeff + wetLeft[i] * (1.0f - reverbFilterCoeff);
            reverbFilterStateR = reverbFilterStateR * reverbFilterCoeff + wetRight[i] * (1.0f - reverbFilterCoeff);

            leftChannel[i] = leftChannel[i] * (1.0f - reverbMix) + reverbFilterStateL * reverbMix;
            if (rightChannel != nullptr)
                rightChannel[i] = rightChannel[i] * (1.0f - reverbMix) + reverbFilterStateR * reverbMix;
        }
    }
}

bool DFAMSynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* DFAMSynthAudioProcessor::createEditor()
{
    return new DFAMSynthAudioProcessorEditor(*this);
}

void DFAMSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DFAMSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::File DFAMSynthAudioProcessor::getPresetsFolder()
{
    auto userAppData = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto presetsFolder = userAppData.getChildFile("DFAM Synth").getChildFile("Presets");

    if (!presetsFolder.exists())
        presetsFolder.createDirectory();

    return presetsFolder;
}

void DFAMSynthAudioProcessor::savePreset(const juce::String& name)
{
    auto presetsFolder = getPresetsFolder();
    auto presetFile = presetsFolder.getChildFile(name + ".xml");

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    if (xml != nullptr)
        xml->writeTo(presetFile);

    refreshPresetList();
}

void DFAMSynthAudioProcessor::loadPreset(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return;

    std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(presetFile);

    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

void DFAMSynthAudioProcessor::refreshPresetList()
{
    presetNames.clear();
    presetFiles.clear();

    auto presetsFolder = getPresetsFolder();
    auto files = presetsFolder.findChildFiles(juce::File::findFiles, false, "*.xml");

    files.sort();

    for (auto& file : files)
    {
        presetNames.add(file.getFileNameWithoutExtension());
        presetFiles.add(file);
    }
}

juce::StringArray DFAMSynthAudioProcessor::getPresetList()
{
    return presetNames;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DFAMSynthAudioProcessor();
}
