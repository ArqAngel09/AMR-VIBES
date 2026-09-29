#include <JuceHeader.h>
#include "PluginProcessor.h"

class AMRVibesMainComponent final : public juce::Component
{
public:
    AMRVibesMainComponent()
    {
        editor.reset (processor.createEditor());
        addAndMakeVisible (*editor);
        setSize (620, 520);

        juce::Component::SafePointer<AMRVibesMainComponent> safeThis (this);
        juce::RuntimePermissions::request (
            juce::RuntimePermissions::recordAudio,
            [safeThis] (bool granted)
            {
                if (auto* self = safeThis.getComponent())
                    self->startAudio (granted);
            });
    }

    ~AMRVibesMainComponent() override
    {
        stopAudio();
        editor.reset();
    }

    void resized() override
    {
        if (editor != nullptr)
            editor->setBounds (getLocalBounds());
    }

private:
    void startAudio (bool microphoneGranted)
    {
        if (! microphoneGranted)
            return;

        const auto error = deviceManager.initialise (1, 2, nullptr, true, {}, nullptr);

        if (error.isEmpty())
        {
            player.setProcessor (&processor);
            deviceManager.addAudioCallback (&player);
        }
    }

    void stopAudio()
    {
        deviceManager.removeAudioCallback (&player);
        player.setProcessor (nullptr);
        deviceManager.closeAudioDevice();
    }

    juce::AudioDeviceManager deviceManager;
    juce::AudioProcessorPlayer player;
    AMRVibesAudioProcessor processor;
    std::unique_ptr<juce::AudioProcessorEditor> editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AMRVibesMainComponent)
};

class AMRVibesApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "AMR VIBES"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    void initialise (const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow> ("AMR VIBES", *this);
    }

    void shutdown() override { mainWindow.reset(); }

    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted (const juce::String&) override {}

private:
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        MainWindow (const juce::String& name, AMRVibesApplication& app)
            : juce::DocumentWindow (name, juce::Colours::black,
                                    juce::DocumentWindow::allButtons),
              owner (app)
        {
            setUsingNativeTitleBar (false);
            setContentOwned (new AMRVibesMainComponent(), true);
            setResizable (true, false);
            centreWithSize (620, 520);
            setVisible (true);
        }

        void closeButtonPressed() override { owner.systemRequestedQuit(); }

    private:
        AMRVibesApplication& owner;
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (AMRVibesApplication)
