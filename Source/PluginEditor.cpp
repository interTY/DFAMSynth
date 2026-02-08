#include "PluginProcessor.h"
#include "PluginEditor.h"

DFAMSynthAudioProcessorEditor::DFAMSynthAudioProcessorEditor(DFAMSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      midiKeyboard(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    auto& apvts = audioProcessor.getAPVTS();

    // === PRESET Controls ===
    audioProcessor.refreshPresetList();
    updatePresetList();

    presetBox.onChange = [this]() {
        int idx = presetBox.getSelectedItemIndex();
        if (idx >= 0 && idx < audioProcessor.presetFiles.size())
            audioProcessor.loadPreset(audioProcessor.presetFiles[idx]);
    };
    addAndMakeVisible(presetBox);

    savePresetButton.setButtonText("SAVE");
    savePresetButton.onClick = [this]() {
        auto* alertWindow = new juce::AlertWindow(
            "Save Preset", "Enter preset name:", juce::MessageBoxIconType::QuestionIcon);
        alertWindow->addTextEditor("name", "", "Name:");
        alertWindow->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, alertWindow](int result) {
                if (result == 1)
                {
                    auto name = alertWindow->getTextEditorContents("name");
                    if (name.isNotEmpty())
                    {
                        audioProcessor.savePreset(name);
                        updatePresetList();
                        for (int i = 0; i < audioProcessor.presetNames.size(); ++i)
                        {
                            if (audioProcessor.presetNames[i] == name)
                            {
                                presetBox.setSelectedItemIndex(i, juce::dontSendNotification);
                                break;
                            }
                        }
                    }
                }
                delete alertWindow;
            }), false);
    };
    addAndMakeVisible(savePresetButton);

    deletePresetButton.setButtonText("DEL");
    deletePresetButton.onClick = [this]() {
        int idx = presetBox.getSelectedItemIndex();
        if (idx >= 0 && idx < audioProcessor.presetFiles.size())
        {
            auto file = audioProcessor.presetFiles[idx];
            auto* alertWindow = new juce::AlertWindow(
                "Delete Preset",
                "Delete \"" + file.getFileNameWithoutExtension() + "\"?",
                juce::MessageBoxIconType::WarningIcon);
            alertWindow->addButton("Delete", 1);
            alertWindow->addButton("Cancel", 0);

            alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
                [this, alertWindow, file](int result) {
                    if (result == 1)
                    {
                        file.deleteFile();
                        audioProcessor.refreshPresetList();
                        updatePresetList();
                    }
                    delete alertWindow;
                }), false);
        }
    };
    addAndMakeVisible(deletePresetButton);

    initPresetButton.setButtonText("INIT");
    initPresetButton.onClick = [this, &apvts]() {
        // Reset all parameters to default values
        for (auto* param : apvts.processor.getParameters())
        {
            if (auto* paramWithID = dynamic_cast<juce::RangedAudioParameter*>(param))
                paramWithID->setValueNotifyingHost(paramWithID->getDefaultValue());
        }
        presetBox.setSelectedItemIndex(-1, juce::dontSendNotification);
    };
    addAndMakeVisible(initPresetButton);

    // === ROW 1 Controls ===
    setupRotarySlider(vcoDecaySlider, vcoDecayLabel, "VCO DECAY");

    seqPitchModBox.addItem("VCO 1&2", 1);
    seqPitchModBox.addItem("OFF", 2);
    seqPitchModBox.addItem("VCO 2", 3);
    addAndMakeVisible(seqPitchModBox);
    seqPitchModLabel.setText("SEQ PITCH MOD", juce::dontSendNotification);
    seqPitchModLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(seqPitchModLabel);

    setupRotarySlider(vco1EgAmtSlider, vco1EgAmtLabel, "VCO1 EG AMT");
    setupRotarySlider(vco1FreqSlider, vco1FreqLabel, "VCO1 FREQ");
    setupRotarySlider(vco1WaveSlider, vco1WaveLabel, "VCO1 WAVE");
    setupRotarySlider(vco1LevelSlider, vco1LevelLabel, "VCO1 LEVEL");

    setupRotarySlider(subLevelSlider, subLevelLabel, "SUB LVL");

    setupRotarySlider(noiseLevelSlider, noiseLevelLabel, "NOISE LEVEL");
    setupRotarySlider(filterCutoffSlider, filterCutoffLabel, "CUTOFF");

    filterModeButton.setButtonText("HIGHPASS");
    addAndMakeVisible(filterModeButton);

    setupRotarySlider(filterResSlider, filterResLabel, "RESONANCE");

    vcaEgModeButton.setButtonText("EG SLOW");
    addAndMakeVisible(vcaEgModeButton);

    setupRotarySlider(vcaLevelSlider, vcaLevelLabel, "VOLUME");

    // === ROW 2 Controls ===
    setupRotarySlider(fmAmountSlider, fmAmountLabel, "1-2 FM AMT");

    hardSyncButton.setButtonText("HARDSYNC");
    addAndMakeVisible(hardSyncButton);

    setupRotarySlider(vco2EgAmtSlider, vco2EgAmtLabel, "VCO2 EG AMT");
    setupRotarySlider(vco2FreqSlider, vco2FreqLabel, "VCO2 FREQ");
    setupRotarySlider(vco2WaveSlider, vco2WaveLabel, "VCO2 WAVE");
    setupRotarySlider(vco2LevelSlider, vco2LevelLabel, "VCO2 LEVEL");
    setupRotarySlider(filterDecaySlider, filterDecayLabel, "VCF DECAY");
    setupRotarySlider(filterEnvAmtSlider, filterEnvAmtLabel, "VCF EG AMT");
    setupRotarySlider(noiseVcfModSlider, noiseVcfModLabel, "NOISE/VCF");
    setupRotarySlider(vcaDecaySlider, vcaDecayLabel, "VCA DECAY");

    // === DELAY Controls ===
    setupRotarySlider(delayTimeSlider, delayTimeLabel, "DLY TIME");
    setupRotarySlider(delayFeedbackSlider, delayFbLabel, "DLY FB");
    setupRotarySlider(delayFilterSlider, delayFilterLabel, "DLY FILT");
    setupRotarySlider(delayMixSlider, delayMixLabel, "DLY MIX");

    // === RING MODULATOR Controls ===
    setupRotarySlider(ringModFreqSlider, ringFreqLabel, "RING FRQ");
    setupRotarySlider(ringModMixSlider, ringMixLabel, "RING MIX");

    // === REVERB Controls ===
    setupRotarySlider(reverbDecaySlider, reverbDecayLabel, "REVERB");
    setupRotarySlider(reverbFilterSlider, reverbFilterLabel, "RVB FILT");
    setupRotarySlider(reverbMixSlider, reverbMixLabel, "RVB MIX");

    // === SEQUENCER Controls ===
    setupRotarySlider(tempoSlider, tempoLabel, "TEMPO");

    tempoMultBox.addItem("1/4x", 1);
    tempoMultBox.addItem("1/2x", 2);
    tempoMultBox.addItem("1x", 3);
    tempoMultBox.addItem("2x", 4);
    tempoMultBox.addItem("4x", 5);
    addAndMakeVisible(tempoMultBox);
    tempoMultLabel.setText("MULT", juce::dontSendNotification);
    tempoMultLabel.setJustificationType(juce::Justification::centred);
    tempoMultLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(tempoMultLabel);

    setupRotarySlider(swingSlider, swingLabel, "SWING");

    seqDirectionBox.addItem("FWD", 1);
    seqDirectionBox.addItem("BWD", 2);
    seqDirectionBox.addItem("PING", 3);
    addAndMakeVisible(seqDirectionBox);
    seqDirLabel.setText("DIRECTION", juce::dontSendNotification);
    seqDirLabel.setJustificationType(juce::Justification::centred);
    seqDirLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(seqDirLabel);

    // Scale quantization controls
    scaleTypeBox.addItem("OFF", 1);
    scaleTypeBox.addItem("Major", 2);
    scaleTypeBox.addItem("Minor", 3);
    scaleTypeBox.addItem("Harm Min", 4);
    scaleTypeBox.addItem("Pent Maj", 5);
    scaleTypeBox.addItem("Pent Min", 6);
    scaleTypeBox.addItem("Blues", 7);
    scaleTypeBox.addItem("Dorian", 8);
    scaleTypeBox.addItem("Phrygian", 9);
    scaleTypeBox.addItem("Lydian", 10);
    scaleTypeBox.addItem("Mixolyd", 11);
    scaleTypeBox.addItem("Locrian", 12);
    scaleTypeBox.addItem("WholeTn", 13);
    addAndMakeVisible(scaleTypeBox);
    scaleTypeLabel.setText("SCALE", juce::dontSendNotification);
    scaleTypeLabel.setJustificationType(juce::Justification::centred);
    scaleTypeLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(scaleTypeLabel);

    scaleRootBox.addItem("C", 1);
    scaleRootBox.addItem("C#", 2);
    scaleRootBox.addItem("D", 3);
    scaleRootBox.addItem("D#", 4);
    scaleRootBox.addItem("E", 5);
    scaleRootBox.addItem("F", 6);
    scaleRootBox.addItem("F#", 7);
    scaleRootBox.addItem("G", 8);
    scaleRootBox.addItem("G#", 9);
    scaleRootBox.addItem("A", 10);
    scaleRootBox.addItem("A#", 11);
    scaleRootBox.addItem("B", 12);
    addAndMakeVisible(scaleRootBox);
    scaleRootLabel.setText("ROOT", juce::dontSendNotification);
    scaleRootLabel.setJustificationType(juce::Justification::centred);
    scaleRootLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(scaleRootLabel);

    hostSyncButton.setButtonText("SYNC");
    hostSyncButton.setClickingTogglesState(true);
    hostSyncButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    addAndMakeVisible(hostSyncButton);

    seqRunButton.setButtonText("RUN");
    seqRunButton.setClickingTogglesState(true);
    seqRunButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    addAndMakeVisible(seqRunButton);

    stopButton.setButtonText("STOP");
    stopButton.onClick = [this]() {
        seqRunButton.setToggleState(false, juce::sendNotification);
    };
    addAndMakeVisible(stopButton);

    freezeButton.setButtonText("FREEZE");
    freezeButton.onClick = [this]() {
        autoRndPitchBox.setSelectedId(1, juce::sendNotification);
        autoRndVelBox.setSelectedId(1, juce::sendNotification);
        autoRndPanBox.setSelectedId(1, juce::sendNotification);
        autoRndWaveBox.setSelectedId(1, juce::sendNotification);
        autoRndRingBox.setSelectedId(1, juce::sendNotification);
        autoRndDelayPitchBox.setSelectedId(1, juce::sendNotification);
    };
    addAndMakeVisible(freezeButton);

    pitchLabel.setText("PITCH", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(pitchLabel);

    velocityLabel.setText("VEL", juce::dontSendNotification);
    velocityLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(velocityLabel);

    panLabel.setText("PAN", juce::dontSendNotification);
    panLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(panLabel);

    waveLabel.setText("WAVE", juce::dontSendNotification);
    waveLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(waveLabel);

    ringLabel.setText("RING", juce::dontSendNotification);
    ringLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(ringLabel);

    delayPitchLabel.setText("DLY", juce::dontSendNotification);
    delayPitchLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(delayPitchLabel);

    // Randomize buttons
    randomPitchButton.setButtonText("RND");
    randomPitchButton.onClick = [this]() {
        for (int i = 0; i < 8; ++i)
            seqPitchSliders[i].setValue(rng.nextFloat() * 48.0f - 24.0f, juce::sendNotification);
    };
    addAndMakeVisible(randomPitchButton);

    randomVelButton.setButtonText("RND");
    randomVelButton.onClick = [this]() {
        for (int i = 0; i < 8; ++i)
            seqVelSliders[i].setValue(rng.nextFloat(), juce::sendNotification);
    };
    addAndMakeVisible(randomVelButton);

    randomPanButton.setButtonText("RND");
    randomPanButton.onClick = [this]() {
        for (int i = 0; i < 8; ++i)
            seqPanSliders[i].setValue(rng.nextFloat() * 2.0f - 1.0f, juce::sendNotification);
    };
    addAndMakeVisible(randomPanButton);

    randomWaveButton.setButtonText("RND");
    randomWaveButton.onClick = [this]() {
        for (int i = 0; i < 8; ++i)
            seqWaveSliders[i].setValue(rng.nextFloat(), juce::sendNotification);
    };
    addAndMakeVisible(randomWaveButton);

    randomRingButton.setButtonText("RND");
    randomRingButton.onClick = [this]() {
        for (int i = 0; i < 8; ++i)
            seqRingModSliders[i].setValue(rng.nextFloat(), juce::sendNotification);
    };
    addAndMakeVisible(randomRingButton);

    randomDelayPitchButton.setButtonText("RND");
    randomDelayPitchButton.onClick = [this]() {
        for (int i = 0; i < 8; ++i)
            seqDelayPitchSliders[i].setValue(rng.nextFloat() * 48.0f - 24.0f, juce::sendNotification);
    };
    addAndMakeVisible(randomDelayPitchButton);

    // Auto-randomize dropdowns
    setupAutoRndComboBox(autoRndPitchBox);
    setupAutoRndComboBox(autoRndVelBox);
    setupAutoRndComboBox(autoRndPanBox);
    setupAutoRndComboBox(autoRndWaveBox);
    setupAutoRndComboBox(autoRndRingBox);
    setupAutoRndComboBox(autoRndDelayPitchBox);

    // Auto-randomize dropdowns - reset knobs to defaults when set to "off"
    autoRndPitchBox.onChange = [this]() {
        if (autoRndPitchBox.getSelectedId() == 1)
            for (int i = 0; i < 8; ++i) seqPitchSliders[i].setValue(0.0, juce::sendNotification);
    };
    autoRndVelBox.onChange = [this]() {
        if (autoRndVelBox.getSelectedId() == 1)
            for (int i = 0; i < 8; ++i) seqVelSliders[i].setValue(0.8, juce::sendNotification);
    };
    autoRndPanBox.onChange = [this]() {
        if (autoRndPanBox.getSelectedId() == 1)
            for (int i = 0; i < 8; ++i) seqPanSliders[i].setValue(0.0, juce::sendNotification);
    };
    autoRndWaveBox.onChange = [this]() {
        if (autoRndWaveBox.getSelectedId() == 1)
            for (int i = 0; i < 8; ++i) seqWaveSliders[i].setValue(0.33, juce::sendNotification);
    };
    autoRndRingBox.onChange = [this]() {
        if (autoRndRingBox.getSelectedId() == 1)
            for (int i = 0; i < 8; ++i) seqRingModSliders[i].setValue(0.5, juce::sendNotification);
    };
    autoRndDelayPitchBox.onChange = [this]() {
        if (autoRndDelayPitchBox.getSelectedId() == 1)
            for (int i = 0; i < 8; ++i) seqDelayPitchSliders[i].setValue(0.0, juce::sendNotification);
    };

    for (int i = 0; i < 8; ++i)
    {
        setupSmallRotarySlider(seqPitchSliders[i]);
        setupSmallRotarySlider(seqVelSliders[i]);
        setupSmallRotarySlider(seqPanSliders[i]);
        setupSmallRotarySlider(seqWaveSliders[i]);
        setupSmallRotarySlider(seqRingModSliders[i]);
        setupSmallRotarySlider(seqDelayPitchSliders[i]);

        stepIndicators[i].setText(juce::String(i + 1), juce::dontSendNotification);
        stepIndicators[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(stepIndicators[i]);
    }

    // === Create Attachments ===
    vcoDecayAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vcoDecay", vcoDecaySlider);
    seqPitchModAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "seqPitchMod", seqPitchModBox);
    vco1EgAmtAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco1EgAmt", vco1EgAmtSlider);
    vco1FreqAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco1Freq", vco1FreqSlider);
    vco1WaveAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco1Wave", vco1WaveSlider);
    vco1LevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco1Level", vco1LevelSlider);
    subLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "subLevel", subLevelSlider);
    noiseLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "noiseLevel", noiseLevelSlider);
    filterCutoffAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "filterCutoff", filterCutoffSlider);
    filterModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "filterMode", filterModeButton);
    filterResAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "filterRes", filterResSlider);
    vcaEgModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "vcaEgMode", vcaEgModeButton);
    vcaLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vcaLevel", vcaLevelSlider);

    fmAmountAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "fmAmount", fmAmountSlider);
    hardSyncAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "hardSync", hardSyncButton);
    vco2EgAmtAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco2EgAmt", vco2EgAmtSlider);
    vco2FreqAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco2Freq", vco2FreqSlider);
    vco2WaveAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco2Wave", vco2WaveSlider);
    vco2LevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vco2Level", vco2LevelSlider);
    filterDecayAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "filterDecay", filterDecaySlider);
    filterEnvAmtAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "filterEnvAmt", filterEnvAmtSlider);
    noiseVcfModAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "noiseVcfMod", noiseVcfModSlider);
    vcaDecayAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vcaDecay", vcaDecaySlider);

    tempoAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "tempo", tempoSlider);
    tempoMultAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "tempoMult", tempoMultBox);
    seqRunAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "seqRun", seqRunButton);

    // Delay attachments
    delayTimeAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "delayTime", delayTimeSlider);
    delayFeedbackAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "delayFeedback", delayFeedbackSlider);
    delayFilterAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "delayFilter", delayFilterSlider);
    delayMixAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "delayMix", delayMixSlider);

    // Reverb attachments
    reverbDecayAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "reverbDecay", reverbDecaySlider);
    reverbFilterAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "reverbFilter", reverbFilterSlider);
    reverbMixAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "reverbMix", reverbMixSlider);

    // Ring modulator attachments
    ringModFreqAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "ringModFreq", ringModFreqSlider);
    ringModMixAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "ringModMix", ringModMixSlider);

    // Sequencer swing/direction attachments
    swingAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "swing", swingSlider);
    seqDirectionAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "seqDirection", seqDirectionBox);
    hostSyncAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "hostSync", hostSyncButton);

    // Glide slider
    glideSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    glideSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(glideSlider);
    glideLabel.setText("GLIDE", juce::dontSendNotification);
    glideLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(glideLabel);
    glideAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "glide", glideSlider);

    // Drone button
    droneButton.setButtonText("DRONE");
    droneButton.setClickingTogglesState(true);
    droneButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::purple);
    addAndMakeVisible(droneButton);
    droneAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "drone", droneButton);

    // Scale quantization attachments
    scaleTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "scaleType", scaleTypeBox);
    scaleRootAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "scaleRoot", scaleRootBox);

    // === MOD MATRIX Setup ===
    // LFO controls
    setupRotarySlider(lfoRateSlider, lfoRateLabel, "LFO RATE");

    lfoWaveBox.addItem("Sine", 1);
    lfoWaveBox.addItem("Triangle", 2);
    lfoWaveBox.addItem("Square", 3);
    lfoWaveBox.addItem("Saw Up", 4);
    lfoWaveBox.addItem("Saw Dn", 5);
    lfoWaveBox.addItem("S&H", 6);
    addAndMakeVisible(lfoWaveBox);
    lfoWaveLabel.setText("LFO WAVE", juce::dontSendNotification);
    lfoWaveLabel.setJustificationType(juce::Justification::centred);
    lfoWaveLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(lfoWaveLabel);

    lfoSyncButton.setButtonText("SYNC");
    lfoSyncButton.setClickingTogglesState(true);
    addAndMakeVisible(lfoSyncButton);

    // Mod slots
    modMatrixLabel.setText("MOD MATRIX", juce::dontSendNotification);
    modMatrixLabel.setJustificationType(juce::Justification::centred);
    modMatrixLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(modMatrixLabel);

    for (int i = 0; i < 4; ++i)
    {
        // Source dropdown
        modSrcBoxes[i].addItem("OFF", 1);
        modSrcBoxes[i].addItem("LFO", 2);
        modSrcBoxes[i].addItem("PitchEnv", 3);
        modSrcBoxes[i].addItem("FiltEnv", 4);
        modSrcBoxes[i].addItem("VCAEnv", 5);
        modSrcBoxes[i].addItem("Velocity", 6);
        modSrcBoxes[i].addItem("Random", 7);
        addAndMakeVisible(modSrcBoxes[i]);

        // Destination dropdown
        modDstBoxes[i].addItem("OFF", 1);
        modDstBoxes[i].addItem("Flt Cut", 2);
        modDstBoxes[i].addItem("Flt Res", 3);
        modDstBoxes[i].addItem("VCO1Pit", 4);
        modDstBoxes[i].addItem("VCO2Pit", 5);
        modDstBoxes[i].addItem("RingFrq", 6);
        modDstBoxes[i].addItem("Pan", 7);
        modDstBoxes[i].addItem("VCO1Lvl", 8);
        modDstBoxes[i].addItem("VCO2Lvl", 9);
        modDstBoxes[i].addItem("VCADecay", 10);
        modDstBoxes[i].addItem("NoiseVCF", 11);
        modDstBoxes[i].addItem("VCFDecay", 12);
        modDstBoxes[i].addItem("FM Amt", 13);
        addAndMakeVisible(modDstBoxes[i]);

        // Amount slider
        modAmtSliders[i].setSliderStyle(juce::Slider::LinearHorizontal);
        modAmtSliders[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(modAmtSliders[i]);
    }

    // LFO attachments
    lfoRateAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "lfoRate", lfoRateSlider);
    lfoWaveAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "lfoWave", lfoWaveBox);
    lfoSyncAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "lfoSync", lfoSyncButton);

    // Mod slot attachments
    for (int i = 0; i < 4; ++i)
    {
        juce::String num = juce::String(i + 1);
        modSrcAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "modSrc" + num, modSrcBoxes[i]);
        modDstAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "modDst" + num, modDstBoxes[i]);
        modAmtAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "modAmt" + num, modAmtSliders[i]);
    }

    // MIDI Keyboard
    midiKeyboard.setOctaveForMiddleC(4);
    addAndMakeVisible(midiKeyboard);

    // MIDI Hold button
    midiHoldButton.setButtonText("HOLD");
    midiHoldButton.setClickingTogglesState(true);
    midiHoldButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    addAndMakeVisible(midiHoldButton);
    midiHoldAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "midiHold", midiHoldButton);

    for (int i = 0; i < 8; ++i)
    {
        seqPitchAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "seqPitch" + juce::String(i + 1), seqPitchSliders[i]);
        seqVelAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "seqVel" + juce::String(i + 1), seqVelSliders[i]);
        seqPanAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "seqPan" + juce::String(i + 1), seqPanSliders[i]);
        seqWaveAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "seqWave_" + juce::String(i + 1), seqWaveSliders[i]);
        seqRingModAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "seqRingMod_" + juce::String(i + 1), seqRingModSliders[i]);
        seqDelayPitchAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "seqDelayPitch_" + juce::String(i + 1), seqDelayPitchSliders[i]);
    }

    startTimerHz(30);
    setSize(1150, 920);
}

