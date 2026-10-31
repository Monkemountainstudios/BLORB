#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr auto brass = 0xffc58a35;
constexpr auto paleBrass = 0xffffcf78;
constexpr auto nearBlack = 0xff0b0907;
}

void BlorbLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float position, float startAngle, float endAngle,
                                         juce::Slider&)
{
    auto area = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                        static_cast<float> (width), static_cast<float> (height));
    area = area.reduced (juce::jmin (8.0f, area.getWidth() * 0.08f));
    const auto diameter = juce::jmin (area.getWidth(), area.getHeight());
    auto knob = juce::Rectangle<float> (diameter, diameter).withCentre (area.getCentre());
    knob = knob.reduced (diameter * 0.13f);

    const auto centre = knob.getCentre();
    const auto tickOuter = diameter * 0.485f;
    const auto drawScaleTick = [&] (float normalised, float length, float thickness,
                                    juce::Colour colour)
    {
        const auto angle = startAngle + normalised * (endAngle - startAngle);
        const auto inner = tickOuter - length;
        g.setColour (colour);
        g.drawLine (centre.x + std::sin (angle) * inner,
                    centre.y - std::cos (angle) * inner,
                    centre.x + std::sin (angle) * tickOuter,
                    centre.y - std::cos (angle) * tickOuter,
                    thickness);
    };

    for (int tick = 0; tick <= 18; ++tick)
    {
        const auto normalised = static_cast<float> (tick) / 20.0f;
        drawScaleTick (normalised, tick % 2 == 0 ? 5.5f : 3.5f, 0.9f,
                       juce::Colour (tick % 2 == 0 ? 0x99c58a35 : 0x647e552a));
    }
    for (int tick = 0; tick <= 10; ++tick)
    {
        const auto normalised = 0.9f + static_cast<float> (tick) * 0.01f;
        drawScaleTick (normalised, tick % 5 == 0 ? 7.0f : 4.5f,
                       tick % 5 == 0 ? 1.25f : 0.75f,
                       juce::Colour (tick % 5 == 0 ? 0xd8ffd080 : 0x90d99a42));
    }

    g.setColour (juce::Colour (0xa8000000));
    g.fillEllipse (knob.translated (-5.0f, 6.0f).expanded (4.0f));
    g.setColour (juce::Colour (0x40000000));
    g.fillEllipse (knob.translated (-8.0f, 9.0f).expanded (5.0f));
    g.setColour (juce::Colour (0xff2b1d10));
    g.fillEllipse (knob.expanded (5.0f));
    g.setColour (juce::Colour (brass));
    g.drawEllipse (knob.expanded (5.0f), 2.0f);

    juce::ColourGradient metal (juce::Colour (paleBrass), knob.getX(), knob.getY(),
                                juce::Colour (0xff5e3715), knob.getRight(), knob.getBottom(), true);
    metal.addColour (0.46, juce::Colour (0xffd89b42));
    metal.addColour (0.7, juce::Colour (0xff79501f));
    g.setGradientFill (metal);
    g.fillEllipse (knob);
    g.setColour (juce::Colour (0x70fff1c8));
    g.drawEllipse (knob.reduced (2.0f), 1.0f);

    const auto angle = startAngle + position * (endAngle - startAngle);
    const auto markerRadius = knob.getWidth() * 0.34f;
    const juce::Point<float> marker { centre.x + std::sin (angle) * markerRadius,
                                      centre.y - std::cos (angle) * markerRadius };
    g.setColour (juce::Colour (0xdd120c07));
    g.fillEllipse (marker.x - 3.2f, marker.y - 3.2f, 6.4f, 6.4f);
    g.setColour (juce::Colour (0x90ffe6a0));
    g.drawEllipse (marker.x - 2.2f, marker.y - 2.2f, 4.4f, 4.4f, 0.8f);
}

void BlorbLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                         bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    const auto switchArea = bounds.removeFromLeft (34.0f).reduced (4.0f, 3.0f);
    g.setColour (juce::Colour (0xff1a120c));
    g.fillRoundedRectangle (switchArea, 5.0f);
    g.setColour (juce::Colour (brass));
    g.drawRoundedRectangle (switchArea, 5.0f, 1.2f);
    const auto pivot = switchArea.getCentre();
    const auto tip = juce::Point<float> (pivot.x + (button.getToggleState() ? 6.0f : -6.0f),
                                         pivot.y - 8.0f);
    g.setColour (juce::Colour (0xffe2a750));
    g.drawLine (pivot.x, pivot.y + 5.0f, tip.x, tip.y, 4.0f);
    g.setColour (juce::Colour (0xffffd98c));
    g.fillEllipse (tip.x - 3.5f, tip.y - 3.5f, 7.0f, 7.0f);
    g.setColour (juce::Colour (0xffd2a35f));
    g.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
    g.drawText (button.getButtonText(), bounds.toNearestInt(), juce::Justification::centredLeft);
}

void BlorbLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height));
    if (box.getName() == "vintageSwitch")
    {
        const auto centre = bounds.getCentre();
        g.setColour (juce::Colour (0xff160f0a));
        g.fillEllipse (centre.x - 15.0f, centre.y - 15.0f, 30.0f, 30.0f);
        g.setColour (juce::Colour (brass));
        g.drawEllipse (centre.x - 15.0f, centre.y - 15.0f, 30.0f, 30.0f, 1.5f);
        const auto count = juce::jmax (2, box.getNumItems());
        const auto index = juce::jmax (0, box.getSelectedItemIndex());
        const auto normalised = static_cast<float> (index) / static_cast<float> (count - 1);
        const auto angle = juce::jmap (normalised, -0.65f, 0.65f);
        const juce::Point<float> tip { centre.x + std::sin (angle) * 18.0f,
                                      centre.y - std::cos (angle) * 18.0f };
        g.setColour (juce::Colour (0xffd9993d));
        g.drawLine (centre.x, centre.y + 7.0f, tip.x, tip.y, 5.0f);
        g.setColour (juce::Colour (paleBrass));
        g.fillEllipse (tip.x - 4.0f, tip.y - 4.0f, 8.0f, 8.0f);
        g.setColour (juce::Colour (0xffd2a35f));
        g.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
        g.drawText (box.getText(), bounds.removeFromBottom (14.0f).toNearestInt(), juce::Justification::centred);
        return;
    }

    g.setColour (juce::Colour (0xff17110c));
    g.fillRoundedRectangle (bounds.reduced (1.0f), 3.0f);
    g.setColour (juce::Colour (0xff765126));
    g.drawRoundedRectangle (bounds.reduced (1.0f), 3.0f, 1.0f);
}

void BlorbLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                             const juce::Colour&, bool over, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    if (button.getName() == "orbButton")
    {
        const auto diameter = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto dome = juce::Rectangle<float> (diameter, diameter).withCentre (bounds.getCentre()).reduced (2.0f);
        g.setColour (juce::Colour (0x99000000));
        g.fillEllipse (dome.translated (-2.0f, 3.0f).expanded (2.0f));
        g.setColour (juce::Colour (0xff35210f));
        g.fillEllipse (dome);
        g.setColour (juce::Colour (0xffc98b35));
        g.drawEllipse (dome, 2.0f);
        auto glass = dome.reduced (5.0f);
        juce::ColourGradient amberGlass (juce::Colour (down ? 0xffffc45d : 0xffff9b19),
                                         glass.getCentreX() - 4.0f, glass.getCentreY() - 5.0f,
                                         juce::Colour (0xff391806), glass.getRight(), glass.getBottom(), true);
        amberGlass.addColour (0.45, juce::Colour (over ? 0xffee8d17 : 0xff9a4d0b));
        g.setGradientFill (amberGlass);
        g.fillEllipse (glass);
        g.setColour (juce::Colour (0x9affefc4));
        g.fillEllipse (glass.getX() + glass.getWidth() * 0.23f, glass.getY() + glass.getHeight() * 0.16f,
                       glass.getWidth() * 0.22f, glass.getHeight() * 0.12f);
        return;
    }

    const auto active = button.getToggleState();
    g.setColour (juce::Colour (down ? 0xff6f461b
                                    : (active ? 0xff7a4615
                                              : (over ? 0xff342113 : 0xff18100b))));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (active ? juce::Colour (0xffffc65f) : juce::Colour (brass));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void FineWheelSlider::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (! isEnabled())
        return;

    const auto direction = wheel.deltaY != 0.0f ? wheel.deltaY : -wheel.deltaX;
    const auto fineStep = 0.0015f * direction;
    const auto current = static_cast<float> (valueToProportionOfLength (getValue()));
    setValue (proportionOfLengthToValue (juce::jlimit (0.0f, 1.0f, current + fineStep)),
              juce::sendNotificationSync);
}

VintageLeverSwitch::VintageLeverSwitch (juce::AudioProcessorValueTreeState& state,
                                        const juce::String& parameterId,
                                        juce::StringArray positionLabels)
    : parameter (state.getParameter (parameterId)), labels (std::move (positionLabels))
{
    jassert (parameter != nullptr);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    startTimerHz (20);
    timerCallback();
}

void VintageLeverSwitch::timerCallback()
{
    if (parameter == nullptr)
        return;

    const auto newPosition = juce::roundToInt (parameter->convertFrom0to1 (parameter->getValue()));
    if (newPosition != selectedPosition)
    {
        selectedPosition = newPosition;
        repaint();
    }
}

void VintageLeverSwitch::mouseDown (const juce::MouseEvent& event)
{
    if (! isEnabled() || parameter == nullptr || labels.size() < 2)
        return;

    const auto usableWidth = juce::jmax (1.0f, static_cast<float> (getWidth() - 1));
    const auto chosen = juce::jlimit (0, labels.size() - 1,
                                     juce::roundToInt (event.position.x / usableWidth
                                                       * static_cast<float> (labels.size() - 1)));
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost (parameter->convertTo0to1 (static_cast<float> (chosen)));
    parameter->endChangeGesture();
    selectedPosition = chosen;
    repaint();
}

