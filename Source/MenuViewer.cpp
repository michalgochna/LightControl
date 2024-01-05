#include "MenuViewer.h"

//==============================================================================
MenuViewer::MenuViewer() :TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto colour =   juce::Component::findColour(juce::ResizableWindow::backgroundColourId);

   
        //= juce::TabbedComponent::TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop);
    //tabs.addTab("Devices", colour, &devicesPage, true);
    //addTab("Settings", colour, new SettingsPage, true);
    setSize(100, 400);
}

MenuViewer::~MenuViewer()
{
    

}

void MenuViewer::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}


void MenuViewer::resized()
{
    //settingsButton.setBounds(40, 30, 200,300);    
    tabs.setBounds(getLocalBounds());

};

void MenuViewer::buttonClicked(juce::Button* button)
{

}

