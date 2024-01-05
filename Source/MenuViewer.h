#pragma once
#include <JuceHeader.h>
#include "Devices.h"
#include "Settings.h"

class MenuViewer :public juce::Button::Listener //, juce::TabbedComponent  
{
public:
    MenuViewer();
    ~MenuViewer() override;
    /*
    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    */


private:
    DevicesPage devicesPage;
    //TabbedComponent tabs
    //{ juce::TabbedButtonBar::Orientation::TabsAtTop };




};