void VintageLeverSwitch::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto hardware = bounds.removeFromTop (bounds.getHeight() - 22.0f);
    const auto pivot = juce::Point<float> (hardware.getCentreX(), hardware.getCentreY() + 4.0f);

    g.setColour (juce::Colour (0x99000000));
    g.fillEllipse (pivot.x - 20.0f, pivot.y - 17.0f, 42.0f, 42.0f);
    juce::ColourGradient bezel (juce::Colour (0xffffcb72), pivot.x - 15.0f, pivot.y - 14.0f,
                                juce::Colour (0xff4d2c11), pivot.x + 16.0f, pivot.y + 17.0f, true);
    g.setGradientFill (bezel);
    g.fillEllipse (pivot.x - 18.0f, pivot.y - 18.0f, 36.0f, 36.0f);
    g.setColour (juce::Colour (0xff201208));
    g.fillEllipse (pivot.x - 11.0f, pivot.y - 11.0f, 22.0f, 22.0f);
    g.setColour (juce::Colour (0xffb6772c));
    g.drawEllipse (pivot.x - 18.0f, pivot.y - 18.0f, 36.0f, 36.0f, 1.4f);

    const auto normalised = labels.size() > 1
                          ? static_cast<float> (selectedPosition) / static_cast<float> (labels.size() - 1)
                          : 0.5f;
    const auto isCentrePosition = labels.size() == 3 && selectedPosition == 1;
    if (isCentrePosition)
    {
        g.setColour (juce::Colour (0x8f000000));
        g.fillEllipse (pivot.x - 6.0f, pivot.y - 4.0f, 15.0f, 15.0f);
        juce::ColourGradient cap (juce::Colour (0xffffdc8b), pivot.x - 4.0f, pivot.y - 5.0f,
                                  juce::Colour (0xff704015), pivot.x + 6.0f, pivot.y + 7.0f, true);
        g.setGradientFill (cap);
        g.fillEllipse (pivot.x - 6.5f, pivot.y - 6.5f, 13.0f, 13.0f);
        g.setColour (juce::Colour (0xff59300f));
        g.drawEllipse (pivot.x - 6.5f, pivot.y - 6.5f, 13.0f, 13.0f, 1.0f);
    }
    else
    {
        const auto armLength = hardware.getWidth() * 0.36f;
        const auto direction = normalised < 0.5f ? -1.0f : 1.0f;
        const auto tip = juce::Point<float> (pivot.x + direction * armLength, pivot.y);
        const auto rootHalf = 4.8f;
        const auto tipHalf = 2.8f;
        juce::Path armShadow;
        armShadow.startNewSubPath (pivot.x, pivot.y - rootHalf + 4.0f);
        armShadow.lineTo (tip.x, tip.y - tipHalf + 4.0f);
        armShadow.lineTo (tip.x, tip.y + tipHalf + 4.0f);
        armShadow.lineTo (pivot.x, pivot.y + rootHalf + 4.0f);
        armShadow.closeSubPath();
        g.setColour (juce::Colour (0x9a000000));
        g.fillPath (armShadow);

        juce::Path taperedArm;
        taperedArm.startNewSubPath (pivot.x, pivot.y - rootHalf);
        taperedArm.lineTo (tip.x, tip.y - tipHalf);
        taperedArm.lineTo (tip.x, tip.y + tipHalf);
        taperedArm.lineTo (pivot.x, pivot.y + rootHalf);
        taperedArm.closeSubPath();
        juce::ColourGradient shaft (juce::Colour (0xffffd480), pivot.x, pivot.y - rootHalf,
                                    juce::Colour (0xff704015), pivot.x, pivot.y + rootHalf, false);
        g.setGradientFill (shaft);
        g.fillPath (taperedArm);
        g.setColour (juce::Colour (0xff5b3210));
        g.strokePath (taperedArm, juce::PathStrokeType (0.9f));

        g.setColour (juce::Colour (0x8f000000));
        g.fillEllipse (tip.x - 6.0f, tip.y - 3.0f, 14.0f, 14.0f);
        juce::ColourGradient cap (juce::Colour (0xffffdc8b), tip.x - 4.0f, tip.y - 5.0f,
                                  juce::Colour (0xff704015), tip.x + 6.0f, tip.y + 7.0f, true);
        g.setGradientFill (cap);
        g.fillEllipse (tip.x - 6.5f, tip.y - 6.5f, 13.0f, 13.0f);
        g.setColour (juce::Colour (0xff59300f));
        g.drawEllipse (tip.x - 6.5f, tip.y - 6.5f, 13.0f, 13.0f, 1.0f);
    }

    g.setFont (juce::FontOptions (10.5f).withStyle ("Bold"));
    for (int i = 0; i < labels.size(); ++i)
    {
        const auto x = labels.size() > 1
                     ? juce::jmap (static_cast<float> (i), 0.0f, static_cast<float> (labels.size() - 1),
                                   bounds.getX(), bounds.getRight() - 42.0f)
                     : bounds.getCentreX() - 21.0f;
        g.setColour (juce::Colour (i == selectedPosition ? 0xffffca6c : 0xff8d6a42));
        g.drawText (labels[i], juce::Rectangle<float> (x, bounds.getY(), 42.0f, bounds.getHeight()),
                    juce::Justification::centred);
    }
}

OrbitDisplay::OrbitDisplay (OrbitAudioProcessor& owner) : processor (owner)
{
    startTimerHz (60);
}

void OrbitDisplay::timerCallback()
{
    const auto newPhase = processor.getOrbitPhase();
    if (phaseInitialised)
    {
        auto delta = newPhase - previousPhase;
        if (delta > 0.5f) delta -= 1.0f;
        if (delta < -0.5f) delta += 1.0f;
        if (std::abs (delta) > 0.00001f)
            motionDirection = delta >= 0.0f ? 1.0f : -1.0f;
    }
    previousPhase = newPhase;
    phaseInitialised = true;
    phase = newPhase;
    rear = processor.getRearFactor();
    rateHz = processor.getCurrentRateHz();
    width = processor.parameters.getRawParameterValue ("width")->load();
    depth = processor.parameters.getRawParameterValue ("depth")->load();
    eventGlow = processor.getEventGlow();
    repaint();
}

void OrbitDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (8.0f);
    const auto diameter = juce::jmin (bounds.getWidth(), bounds.getHeight());
    auto dial = juce::Rectangle<float> (diameter, diameter).withCentre (bounds.getCentre());
    const auto centre = dial.getCentre();
    const auto radius = diameter * 0.39f;
    const auto speedGlow = juce::jlimit (0.0f, 1.0f, std::log10 (1.0f + rateHz) / std::log10 (9.0f));
    const auto illumination = juce::jlimit (0.0f, 1.0f, 0.42f + speedGlow * 0.25f + eventGlow * 0.33f);

    g.setColour (juce::Colour (0xbb000000));
    g.fillEllipse (dial.translated (7.0f, 10.0f).expanded (5.0f));
    juce::ColourGradient outerMetal (juce::Colour (0xffffbd58), dial.getX(), dial.getY(),
                                     juce::Colour (0xff321b0a), dial.getRight(), dial.getBottom(), true);
    outerMetal.addColour (0.28, juce::Colour (0xff89551e));
    outerMetal.addColour (0.55, juce::Colour (0xffd18a31));
    outerMetal.addColour (0.78, juce::Colour (0xff4b2a0e));
    g.setGradientFill (outerMetal);
    g.fillEllipse (dial);
    g.setColour (juce::Colour (0xff160e08));
    g.fillEllipse (dial.reduced (diameter * 0.045f));
    g.setColour (juce::Colour (0xa0ffe0a0));
    g.drawEllipse (dial.reduced (diameter * 0.009f), 1.5f);
    g.setColour (juce::Colour (0xff4a290e));
    g.drawEllipse (dial.reduced (diameter * 0.041f), 3.0f);
    g.setColour (juce::Colour (0xffd79a3e));
    g.drawEllipse (dial.reduced (diameter * 0.052f), 2.0f);

    const auto ringRadius = diameter * 0.474f;
    for (int mark = 0; mark < 120; ++mark)
    {
        const auto unit = std::fmod (static_cast<float> (mark) * 0.6180339887f, 1.0f);
        const auto angle = unit * juce::MathConstants<float>::twoPi;
        const auto radialJitter = std::sin (static_cast<float> (mark) * 12.9898f) * diameter * 0.010f;
        const auto p = centre + juce::Point<float> (std::cos (angle), std::sin (angle))
                                * (ringRadius + radialJitter);
        const auto fleck = 0.8f + 1.3f * std::abs (std::sin (static_cast<float> (mark) * 4.73f));
        g.setColour (mark % 3 == 0 ? juce::Colour (0x60421f08) : juce::Colour (0x45ffe0a0));
        g.fillEllipse (p.x - fleck * 0.5f, p.y - fleck * 0.5f, fleck, fleck);
    }

    auto face = dial.reduced (diameter * 0.075f);
    juce::ColourGradient amber (juce::Colour (0xff140d08), centre.x, face.getBottom(),
                                juce::Colour::fromFloatRGBA (0.95f, 0.56f, 0.12f, illumination),
                                centre.x, face.getY(), false);
    amber.addColour (0.58, juce::Colour::fromFloatRGBA (0.34f, 0.19f, 0.06f, 1.0f));
    g.setGradientFill (amber);
    g.fillEllipse (face);

    for (int tick = 0; tick < 72; ++tick)
    {
        const auto a = juce::MathConstants<float>::twoPi * static_cast<float> (tick) / 72.0f;
        const auto major = tick % 9 == 0;
        const auto outer = radius * 1.04f;
        const auto inner = outer - (major ? 13.0f : 6.0f);
        g.setColour (juce::Colour (major ? 0xc0f4c16e : 0x80c58a35));
        g.drawLine (centre.x + std::sin (a) * inner, centre.y - std::cos (a) * inner,
                    centre.x + std::sin (a) * outer, centre.y - std::cos (a) * outer,
                    major ? 1.5f : 0.8f);
    }

    g.setColour (juce::Colour (0x70e6b15f));
    g.drawLine (centre.x, centre.y - radius, centre.x, centre.y + radius, 1.0f);
    g.drawLine (centre.x - radius, centre.y, centre.x + radius, centre.y, 1.0f);

    const auto ovalW = radius * (0.18f + 0.78f * width);
    const auto ovalH = radius * (0.18f + 0.78f * depth);
    auto orbitOval = juce::Rectangle<float> (ovalW * 2.0f, ovalH * 2.0f).withCentre (centre);
    for (int glowLayer = 7; glowLayer >= 1; --glowLayer)
    {
        const auto expansion = static_cast<float> (glowLayer) * 5.0f;
        const auto alpha = (0.010f + illumination * 0.009f + eventGlow * 0.006f)
                         * static_cast<float> (8 - glowLayer);
        g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.55f, 0.07f, alpha));
        g.fillEllipse (orbitOval.expanded (expansion));
    }
    g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.68f, 0.14f,
                                               0.055f + illumination * 0.045f));
    g.fillEllipse (orbitOval);
    juce::ColourGradient centreFalloff (juce::Colour (0x46110804), centre.x, centre.y,
                                        juce::Colours::transparentBlack,
                                        centre.x + juce::jmax (12.0f, ovalW * 0.92f), centre.y, true);
    g.setGradientFill (centreFalloff);
    g.fillEllipse (orbitOval);
    for (int halo = 4; halo >= 1; --halo)
    {
        g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.56f, 0.08f,
                                                   (0.018f + eventGlow * 0.018f) * halo));
        g.drawEllipse (orbitOval.expanded (static_cast<float> (halo) * 2.0f),
                       static_cast<float> (halo) * 1.7f);
    }

    const auto trailHeadAngle = phase * juce::MathConstants<float>::twoPi
                              - juce::MathConstants<float>::halfPi;
    constexpr int trailSegments = 112;
    for (int segment = 0; segment < trailSegments; ++segment)
    {
        const auto t0 = static_cast<float> (segment) / static_cast<float> (trailSegments);
        const auto t1 = static_cast<float> (segment + 1) / static_cast<float> (trailSegments);
        const auto a0 = trailHeadAngle - motionDirection * t0 * juce::MathConstants<float>::twoPi;
        const auto a1 = trailHeadAngle - motionDirection * t1 * juce::MathConstants<float>::twoPi;
        const juce::Point<float> p0 { centre.x + std::cos (a0) * ovalW,
                                      centre.y + std::sin (a0) * ovalH };
        const juce::Point<float> p1 { centre.x + std::cos (a1) * ovalW,
                                      centre.y + std::sin (a1) * ovalH };
        const auto persistence = std::pow (1.0f - t0, 1.75f);
        g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.86f, 0.54f, 0.86f * persistence));
        g.drawLine (p0.x, p0.y, p1.x, p1.y, 0.65f + 1.0f * persistence);
    }

    g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
    g.setColour (juce::Colour (0xff2b1708));
    g.drawText ("F R O N T", juce::Rectangle<float> (centre.x - 55.0f, face.getY() + 12.0f, 110.0f, 18.0f), juce::Justification::centred);
    g.drawText ("R E A R", juce::Rectangle<float> (centre.x - 55.0f, face.getBottom() - 30.0f, 110.0f, 18.0f), juce::Justification::centred);
    g.drawText ("L E F T", juce::Rectangle<float> (face.getX() + 6.0f, centre.y - 10.0f, 70.0f, 20.0f), juce::Justification::centredLeft);
    g.drawText ("R I G H T", juce::Rectangle<float> (face.getRight() - 82.0f, centre.y - 10.0f, 76.0f, 20.0f), juce::Justification::centredRight);

    const auto drawPole = [&] (juce::Point<float> point, float brightness)
    {
        g.setColour (juce::Colour (0x66000000));
        g.fillEllipse (point.x - 4.0f, point.y - 1.0f, 10.0f, 10.0f);
        g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.68f, 0.18f, 0.3f + brightness * 0.7f));
        g.fillEllipse (point.x - 4.0f, point.y - 4.0f, 8.0f, 8.0f);
        g.setColour (juce::Colour (0xff5b2d0b));
        g.drawEllipse (point.x - 4.0f, point.y - 4.0f, 8.0f, 8.0f, 1.0f);
    };
    drawPole ({ centre.x, centre.y - radius }, 1.0f - rear);
    drawPole ({ centre.x, centre.y + radius }, rear);

    const auto needleAngle = phase * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
    const juce::Point<float> direction { std::cos (needleAngle), std::sin (needleAngle) };
    const juce::Point<float> perpendicular { -direction.y, direction.x };
    const auto tip = centre + direction * (radius * 0.87f);
    const auto baseA = centre + perpendicular * 15.0f - direction * 4.0f;
    const auto baseB = centre - perpendicular * 15.0f - direction * 4.0f;
    const auto ridge = centre + direction * (radius * 0.67f);

    const juce::Point<float> shadowOffset { 8.0f + direction.y * 5.0f,
                                            12.0f - direction.x * 4.0f };
    const auto shadowCentre = centre + shadowOffset * 0.62f;
    const auto shadowTip = tip + shadowOffset * 1.45f;
    const auto shadowRear = centre - direction * 10.0f + shadowOffset * 0.52f;
    juce::Path needleShadow;
    needleShadow.startNewSubPath (shadowRear);
    needleShadow.lineTo (shadowCentre + perpendicular * 10.0f + direction * 3.0f);
    needleShadow.lineTo (shadowTip);
    needleShadow.lineTo (shadowCentre - perpendicular * 10.0f + direction * 3.0f);
    needleShadow.closeSubPath();
    g.setColour (juce::Colour (0x80000000));
    g.fillPath (needleShadow);

    const auto spineBase = centre - direction * 3.0f;
    juce::Path brightFace;
    brightFace.startNewSubPath (baseA);
    brightFace.lineTo (tip);
    brightFace.lineTo (spineBase);
    brightFace.closeSubPath();
    juce::ColourGradient brightMetal (juce::Colour (0xffffedbd), baseA.x, baseA.y,
                                      juce::Colour (0xffa86c28), tip.x, tip.y, false);
    g.setGradientFill (brightMetal);
    g.fillPath (brightFace);

    juce::Path convexFace;
    convexFace.startNewSubPath (spineBase);
    convexFace.lineTo (tip);
    convexFace.lineTo (baseB);
    convexFace.closeSubPath();
    juce::ColourGradient convexMetal (juce::Colour (0xff7a4718), spineBase.x, spineBase.y,
                                      juce::Colour (0xff160d07), baseB.x, baseB.y, false);
    convexMetal.addColour (0.48, juce::Colour (0xff3b220e));
    g.setGradientFill (convexMetal);
    g.fillPath (convexFace);

    juce::Path needleOutline;
    needleOutline.startNewSubPath (baseA);
    needleOutline.lineTo (tip);
    needleOutline.lineTo (baseB);
    needleOutline.closeSubPath();
    g.setColour (juce::Colour (0xffffd17a));
    g.drawLine (centre.x, centre.y, ridge.x, ridge.y, 1.2f);
    g.setColour (juce::Colour (0xff4b2a10));
    g.strokePath (needleOutline, juce::PathStrokeType (1.2f));

    g.setColour (juce::Colour (0xaa000000));
    g.fillEllipse (centre.x - 17.0f + 4.0f, centre.y - 17.0f + 5.0f, 34.0f, 34.0f);
    juce::ColourGradient hubMetal (juce::Colour (0xffffd378), centre.x - 12.0f, centre.y - 12.0f,
                                   juce::Colour (0xff603511), centre.x + 12.0f, centre.y + 12.0f, true);
    g.setGradientFill (hubMetal);
    g.fillEllipse (centre.x - 16.0f, centre.y - 16.0f, 32.0f, 32.0f);
    g.setColour (juce::Colour (0xffffd37b));
    g.drawEllipse (centre.x - 16.0f, centre.y - 16.0f, 32.0f, 32.0f, 1.5f);

    juce::Path glassReflection;
    const auto reflectionBounds = face.reduced (12.0f);
    glassReflection.addArc (reflectionBounds.getX(), reflectionBounds.getY(),
                            reflectionBounds.getWidth(), reflectionBounds.getHeight(),
                            3.55f, 5.35f, true);
    g.setColour (juce::Colour (0x26ffffff));
    g.strokePath (glassReflection, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved));

    g.setColour (juce::Colour (0xffd4a55d));
    g.setFont (juce::FontOptions (12.0f));
    auto rateReadout = getLocalBounds().removeFromBottom (15);
    g.drawText (juce::String (rateHz, 2) + " REV/S", rateReadout,
                juce::Justification::centredBottom);
}

