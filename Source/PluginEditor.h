#pragma once
#include "PluginProcessor.h"

// One XY pad = one joystick: X axis -> macro 1, Y axis -> macro 2.
// Shows the live stick position and can also be dragged with the mouse.
class XYPad : public juce::Component
{
public:
    XYPad (juce::RangedAudioParameter& xParam, juce::RangedAudioParameter& yParam, juce::String title);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void setFromMouse (juce::Point<float>);

    juce::RangedAudioParameter& xp;
    juce::RangedAudioParameter& yp;
    juce::String label;
};

class XboxMacroEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit XboxMacroEditor (XboxMacroProcessor&);
    ~XboxMacroEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    XboxMacroProcessor& proc;
    XYPad leftPad, rightPad;
    juce::ToggleButton latchButton { "Latch on release" };
    juce::AudioProcessorValueTreeState::ButtonAttachment latchAttach;
    bool wasConnected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XboxMacroEditor)
};
