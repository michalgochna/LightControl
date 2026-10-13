/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
LightControlAudioProcessor::LightControlAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    loadDefaults();   // a project that has its own saved state overrides this in setStateInformation()
    lights.start();
}

LightControlAudioProcessor::~LightControlAudioProcessor()
{
}

//==============================================================================
const juce::String LightControlAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LightControlAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LightControlAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LightControlAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LightControlAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LightControlAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LightControlAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LightControlAudioProcessor::setCurrentProgram (int)
{
}

const juce::String LightControlAudioProcessor::getProgramName (int)
{
    return {};
}

void LightControlAudioProcessor::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void LightControlAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void LightControlAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LightControlAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LightControlAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // The plugin doesn't touch the audio, it only passes it through.
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto state = TransportState::Stopped;

    if (auto* head = getPlayHead())
    {
        if (auto position = head->getPosition())
        {
            if (position->getIsRecording())
                state = TransportState::Recording;
            else if (position->getIsPlaying())
                state = TransportState::Playing;
        }
    }

    lights.notifyBlock (state);
}

//==============================================================================
bool LightControlAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LightControlAudioProcessor::createEditor()
{
    return new LightControlAudioProcessorEditor (*this);
}

//==============================================================================
std::unique_ptr<juce::XmlElement> LightControlAudioProcessor::createStateXml (bool includeUi) const
{
    auto xml = std::make_unique<juce::XmlElement> ("LightControl");
    xml->setAttribute ("colourRecording", lights.getColour (TransportState::Recording).toString());
    xml->setAttribute ("colourPlaying",   lights.getColour (TransportState::Playing).toString());
    xml->setAttribute ("colourStopped",   lights.getColour (TransportState::Stopped).toString());
    xml->setAttribute ("openRgbEnabled", lights.isOpenRgbEnabled());
    xml->setAttribute ("host", lights.getHost());
    xml->setAttribute ("port", lights.getPort());
    xml->setAttribute ("disabledDevices", lights.getDisabledDevices().joinIntoString ("\n"));

    const auto art = lights.getArtNetConfig();
    xml->setAttribute ("artEnabled", art.enabled);
    xml->setAttribute ("artIp", art.ip);
    xml->setAttribute ("artUniverse", art.universe);
    xml->setAttribute ("artStartChannel", art.startChannel);
    xml->setAttribute ("artFixtures", art.fixtures);
    xml->setAttribute ("artLayout", (int) art.layout);

    if (includeUi)
    {
        xml->setAttribute ("uiWidth", uiWidth.load());
        xml->setAttribute ("uiHeight", uiHeight.load());
        xml->setAttribute ("uiSettingsVisible", uiSettingsVisible.load());
    }

    return xml;
}

void LightControlAudioProcessor::applyStateXml (const juce::XmlElement& xml, bool includeUi)
{
    if (! xml.hasTagName ("LightControl"))
        return;

    const auto restore = [&] (TransportState s, const char* attr)
    {
        if (xml.hasAttribute (attr))
            lights.setColour (s, juce::Colour::fromString (xml.getStringAttribute (attr)));
    };
    restore (TransportState::Recording, "colourRecording");
    restore (TransportState::Playing,   "colourPlaying");
    restore (TransportState::Stopped,   "colourStopped");

    if (xml.hasAttribute ("openRgbEnabled"))
        lights.setOpenRgbEnabled (xml.getBoolAttribute ("openRgbEnabled", true));

    if (xml.hasAttribute ("host"))
        lights.setServer (xml.getStringAttribute ("host"), xml.getIntAttribute ("port", 6742));

    lights.setDisabledDevices (juce::StringArray::fromLines (xml.getStringAttribute ("disabledDevices")));

    ArtNetConfig art;
    art.enabled      = xml.getBoolAttribute ("artEnabled", art.enabled);
    art.ip           = xml.getStringAttribute ("artIp", art.ip);
    art.universe     = juce::jlimit (0, 32767, xml.getIntAttribute ("artUniverse", art.universe));
    art.startChannel = juce::jlimit (1, 512, xml.getIntAttribute ("artStartChannel", art.startChannel));
    art.fixtures     = juce::jlimit (1, 512, xml.getIntAttribute ("artFixtures", art.fixtures));
    art.layout       = (DmxLayout) juce::jlimit (0, 3, xml.getIntAttribute ("artLayout", (int) art.layout));
    lights.setArtNetConfig (art);

    if (includeUi)
    {
        uiWidth = juce::jlimit (300, 4000, xml.getIntAttribute ("uiWidth", uiWidth.load()));
        uiHeight = juce::jlimit (240, 3000, xml.getIntAttribute ("uiHeight", uiHeight.load()));
        uiSettingsVisible = xml.getBoolAttribute ("uiSettingsVisible", uiSettingsVisible.load());
    }
}

void LightControlAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary (*createStateXml (true), destData);
}

void LightControlAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        applyStateXml (*xml, true);
}

juce::File LightControlAudioProcessor::getDefaultsFile()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("LightControl")
               .getChildFile ("defaults.xml");
}

bool LightControlAudioProcessor::saveAsDefaults() const
{
    const auto file = getDefaultsFile();

    if (! file.getParentDirectory().createDirectory().wasOk())
        return false;

    juce::TemporaryFile temp (file);   // write next to the target, then swap, so a crash can't leave a half-written file
    return createStateXml (false)->writeTo (temp.getFile()) && temp.overwriteTargetFileWithTemporary();
}

void LightControlAudioProcessor::loadDefaults()
{
    lights.resetToFactorySettings();

    if (auto xml = juce::parseXML (getDefaultsFile()))
        applyStateXml (*xml, false);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LightControlAudioProcessor();
}