OrbitAudioProcessorEditor::OrbitAudioProcessorEditor (OrbitAudioProcessor& owner)
    : AudioProcessorEditor (&owner),
      processor (owner),
      orbitDisplay (owner),
      dopplerSwitch (owner.parameters, "doppler", { "OFF", "1/2", "1" }),
      eventGridSwitch (owner.parameters, "eventGrid", { "1", "1/10" }),
      sourceAttachment (owner.parameters, "source", sourceBox),
      rhythmAttachment (owner.parameters, "rhythm", rhythmBox),
      modelAttachment (owner.parameters, "model", modelBox),
      itdAttachment (owner.parameters, "itd", itdButton),
      frontLobesAttachment (owner.parameters, "frontLobes", frontLobesButton),
      syncAttachment (owner.parameters, "sync", syncButton),
      swirlAttachment (owner.parameters, "swirl", swirlButton),
      divisionAttachment (owner.parameters, "division", divisionBox),
      freeRateAttachment (owner.parameters, "freeRate", freeRateSlider),
      widthAttachment (owner.parameters, "width", widthSlider),
      depthAttachment (owner.parameters, "depth", depthSlider),
      rearBlendAttachment (owner.parameters, "rearBlend", rearBlendSlider),
      spectralArcAttachment (owner.parameters, "spectralArc", spectralArcSlider),
      frontSpaceAttachment (owner.parameters, "frontSpace", frontSpaceSlider),
      chanceAttachment (owner.parameters, "chance", chanceSlider),
      mixAttachment (owner.parameters, "mix", mixSlider),
      frequencyAttachment (owner.parameters, "frequency", frequencySlider)
{
    setLookAndFeel (&lookAndFeel);
    setSize (1100, 780);
    setResizable (true, true);
    setResizeLimits (1000, 730, 1450, 1000);

    sourceBox.addItemList ({ "Input", "Sine", "Saw", "White Noise", "Loaded WAV" }, 1);
    rhythmBox.addItemList ({ "Continuous", "1/8 notes", "1/16 notes" }, 1);
    modelBox.addItemList ({ "Original", "Refined" }, 1);
    divisionBox.addItemList (OrbitAudioProcessor::divisionNames(), 1);
    configureKnob (freeRateSlider, " Hz");
    configureKnob (widthSlider);
    configureKnob (depthSlider);
    configureKnob (rearBlendSlider);
    configureKnob (spectralArcSlider, " deg");
    configureKnob (frontSpaceSlider);
    configureKnob (chanceSlider, "%");
    configureKnob (mixSlider);
    configureKnob (frequencySlider, " Hz");
    freeRateSlider.setNumDecimalPlacesToDisplay (2);
    freeRateSlider.textFromValueFunction = [] (double value)
    {
        return juce::String (value, 2);
    };
    freeRateSlider.updateText();
    for (auto* knob : { &widthSlider, &depthSlider, &chanceSlider, &mixSlider })
        knob->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

    configureLabel (sourceLabel, "SOURCE");
    loadedFileLabel.setText ("No WAV loaded", juce::dontSendNotification);
    loadedFileLabel.setJustificationType (juce::Justification::centred);
    loadedFileLabel.setColour (juce::Label::textColourId, juce::Colour (0xff8e7654));
    loadedFileLabel.setFont (juce::FontOptions (11.0f));
    configureLabel (rhythmLabel, "TEST RHYTHM");
    configureLabel (modelLabel, "SPATIAL MODEL");
    configureLabel (speedLabel, "SPEED");
    configureLabel (widthLabel, "WIDTH");
    configureLabel (depthLabel, "DEPTH");
    configureLabel (rearBlendLabel, "REAR BLEND");
    configureLabel (spectralArcLabel, "SPECTRAL ARC");
    configureLabel (frontSpaceLabel, "FRONT SPACE");
    configureLabel (dopplerLabel, "DOPPLER");
    configureLabel (chanceLabel, "CHANCE");
    configureLabel (mixLabel, "MIX");
    configureLabel (frequencyLabel, "TEST TONE");
    configureLabel (eventGridLabel, "EVENT GRID");
    eventOddsLabel.setText ("HALF 25%   DOUBLE 25%   SPEED RUN 15%   REVERSE 10%   NOTHING 25%",
                            juce::dontSendNotification);
    eventOddsLabel.setJustificationType (juce::Justification::centredLeft);
    eventOddsLabel.setColour (juce::Label::textColourId, juce::Colour (0xffb9955d));
    eventOddsLabel.setFont (juce::FontOptions (12.0f));

    for (auto* component : std::initializer_list<juce::Component*> {
             &orbitDisplay, &advancedButton, &factoryResetButton, &sourceBox, &loadWavButton, &loadedFileLabel,
             &rhythmBox, &modelBox, &itdButton, &frontLobesButton,
             &syncButton, &swirlButton, &divisionBox, &freeRateSlider,
             &widthSlider, &depthSlider, &rearBlendSlider, &spectralArcSlider, &frontSpaceSlider,
             &dopplerSwitch, &chanceSlider, &mixSlider, &frequencySlider,
             &eventGridSwitch,
             &sourceLabel, &rhythmLabel, &modelLabel, &speedLabel, &widthLabel,
             &depthLabel, &rearBlendLabel, &spectralArcLabel, &frontSpaceLabel,
             &dopplerLabel, &chanceLabel, &mixLabel, &frequencyLabel,
             &eventGridLabel, &eventOddsLabel })
        addAndMakeVisible (*component);

    syncButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffd7aa62));
    swirlButton.setClickingTogglesState (true);
    swirlButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff24190f));
    swirlButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffd7aa62));
    swirlButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffffd98c));
    swirlButton.setTooltip ("Cycle orbit speed continuously from 0.5x to 2x at one tenth of its instantaneous speed");
    itdButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffd7aa62));
    frontLobesButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffd7aa62));
    loadWavButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff24190f));
    loadWavButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffd7aa62));
    factoryResetButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff24190f));
    factoryResetButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffd7aa62));
    factoryResetButton.setTooltip ("Restore the factory spatial calibration settings");
    advancedButton.setName ("orbButton");
    advancedButton.setColour (juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    advancedButton.setTooltip ("Open listener calibration and test controls");
    advancedButton.onClick = [this]
    {
        advancedVisible = ! advancedVisible;
        updateAdvancedVisibility();
        resized();
        repaint();
    };
    factoryResetButton.onClick = [this]
    {
        for (const auto* parameterId : { "model", "itd", "frontLobes",
                                         "rearBlend", "spectralArc", "frontSpace" })
        {
            if (auto* parameter = processor.parameters.getParameter (parameterId))
            {
                parameter->beginChangeGesture();
                parameter->setValueNotifyingHost (parameter->getDefaultValue());
                parameter->endChangeGesture();
            }
        }
    };
    loadWavButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Load a WAV into BLORB", juce::File(), "*.wav");
        auto safeEditor = juce::Component::SafePointer<OrbitAudioProcessorEditor> (this);
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                                      | juce::FileBrowserComponent::canSelectFiles,
                                  [safeThis = std::move (safeEditor)]
                                  (const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr)
                return;

            const auto file = chooser.getResult();
            if (! file.existsAsFile())
                return;

            juce::String error;
            if (safeThis->processor.loadAudioFile (file, error))
            {
                safeThis->loadedFileLabel.setText (file.getFileName(), juce::dontSendNotification);
                if (auto* parameter = safeThis->processor.parameters.getParameter ("source"))
                    parameter->setValueNotifyingHost (parameter->convertTo0to1 (4.0f));
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                                                        "Could not load WAV", error);
            }
        });
    };
    updateAdvancedVisibility();
    startTimerHz (10);
    timerCallback();
}