DFAMSynthAudioProcessorEditor::~DFAMSynthAudioProcessorEditor()
{
    stopTimer();
}

void DFAMSynthAudioProcessorEditor::setupRotarySlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(11.0f));
    addAndMakeVisible(label);
}

void DFAMSynthAudioProcessorEditor::setupSmallRotarySlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);
}

void DFAMSynthAudioProcessorEditor::setupAutoRndComboBox(juce::ComboBox& box)
{
    box.addItem("off", 1);
    box.addItem("1", 2);
    box.addItem("4", 3);
    box.addItem("8", 4);
    box.addItem("rnd", 5);
    box.setSelectedId(1);
    addAndMakeVisible(box);
}

void DFAMSynthAudioProcessorEditor::updatePresetList()
{
    presetBox.clear(juce::dontSendNotification);

    auto& names = audioProcessor.presetNames;
    if (names.isEmpty())
    {
        presetBox.addItem("(no presets)", 1);
        presetBox.setSelectedItemIndex(0, juce::dontSendNotification);
    }
    else
    {
        for (int i = 0; i < names.size(); ++i)
            presetBox.addItem(names[i], i + 1);
    }
}

void DFAMSynthAudioProcessorEditor::checkAutoRandomize()
{
    int currentStep = audioProcessor.getCurrentSequencerStep();
    bool running = audioProcessor.isSequencerRunning();

    if (!running)
    {
        lastSeqStep = -1;
        stepCounter = 0;
        return;
    }

    // Detect step change
    if (currentStep != lastSeqStep)
    {
        lastSeqStep = currentStep;
        stepCounter++;

        auto shouldRandomize = [&](juce::ComboBox& box) -> bool {
            int sel = box.getSelectedId();
            if (sel == 1) return false;        // off
            if (sel == 2) return (stepCounter % 8) == 0;   // every 1 cycle (8 steps)
            if (sel == 3) return (stepCounter % 32) == 0;  // every 4 cycles (32 steps)
            if (sel == 4) return (stepCounter % 64) == 0;  // every 8 cycles (64 steps)
            if (sel == 5) return rng.nextFloat() < 0.25f;  // random chance
            return false;
        };

        if (shouldRandomize(autoRndPitchBox))
        {
            for (int i = 0; i < 8; ++i)
                seqPitchSliders[i].setValue(rng.nextFloat() * 48.0f - 24.0f, juce::sendNotification);
        }

        if (shouldRandomize(autoRndVelBox))
        {
            for (int i = 0; i < 8; ++i)
                seqVelSliders[i].setValue(rng.nextFloat(), juce::sendNotification);
        }

        if (shouldRandomize(autoRndPanBox))
        {
            for (int i = 0; i < 8; ++i)
                seqPanSliders[i].setValue(rng.nextFloat() * 2.0f - 1.0f, juce::sendNotification);
        }

        if (shouldRandomize(autoRndWaveBox))
        {
            for (int i = 0; i < 8; ++i)
                seqWaveSliders[i].setValue(rng.nextFloat(), juce::sendNotification);
        }

        if (shouldRandomize(autoRndRingBox))
        {
            for (int i = 0; i < 8; ++i)
                seqRingModSliders[i].setValue(rng.nextFloat(), juce::sendNotification);
        }

        if (shouldRandomize(autoRndDelayPitchBox))
        {
            for (int i = 0; i < 8; ++i)
                seqDelayPitchSliders[i].setValue(rng.nextFloat() * 48.0f - 24.0f, juce::sendNotification);
        }
    }
}

void DFAMSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(25, 25, 30));

    // Title bar background
    g.setColour(juce::Colour(35, 35, 40));
    g.fillRect(0, 0, getWidth(), 45);

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText("Drummer from the better Father", 0, 8, getWidth(), 30, juce::Justification::centred);

    // Row backgrounds
    g.setColour(juce::Colour(40, 40, 45));
    g.fillRoundedRectangle(8.0f, 50.0f, (float)getWidth() - 16, 115.0f, 6);      // Row 1: OSC1
    g.fillRoundedRectangle(8.0f, 170.0f, (float)getWidth() - 16, 115.0f, 6);     // Row 2: OSC2
    g.fillRoundedRectangle(8.0f, 290.0f, (float)getWidth() - 16, 115.0f, 6);     // Row 3: FX + LFO
    g.fillRoundedRectangle(8.0f, 410.0f, (float)getWidth() - 16, 45.0f, 6);      // Row 4: Transport
    g.fillRoundedRectangle(8.0f, 460.0f, (float)getWidth() - 16, 265.0f, 6);     // Row 5: Sequencer
    g.fillRoundedRectangle(8.0f, 730.0f, (float)getWidth() - 16, 110.0f, 6);     // Row 6: Mod Matrix (doubled height)
    g.fillRoundedRectangle(8.0f, 845.0f, (float)getWidth() - 16, 65.0f, 6);      // Row 7: MIDI Keyboard

    // Section dividers in FX row
    g.setColour(juce::Colour(55, 55, 60));
    g.fillRect(400, 300, 2, 95);   // After Delay
    g.fillRect(600, 300, 2, 95);   // After Ring Mod

    // Section labels in FX row
    g.setColour(juce::Colour(140, 140, 150));
    g.setFont(11.0f);
    g.drawText("DELAY", 20, 293, 60, 14, juce::Justification::centredLeft);
    g.drawText("RING MOD", 415, 293, 80, 14, juce::Justification::centredLeft);
    g.drawText("REVERB", 615, 293, 70, 14, juce::Justification::centredLeft);

    // Horizontal lines between sequencer lanes
    g.setColour(juce::Colour(50, 50, 55));
    const int seqY = 460;
    const int seqRowH = 42;
    const int stepW = 100;
    const int stepStartX = 85 + 55 + 20;  // seqCtrlW + seqLabelW + 20
    const int lineStartX = stepStartX;
    const int lineEndX = stepStartX + 8 * stepW - 10;

    // Horizontal lines between lanes
    for (int i = 1; i < 6; ++i)
    {
        int y = seqY + 22 + seqRowH * i - 2;
        g.drawLine((float)lineStartX, (float)y, (float)lineEndX, (float)y, 1.0f);
    }

    // Vertical lines between steps
    for (int i = 1; i < 8; ++i)
    {
        int x = stepStartX + i * stepW - 5;  // Between steps
        g.drawLine((float)x, (float)(seqY + 22), (float)x, (float)(seqY + 22 + seqRowH * 6 - 4), 1.0f);
    }
}

