#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <memory>

// 4 macros: each joystick is an XY pad, X = macro 1, Y = macro 2.
// They are normal automatable parameters AND get sent out as MIDI CC.
class XboxMacroProcessor : public juce::AudioProcessor
{
public:
    XboxMacroProcessor();
    ~XboxMacroProcessor() override;

    static constexpr int ccBase = 20; // CC 20..23 = L-X, L-Y, R-X, R-Y

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Xbox Macro Pad"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<bool> padConnected { false };

private:
    class GamepadThread;
    std::unique_ptr<GamepadThread> gamepad;

    std::atomic<float>* macro[4] {};
    int lastCC[4] { -1, -1, -1, -1 };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XboxMacroProcessor)
};
