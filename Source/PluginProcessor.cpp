#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr auto sourceId = "source";
constexpr auto rhythmId = "rhythm";
constexpr auto modelId = "model";
constexpr auto itdId = "itd";
constexpr auto frontLobesId = "frontLobes";
constexpr auto syncId = "sync";
constexpr auto divisionId = "division";
constexpr auto freeRateId = "freeRate";
constexpr auto swirlId = "swirl";
constexpr auto widthId = "width";
constexpr auto depthId = "depth";
constexpr auto rearBlendId = "rearBlend";
constexpr auto spectralArcId = "spectralArc";
constexpr auto frontSpaceId = "frontSpace";
constexpr auto dopplerId = "doppler";
constexpr auto chanceId = "chance";
constexpr auto eventGridId = "eventGrid";
constexpr auto mixId = "mix";
constexpr auto frequencyId = "frequency";

constexpr double twoPi = juce::MathConstants<double>::twoPi;

juce::NormalisableRange<float> makeFreeRateRange()
{
    constexpr float minimumRate = 0.02f;
    constexpr float musicalCeiling = 8.0f;
    constexpr float ludicrousCeiling = 64.0f;
    constexpr float musicalProportion = 0.90f;

    return { minimumRate, ludicrousCeiling,
        [=] (float, float, float normalised)
        {
            if (normalised <= musicalProportion)
                return minimumRate * std::pow (musicalCeiling / minimumRate,
                                               normalised / musicalProportion);

            const auto extremePosition = (normalised - musicalProportion)
                                       / (1.0f - musicalProportion);
            return musicalCeiling * std::pow (ludicrousCeiling / musicalCeiling,
                                              extremePosition * extremePosition * extremePosition);
        },
        [=] (float, float, float rate)
        {
            rate = juce::jlimit (minimumRate, ludicrousCeiling, rate);
            if (rate <= musicalCeiling)
                return musicalProportion * std::log (rate / minimumRate)
                     / std::log (musicalCeiling / minimumRate);

            const auto logarithmicPosition = std::log (rate / musicalCeiling)
                                           / std::log (ludicrousCeiling / musicalCeiling);
            return musicalProportion + (1.0f - musicalProportion) * std::cbrt (logarithmicPosition);
        },
        [] (float, float, float rate)
        {
            return std::round (rate * 1000.0f) / 1000.0f;
        } };
}

// Duration of one complete orbit in quarter notes, slowest to fastest.
constexpr std::array<double, 18> divisionQuarterNotes {
    6.0, 4.0, 8.0 / 3.0,
    3.0, 2.0, 4.0 / 3.0,
    1.5, 1.0, 2.0 / 3.0,
    0.75, 0.5, 1.0 / 3.0,
    0.375, 0.25, 1.0 / 6.0,
    0.1875, 0.125, 1.0 / 12.0
};
}

OrbitAudioProcessor::OrbitAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "ORBIT_STATE", createParameterLayout())
{
    formatManager.registerBasicFormats();
}

const juce::StringArray& OrbitAudioProcessor::divisionNames()
{
    static const juce::StringArray names {
        "1/1 dotted", "1/1", "1/1 triplet",
        "1/2 dotted", "1/2", "1/2 triplet",
        "1/4 dotted", "1/4", "1/4 triplet",
        "1/8 dotted", "1/8", "1/8 triplet",
        "1/16 dotted", "1/16", "1/16 triplet",
        "1/32 dotted", "1/32", "1/32 triplet"
    };
    return names;
}

juce::AudioProcessorValueTreeState::ParameterLayout OrbitAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> result;

    result.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { sourceId, 1 }, "Test Source",
        juce::StringArray { "Input", "Sine", "Saw", "White Noise", "Loaded WAV" }, 0));
    result.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { rhythmId, 1 }, "Test Rhythm",
        juce::StringArray { "Continuous", "1/8 notes", "1/16 notes" }, 0));
    result.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { modelId, 1 }, "Spatial Model",
        juce::StringArray { "Original", "Refined" }, 1));
    result.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { itdId, 1 }, "Interaural Delay", true));
    result.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { frontLobesId, 1 }, "Dual-Lobe Front", true));
    result.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { syncId, 1 }, "Tempo Sync", true));
    result.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { divisionId, 1 }, "Orbit Duration", divisionNames(), 1));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { freeRateId, 1 }, "Free Rate",
        makeFreeRateRange(), 0.25f, "Hz"));
    result.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { swirlId, 1 }, "Swirl", false));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { widthId, 1 }, "Width", 0.0f, 1.0f, 1.0f));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { depthId, 1 }, "Depth", 0.0f, 1.0f, 0.45f));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { rearBlendId, 1 }, "Rear Blend", 0.0f, 1.0f, 0.75f));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { spectralArcId, 1 }, "Spectral Arc",
        juce::NormalisableRange<float> { 90.0f, 130.0f, 1.0f }, 110.0f, " degrees"));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { frontSpaceId, 1 }, "Front Space", 0.0f, 1.0f, 0.50f));
    result.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { dopplerId, 1 }, "Doppler Shadow",
        juce::StringArray { "Off", "1/2", "1" }, 2));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { chanceId, 1 }, "Chance", 0.0f, 100.0f, 0.0f));
    result.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { eventGridId, 1 }, "Event Grid",
        juce::StringArray { "1 revolution", "1/10 revolution" }, 0));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { mixId, 1 }, "Mix", 0.0f, 1.0f, 1.0f));
    result.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { frequencyId, 1 }, "Test Frequency",
        juce::NormalisableRange<float> { 40.0f, 2000.0f, 0.1f, 0.35f }, 220.0f, "Hz"));

    return { result.begin(), result.end() };
}