void DFAMSynthAudioProcessorEditor::resized()
{
    const int knobW = 85;
    const int knobH = 75;
    const int labelH = 15;
    const int margin = 15;
    const int colW = knobW + 12;

    int row1Y = 55;     // OSC1
    int row2Y = 175;    // OSC2
    int row3Y = 295;    // FX + LFO
    int transY = 415;   // Transport row
    int seqY = 465;     // Sequencer
    int modY = 735;     // Mod Matrix

    // === PRESET Controls (top right in title bar) ===
    presetBox.setBounds(getWidth() - 320, 10, 160, 26);
    initPresetButton.setBounds(getWidth() - 155, 10, 45, 26);
    savePresetButton.setBounds(getWidth() - 105, 10, 45, 26);
    deletePresetButton.setBounds(getWidth() - 55, 10, 45, 26);

    // === ROW 1: OSC1 + Filter + Noise ===
    int x = margin;

    vcoDecayLabel.setBounds(x, row1Y, knobW, labelH);
    vcoDecaySlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    seqPitchModLabel.setBounds(x, row1Y, knobW, labelH);
    seqPitchModBox.setBounds(x + 8, row1Y + labelH + 25, knobW - 16, 26);
    x += colW;

    vco1EgAmtLabel.setBounds(x, row1Y, knobW, labelH);
    vco1EgAmtSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    vco1FreqLabel.setBounds(x, row1Y, knobW, labelH);
    vco1FreqSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    vco1WaveLabel.setBounds(x, row1Y, knobW, labelH);
    vco1WaveSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    vco1LevelLabel.setBounds(x, row1Y, knobW, labelH);
    vco1LevelSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    subLevelLabel.setBounds(x, row1Y, knobW, labelH);
    subLevelSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    noiseLevelLabel.setBounds(x, row1Y, knobW, labelH);
    noiseLevelSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    filterCutoffLabel.setBounds(x, row1Y, knobW, labelH);
    filterCutoffSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    filterModeButton.setBounds(x + 8, row1Y + labelH + knobH + 2, knobW - 16, 20);
    x += colW;

    filterResLabel.setBounds(x, row1Y, knobW, labelH);
    filterResSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    x += colW;

    vcaLevelLabel.setBounds(x, row1Y, knobW, labelH);
    vcaLevelSlider.setBounds(x, row1Y + labelH, knobW, knobH);
    vcaEgModeButton.setBounds(x + 8, row1Y + labelH + knobH + 2, knobW - 16, 20);

    // === ROW 2: OSC2 + VCF + Decay ===
    x = margin;

    fmAmountLabel.setBounds(x, row2Y, knobW, labelH);
    fmAmountSlider.setBounds(x, row2Y + labelH, knobW, knobH);
    hardSyncButton.setBounds(x + 8, row2Y + labelH + knobH + 2, knobW - 16, 20);
    x += colW;

    vco2EgAmtLabel.setBounds(x, row2Y, knobW, labelH);
    vco2EgAmtSlider.setBounds(x, row2Y + labelH, knobW, knobH);
    x += colW;

    vco2FreqLabel.setBounds(x, row2Y, knobW, labelH);
    vco2FreqSlider.setBounds(x, row2Y + labelH, knobW, knobH);
    x += colW;

    vco2WaveLabel.setBounds(x, row2Y, knobW, labelH);
    vco2WaveSlider.setBounds(x, row2Y + labelH, knobW, knobH);
    x += colW;

    vco2LevelLabel.setBounds(x, row2Y, knobW, labelH);
    vco2LevelSlider.setBounds(x, row2Y + labelH, knobW, knobH);
    x += colW;

    x += colW; // gap

    filterDecayLabel.setBounds(x, row2Y, knobW, labelH);
    filterDecaySlider.setBounds(x, row2Y + labelH, knobW, knobH);
    x += colW;

    filterEnvAmtLabel.setBounds(x, row2Y, knobW, labelH);
    filterEnvAmtSlider.setBounds(x, row2Y + labelH, knobW, knobH);
    x += colW;

    noiseVcfModLabel.setBounds(x, row2Y, knobW, labelH);
    noiseVcfModSlider.setBounds(x, row2Y + labelH, knobW, knobH);
    x += colW;

    vcaDecayLabel.setBounds(x, row2Y, knobW, labelH);
    vcaDecaySlider.setBounds(x, row2Y + labelH, knobW, knobH);

    // === ROW 3: FX (Delay | Ring Mod | Reverb | LFO) ===
    x = margin;

    // DELAY
    delayTimeLabel.setBounds(x, row3Y + 12, knobW, labelH);
    delayTimeSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW;

    delayFbLabel.setBounds(x, row3Y + 12, knobW, labelH);
    delayFeedbackSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW;

    delayFilterLabel.setBounds(x, row3Y + 12, knobW, labelH);
    delayFilterSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW;

    delayMixLabel.setBounds(x, row3Y + 12, knobW, labelH);
    delayMixSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW + 15;

    // RING MODULATOR
    ringFreqLabel.setBounds(x, row3Y + 12, knobW, labelH);
    ringModFreqSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW;

    ringMixLabel.setBounds(x, row3Y + 12, knobW, labelH);
    ringModMixSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW + 15;

    // REVERB
    reverbDecayLabel.setBounds(x, row3Y + 12, knobW, labelH);
    reverbDecaySlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW;

    reverbFilterLabel.setBounds(x, row3Y + 12, knobW, labelH);
    reverbFilterSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW;

    reverbMixLabel.setBounds(x, row3Y + 12, knobW, labelH);
    reverbMixSlider.setBounds(x, row3Y + 12 + labelH, knobW, knobH);
    x += colW + 15;

    // LFO controls moved to Mod Matrix section below

    // === TRANSPORT ROW ===
    x = margin;
    seqRunButton.setBounds(x, transY + 8, 70, 28);
    x += 80;
    hostSyncButton.setBounds(x, transY + 8, 70, 28);
    x += 80;
    stopButton.setBounds(x, transY + 8, 70, 28);
    x += 80;
    freezeButton.setBounds(x, transY + 8, 70, 28);
    x += 100;

    // Scale controls in transport row (aligned with buttons)
    scaleTypeLabel.setBounds(x, transY + 12, 50, 20);
    scaleTypeBox.setBounds(x + 55, transY + 8, 100, 28);
    x += 170;

    scaleRootLabel.setBounds(x, transY + 12, 45, 20);
    scaleRootBox.setBounds(x + 50, transY + 8, 70, 28);
    x += 140;

    // Glide slider and Drone button
    glideLabel.setBounds(x, transY + 12, 50, 20);
    glideSlider.setBounds(x + 55, transY + 10, 100, 24);
    droneButton.setBounds(x + 160, transY + 8, 70, 28);

    // === SEQUENCER Layout ===
    const int seqRowH = 42;
    const int seqLabelW = 55;
    const int seqCtrlW = 85;
    const int stepW = 100;
    const int stepStartX = seqCtrlW + seqLabelW + 20;
    const int rndBtnW = 42;
    const int autoRndW = 55;

    // Left controls column (Tempo, Tempo Mult, Swing, Direction)
    int ctrlX = margin;
    int ctrlY = seqY + 5;

    tempoLabel.setBounds(ctrlX, ctrlY, seqCtrlW, labelH);
    tempoSlider.setBounds(ctrlX, ctrlY + labelH, seqCtrlW, 50);
    ctrlY += 68;

    tempoMultLabel.setBounds(ctrlX, ctrlY, seqCtrlW, labelH);
    tempoMultBox.setBounds(ctrlX, ctrlY + labelH, seqCtrlW, 24);
    ctrlY += 45;

    swingLabel.setBounds(ctrlX, ctrlY, seqCtrlW, labelH);
    swingSlider.setBounds(ctrlX, ctrlY + labelH, seqCtrlW, 50);
    ctrlY += 68;

    seqDirLabel.setBounds(ctrlX, ctrlY, seqCtrlW, labelH);
    seqDirectionBox.setBounds(ctrlX, ctrlY + labelH, seqCtrlW, 24);

    // Row labels
    int rowY = seqY + 20;
    pitchLabel.setBounds(seqCtrlW + 15, rowY, seqLabelW, 20);
    rowY += seqRowH;
    velocityLabel.setBounds(seqCtrlW + 15, rowY, seqLabelW, 20);
    rowY += seqRowH;
    panLabel.setBounds(seqCtrlW + 15, rowY, seqLabelW, 20);
    rowY += seqRowH;
    waveLabel.setBounds(seqCtrlW + 15, rowY, seqLabelW, 20);
    rowY += seqRowH;
    ringLabel.setBounds(seqCtrlW + 15, rowY, seqLabelW, 20);
    rowY += seqRowH;
    delayPitchLabel.setBounds(seqCtrlW + 15, rowY, seqLabelW, 20);

    // Step indicators and sliders
    for (int i = 0; i < 8; ++i)
    {
        int sx = stepStartX + i * stepW;
        stepIndicators[i].setBounds(sx, seqY + 2, stepW - 10, 20);
        seqPitchSliders[i].setBounds(sx, seqY + 22, stepW - 10, seqRowH - 4);
        seqVelSliders[i].setBounds(sx, seqY + 22 + seqRowH, stepW - 10, seqRowH - 4);
        seqPanSliders[i].setBounds(sx, seqY + 22 + seqRowH * 2, stepW - 10, seqRowH - 4);
        seqWaveSliders[i].setBounds(sx, seqY + 22 + seqRowH * 3, stepW - 10, seqRowH - 4);
        seqRingModSliders[i].setBounds(sx, seqY + 22 + seqRowH * 4, stepW - 10, seqRowH - 4);
        seqDelayPitchSliders[i].setBounds(sx, seqY + 22 + seqRowH * 5, stepW - 10, seqRowH - 4);
    }

    // RND buttons and auto-randomize dropdowns (moved up to fit)
    int rndX = stepStartX + 8 * stepW + 8;
    rowY = seqY + 22;
    randomPitchButton.setBounds(rndX, rowY + 4, rndBtnW, 22);
    autoRndPitchBox.setBounds(rndX + rndBtnW + 5, rowY + 4, autoRndW, 22);
    rowY += seqRowH;
    randomVelButton.setBounds(rndX, rowY + 4, rndBtnW, 22);
    autoRndVelBox.setBounds(rndX + rndBtnW + 5, rowY + 4, autoRndW, 22);
    rowY += seqRowH;
    randomPanButton.setBounds(rndX, rowY + 4, rndBtnW, 22);
    autoRndPanBox.setBounds(rndX + rndBtnW + 5, rowY + 4, autoRndW, 22);
    rowY += seqRowH;
    randomWaveButton.setBounds(rndX, rowY + 4, rndBtnW, 22);
    autoRndWaveBox.setBounds(rndX + rndBtnW + 5, rowY + 4, autoRndW, 22);
    rowY += seqRowH;
    randomRingButton.setBounds(rndX, rowY + 4, rndBtnW, 22);
    autoRndRingBox.setBounds(rndX + rndBtnW + 5, rowY + 4, autoRndW, 22);
    rowY += seqRowH;
    randomDelayPitchButton.setBounds(rndX, rowY + 4, rndBtnW, 22);
    autoRndDelayPitchBox.setBounds(rndX + rndBtnW + 5, rowY + 4, autoRndW, 22);

    // === MOD MATRIX Layout (2 rows of 2 slots) ===
    modMatrixLabel.setBounds(margin, modY + 5, 100, 20);

    // LFO controls on the left
    lfoRateLabel.setBounds(margin, modY + 28, 60, 14);
    lfoRateSlider.setBounds(margin, modY + 42, 70, 60);
    lfoWaveLabel.setBounds(margin + 75, modY + 28, 60, 14);
    lfoWaveBox.setBounds(margin + 75, modY + 45, 75, 24);
    lfoSyncButton.setBounds(margin + 75, modY + 73, 75, 22);

    const int srcDstW = 85;
    const int amtW = 100;
    const int slotStartX = margin + 170;
    const int slotSpacing = srcDstW * 2 + amtW + 30;
    const int modRow1Y = modY + 8;
    const int modRow2Y = modY + 58;

    // Row 1: Slots 1 and 2
    for (int i = 0; i < 2; ++i)
    {
        int slotX = slotStartX + i * slotSpacing;
        modSrcBoxes[i].setBounds(slotX, modRow1Y, srcDstW, 24);
        modDstBoxes[i].setBounds(slotX + srcDstW + 5, modRow1Y, srcDstW, 24);
        modAmtSliders[i].setBounds(slotX + srcDstW * 2 + 10, modRow1Y, amtW, 24);
    }

    // Row 2: Slots 3 and 4
    for (int i = 2; i < 4; ++i)
    {
        int slotX = slotStartX + (i - 2) * slotSpacing;
        modSrcBoxes[i].setBounds(slotX, modRow2Y, srcDstW, 24);
        modDstBoxes[i].setBounds(slotX + srcDstW + 5, modRow2Y, srcDstW, 24);
        modAmtSliders[i].setBounds(slotX + srcDstW * 2 + 10, modRow2Y, amtW, 24);
    }

    // === MIDI Keyboard ===
    int midiY = modY + 115;
    midiHoldButton.setBounds(margin, midiY, 55, 50);
    midiKeyboard.setBounds(margin + 65, midiY, getWidth() - margin * 2 - 65, 55);
}

void DFAMSynthAudioProcessorEditor::timerCallback()
{
    // Check for auto-randomization
    checkAutoRandomize();

    int currentStep = audioProcessor.getCurrentSequencerStep();
    bool running = audioProcessor.isSequencerRunning();

    for (int i = 0; i < 8; ++i)
    {
        if (running && i == currentStep)
        {
            stepIndicators[i].setColour(juce::Label::backgroundColourId, juce::Colours::red);
            stepIndicators[i].setColour(juce::Label::textColourId, juce::Colours::white);
        }
        else
        {
            stepIndicators[i].setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
            stepIndicators[i].setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        }
    }
}
