/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "LightWorker.h"


//==============================================================================
/**
*/
class LightControlAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    LightControlAudioProcessor();
    ~LightControlAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    LightWorker lights;

    // Window layout, stored in the project (not in the global defaults). Only touched from the message thread.
    std::atomic<int> uiWidth { 780 };
    std::atomic<int> uiHeight { 560 };
    std::atomic<bool> uiSettingsVisible { true };

    /** Global defaults, used by new plugin instances (a saved project state overrides them). */
    static juce::File getDefaultsFile();
    bool saveAsDefaults() const;        // returns false if the file couldn't be written
    void loadDefaults();                // the defaults file, or factory settings if there is none

private:
    std::unique_ptr<juce::XmlElement> createStateXml (bool includeUi) const;
    void applyStateXml (const juce::XmlElement& xml, bool includeUi);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightControlAudioProcessor)
};
