#include <LaRoomyApi_nRF52.h>

// Check out the full documentation at: https://api.laroomy.com/

// define the pin names
#define EX_LEVEL_LED 2
#define RED_PIN 10
#define GREEN_PIN 9
#define BLUE_PIN 6

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define SP_RGB_SELCTOR_ID 1
#define SP_COLORTABLE_SELECTOR_ID 2
#define SP_EX_LEVEL_SELECTOR_ID 3
#define SP_EVENT_TYPE_SWITCH_ID 4

// RGB control object (with pin definitions) - https://api.laroomy.com/p/helper-classes.html#laroomyApiRefMIDRGBCtrl
RGBControl rgbHelper(RED_PIN, GREEN_PIN, BLUE_PIN);

// define the callback for the remote app-user events - https://api.laroomy.com/p/laroomy-app-callback.html
class RemoteEvents : public ILaroomyAppCallback
{
public:
    // receive connection state change info
    void onConnectionStateChanged(bool newState) override
    {
        if (newState)
        {
            Serial.println("Connected!");
            digitalWrite(LED_BUILTIN, HIGH);
        }
        else
        {
            Serial.println("Disconnected!");
            digitalWrite(LED_BUILTIN, LOW);
        }
    }

    // receive rgb state notifications
    void onRGBSelectorStateChanged(cID rgbSelectorID, const RGBSelectorState &state) override
    {
        rgbHelper.applyStateChange(state);
    }

    // receive index changes of the program selector (option-selector)
    void onOptionSelectorIndexChanged(cID optionSelectorID, unsigned int newIndex) override
    {
        ColorCollection colorCollection;

        switch (newIndex)
        {
        case 0:
            // select the default table (NOTE: this is the default table in the rgbHelper class)
            colorCollection.AddItem(Colors::Red);
            colorCollection.AddItem(Colors::Green);
            colorCollection.AddItem(Colors::Blue);
            colorCollection.AddItem(Colors::Magenta);
            colorCollection.AddItem(Colors::Cyan);
            colorCollection.AddItem(Colors::Yellow);
            colorCollection.AddItem(Colors::Pink);
            colorCollection.AddItem(Colors::Orange);
            colorCollection.AddItem(Colors::Purple);
            colorCollection.AddItem(Colors::White);
            break;
        case 1:
            // light colors
            colorCollection.AddItem(Colors::LightRed);
            colorCollection.AddItem(Colors::LightGreen);
            colorCollection.AddItem(Colors::LightBlue);
            colorCollection.AddItem(Colors::LightCyan);
            colorCollection.AddItem(Colors::LightYellow);
            colorCollection.AddItem(Colors::LightPink);
            colorCollection.AddItem(Colors::LightPurple);
            break;
        case 2:
            // dark colors
            colorCollection.AddItem(Colors::DarkRed);
            colorCollection.AddItem(Colors::DarkGreen);
            colorCollection.AddItem(Colors::DarkBlue);
            colorCollection.AddItem(Colors::DarkCyan);
            colorCollection.AddItem(Colors::DarkYellow);
            colorCollection.AddItem(Colors::DarkPink);
            colorCollection.AddItem(Colors::DarkPurple);
            break;
        case 3:
            // other colors
            colorCollection.AddItem(Colors::OrangeRed);
            colorCollection.AddItem(Colors::SeaGreen);
            colorCollection.AddItem(Colors::Turquoise);
            colorCollection.AddItem(Colors::GreenYellow);
            colorCollection.AddItem(Colors::PaleRed);
            colorCollection.AddItem(Colors::Mint);
            colorCollection.AddItem(Colors::Gold);
            colorCollection.AddItem(Colors::YellowGreen);
            break;
        default:
            break;
        }
        // set the new color collection in the rgb helper
        rgbHelper.setCustomColorSelection(colorCollection);
    }

    // receive extended level selector state changes
    void onExtendedLevelSelectorStateChanged(cID extendedLevelSelectorID, const ExtendedLevelSelectorState &state) override
    {
        // create monitor data
        String msg = "Ex-Level-Selector state changed.\r\nNewState: ";
        msg += state.levelValue;
        msg += "\r\nTracking-Type: ";

        // catch tracking type
        switch (state.trackingType)
        {
        case ExLevelTrackingType::START:
            msg += "START\r\n";
            break;
        case ExLevelTrackingType::INTERTRACK:
            msg += "TRACKING\r\n";
            break;
        case ExLevelTrackingType::END:
            msg += "END\r\n";
            break;
        default:
            break;
        }
        // monitor
        Serial.print(msg.c_str());

        // apply new value
        if (state.isOn)
        {
            analogWrite(EX_LEVEL_LED, state.levelValue);
        }
        else
        {
            analogWrite(EX_LEVEL_LED, 0);
        }
    }

