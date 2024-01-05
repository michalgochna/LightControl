
#include <JuceHeader.h>


class ListenerComponent : public juce::Component, public juce::ChangeListener
{
public:
	ListenerComponent();



	void paint(juce::Graphics& g) override;
	void resized() override;
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
	juce::TextButton receiverButton{ "Receiver" };
};