bool OrbitAudioProcessor::loadAudioFile (const juce::File& file, juce::String& errorMessage)
{
    auto reader = std::unique_ptr<juce::AudioFormatReader> (formatManager.createReaderFor (file));
    if (reader == nullptr)
    {
        errorMessage = "BLORB could not read that audio file.";
        return false;
    }

    constexpr double maximumLengthSeconds = 600.0;
    if (reader->lengthInSamples <= 1
        || reader->lengthInSamples > static_cast<juce::int64> (reader->sampleRate * maximumLengthSeconds))
    {
        errorMessage = "Please choose a WAV file between one sample and ten minutes long.";
        return false;
    }

    auto audio = std::make_shared<LoadedAudio>();
    const auto channels = juce::jlimit (1, 2, static_cast<int> (reader->numChannels));
    const auto length = static_cast<int> (reader->lengthInSamples);
    audio->buffer.setSize (channels, length);
    if (! reader->read (&audio->buffer, 0, length, 0, true, channels > 1))
    {
        errorMessage = "BLORB could not decode that WAV file.";
        return false;
    }

    audio->sampleRate = reader->sampleRate;
    audio->fileName = file.getFileName();
    std::atomic_store_explicit (&loadedAudio, std::shared_ptr<const LoadedAudio> (audio),
                                std::memory_order_release);
    loadedPlaybackPosition.store (0.0, std::memory_order_relaxed);
    errorMessage.clear();
    return true;
}

void OrbitAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = juce::jmax (1.0, sampleRate);
    lowPassStateL = lowPassStateR = 0.0f;
    shadowStateL = shadowStateR = 0.0f;
    rearLowStateL = rearLowStateR = 0.0f;
    rearHighStateL = rearHighStateR = 0.0f;
    frontPresenceStateL = frontPresenceStateR = 0.0f;
    frontPresenceHighStateL = frontPresenceHighStateR = 0.0f;
    frontTopStateL = frontTopStateR = 0.0f;
    rearMuffleStateL = rearMuffleStateR = 0.0f;
    rearMidLowStateL = rearMidLowStateR = 0.0f;
    rearMidHighStateL = rearMidHighStateR = 0.0f;
    frontReflectionFilterStateL = frontReflectionFilterStateR = 0.0f;
    dopplerFilterStateL = dopplerFilterStateR = 0.0f;
    legacyRearStateL = legacyRearStateR = 0.0f;
    ghostFilterState = 0.0f;
    rhythmEnvelope = 1.0f;
    eventGlow = 0.0f;
    eventProgress = decisionProgress = 0.0;
    swirlPhase = 1.0 / 6.0;
    swirlWasEnabled = false;
    orbitDirection = 1;
    activeEvent = TemporaryEvent::none;
    eventRandom.setSeed (0x424c4f5242LL);

    const auto maximumDelaySamples = juce::roundToInt (currentSampleRate * 0.020) + 4;
    delayBufferL.assign (static_cast<size_t> (maximumDelaySamples), 0.0f);
    delayBufferR.assign (static_cast<size_t> (maximumDelaySamples), 0.0f);
    ghostDelayBuffer.assign (static_cast<size_t> (maximumDelaySamples), 0.0f);
    frontReflectionBuffer.assign (static_cast<size_t> (maximumDelaySamples), 0.0f);
    dopplerDelayBufferL.assign (static_cast<size_t> (maximumDelaySamples), 0.0f);
    dopplerDelayBufferR.assign (static_cast<size_t> (maximumDelaySamples), 0.0f);
    delayWritePosition = ghostWritePosition = 0;
    frontReflectionWritePosition = 0;
    dopplerWritePosition = 0;
}

bool OrbitAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    const auto input = layouts.getMainInputChannelSet();
    return output == juce::AudioChannelSet::stereo()
        && (input == juce::AudioChannelSet::stereo() || input == juce::AudioChannelSet::mono());
}

double OrbitAudioProcessor::getRateHz() const
{
    if (parameters.getRawParameterValue (syncId)->load() < 0.5f)
        return parameters.getRawParameterValue (freeRateId)->load();

    double bpm = 120.0;
    if (auto* hostPlayHead = getPlayHead())
        if (auto position = hostPlayHead->getPosition())
            if (auto hostBpm = position->getBpm())
                bpm = juce::jlimit (20.0, 400.0, *hostBpm);

    const auto index = juce::jlimit (0, static_cast<int> (divisionQuarterNotes.size()) - 1,
                                     juce::roundToInt (parameters.getRawParameterValue (divisionId)->load()));
    const auto secondsPerOrbit = divisionQuarterNotes[static_cast<size_t> (index)] * 60.0 / bpm;
    return 1.0 / secondsPerOrbit;
}