    // receive switch state changes
    void onSwitchStateChanged(cID switchID, bool newState) override
    {
        // apply the new tracking setup to the extended level selector and update it
        auto exLState =
            LaRoomyApi.getExtendedLevelSelectorState(SP_EX_LEVEL_SELECTOR_ID);

        if (newState)
        {
            exLState.transmitOnlyStartAndEndOfTracking = false;
        }
        else
        {
            exLState.transmitOnlyStartAndEndOfTracking = true;
        }

        LaRoomyApi.updateExLevelState(SP_EX_LEVEL_SELECTOR_ID, exLState);
    }
};

void setup()
{
    // put your setup code here, to run once:

    // monitor output for evaluation
    Serial.begin(115200);

    pinMode(EX_LEVEL_LED, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    analogWrite(EX_LEVEL_LED, 0);
    digitalWrite(LED_BUILTIN, LOW);

    // begin - https://api.laroomy.com/p/laroomy-api-class.html
    LaRoomyApi.begin();

    // set the bluetooth name
    LaRoomyApi.setBluetoothName("Nano 33 Ble");

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    // create the rgb-property
    RGBSelector rgbSelector;
    rgbSelector.imageID = LaRoomyImages::RGB_FLOWER_020;
    rgbSelector.rgbSelectorDescription = "Select Color";
    rgbSelector.rgbSelectorID = SP_RGB_SELCTOR_ID;

    /* Optional: set an initial rgb state (if it was saved before power-off)
    rgbSelector.rgbState = ...

    https://api.laroomy.com/p/property-state-classes.html#laroomyApiRefMIDRGBSS

    Optional: hide control elements on the rgbSelector Page by setting the flag value
    Example:
    rgbSelector.rgbState.flags = RGBSelectorFlags::RSF_HIDE_SOFT_HARD_TRANSITION_SWITCH; */

    // add the rgb property
    LaRoomyApi.addDeviceProperty(rgbSelector);

    // create the option-selector to select color-collections for the rgb-program
    OptionSelector os;
    os.imageID = LaRoomyImages::RGB_CIRCLES_019;
    os.optionSelectorDescription = "Select Color Program";
    os.optionSelectorID = SP_COLORTABLE_SELECTOR_ID;
    os.selectedIndex = 0;
    os.addOption("Default");
    os.addOption("Light Colors");
    os.addOption("Dark Colors");
    os.addOption("Other Colors");

    // add the option selector
    LaRoomyApi.addDeviceProperty(os);

    // create the extended level selector property
    ExtendedLevelSelector exLevelSelector;
    exLevelSelector.imageID = LaRoomyImages::LEVEL_ADJUST_043;
    exLevelSelector.extendedLevelSelectorDescription = "Select Level";
    exLevelSelector.extendedLevelSelectorID = SP_EX_LEVEL_SELECTOR_ID;
    // optional: set an initial ex-level state and scope
    exLevelSelector.exLevelState.maxValue = 255;
    exLevelSelector.exLevelState.minValue = 0;

    // add the ex-level property
    LaRoomyApi.addDeviceProperty(exLevelSelector);

    // create the tracking type switch
    Switch sw;
    sw.imageID = LaRoomyImages::SYNC_073;
    sw.switchDescription = "Level Full-Tracking Messages";
    sw.switchState = 1;
    sw.switchID = SP_EVENT_TYPE_SWITCH_ID;

    // add the switch
    LaRoomyApi.addDeviceProperty(sw);

    // finally call run to apply the setup and start bluetooth advertising
    LaRoomyApi.run(); 

    // begin rgb control
    rgbHelper.begin();
}

void loop()
{
    // put your main code here, to run repeatedly:

    // the >onLoop< method must be implemented here to permanently check for incoming transmissions
    LaRoomyApi.onLoop();

    // the >onLoop< method must be implemented here to control the fading
    rgbHelper.onLoop();
}