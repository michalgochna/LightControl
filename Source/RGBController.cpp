
#include "OpenRGBController.h"
#include "OpenRGB/Client.hpp"
#include<string>
using orgb::ConnectStatus;
using orgb::DeviceListResult;
using orgb::RequestStatus;
using orgb::DeviceType;
using orgb::Device;
using namespace std;
using namespace std::chrono;


OpenRGBController::OpenRGBController()
{
    OpenRGBController::connect();
    OpenRGBController::getDevices();   
}


void OpenRGBController::connect()
{
    
    connection = client.connect("127.0.0.1");  // you can also use Windows computer name
    if (connection != ConnectStatus::Success)
    {
        fprintf(stdout, "failed to connect: %s (error code: %d)\n",
            enumString(connection), int(client.getLastSystemError()));
        return;
    }
}

void OpenRGBController::getDevices()
{
    deviceList = client.requestDeviceList();
    if (deviceList.status != RequestStatus::Success)
    {
        fprintf(stdout, "failed to get device list: %s (error code: %d)\n",
            enumString(deviceList.status), int(client.getLastSystemError()));
        return;
    }
 //   printf("devices %d = [\n", deviceList.devices.size() );
    for (const orgb::Device& device : deviceList.devices)
    {

        //printf("%s \n",device.name.c_str());
        //printf("%d \n",  int(device.type));
        client.switchToCustomMode(device);
        std::this_thread::sleep_for(milliseconds(10));
        client.switchToCustomModeX(device);
        std::this_thread::sleep_for(milliseconds(10));
        // device.idx index in the device list
    }
//    printf("]\n");

}


void OpenRGBController::paint(juce::Graphics& g) 
{
    string names;
    for (const orgb::Device& device : deviceList.devices)
    {
        names.append(device.name.c_str());
        names.append(" ");
    }

    
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);

        
/*    int height = getLocalBounds().getHeight();
    int width = getLocalBounds().getWidth() / deviceList.devices.size();
    int X1 = (1 - device.idx) *  getLocalBounds().getWidth() / deviceList.devices.size() ;
    int X2 = X1 + width;
    */
    //juce::Rectangle(X1, 0, X2, height), juce::Justification::topLeft, 1);
    //juce::ToggleButton(device.name.c_str()) ;
    g.drawFittedText(names, getLocalBounds(), juce::Justification::topLeft, 1);
    


}

void OpenRGBController::resized()
{

    
}

void OpenRGBController::setColor(juce::Colour c) {   //Colour -> ARGB

 //   const Device* keyboard = nullptr;
 //   keyboard = &deviceList.devices.findX(DeviceType::Keyboard);

    for (const orgb::Device& device : deviceList.devices)
    {
        client.setDeviceColorX(device, { c.getRed(), c.getGreen(), c.getBlue()});
        std::this_thread::sleep_for(milliseconds(10));
    }



}

void OpenRGBController::changeListenerCallback(juce::ChangeBroadcaster*) {
    setColor(juce::Colours::magenta);
}

orgb::DeviceListResult OpenRGBController::getDeviceList() {
    return deviceList;
}