float OrbitAudioProcessor::smoothstep (float value) noexcept
{
    value = juce::jlimit (0.0f, 1.0f, value);
    return value * value * (3.0f - 2.0f * value);
}

float OrbitAudioProcessor::readDelay (const std::vector<float>& buffer, int writePosition,
                                      float delaySamples) const noexcept
{
    if (buffer.empty())
        return 0.0f;

    auto readPosition = static_cast<float> (writePosition) - delaySamples;
    const auto size = static_cast<float> (buffer.size());
    while (readPosition < 0.0f)
        readPosition += size;

    const auto bufferSize = static_cast<int> (buffer.size());
    const auto indexA = static_cast<int> (readPosition) % bufferSize;
    const auto indexBefore = (indexA + bufferSize - 1) % bufferSize;
    const auto indexB = (indexA + 1) % bufferSize;
    const auto indexAfter = (indexA + 2) % bufferSize;
    const auto fraction = readPosition - std::floor (readPosition);
    const auto before = buffer[static_cast<size_t> (indexBefore)];
    const auto current = buffer[static_cast<size_t> (indexA)];
    const auto next = buffer[static_cast<size_t> (indexB)];
    const auto after = buffer[static_cast<size_t> (indexAfter)];
    const auto c0 = current;
    const auto c1 = 0.5f * (next - before);
    const auto c2 = before - 2.5f * current + 2.0f * next - 0.5f * after;
    const auto c3 = 0.5f * (after - before) + 1.5f * (current - next);
    return ((c3 * fraction + c2) * fraction + c1) * fraction + c0;
}

void OrbitAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2 || numSamples == 0)
        return;

    const auto inputChannels = getTotalNumInputChannels();
    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);

    const auto source = juce::roundToInt (parameters.getRawParameterValue (sourceId)->load());
    const auto rhythm = juce::roundToInt (parameters.getRawParameterValue (rhythmId)->load());
    const auto refinedModel = parameters.getRawParameterValue (modelId)->load() >= 0.5f;
    const auto useInterauralDelay = refinedModel && parameters.getRawParameterValue (itdId)->load() >= 0.5f;
    const auto useFrontLobes = refinedModel && parameters.getRawParameterValue (frontLobesId)->load() >= 0.5f;
    const auto width = parameters.getRawParameterValue (widthId)->load();
    const auto depth = parameters.getRawParameterValue (depthId)->load();
    const auto rearBlend = parameters.getRawParameterValue (rearBlendId)->load();
    const auto spectralArcDegrees = parameters.getRawParameterValue (spectralArcId)->load();
    const auto frontSpace = parameters.getRawParameterValue (frontSpaceId)->load();
    const auto dopplerMode = juce::roundToInt (parameters.getRawParameterValue (dopplerId)->load());
    const auto dopplerAmount = dopplerMode == 0 ? 0.0f : (dopplerMode == 1 ? 0.75f : 1.50f);
    const auto eventChance = parameters.getRawParameterValue (chanceId)->load();
    const auto useSwirl = parameters.getRawParameterValue (swirlId)->load() >= 0.5f;
    const auto eventDivisions = parameters.getRawParameterValue (eventGridId)->load() < 0.5f ? 1.0 : 10.0;
    const auto speedRunDuration = eventDivisions > 1.0 ? 10.0 : 4.0;
    if (eventChance <= 0.0f)
    {
        orbitDirection = 1;
        activeEvent = TemporaryEvent::none;
        eventProgress = 0.0;
        decisionProgress = 0.0;
    }
    const auto wetMix = parameters.getRawParameterValue (mixId)->load();
    const auto testFrequency = parameters.getRawParameterValue (frequencyId)->load();
    const auto rateHz = getRateHz();
    const auto oscillatorIncrement = static_cast<double> (testFrequency) / currentSampleRate;
    const auto audioFile = std::atomic_load_explicit (&loadedAudio, std::memory_order_acquire);
    auto filePosition = loadedPlaybackPosition.load (std::memory_order_relaxed);
    const auto fileIncrement = audioFile != nullptr ? audioFile->sampleRate / currentSampleRate : 1.0;

    double bpm = 120.0;
    double hostPpq = 0.0;
    bool hasHostPpq = false;
    if (auto* hostPlayHead = getPlayHead())
        if (auto position = hostPlayHead->getPosition())
        {
            if (auto hostBpm = position->getBpm())
                bpm = juce::jlimit (20.0, 400.0, *hostBpm);
            if (auto ppq = position->getPpqPosition())
            {
                hostPpq = *ppq;
                hasHostPpq = true;
            }
        }

    const auto rhythmMultiplier = rhythm == 1 ? 2.0 : 4.0;
    if (rhythm != 0 && hasHostPpq)
        rhythmPhase = hostPpq * rhythmMultiplier - std::floor (hostPpq * rhythmMultiplier);
    const auto rhythmIncrement = rhythm == 0 ? 0.0 : (bpm / 60.0) * rhythmMultiplier / currentSampleRate;

    const auto extremeDepth = smoothstep ((depth - 0.9f) * 10.0f);
    const auto originalRearAttenuationDb = 18.0f * std::pow (depth, 1.4f) + 42.0f * extremeDepth;
    const auto spectralArcRadians = spectralArcDegrees * juce::MathConstants<float>::pi / 180.0f;
    const auto arcEdgeCosine = std::cos (spectralArcRadians);
    constexpr auto arcCoreCosine = 0.5f; // Fully front/rear inside 60 degrees of either pole.
    const auto envelopeAttack = std::exp (-1.0f / static_cast<float> (currentSampleRate * 0.0015));
    const auto envelopeRelease = std::exp (-1.0f / static_cast<float> (currentSampleRate * 0.014));
    double effectiveRateHz = rateHz;

    if (useSwirl && ! swirlWasEnabled)
        swirlPhase = 1.0 / 6.0; // Begin at 1x, travelling toward the 2x peak.
    swirlWasEnabled = useSwirl;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto eventSpeedMultiplier = 1.0;
        if (activeEvent == TemporaryEvent::halfSpeed)
            eventSpeedMultiplier = 0.5;
        else if (activeEvent == TemporaryEvent::doubleSpeed)
            eventSpeedMultiplier = 2.0;
        else if (activeEvent == TemporaryEvent::speedRun)
        {
            const auto runPosition = juce::jlimit (0.0, 1.0, eventProgress / speedRunDuration);
            eventSpeedMultiplier = 1.0 + (runPosition < 0.5
                ? runPosition * 2.0
                : (1.0 - runPosition) * 2.0);
        }

        auto swirlMultiplier = 1.0;
        if (useSwirl)
        {
            const auto triangle = swirlPhase < 0.5
                                ? swirlPhase * 2.0
                                : (1.0 - swirlPhase) * 2.0;
            swirlMultiplier = 0.5 + 1.5 * triangle;
        }

        effectiveRateHz = rateHz * eventSpeedMultiplier * swirlMultiplier;
        const auto phaseIncrement = static_cast<double> (orbitDirection) * effectiveRateHz / currentSampleRate;

        if (useSwirl)
        {
            swirlPhase += effectiveRateHz * 0.1 / currentSampleRate;
            swirlPhase -= std::floor (swirlPhase);
        }

        const float dryL = left[sample];
        const float dryR = inputChannels > 1 ? right[sample] : dryL;
        float inputL = dryL;
        float inputR = dryR;

        if (source >= 1 && source <= 3)
        {
            float generated = 0.0f;
            if (source == 1)
                generated = std::sin (static_cast<float> (twoPi * oscillatorPhase));
            else if (source == 2)
                generated = static_cast<float> (2.0 * oscillatorPhase - 1.0);
            else
                generated = noiseGenerator.nextFloat() * 2.0f - 1.0f;

            if (rhythm != 0)
            {
                const auto gateTarget = rhythmPhase < 0.22 ? 1.0f : 0.0f;
                const auto envelopeCoefficient = gateTarget > rhythmEnvelope ? envelopeAttack : envelopeRelease;
                rhythmEnvelope = gateTarget + envelopeCoefficient * (rhythmEnvelope - gateTarget);
                rhythmPhase += rhythmIncrement;
                rhythmPhase -= std::floor (rhythmPhase);
            }
            else
            {
                rhythmEnvelope = 1.0f;
            }

            generated *= 0.18f * rhythmEnvelope;
            inputL = inputR = generated;
            oscillatorPhase += oscillatorIncrement;
            oscillatorPhase -= std::floor (oscillatorPhase);
        }
        else if (source == 4)
        {
            inputL = inputR = 0.0f;
            if (audioFile != nullptr && audioFile->buffer.getNumSamples() > 1)
            {
                const auto length = audioFile->buffer.getNumSamples();
                while (filePosition >= static_cast<double> (length))
                    filePosition -= static_cast<double> (length);

                const auto indexA = static_cast<int> (filePosition);
                const auto indexB = (indexA + 1) % length;
                const auto fraction = static_cast<float> (filePosition - static_cast<double> (indexA));
                const auto readChannel = [&] (int channel)
                {
                    const auto* samples = audioFile->buffer.getReadPointer (
                        juce::jmin (channel, audioFile->buffer.getNumChannels() - 1));
                    return samples[indexA] + fraction * (samples[indexB] - samples[indexA]);
                };

                inputL = readChannel (0);
                inputR = readChannel (1);

                // A short edge fade makes automatic looping safe even for tightly trimmed speech.
                const auto edgeLength = juce::jmax (1.0, audioFile->sampleRate * 0.008);
                const auto distanceFromEnd = static_cast<double> (length - 1) - filePosition;
                const auto edgeGain = static_cast<float> (juce::jlimit (0.0, 1.0,
                    juce::jmin (filePosition / edgeLength, distanceFromEnd / edgeLength)));
                inputL *= edgeGain;
                inputR *= edgeGain;
                filePosition += fileIncrement;
            }
        }

        const auto angle = static_cast<float> (twoPi * phase);
        const auto sine = std::sin (angle);
        const auto cosine = std::cos (angle);
        const auto rear = 0.5f * (1.0f - cosine);
        const auto pan = juce::jlimit (-1.0f, 1.0f, width * sine);
        const auto panAngle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        const auto panL = std::cos (panAngle);
        const auto panR = std::sin (panAngle);

        const auto mid = 0.5f * (inputL + inputR);
        const auto side = 0.5f * (inputL - inputR);
        const auto sideRotation = 0.70710678f * cosine;
        float wetL = mid * panL + side * sideRotation;
        float wetR = mid * panR - side * sideRotation;

        if (! refinedModel)
        {
            const auto spectralAmount = std::pow (juce::jlimit (0.0f, 1.0f, depth * rear), 1.65f);
            const auto cutoffHz = 20000.0f * std::pow (700.0f / 20000.0f, spectralAmount);
            const auto coefficient = std::exp (-juce::MathConstants<float>::twoPi * cutoffHz
                                               / static_cast<float> (currentSampleRate));
            lowPassStateL = (1.0f - coefficient) * wetL + coefficient * lowPassStateL;
            lowPassStateR = (1.0f - coefficient) * wetR + coefficient * lowPassStateR;
            const auto rearGain = juce::Decibels::decibelsToGain (-originalRearAttenuationDb * rear * rear);
            wetL = lowPassStateL * rearGain;
            wetR = lowPassStateR * rearGain;
        }
        else
        {
            // Forward-facing ears create overlapping front/rear spectral regions around the sides.
            const auto rawFrontWeight = smoothstep ((cosine - arcEdgeCosine)
                                                    / (arcCoreCosine - arcEdgeCosine));
            const auto rawRearWeight = smoothstep ((-cosine - arcEdgeCosine)
                                                   / (arcCoreCosine - arcEdgeCosine));
            const auto spectralWeightSum = juce::jmax (0.0001f, rawFrontWeight + rawRearWeight);
            const auto frontWeight = rawFrontWeight / spectralWeightSum;
            const auto rearWeight = rawRearWeight / spectralWeightSum;

            // Lateral head shadow: the far ear keeps a quieter, darker copy except at true hard-pan.
            const auto lateral = width * sine;
            const auto shadowL = juce::jmax (0.0f, lateral);
            const auto shadowR = juce::jmax (0.0f, -lateral);
            const auto shadowCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 3200.0f
                                                     / static_cast<float> (currentSampleRate));
            shadowStateL = (1.0f - shadowCoefficient) * wetL + shadowCoefficient * shadowStateL;
            shadowStateR = (1.0f - shadowCoefficient) * wetR + shadowCoefficient * shadowStateR;
            const auto shadowMixL = smoothstep (shadowL);
            const auto shadowMixR = smoothstep (shadowR);
            wetL += shadowMixL * (shadowStateL - wetL);
            wetR += shadowMixR * (shadowStateR - wetR);

            if (! delayBufferL.empty())
            {
                delayBufferL[static_cast<size_t> (delayWritePosition)] = wetL;
                delayBufferR[static_cast<size_t> (delayWritePosition)] = wetR;
                const auto maximumItdSamples = useInterauralDelay ? static_cast<float> (currentSampleRate * 0.00055) : 0.0f;
                // One common sample of look-behind keeps the cubic fractional interpolation causal.
                wetL = readDelay (delayBufferL, delayWritePosition, 1.0f + shadowL * maximumItdSamples);
                wetR = readDelay (delayBufferR, delayWritePosition, 1.0f + shadowR * maximumItdSamples);
                delayWritePosition = (delayWritePosition + 1) % static_cast<int> (delayBufferL.size());
            }

            const auto rearInputL = wetL;
            const auto rearInputR = wetR;

            // A filtered moving-delay shadow supplies creative Doppler and phasing while the
            // direct source remains positionally stable. cos(2 theta) gives two radial cycles.
            auto dopplerShadowL = 0.0f;
            auto dopplerShadowR = 0.0f;
            if (! dopplerDelayBufferL.empty())
            {
                dopplerDelayBufferL[static_cast<size_t> (dopplerWritePosition)] = rearInputL;
                dopplerDelayBufferR[static_cast<size_t> (dopplerWritePosition)] = rearInputR;
                const auto orbitExtent = std::sqrt (0.5f * (width * width + depth * depth));
                const auto baseDelaySamples = static_cast<float> (currentSampleRate * 0.0150);
                const auto excursionSamples = static_cast<float> (currentSampleRate * 0.00025)
                                            * dopplerAmount * orbitExtent;
                const auto radialPosition = std::cos (2.0f * angle);
                const auto dopplerDelaySamples = baseDelaySamples + excursionSamples * radialPosition;
                dopplerShadowL = readDelay (dopplerDelayBufferL, dopplerWritePosition, dopplerDelaySamples);
                dopplerShadowR = readDelay (dopplerDelayBufferR, dopplerWritePosition, dopplerDelaySamples);
                dopplerWritePosition = (dopplerWritePosition + 1)
                                     % static_cast<int> (dopplerDelayBufferL.size());
            }

            const auto dopplerFilterCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 5600.0f
                                                           / static_cast<float> (currentSampleRate));
            dopplerFilterStateL = (1.0f - dopplerFilterCoefficient) * dopplerShadowL
                                + dopplerFilterCoefficient * dopplerFilterStateL;
            dopplerFilterStateR = (1.0f - dopplerFilterCoefficient) * dopplerShadowR
                                + dopplerFilterCoefficient * dopplerFilterStateR;

            // A restrained upper-mid presence band avoids the overhead cue that a full high shelf can create.
            const auto frontPresenceLowCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 1800.0f
                                                               / static_cast<float> (currentSampleRate));
            const auto frontPresenceHighCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 6500.0f
                                                                / static_cast<float> (currentSampleRate));
            frontPresenceStateL = (1.0f - frontPresenceLowCoefficient) * wetL
                                + frontPresenceLowCoefficient * frontPresenceStateL;
            frontPresenceStateR = (1.0f - frontPresenceLowCoefficient) * wetR
                                + frontPresenceLowCoefficient * frontPresenceStateR;
            frontPresenceHighStateL = (1.0f - frontPresenceHighCoefficient) * wetL
                                    + frontPresenceHighCoefficient * frontPresenceHighStateL;
            frontPresenceHighStateR = (1.0f - frontPresenceHighCoefficient) * wetR
                                    + frontPresenceHighCoefficient * frontPresenceHighStateR;

            const auto orbitScale = std::sqrt (0.5f * (width * width + depth * depth));
            const auto inwardTravel = smoothstep ((orbitScale - 0.35f) / 0.65f);
            const auto lobeAngle = juce::degreesToRadians (60.0f - 45.0f * inwardTravel);
            const auto signedFrontAngle = std::atan2 (sine, cosine);
            const auto distanceFromLobe = std::abs (std::abs (signedFrontAngle) - lobeAngle);
            constexpr auto lobeSigma = juce::MathConstants<float>::pi / 8.0f; // 22.5 degrees
            const auto lobeClarity = std::exp (-0.5f * std::pow (distanceFromLobe / lobeSigma, 2.0f));
            const auto frontClarityWeight = useFrontLobes
                                          ? frontWeight * (0.22f + 0.78f * lobeClarity)
                                          : frontWeight;
            const auto frontPresenceAmount = 0.22f * depth * frontClarityWeight;
            wetL += (frontPresenceHighStateL - frontPresenceStateL) * frontPresenceAmount;
            wetR += (frontPresenceHighStateR - frontPresenceStateR) * frontPresenceAmount;

            // The close frontal pole remains direct and room-anchored, but is fractionally
            // softer above 9 kHz than the two off-centre clarity lobes.
            const auto frontTopCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 9000.0f
                                                      / static_cast<float> (currentSampleRate));
            frontTopStateL = (1.0f - frontTopCoefficient) * wetL + frontTopCoefficient * frontTopStateL;
            frontTopStateR = (1.0f - frontTopCoefficient) * wetR + frontTopCoefficient * frontTopStateR;
            const auto frontPole = std::pow (juce::jmax (0.0f, cosine), 6.0f);
            const auto frontTopSoftening = useFrontLobes ? 0.10f * depth * frontPole : 0.0f;
            wetL += frontTopSoftening * (frontTopStateL - wetL);
            wetR += frontTopSoftening * (frontTopStateR - wetR);

            // Two fixed bands create a restrained rear notch/shelf without implying a huge vertical sweep.
            const auto coefficient5500 = std::exp (-juce::MathConstants<float>::twoPi * 5500.0f
                                                   / static_cast<float> (currentSampleRate));
            const auto coefficient11000 = std::exp (-juce::MathConstants<float>::twoPi * 11000.0f
                                                    / static_cast<float> (currentSampleRate));
            rearLowStateL = (1.0f - coefficient5500) * wetL + coefficient5500 * rearLowStateL;
            rearLowStateR = (1.0f - coefficient5500) * wetR + coefficient5500 * rearLowStateR;
            rearHighStateL = (1.0f - coefficient11000) * wetL + coefficient11000 * rearHighStateL;
            rearHighStateR = (1.0f - coefficient11000) * wetR + coefficient11000 * rearHighStateR;
            const auto rearAmount = depth * rearWeight;
            wetL -= (rearHighStateL - rearLowStateL) * (0.48f * rearAmount)
                  + (wetL - rearHighStateL) * (0.25f * rearAmount);
            wetR -= (rearHighStateR - rearLowStateR) * (0.48f * rearAmount)
                  + (wetR - rearHighStateR) * (0.25f * rearAmount);

            // Add a little extra fixed high cut around the exact rear pole.
            const auto rearPoleFocus = std::pow (rear, 4.0f);
            const auto rearMuffleCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 4000.0f
                                                        / static_cast<float> (currentSampleRate));
            rearMuffleStateL = (1.0f - rearMuffleCoefficient) * wetL
                             + rearMuffleCoefficient * rearMuffleStateL;
            rearMuffleStateR = (1.0f - rearMuffleCoefficient) * wetR
                             + rearMuffleCoefficient * rearMuffleStateR;
            const auto rearMuffleMix = depth * (0.18f * rearWeight * rearWeight
                                              + 0.14f * rearPoleFocus);
            wetL += rearMuffleMix * (rearMuffleStateL - wetL);
            wetR += rearMuffleMix * (rearMuffleStateR - wetR);

            // A very shallow fixed 800 Hz-2.6 kHz dip reduces rear presence without
            // turning the whole rear hemisphere into a telephone filter.
            const auto rearMidLowCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 800.0f
                                                        / static_cast<float> (currentSampleRate));
            const auto rearMidHighCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 2600.0f
                                                         / static_cast<float> (currentSampleRate));
            rearMidLowStateL = (1.0f - rearMidLowCoefficient) * wetL
                             + rearMidLowCoefficient * rearMidLowStateL;
            rearMidLowStateR = (1.0f - rearMidLowCoefficient) * wetR
                             + rearMidLowCoefficient * rearMidLowStateR;
            rearMidHighStateL = (1.0f - rearMidHighCoefficient) * wetL
                              + rearMidHighCoefficient * rearMidHighStateL;
            rearMidHighStateR = (1.0f - rearMidHighCoefficient) * wetR
                              + rearMidHighCoefficient * rearMidHighStateR;
            const auto rearMidDip = 0.11f * depth * rearPoleFocus;
            wetL -= (rearMidHighStateL - rearMidLowStateL) * rearMidDip;
            wetR -= (rearMidHighStateR - rearMidLowStateR) * rearMidDip;

            // A fixed filtered state is crossfaded in near the rear. Avoiding a swept cutoff
            // prevents the filter's changing phase from masquerading as pitch movement.
            const auto extremeCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 900.0f
                                                      / static_cast<float> (currentSampleRate));
            lowPassStateL = (1.0f - extremeCoefficient) * wetL + extremeCoefficient * lowPassStateL;
            lowPassStateR = (1.0f - extremeCoefficient) * wetR + extremeCoefficient * lowPassStateR;
            const auto extremeFilterMix = smoothstep (extremeDepth * rearWeight);
            const auto extremeToneL = wetL + extremeFilterMix * (lowPassStateL - wetL);
            const auto extremeToneR = wetR + extremeFilterMix * (lowPassStateR - wetR);

            // Tiny asymmetric early reflections provide a front-facing room anchor.
            auto frontReflectionL = 0.0f;
            auto frontReflectionR = 0.0f;
            if (! frontReflectionBuffer.empty())
            {
                frontReflectionBuffer[static_cast<size_t> (frontReflectionWritePosition)] = 0.5f * (inputL + inputR);
                frontReflectionL = 0.72f * readDelay (frontReflectionBuffer, frontReflectionWritePosition,
                                                     static_cast<float> (currentSampleRate * 0.0043))
                                 + 0.28f * readDelay (frontReflectionBuffer, frontReflectionWritePosition,
                                                     static_cast<float> (currentSampleRate * 0.0091));
                frontReflectionR = 0.72f * readDelay (frontReflectionBuffer, frontReflectionWritePosition,
                                                     static_cast<float> (currentSampleRate * 0.0051))
                                 + 0.28f * readDelay (frontReflectionBuffer, frontReflectionWritePosition,
                                                     static_cast<float> (currentSampleRate * 0.0082));
                frontReflectionWritePosition = (frontReflectionWritePosition + 1)
                                             % static_cast<int> (frontReflectionBuffer.size());
            }
            const auto frontReflectionCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 7200.0f
                                                             / static_cast<float> (currentSampleRate));
            frontReflectionFilterStateL = (1.0f - frontReflectionCoefficient) * frontReflectionL
                                        + frontReflectionCoefficient * frontReflectionFilterStateL;
            frontReflectionFilterStateR = (1.0f - frontReflectionCoefficient) * frontReflectionR
                                        + frontReflectionCoefficient * frontReflectionFilterStateR;
            const auto frontReflectionGain = 0.12f * frontSpace * depth * frontWeight;

            // A quiet, delayed mono shadow increases apparent distance but vanishes at maximum Depth.
            const auto ghostInput = 0.5f * (inputL + inputR);
            const auto ghostCoefficient = std::exp (-juce::MathConstants<float>::twoPi * 3800.0f
                                                    / static_cast<float> (currentSampleRate));
            ghostFilterState = (1.0f - ghostCoefficient) * ghostInput + ghostCoefficient * ghostFilterState;
            auto ghost = 0.0f;
            if (! ghostDelayBuffer.empty())
            {
                ghostDelayBuffer[static_cast<size_t> (ghostWritePosition)] = ghostFilterState;
                ghost = readDelay (ghostDelayBuffer, ghostWritePosition,
                                   static_cast<float> (currentSampleRate * 0.0073));
                ghostWritePosition = (ghostWritePosition + 1) % static_cast<int> (ghostDelayBuffer.size());
            }

            // Through 90% Depth, spectral cues move around the head without deliberate distance loss.
            // The final 10% stretches the path into an impossible super-ellipse: both poles recede,
            // while the rear travels farther than the front.
            const auto axialWeight = std::pow (std::abs (cosine), 4.0f);
            const auto commonExtremeDb = 10.0f * extremeDepth * axialWeight;
            const auto additionalRearDb = 22.0f * extremeDepth * rearWeight * axialWeight;
            const auto spatialGain = juce::Decibels::decibelsToGain (-(commonExtremeDb + additionalRearDb));
            const auto ghostGain = 0.07f * rearAmount * (1.0f - extremeDepth);
            const auto refinedL = (extremeToneL + frontReflectionFilterStateL * frontReflectionGain)
                                * spatialGain + ghost * ghostGain;
            const auto refinedR = (extremeToneR + frontReflectionFilterStateR * frontReflectionGain)
                                * spatialGain + ghost * ghostGain;

            // Blend in the old, strongly darkened rear only near the back hemisphere.
            // Head shadow and ITD remain those of the refined model on both paths.
            const auto legacySpectralAmount = std::pow (juce::jlimit (0.0f, 1.0f, depth), 1.65f);
            const auto legacyCutoff = 20000.0f * std::pow (700.0f / 20000.0f, legacySpectralAmount);
            const auto legacyCoefficient = std::exp (-juce::MathConstants<float>::twoPi * legacyCutoff
                                                     / static_cast<float> (currentSampleRate));
            legacyRearStateL = (1.0f - legacyCoefficient) * rearInputL + legacyCoefficient * legacyRearStateL;
            legacyRearStateR = (1.0f - legacyCoefficient) * rearInputR + legacyCoefficient * legacyRearStateR;
            const auto legacyL = legacyRearStateL * spatialGain;
            const auto legacyR = legacyRearStateR * spatialGain;
            const auto hybridAmount = rearBlend * std::pow (rearWeight, 1.35f);
            wetL = refinedL + hybridAmount * (legacyL - refinedL);
            wetR = refinedR + hybridAmount * (legacyR - refinedR);
            const auto dopplerShadowGain = 0.14f * dopplerAmount * spatialGain;
            wetL += dopplerFilterStateL * dopplerShadowGain;
            wetR += dopplerFilterStateR * dopplerShadowGain;
        }

        left[sample] = dryL + wetMix * (wetL - dryL);
        right[sample] = dryR + wetMix * (wetR - dryR);

        phase += phaseIncrement;
        phase -= std::floor (phase);

        const auto travelledPhase = std::abs (phaseIncrement);
        if (activeEvent != TemporaryEvent::none)
        {
            eventProgress += travelledPhase;
            const auto eventDuration = activeEvent == TemporaryEvent::speedRun ? speedRunDuration : 1.0;
            if (eventProgress >= eventDuration)
            {
                activeEvent = TemporaryEvent::none;
                eventProgress = 0.0;
            }
        }

        decisionProgress += travelledPhase * eventDivisions;
        if (decisionProgress >= 1.0)
        {
            decisionProgress -= std::floor (decisionProgress);
            if (activeEvent == TemporaryEvent::none
                && eventRandom.nextFloat() * 100.0f < eventChance)
            {
                const auto selection = eventRandom.nextFloat() * 100.0f;
                if (selection < 25.0f)
                {
                    activeEvent = TemporaryEvent::halfSpeed;
                    eventGlow = 1.0f;
                }
                else if (selection < 50.0f)
                {
                    activeEvent = TemporaryEvent::doubleSpeed;
                    eventGlow = 1.0f;
                }
                else if (selection < 65.0f)
                {
                    activeEvent = TemporaryEvent::speedRun;
                    eventGlow = 1.0f;
                }
                else if (selection < 75.0f)
                {
                    orbitDirection = -orbitDirection;
                    eventGlow = 1.0f;
                }
                // 75-100 deliberately means: nothing happens.

                eventProgress = 0.0;
            }
        }
    }

    const auto finalAngle = static_cast<float> (twoPi * phase);
    displayedPhase.store (static_cast<float> (phase), std::memory_order_relaxed);
    displayedRear.store (0.5f * (1.0f - std::cos (finalAngle)), std::memory_order_relaxed);
    displayedRateHz.store (static_cast<float> (effectiveRateHz), std::memory_order_relaxed);
    eventGlow *= std::exp (-static_cast<float> (numSamples) / static_cast<float> (currentSampleRate * 0.45));
    displayedEventGlow.store (eventGlow, std::memory_order_relaxed);
    if (source == 4)
        loadedPlaybackPosition.store (filePosition, std::memory_order_relaxed);
}

void OrbitAudioProcessor::getStateInformation (juce::MemoryBlock& destination)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destination);
}

void OrbitAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* OrbitAudioProcessor::createEditor()
{
    return new OrbitAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OrbitAudioProcessor();
}
