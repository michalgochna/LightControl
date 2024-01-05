#include "Settings.h"


SettingsPage::SettingsPage()
{
    serverIp.setText("127.0.0.1", juce::NotificationType::dontSendNotification);
    serverIpLabel.setText("Server IP:", juce::NotificationType::dontSendNotification);
    serverPort.setText("6742", juce::NotificationType::dontSendNotification);
    serverPortLabel.setText("Server port: ", juce::NotificationType::dontSendNotification);
    about.setText("This plugin was made by Michal Gochna", juce::NotificationType::dontSendNotification);

    juce::Component::addAndMakeVisible(about);
    juce::Component::addAndMakeVisible(aboutLabel);
    juce::Component::addAndMakeVisible(serverIp);
    juce::Component::addAndMakeVisible(serverIpLabel);
    juce::Component::addAndMakeVisible(serverPort);
    juce::Component::addAndMakeVisible(serverPortLabel);

}


SettingsPage::~SettingsPage()
{

};

void SettingsPage::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
};

void SettingsPage::resized()
{

    settingsMenu.items.clear();

    juce::Rectangle <int> zeroBounds;
    zeroBounds.setBounds(0, 0, 0, 0);
    settingsMenu.performLayout(zeroBounds);


    juce::FlexBox item1;
    juce::Rectangle <int> settingsItemBounds(210, 60);
    item1.flexWrap = juce::FlexBox::Wrap::wrap;
    item1.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    item1.alignContent = juce::FlexBox::AlignContent::flexStart;
    item1.items.add(juce::FlexItem(100, 50, serverIpLabel));
    item1.items.add(juce::FlexItem(100, 50, serverIp));
    item1.performLayout(settingsItemBounds);

    juce::FlexBox item2;
    item2.flexWrap = juce::FlexBox::Wrap::wrap;
    item2.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    item2.alignContent = juce::FlexBox::AlignContent::spaceAround;
    item2.items.add(juce::FlexItem(100, 50, serverPortLabel));
    item2.items.add(juce::FlexItem(100, 50, serverPort));
    item2.performLayout(settingsItemBounds);

    juce::FlexBox settingsItems;
    juce::Rectangle <int> settingsItemsBounds = juce::Component::getLocalBounds().withTrimmedBottom(100);

    settingsItems.flexWrap = juce::FlexBox::Wrap::wrap;
    settingsItems.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    settingsItems.alignContent = juce::FlexBox::AlignContent::flexStart;
    settingsItems.items.add(juce::FlexItem(juce::Component::getLocalBounds().getWidth(), 50, item1));
    settingsItems.items.add(juce::FlexItem(juce::Component::getLocalBounds().getWidth(), 50, item2));
    settingsItems.performLayout(settingsItemsBounds);




    juce::FlexBox item_about;
    std::cout << "item about ";
    std::cout << item_about.items.isEmpty() << std::endl;
    juce::Rectangle <int> settingsAboutBounds(100, 100);
    item_about.flexWrap = juce::FlexBox::Wrap::wrap;
    item_about.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    item_about.alignContent = juce::FlexBox::AlignContent::spaceAround;
    item_about.items.add(juce::FlexItem(juce::Component::getLocalBounds().getWidth(), 100, about));
    item_about.performLayout(juce::Component::getLocalBounds().removeFromBottom(settingsAboutBounds.getHeight()));

    juce::FlexBox settingsMenu;
    juce::Rectangle <int> settingsBounds = juce::Component::getLocalBounds();

    settingsMenu.flexWrap = juce::FlexBox::Wrap::wrap;
    settingsMenu.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    settingsMenu.alignContent = juce::FlexBox::AlignContent::flexStart;
    settingsMenu.items.add(juce::FlexItem(settingsItemsBounds.getWidth(), settingsItemsBounds.getHeight(), settingsItems));
    settingsMenu.items.add(juce::FlexItem(juce::Component::getLocalBounds().getWidth(), 50, item_about));
    settingsMenu.performLayout(settingsBounds);

};