OrbitAudioProcessorEditor::~OrbitAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void OrbitAudioProcessorEditor::configureKnob (juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 20);
    slider.setTextValueSuffix (suffix);
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffd18a2e));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff3b2b1c));
    slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xffffca68));
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe4bc78));
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff15110d));
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff3d2b1b));
}

void OrbitAudioProcessorEditor::configureLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffd7aa62));
    label.setFont (juce::FontOptions (13.0f).withStyle ("Bold"));
}

void OrbitAudioProcessorEditor::updateAdvancedVisibility()
{
    for (auto* component : std::initializer_list<juce::Component*> {
             &sourceBox, &loadWavButton, &loadedFileLabel, &rhythmBox, &modelBox,
             &itdButton, &frontLobesButton, &frequencySlider,
             &sourceLabel, &rhythmLabel, &modelLabel, &frequencyLabel,
             &rearBlendSlider, &spectralArcSlider, &frontSpaceSlider,
             &rearBlendLabel, &spectralArcLabel, &frontSpaceLabel,
             &factoryResetButton, &eventOddsLabel })
        component->setVisible (advancedVisible);

    swirlButton.setVisible (! advancedVisible);
}

void OrbitAudioProcessorEditor::timerCallback()
{
    const auto synced = processor.parameters.getRawParameterValue ("sync")->load() >= 0.5f;
    divisionBox.setVisible (synced);
    freeRateSlider.setVisible (! synced);

    const auto source = juce::roundToInt (processor.parameters.getRawParameterValue ("source")->load());
    frequencySlider.setEnabled (source == 1 || source == 2);
    frequencyLabel.setEnabled (source == 1 || source == 2);
    rhythmBox.setEnabled (source >= 1 && source <= 3);
    rhythmLabel.setEnabled (source >= 1 && source <= 3);

    const auto refined = processor.parameters.getRawParameterValue ("model")->load() >= 0.5f;
    itdButton.setEnabled (refined);
    frontLobesButton.setEnabled (refined);
    rearBlendSlider.setEnabled (refined);
    rearBlendLabel.setEnabled (refined);
    spectralArcSlider.setEnabled (refined);
    spectralArcLabel.setEnabled (refined);
    frontSpaceSlider.setEnabled (refined);
    frontSpaceLabel.setEnabled (refined);
    dopplerSwitch.setEnabled (refined);
    dopplerLabel.setEnabled (refined);
}

void OrbitAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient panel (juce::Colour (0xff19130e), 0.0f, 0.0f,
                                juce::Colour (nearBlack), 0.0f, static_cast<float> (getHeight()), false);
    g.setGradientFill (panel);
    g.fillRect (getLocalBounds());

    auto frame = getLocalBounds().toFloat().reduced (14.0f);
    g.setColour (juce::Colour (0xff080604));
    g.fillRoundedRectangle (frame, 11.0f);
    g.setColour (juce::Colour (0xff5b3d1d));
    g.drawRoundedRectangle (frame, 11.0f, 2.0f);
    g.setColour (juce::Colour (0x607e5426));
    g.drawRoundedRectangle (frame.reduced (7.0f), 8.0f, 1.0f);

    const auto drawScrew = [&] (float x, float y)
    {
        g.setColour (juce::Colour (0xff0a0705));
        g.fillEllipse (x - 7.0f, y - 6.0f, 16.0f, 16.0f);
        juce::ColourGradient screw (juce::Colour (0xffd29235), x - 5.0f, y - 5.0f,
                                    juce::Colour (0xff3d2510), x + 6.0f, y + 6.0f, true);
        g.setGradientFill (screw);
        g.fillEllipse (x - 6.0f, y - 6.0f, 12.0f, 12.0f);
        g.setColour (juce::Colour (0xff2b180b));
        g.drawLine (x - 3.0f, y - 3.0f, x + 3.0f, y + 3.0f, 1.1f);
    };
    drawScrew (29.0f, 29.0f); drawScrew (getWidth() - 29.0f, 29.0f);
    drawScrew (29.0f, getHeight() - 29.0f); drawScrew (getWidth() - 29.0f, getHeight() - 29.0f);

    g.setColour (juce::Colour (0xffe1b66d));
    g.setFont (juce::FontOptions (36.0f).withStyle ("Bold"));
    g.drawText ("BL", 38, 20, 58, 42, juce::Justification::centredRight);
    g.drawText ("RB", 132, 20, 72, 42, juce::Justification::centredLeft);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("BINAURAL LOCATION & ORBITAL ROTATION BOX", 40, 59, 340, 18,
                juce::Justification::centredLeft);

    auto switchPanel = juce::Rectangle<float> (static_cast<float> (getWidth() - 178), 135.0f, 136.0f, 292.0f);
    g.setColour (juce::Colour (0xaa0a0806));
    g.fillRoundedRectangle (switchPanel, 9.0f);
    g.setColour (juce::Colour (0xff543719));
    g.drawRoundedRectangle (switchPanel, 9.0f, 1.3f);

    auto swirlPanel = juce::Rectangle<float> (42.0f, 238.0f, 136.0f, 92.0f);
    g.setColour (juce::Colour (0xaa0a0806));
    g.fillRoundedRectangle (swirlPanel, 9.0f);
    g.setColour (juce::Colour (0xff543719));
    g.drawRoundedRectangle (swirlPanel, 9.0f, 1.3f);
    g.setColour (juce::Colour (0xffb98a4e));
    g.setFont (juce::FontOptions (10.0f));
    g.drawText ("SPEED MOTION", swirlPanel.removeFromTop (25.0f).toNearestInt(),
                juce::Justification::centred);

    auto controlDeck = juce::Rectangle<float> (28.0f, static_cast<float> (getHeight() - 158),
                                                static_cast<float> (getWidth() - 56), 122.0f);
    g.setColour (juce::Colour (0x99100c09));
    g.fillRoundedRectangle (controlDeck, 10.0f);
    g.setColour (juce::Colour (0xff4b3219));
    g.drawRoundedRectangle (controlDeck, 10.0f, 1.2f);

    if (advancedVisible)
    {
        auto advancedPanel = juce::Rectangle<float> (26.0f, 84.0f, 258.0f, 520.0f);
        g.setColour (juce::Colour (0xef0a0806));
        g.fillRoundedRectangle (advancedPanel, 8.0f);
        g.setColour (juce::Colour (0xff735027));
        g.drawRoundedRectangle (advancedPanel, 8.0f, 1.4f);
        g.setColour (juce::Colour (0xffd2a35f));
        g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
        g.drawText ("LISTENER CALIBRATION / TEST BENCH", advancedPanel.toNearestInt().removeFromTop (28),
                    juce::Justification::centred);
    }
}

