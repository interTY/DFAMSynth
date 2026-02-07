#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DFAMSynthAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Timer
{
public:
    DFAMSynthAudioProcessorEditor(DFAMSynthAudioProcessor&);
    ~DFAMSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    DFAMSynthAudioProcessor& audioProcessor;

    // Preset controls
    juce::ComboBox presetBox;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;
    juce::TextButton initPresetButton;
    void updatePresetList();

    // === ROW 1 ===
    juce::Slider vcoDecaySlider;
    juce::ComboBox seqPitchModBox;
    juce::Slider vco1EgAmtSlider;
    juce::Slider vco1FreqSlider;
    juce::Slider vco1WaveSlider;
    juce::Slider vco1LevelSlider;
    juce::Slider subLevelSlider;
    juce::Slider noiseLevelSlider;
    juce::Slider filterCutoffSlider;
    juce::ToggleButton filterModeButton;
    juce::Slider filterResSlider;
    juce::ToggleButton vcaEgModeButton;
    juce::Slider vcaLevelSlider;

    // === ROW 2 ===
    juce::Slider fmAmountSlider;
    juce::ToggleButton hardSyncButton;
    juce::Slider vco2EgAmtSlider;
    juce::Slider vco2FreqSlider;
    juce::Slider vco2WaveSlider;
    juce::Slider vco2LevelSlider;
    juce::Slider filterDecaySlider;
    juce::Slider filterEnvAmtSlider;
    juce::Slider noiseVcfModSlider;
    juce::Slider vcaDecaySlider;

    // === DELAY ===
    juce::Slider delayTimeSlider;
    juce::Slider delayFeedbackSlider;
    juce::Slider delayFilterSlider;
    juce::Slider delayMixSlider;

    // === REVERB ===
    juce::Slider reverbDecaySlider;
    juce::Slider reverbFilterSlider;
    juce::Slider reverbMixSlider;

    // === RING MODULATOR ===
    juce::Slider ringModFreqSlider;
    juce::Slider ringModMixSlider;

    // === SEQUENCER ROW ===
    juce::Slider tempoSlider;
    juce::ComboBox tempoMultBox;
    juce::Label tempoMultLabel;
    juce::Slider swingSlider;
    juce::ComboBox seqDirectionBox;
    juce::TextButton hostSyncButton;
    juce::TextButton seqRunButton;
    juce::TextButton stopButton;
    juce::TextButton freezeButton;
    juce::TextButton randomPitchButton;
    juce::TextButton randomVelButton;
    juce::TextButton randomPanButton;
    juce::TextButton randomWaveButton;
    juce::TextButton randomRingButton;
    juce::TextButton randomDelayPitchButton;
    juce::ComboBox autoRndPitchBox;
    juce::ComboBox autoRndVelBox;
    juce::ComboBox autoRndPanBox;
    juce::ComboBox autoRndWaveBox;
    juce::ComboBox autoRndRingBox;
    juce::ComboBox autoRndDelayPitchBox;
    std::array<juce::Slider, 8> seqPitchSliders;
    std::array<juce::Slider, 8> seqVelSliders;
    std::array<juce::Slider, 8> seqPanSliders;
    std::array<juce::Slider, 8> seqWaveSliders;
    std::array<juce::Slider, 8> seqRingModSliders;
    std::array<juce::Slider, 8> seqDelayPitchSliders;
    std::array<juce::Label, 8> stepIndicators;

    // Labels
    juce::Label vcoDecayLabel, seqPitchModLabel, vco1EgAmtLabel, vco1FreqLabel, vco1WaveLabel, vco1LevelLabel;
    juce::Label subLevelLabel;
    juce::Label noiseLevelLabel, filterCutoffLabel, filterResLabel, vcaLevelLabel;
    juce::Label fmAmountLabel, vco2EgAmtLabel, vco2FreqLabel, vco2WaveLabel, vco2LevelLabel;
    juce::Label filterDecayLabel, filterEnvAmtLabel, noiseVcfModLabel, vcaDecayLabel;
    juce::Label tempoLabel, pitchLabel, velocityLabel, panLabel, waveLabel, ringLabel, delayPitchLabel;
    juce::Label delayTimeLabel, delayFbLabel, delayFilterLabel, delayMixLabel;
    juce::Label reverbDecayLabel, reverbFilterLabel, reverbMixLabel;
    juce::Label ringFreqLabel, ringMixLabel;
    juce::Label swingLabel, seqDirLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcoDecayAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> seqPitchModAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco1EgAmtAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco1FreqAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco1WaveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco1LevelAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subLevelAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseLevelAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> filterModeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vcaEgModeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaLevelAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fmAmountAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hardSyncAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco2EgAmtAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco2FreqAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco2WaveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vco2LevelAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterDecayAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterEnvAmtAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseVcfModAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaDecayAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> tempoMultAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> seqRunAtt;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> seqPitchAtts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> seqVelAtts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> seqPanAtts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> seqWaveAtts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> seqRingModAtts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> seqDelayPitchAtts;

    // Delay attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayFeedbackAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayFilterAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAtt;

    // Reverb attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDecayAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbFilterAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbMixAtt;

    // Ring modulator attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ringModFreqAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ringModMixAtt;

    // Sequencer swing/direction attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> swingAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> seqDirectionAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hostSyncAtt;

    // Glide and Drone
    juce::Slider glideSlider;
    juce::Label glideLabel;
    juce::TextButton droneButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glideAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> droneAtt;

    // Scale quantization
    juce::ComboBox scaleTypeBox;
    juce::ComboBox scaleRootBox;
    juce::Label scaleTypeLabel;
    juce::Label scaleRootLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleTypeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleRootAtt;

    // === MOD MATRIX ===
    // LFO controls
    juce::Slider lfoRateSlider;
    juce::Label lfoRateLabel;
    juce::ComboBox lfoWaveBox;
    juce::Label lfoWaveLabel;
    juce::ToggleButton lfoSyncButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoWaveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoSyncAtt;

    // Mod slots (4 slots)
    std::array<juce::ComboBox, 4> modSrcBoxes;
    std::array<juce::ComboBox, 4> modDstBoxes;
    std::array<juce::Slider, 4> modAmtSliders;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>, 4> modSrcAtts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>, 4> modDstAtts;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 4> modAmtAtts;
    juce::Label modMatrixLabel;

    // MIDI Keyboard
    juce::MidiKeyboardComponent midiKeyboard;
    juce::TextButton midiHoldButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> midiHoldAtt;

    void setupRotarySlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void setupSmallRotarySlider(juce::Slider& slider);
    void setupAutoRndComboBox(juce::ComboBox& box);
    void checkAutoRandomize();

    int lastSeqStep = -1;
    int stepCounter = 0;
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DFAMSynthAudioProcessorEditor)
};
