#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include <vector>

class OrbitAudioProcessor final : public juce::AudioProcessor
{
public:
    OrbitAudioProcessor();
    ~OrbitAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    float getOrbitPhase() const noexcept { return displayedPhase.load (std::memory_order_relaxed); }
    float getRearFactor() const noexcept { return displayedRear.load (std::memory_order_relaxed); }
    float getCurrentRateHz() const noexcept { return displayedRateHz.load (std::memory_order_relaxed); }
    float getEventGlow() const noexcept { return displayedEventGlow.load (std::memory_order_relaxed); }
    bool loadAudioFile (const juce::File&, juce::String& errorMessage);

    juce::AudioProcessorValueTreeState parameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static const juce::StringArray& divisionNames();

private:
    enum class TemporaryEvent
    {
        none,
        halfSpeed,
        doubleSpeed,
        speedRun
    };

    struct LoadedAudio
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
        juce::String fileName;
    };

    double currentSampleRate = 44100.0;
    double phase = 0.0;
    double oscillatorPhase = 0.0;
    double rhythmPhase = 0.0;
    double eventProgress = 0.0;
    double decisionProgress = 0.0;
    double swirlPhase = 1.0 / 6.0;
    int orbitDirection = 1;
    bool swirlWasEnabled = false;
    TemporaryEvent activeEvent = TemporaryEvent::none;
    float rhythmEnvelope = 1.0f;
    float eventGlow = 0.0f;
    float lowPassStateL = 0.0f;
    float lowPassStateR = 0.0f;
    float shadowStateL = 0.0f;
    float shadowStateR = 0.0f;
    float rearLowStateL = 0.0f;
    float rearLowStateR = 0.0f;
    float rearHighStateL = 0.0f;
    float rearHighStateR = 0.0f;
    float frontPresenceStateL = 0.0f;
    float frontPresenceStateR = 0.0f;
    float frontPresenceHighStateL = 0.0f;
    float frontPresenceHighStateR = 0.0f;
    float frontTopStateL = 0.0f;
    float frontTopStateR = 0.0f;
    float rearMuffleStateL = 0.0f;
    float rearMuffleStateR = 0.0f;
    float rearMidLowStateL = 0.0f;
    float rearMidLowStateR = 0.0f;
    float rearMidHighStateL = 0.0f;
    float rearMidHighStateR = 0.0f;
    float frontReflectionFilterStateL = 0.0f;
    float frontReflectionFilterStateR = 0.0f;
    float dopplerFilterStateL = 0.0f;
    float dopplerFilterStateR = 0.0f;
    float legacyRearStateL = 0.0f;
    float legacyRearStateR = 0.0f;
    float ghostFilterState = 0.0f;
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    std::vector<float> ghostDelayBuffer;
    std::vector<float> frontReflectionBuffer;
    std::vector<float> dopplerDelayBufferL;
    std::vector<float> dopplerDelayBufferR;
    int delayWritePosition = 0;
    int ghostWritePosition = 0;
    int frontReflectionWritePosition = 0;
    int dopplerWritePosition = 0;
    juce::Random noiseGenerator;
    juce::Random eventRandom;
    juce::AudioFormatManager formatManager;
    std::shared_ptr<const LoadedAudio> loadedAudio;
    std::atomic<double> loadedPlaybackPosition { 0.0 };

    std::atomic<float> displayedPhase { 0.0f };
    std::atomic<float> displayedRear { 0.0f };
    std::atomic<float> displayedRateHz { 0.25f };
    std::atomic<float> displayedEventGlow { 0.0f };

    double getRateHz() const;
    static float smoothstep (float value) noexcept;
    float readDelay (const std::vector<float>& buffer, int writePosition, float delaySamples) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrbitAudioProcessor)
};