void OrbitAudioProcessorEditor::resized()
{
    const auto w = getWidth();
    const auto h = getHeight();
    advancedButton.setBounds (92, 19, 42, 42);

    const auto dialSize = juce::jmin (560, h - 200);
    orbitDisplay.setBounds ((w - dialSize) / 2, 48, dialSize, dialSize);

    dopplerLabel.setBounds (w - 176, 153, 126, 20);
    dopplerSwitch.setBounds (w - 170, 177, 114, 82);
    eventGridLabel.setBounds (w - 176, 299, 126, 20);
    eventGridSwitch.setBounds (w - 170, 323, 114, 82);

    auto controls = juce::Rectangle<int> (40, h - 150, w - 80, 108);
    const auto cellWidth = controls.getWidth() / 5;
    auto speedCell = controls.removeFromLeft (cellWidth).reduced (14, 2);
    auto widthCell = controls.removeFromLeft (cellWidth).reduced (14, 2);
    auto depthCell = controls.removeFromLeft (cellWidth).reduced (14, 2);
    auto mixCell = controls.removeFromLeft (cellWidth).reduced (14, 2);
    auto chanceCell = controls.reduced (14, 2);

    speedLabel.setBounds (speedCell.removeFromTop (18));
    auto speedModeRow = speedCell.removeFromTop (22);
    syncButton.setBounds (speedModeRow);
    swirlButton.setBounds (62, 274, 96, 32);
    divisionBox.setBounds (speedCell.withSizeKeepingCentre (speedCell.getWidth() - 14, 28));
    freeRateSlider.setBounds (speedCell);
    widthLabel.setBounds (widthCell.removeFromTop (18));
    widthSlider.setBounds (widthCell);
    depthLabel.setBounds (depthCell.removeFromTop (18));
    depthSlider.setBounds (depthCell);
    mixLabel.setBounds (mixCell.removeFromTop (18));
    mixSlider.setBounds (mixCell);
    chanceLabel.setBounds (chanceCell.removeFromTop (18));
    chanceSlider.setBounds (chanceCell);

    auto advanced = juce::Rectangle<int> (46, 126, 238, 470);
    sourceLabel.setBounds (advanced.removeFromTop (18));
    sourceBox.setBounds (advanced.removeFromTop (25));
    loadWavButton.setBounds (advanced.removeFromTop (25));
    loadedFileLabel.setBounds (advanced.removeFromTop (17));
    frequencyLabel.setBounds (advanced.removeFromTop (18));
    frequencySlider.setBounds (advanced.removeFromTop (54));
    rhythmLabel.setBounds (advanced.removeFromTop (18));
    rhythmBox.setBounds (advanced.removeFromTop (25));
    modelLabel.setBounds (advanced.removeFromTop (18));
    modelBox.setBounds (advanced.removeFromTop (25));
    itdButton.setBounds (advanced.removeFromTop (22));
    frontLobesButton.setBounds (advanced.removeFromTop (22));
    advanced.removeFromTop (6);

    auto calibration = advanced.removeFromTop (98);
    const auto calibrationWidth = calibration.getWidth() / 3;
    auto rearCell = calibration.removeFromLeft (calibrationWidth).reduced (2);
    auto arcCell = calibration.removeFromLeft (calibrationWidth).reduced (2);
    auto frontCell = calibration.reduced (2);
    rearBlendLabel.setBounds (rearCell.removeFromTop (18));
    rearBlendSlider.setBounds (rearCell);
    spectralArcLabel.setBounds (arcCell.removeFromTop (18));
    spectralArcSlider.setBounds (arcCell);
    frontSpaceLabel.setBounds (frontCell.removeFromTop (18));
    frontSpaceSlider.setBounds (frontCell);
    advanced.removeFromTop (4);
    factoryResetButton.setBounds (advanced.removeFromTop (26));
    advanced.removeFromTop (3);
    eventOddsLabel.setBounds (advanced.removeFromTop (42));
}
