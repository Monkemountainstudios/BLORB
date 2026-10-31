#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class BlorbLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float,
                           float, float, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int,
                       juce::ComboBox&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool, bool) override;
};

class FineWheelSlider final : public juce::Slider
{
public:
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
};

class VintageLeverSwitch final : public juce::Component, private juce::Timer
{
public:
    VintageLeverSwitch (juce::AudioProcessorValueTreeState&, const juce::String& parameterId,
                         juce::StringArray positionLabels);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    juce::RangedAudioParameter* parameter = nullptr;
    juce::StringArray labels;
    int selectedPosition = 0;
};

class OrbitDisplay final : public juce::Component, private juce::Timer
{
public:
    explicit OrbitDisplay (OrbitAudioProcessor&);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    OrbitAudioProcessor& processor;
    float phase = 0.0f;
    float rear = 0.0f;
    float rateHz = 0.0f;
    float width = 1.0f;
    float depth = 0.5f;
    float eventGlow = 0.0f;
    float previousPhase = 0.0f;
    float motionDirection = 1.0f;
    bool phaseInitialised = false;
};

class OrbitAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit OrbitAudioProcessorEditor (OrbitAudioProcessor&);
    ~OrbitAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void updateAdvancedVisibility();
    static void configureKnob (juce::Slider&, const juce::String& suffix = {});
    static void configureLabel (juce::Label&, const juce::String&);

    OrbitAudioProcessor& processor;
    BlorbLookAndFeel lookAndFeel;
    OrbitDisplay orbitDisplay;
    bool advancedVisible = false;

    juce::TextButton advancedButton { "O" };
    juce::TextButton factoryResetButton { "FACTORY RESET" };

    juce::ComboBox sourceBox;
    juce::TextButton loadWavButton { "LOAD WAV..." };
    juce::ComboBox rhythmBox;
    juce::ComboBox modelBox;
    juce::ToggleButton itdButton { "INTERAURAL DELAY" };
    juce::ToggleButton frontLobesButton { "DUAL-LOBE FRONT" };
    juce::ToggleButton syncButton { "DAW SYNC" };
    juce::TextButton swirlButton { "SWIRL" };
    juce::ComboBox divisionBox;
    FineWheelSlider freeRateSlider;
    juce::Slider widthSlider;
    juce::Slider depthSlider;
    juce::Slider rearBlendSlider;
    juce::Slider spectralArcSlider;
    juce::Slider frontSpaceSlider;
    VintageLeverSwitch dopplerSwitch;
    juce::Slider chanceSlider;
    juce::Slider mixSlider;
    juce::Slider frequencySlider;
    VintageLeverSwitch eventGridSwitch;

    juce::Label sourceLabel;
    juce::Label loadedFileLabel;
    juce::Label rhythmLabel;
    juce::Label modelLabel;
    juce::Label speedLabel;
    juce::Label widthLabel;
    juce::Label depthLabel;
    juce::Label rearBlendLabel;
    juce::Label spectralArcLabel;
    juce::Label frontSpaceLabel;
    juce::Label dopplerLabel;
    juce::Label chanceLabel;
    juce::Label mixLabel;
    juce::Label frequencyLabel;
    juce::Label eventGridLabel;
    juce::Label eventOddsLabel;

    ComboAttachment sourceAttachment;
    ComboAttachment rhythmAttachment;
    ComboAttachment modelAttachment;
    ButtonAttachment itdAttachment;
    ButtonAttachment frontLobesAttachment;
    ButtonAttachment syncAttachment;
    ButtonAttachment swirlAttachment;
    ComboAttachment divisionAttachment;
    SliderAttachment freeRateAttachment;
    SliderAttachment widthAttachment;
    SliderAttachment depthAttachment;
    SliderAttachment rearBlendAttachment;
    SliderAttachment spectralArcAttachment;
    SliderAttachment frontSpaceAttachment;
    SliderAttachment chanceAttachment;
    SliderAttachment mixAttachment;
    SliderAttachment frequencyAttachment;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrbitAudioProcessorEditor)
};
