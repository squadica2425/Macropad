#include "PluginEditor.h"

//==============================================================================
XYPad::XYPad (juce::RangedAudioParameter& xParam, juce::RangedAudioParameter& yParam, juce::String title)
    : xp (xParam), yp (yParam), label (std::move (title))
{
}

void XYPad::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced (1.0f);

    g.setColour (juce::Colour (0xff1b1d23));
    g.fillRoundedRectangle (area, 8.0f);

    // grid
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    for (int i = 1; i < 8; ++i)
    {
        const float fx = area.getX() + area.getWidth()  * (float) i / 8.0f;
        const float fy = area.getY() + area.getHeight() * (float) i / 8.0f;
        g.drawLine (fx, area.getY(), fx, area.getBottom());
        g.drawLine (area.getX(), fy, area.getRight(), fy);
    }

    // centre cross
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawLine (area.getCentreX(), area.getY(), area.getCentreX(), area.getBottom());
    g.drawLine (area.getX(), area.getCentreY(), area.getRight(), area.getCentreY());

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawRoundedRectangle (area, 8.0f, 1.0f);

    // puck
    const float px = area.getX() + xp.getValue() * area.getWidth();
    const float py = area.getBottom() - yp.getValue() * area.getHeight();
    g.setColour (juce::Colour (0xff44d1a8));
    g.fillEllipse (px - 9.0f, py - 9.0f, 18.0f, 18.0f);
    g.setColour (juce::Colours::white);
    g.drawEllipse (px - 9.0f, py - 9.0f, 18.0f, 18.0f, 1.5f);

    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.setFont (13.0f);
    g.drawText (label, getLocalBounds().reduced (8, 6), juce::Justification::topLeft);
}

void XYPad::setFromMouse (juce::Point<float> p)
{
    auto area = getLocalBounds().toFloat().reduced (1.0f);
    const float nx = juce::jlimit (0.0f, 1.0f, (p.x - area.getX()) / area.getWidth());
    const float ny = juce::jlimit (0.0f, 1.0f, 1.0f - (p.y - area.getY()) / area.getHeight());
    xp.setValueNotifyingHost (nx);
    yp.setValueNotifyingHost (ny);
    repaint();
}

void XYPad::mouseDown (const juce::MouseEvent& e)
{
    xp.beginChangeGesture();
    yp.beginChangeGesture();
    setFromMouse (e.position);
}

void XYPad::mouseDrag (const juce::MouseEvent& e) { setFromMouse (e.position); }

void XYPad::mouseUp (const juce::MouseEvent&)
{
    xp.endChangeGesture();
    yp.endChangeGesture();
}

//==============================================================================
XboxMacroEditor::XboxMacroEditor (XboxMacroProcessor& p)
    : juce::AudioProcessorEditor (&p),
      proc (p),
      leftPad  (*p.apvts.getParameter ("lx"), *p.apvts.getParameter ("ly"), "Left stick: Macro 1 (X) / Macro 2 (Y)"),
      rightPad (*p.apvts.getParameter ("rx"), *p.apvts.getParameter ("ry"), "Right stick: Macro 1 (X) / Macro 2 (Y)"),
      latchAttach (p.apvts, "latch", latchButton)
{
    addAndMakeVisible (leftPad);
    addAndMakeVisible (rightPad);
    addAndMakeVisible (latchButton);

    setSize (660, 400);
    startTimerHz (30);
}

void XboxMacroEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff121419));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("Xbox Macro Pad", 16, 8, 300, 28, juce::Justification::centredLeft);

    const bool connected = proc.padConnected.load();
    g.setFont (13.0f);
    g.setColour (connected ? juce::Colour (0xff44d1a8) : juce::Colour (0xffe5645a));
    g.drawText (connected ? "Controller connected" : "No controller found (Windows XInput)",
                getWidth() - 316, 8, 300, 28, juce::Justification::centredRight);
}

void XboxMacroEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (30);
    auto bottom = area.removeFromBottom (30);
    latchButton.setBounds (bottom.removeFromLeft (200));

    auto left = area.removeFromLeft ((area.getWidth() - 16) / 2);
    area.removeFromLeft (16);
    leftPad.setBounds (left);
    rightPad.setBounds (area);
}

void XboxMacroEditor::timerCallback()
{
    leftPad.repaint();
    rightPad.repaint();

    const bool now = proc.padConnected.load();
    if (now != wasConnected)
    {
        wasConnected = now;
        repaint();
    }
}
