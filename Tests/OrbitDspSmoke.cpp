#include "PluginProcessor.h"
#include <cmath>
#include <iostream>

namespace
{
void setParameter (OrbitAudioProcessor& processor, const char* id, float plainValue)
{
    auto* parameter = processor.parameters.getParameter (id);
    jassert (parameter != nullptr);
    parameter->setValueNotifyingHost (parameter->convertTo0to1 (plainValue));
}

bool bufferIsFinite (const juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (! std::isfinite (buffer.getSample (channel, sample)))
                return false;
    return true;
}
}

int main()
{
    juce::ScopedJuceInitialiser_GUI initialiseJuce;
    OrbitAudioProcessor processor;
    processor.prepareToPlay (48000.0, 256);

    setParameter (processor, "source", 1.0f); // Sine
    setParameter (processor, "sync", 0.0f);
    setParameter (processor, "freeRate", 2.0f);
    setParameter (processor, "width", 1.0f);
    setParameter (processor, "depth", 0.75f);
    setParameter (processor, "rearBlend", 0.75f);
    setParameter (processor, "spectralArc", 110.0f);
    setParameter (processor, "frontSpace", 0.50f);
    setParameter (processor, "doppler", 2.0f);
    setParameter (processor, "mix", 1.0f);
    setParameter (processor, "model", 1.0f);
    setParameter (processor, "itd", 1.0f);
    setParameter (processor, "frontLobes", 1.0f);

    juce::AudioBuffer<float> audio (2, 256);
    juce::MidiBuffer midi;
    float minimumBalance = 1.0f;
    float maximumBalance = -1.0f;

    for (int block = 0; block < 120; ++block)
    {
        audio.clear();
        processor.processBlock (audio, midi);
        if (! bufferIsFinite (audio))
        {
            std::cerr << "Non-finite output during sine orbit\n";
            return 1;
        }

        const auto left = audio.getRMSLevel (0, 0, audio.getNumSamples());
        const auto right = audio.getRMSLevel (1, 0, audio.getNumSamples());
        const auto balance = (right - left) / juce::jmax (0.000001f, right + left);
        minimumBalance = juce::jmin (minimumBalance, balance);
        maximumBalance = juce::jmax (maximumBalance, balance);
    }

    if (minimumBalance > -0.45f || maximumBalance < 0.45f)
    {
        std::cerr << "Orbit did not traverse both stereo sides: "
                  << minimumBalance << " to " << maximumBalance << "\n";
        return 2;
    }

    setParameter (processor, "swirl", 1.0f);
    float minimumSwirlRate = 1000.0f;
    float maximumSwirlRate = 0.0f;
    for (int block = 0; block < 1200; ++block)
    {
        audio.clear();
        processor.processBlock (audio, midi);
        minimumSwirlRate = juce::jmin (minimumSwirlRate, processor.getCurrentRateHz());
        maximumSwirlRate = juce::jmax (maximumSwirlRate, processor.getCurrentRateHz());
        if (! bufferIsFinite (audio))
        {
            std::cerr << "Non-finite output during swirl\n";
            return 3;
        }
    }

    if (minimumSwirlRate > 1.10f || maximumSwirlRate < 3.80f)
    {
        std::cerr << "Swirl did not reach its 0.5x/2x range: "
                  << minimumSwirlRate << " to " << maximumSwirlRate << " Hz\n";
        return 4;
    }
    setParameter (processor, "swirl", 0.0f);

    setParameter (processor, "source", 3.0f); // White noise
    setParameter (processor, "rhythm", 2.0f); // Sixteenth-note bursts
    setParameter (processor, "depth", 1.0f);
    setParameter (processor, "chance", 100.0f);
    setParameter (processor, "eventGrid", 1.0f); // Ten decisions per revolution
    for (int block = 0; block < 40; ++block)
    {
        audio.clear();
        processor.processBlock (audio, midi);
        if (! bufferIsFinite (audio))
        {
            std::cerr << "Non-finite output during noise orbit\n";
            return 5;
        }
    }

    std::cout << "BLORB DSP smoke test passed. Stereo balance range: "
              << minimumBalance << " to " << maximumBalance
              << ", swirl range: " << minimumSwirlRate << " to " << maximumSwirlRate << " Hz\n";
    return 0;
}
