#include "PluginProcessor.h"
#include "PluginEditor.h"

#if JUCE_WINDOWS
 #include <windows.h>
 #include <xinput.h>
#endif

static const char* const macroIds[4] = { "lx", "ly", "rx", "ry" };

//==============================================================================
// Polls controller 1 and writes stick positions into the parameters.
class XboxMacroProcessor::GamepadThread : public juce::Thread
{
public:
    explicit GamepadThread (XboxMacroProcessor& p) : juce::Thread ("Xbox poll"), proc (p) {}

    void run() override
    {
       #if JUCE_WINDOWS
        float last[4] {};
        constexpr float deadzone = 0.12f;

        auto norm = [&] (SHORT raw)
        {
            float v = juce::jlimit (-1.0f, 1.0f, (float) raw / 32767.0f);
            float a = std::abs (v);
            if (a < deadzone) return 0.0f;
            return (v < 0 ? -1.0f : 1.0f) * (a - deadzone) / (1.0f - deadzone);
        };

        while (! threadShouldExit())
        {
            XINPUT_STATE s {};
            const bool ok = XInputGetState (0, &s) == ERROR_SUCCESS;
            proc.padConnected = ok;

            if (ok)
            {
                const float now[4] = { norm (s.Gamepad.sThumbLX), norm (s.Gamepad.sThumbLY),
                                       norm (s.Gamepad.sThumbRX), norm (s.Gamepad.sThumbRY) };
                const bool latch = proc.apvts.getRawParameterValue ("latch")->load() > 0.5f;

                for (int stick = 0; stick < 2; ++stick)
                {
                    const int ix = stick * 2, iy = ix + 1;
                    const bool moved = now[ix] != last[ix] || now[iy] != last[iy];
                    const bool centred = now[ix] == 0.0f && now[iy] == 0.0f;

                    // Latch: hold last value when the stick is released.
                    // Absolute: value follows the stick and springs back to centre.
                    if (moved && ! (latch && centred))
                        for (int i : { ix, iy })
                            if (auto* p = proc.apvts.getParameter (macroIds[i]))
                                p->setValueNotifyingHost (0.5f + 0.5f * now[i]);

                    last[ix] = now[ix];
                    last[iy] = now[iy];
                }
            }
            wait (4); // ~250 Hz
        }
       #endif
    }

private:
    XboxMacroProcessor& proc;
};

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout XboxMacroProcessor::createLayout()
{
    const char* names[4] = { "Left Macro 1 (X)", "Left Macro 2 (Y)",
                             "Right Macro 1 (X)", "Right Macro 2 (Y)" };
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 0; i < 4; ++i)
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { macroIds[i], 1 }, names[i],
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "latch", 1 }, "Latch on release", false));

    return layout;
}

XboxMacroProcessor::XboxMacroProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    for (int i = 0; i < 4; ++i)
        macro[i] = apvts.getRawParameterValue (macroIds[i]);

   #if JUCE_WINDOWS
    gamepad = std::make_unique<GamepadThread> (*this);
    gamepad->startThread();
   #endif
}

XboxMacroProcessor::~XboxMacroProcessor()
{
    if (gamepad != nullptr)
        gamepad->stopThread (500);
}

bool XboxMacroProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == l.getMainOutputChannelSet()
        && ! l.getMainOutputChannelSet().isDisabled();
}

void XboxMacroProcessor::processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    // Audio passes straight through; macros go out as MIDI CC when they change.
    for (int i = 0; i < 4; ++i)
    {
        const int v = juce::jlimit (0, 127, juce::roundToInt (macro[i]->load() * 127.0f));
        if (v != lastCC[i])
        {
            midi.addEvent (juce::MidiMessage::controllerEvent (1, ccBase + i, v), 0);
            lastCC[i] = v;
        }
    }
}

juce::AudioProcessorEditor* XboxMacroProcessor::createEditor()
{
    return new XboxMacroEditor (*this);
}

void XboxMacroProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void XboxMacroProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XboxMacroProcessor();
}
