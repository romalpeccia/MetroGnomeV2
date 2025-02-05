/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MetroGnome2AudioProcessor::MetroGnome2AudioProcessor()
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

    apvts.addParameterListener("ON/OFF", parameterChangeListener.get());
    apvts.addParameterListener("BPM", parameterChangeListener.get());
    apvts.addParameterListener("SUBDIVISION_1", parameterChangeListener.get());
    apvts.addParameterListener("SUBDIVISION_2", parameterChangeListener.get());
}

MetroGnome2AudioProcessor::~MetroGnome2AudioProcessor()
{
    apvts.removeParameterListener("ON/OFF", parameterChangeListener.get());
    apvts.removeParameterListener("BPM", parameterChangeListener.get());
    apvts.removeParameterListener("SUBDIVISION_1", parameterChangeListener.get());
    apvts.removeParameterListener("SUBDIVISION_2", parameterChangeListener.get());
}


juce::AudioProcessorValueTreeState::ParameterLayout MetroGnome2AudioProcessor::createParameterLayout() {
    //Creates all the parameters that change based on the user input and returns them in a AudioProcessorValueTreeState::ParameterLayout object

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterBool>("ON/OFF", "On/Off", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>("BPM", "bpm", juce::NormalisableRange<float>(1.f, 300.f, 0.1f, 0.25f), 120.f));
    layout.add(std::make_unique<juce::AudioParameterInt>("SUBDIVISION_1", "Subdivision 1", 1, MAX_LENGTH, 1));
    layout.add(std::make_unique<juce::AudioParameterInt>("SUBDIVISION_2", "Subdivision 2", 1, MAX_LENGTH, 4));

    return layout;

}

void MetroGnome2AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MetroGnome2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    /*
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data...
    }
    */


    metronome.processBlock(buffer);



}










juce::AudioProcessorEditor* MetroGnome2AudioProcessor::createEditor()
{

    //return GenericAudioProcessorEditor for generic sliders magically linked to APVTS (used for debugging/prototyping)
    //return new juce::GenericAudioProcessorEditor(*this);

    return new MetroGnome2AudioProcessorEditor (*this);
}

//==============================================================================
void MetroGnome2AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void MetroGnome2AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }

}


//==============================================================================
const juce::String MetroGnome2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MetroGnome2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MetroGnome2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MetroGnome2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MetroGnome2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MetroGnome2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MetroGnome2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void MetroGnome2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MetroGnome2AudioProcessor::getProgramName (int index)
{
    return {};
}

void MetroGnome2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================


void MetroGnome2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MetroGnome2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

//==============================================================================
bool MetroGnome2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetroGnome2AudioProcessor();
}
