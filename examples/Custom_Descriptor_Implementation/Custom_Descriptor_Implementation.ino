#include <LaRoomyApi_nRF52.h>

// Check out the full documentation at: https://api.laroomy.com/

/* This is a brief example how to implement language-dependend strings for the properties and groups */

// define the pin names
#define LED_1 2
#define LED_2 3

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define SP_EXAMPLE_BUTTON_ONE 1
#define SP_EXAMPLE_BUTTON_TWO 2
#define SP_EXAMPLE_LEVELINDICATOR 3
#define SP_EXAMPLE_TEXTDISPLAY 4
#define SP_EXAMPLE_OPTIONSELECTOR 5

#define SP_EXAMPLE_GROUP 1

// define the callback for the remote app events - https://api.laroomy.com/p/laroomy-app-callback.html
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

    // receive button pressed events
    void onButtonPressed(cID buttonID) override
    {
        // check which button was pressed and execute an action, respectively
        if (buttonID == SP_EXAMPLE_BUTTON_ONE)
        {
            // toggle led1
            digitalRead(LED_1) == LOW ? digitalWrite(LED_1, HIGH) : digitalWrite(LED_1, LOW);
        }
        else if (buttonID == SP_EXAMPLE_BUTTON_TWO)
        {
            // toggle led2
            digitalRead(LED_2) == LOW ? digitalWrite(LED_2, HIGH) : digitalWrite(LED_2, LOW);
        }
    }
};

// define the callback for the descriptions of the properties and groups
//      https://api.laroomy.com/p/descriptor-callback.html
class DescriptorCallback : public IElementDescriptionCallback
{
    /*
        NOTE:   - The format of the language identifier is in ISO 639-1 format
                - The descriptor must be in the raw syntax: Some properties have special syntax for its content. The raw sytax could
                  be hardcoded or the higher level property classes can be used to construct the raw syntax. Examples to do that are shown
                  below. Properties with special syntax are: Button, LevelIndicator, TextDisplay and OptionSelector.

        https://api.laroomy.com/2023/03/property-descriptor-syntax.html
    */

    // this callback method is called every time when a description for a specific property is required
    void onPropertyDescriptionRequired(cID propertyID, const String &langID, String &descriptor) override
    {
        if (langID == "en")
        {
            if (propertyID == SP_EXAMPLE_BUTTON_ONE)
            {
                // here the raw descriptor is hardcoded, the descriptor for the button property consists of
                // the button-description and the button-text separated with ';;'
                descriptor = "LED One;;Toggle";
            }
            else if (propertyID == SP_EXAMPLE_BUTTON_TWO)
            {
                // here the button class is used to construct the descriptor
                Button btn;
                btn.buttonDescriptor = "LED Two";
                btn.buttonText = "Toggle";
                descriptor = btn.toDevicePropertyDescriptor();
            }
            else if (propertyID == SP_EXAMPLE_LEVELINDICATOR)
            {
                // here the raw descriptor is hardcoded, the descriptor for the level-indicator property consists of
                // the description and an OPTIONAL color value separated with ';;'. So when this is hardcoded, the level value
                // color must be added each time the property descriptor is requested. An easier implementation is shown in the de section.
                descriptor = "Level Indication;;";
                descriptor += Colors::LightGreen.toString();
            }
            else if (propertyID == SP_EXAMPLE_TEXTDISPLAY)
            {
                // here the raw descriptor is hardcoded, the descriptor for the text property consists of the description
                // and an OPTIONAL color value separated with ';;'. So when this is hardcoded, the rounded rectangle
                // color must be added each time the property descriptor is requested. An easier implementation is shown in the de section.
                descriptor = "This is a Color:;;";
                descriptor += Colors::PaleRed.toString();
            }
            else if (propertyID == SP_EXAMPLE_OPTIONSELECTOR)
            {
                // here the raw descriptor is hardcoded, the descriptor for the option selector consists of
                // the description and the option strings appended, all separated with ';;'. An easier implementation is shown in the de section.
                descriptor = "Select an option:;;Option 1;;Option 2;;Option 3";
            }
        }
        else if (langID == "de")
        {
            if (propertyID == SP_EXAMPLE_BUTTON_ONE)
            {
                // here the raw descriptor is hardcoded
                descriptor = "LED Eins;;Umschalten";
            }
            else if (propertyID == SP_EXAMPLE_BUTTON_TWO)
            {
                // here the button class is used to construct the descriptor
                Button btn;
                btn.buttonDescriptor = "LED Zwei";
                btn.buttonText = "Umschalten";
                descriptor = btn.toDevicePropertyDescriptor();
            }
            else if (propertyID == SP_EXAMPLE_LEVELINDICATOR)
            {
                // here the LevelIndicator class is used to construct the raw descriptor. By calling the constructor with the property ID
                // the internal property element is automatically retrieved and it is not necessary to set the color value each time. The member function
                // 'toPropertyDescriptor' is used to build the descriptor:
                LevelIndicator li(SP_EXAMPLE_LEVELINDICATOR);
                li.levelIndicatorDescription = "Pegel Darstellung";
                descriptor = li.toPropertyDescriptor();
            }
            else if (propertyID == SP_EXAMPLE_TEXTDISPLAY)
            {
                // here the TextDisplay class is used to construct the raw descriptor. By calling the constructor with the property ID
                // the internal property element is automatically retrieved and it is not necessary to set the param for the colorRect each time.
                // The member function 'toPropertyDescriptor' is used to build the descriptor:
                TextDisplay td(SP_EXAMPLE_TEXTDISPLAY);
                td.textToDisplay = "Das ist eine Farbe:";
                descriptor = td.toPropertyDescriptor();
            }
            else if (propertyID == SP_EXAMPLE_OPTIONSELECTOR)
            {
                // here the OptionSelector class is used to construct the raw descriptor. By calling the constructor with the property ID
                // the internal property element is automatically retrieved. If the option-strings of the selector are not lang depended, it is
                // not necessary to set them again.
                // The member function 'toPropertyDescriptor' is used to build the descriptor:
                OptionSelector os(SP_EXAMPLE_OPTIONSELECTOR);
                os.optionSelectorDescription = Convert::escapeNonAsciiCharacters(L"Option wählen:"); // ä is not a valid ansi char, so convert it
                descriptor = os.toDevicePropertyDescriptor();
            }
        }
        else
        {
            // TODO: add more languages

            if (propertyID == SP_EXAMPLE_BUTTON_ONE)
            {
                descriptor = "Led 1;;Toggle";
            }
            else if (propertyID == SP_EXAMPLE_BUTTON_TWO)
            {
                descriptor = "Led 2;;Toggle";
            }
            else if (propertyID == SP_EXAMPLE_LEVELINDICATOR)
            {
                descriptor = "Level";
            }
            else if (propertyID == SP_EXAMPLE_TEXTDISPLAY)
            {
                descriptor = "Text";
            }
            else if (propertyID == SP_EXAMPLE_OPTIONSELECTOR)
            {
                descriptor = "Select option:;;One;;Two;;Three";
            }
        }
    }

    // this callback method is called when a description for a specific group is required
    void onGroupDescriptionRequired(cID groupID, const String &langID, String &description) override
    {
        if (langID == "de")
        {
            if (groupID == SP_EXAMPLE_GROUP)
            {
                description = "Gruppe 1";
            }
        }
        else
        {
            if (groupID == SP_EXAMPLE_GROUP)
            {
                description = "Group 1";
            }
        }
    }
};

void setup()
{
    // put your setup code here, to run once:

    // monitor output for evaluation
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);

    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);

    // begin - https://api.laroomy.com/p/laroomy-api-class.html
    LaRoomyApi.begin();

    // set the bluetooth name
    LaRoomyApi.setBluetoothName("Nano 33 Ble");

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    // create an object of the descriptor callback and set it as handler
    LaRoomyApi.setDescriptionCallback(
        dynamic_cast<IElementDescriptionCallback *>(
            new DescriptorCallback()));

    /*
        Some properties are added for demonstration purposes.
        But the strings for the property and group descriptions are not set because
        the descriptor-callback will do that.
    */

    // create the group
    DevicePropertyGroup group1;
    group1.imageID = LaRoomyImages::GROUP_180;
    group1.groupID = SP_EXAMPLE_GROUP;

    // create button one
    Button b1;
    b1.buttonID = SP_EXAMPLE_BUTTON_ONE;
    b1.imageID = LaRoomyImages::LIGHT_BULB_004;
    // add it to group
    group1.addDeviceProperty(b1);

    // create button two
    Button b2;
    b2.buttonID = SP_EXAMPLE_BUTTON_TWO;
    b2.imageID = LaRoomyImages::STANDARD_LAMP_006;
    // add it to group
    group1.addDeviceProperty(b2);

    // create level indicator
    LevelIndicator li;
    li.imageID = LaRoomyImages::BATTERY_50P_034;
    li.level = 127;
    li.levelIndicatorID = SP_EXAMPLE_LEVELINDICATOR;
    li.valueColor = Colors::LightGreen;
    // add it to group
    group1.addDeviceProperty(li);

    // create text display
    TextDisplay td;
    td.imageID = LaRoomyImages::NOTIFICATION_TEXT_068;
    td.textDisplayID = SP_EXAMPLE_TEXTDISPLAY;
    td.colorRect = Colors::PaleRed;
    // add it to group
    group1.addDeviceProperty(td);

    // create option selector
    OptionSelector os;
    os.imageID = LaRoomyImages::OPTION_LIST_170;
    os.optionSelectorID = SP_EXAMPLE_OPTIONSELECTOR;
    os.selectedIndex = 0;
    // the options are added because in this case they are not language dependend
    os.addOption("Option 1");
    os.addOption("Option 2");
    os.addOption("Option 3");
    // add it to group
    group1.addDeviceProperty(os);

    // add the group
    LaRoomyApi.addDevicePropertyGroup(group1);

    // run
    LaRoomyApi.run();
}

void loop()
{
    // put your main code here, to run repeatedly:

    // handle onLoop
    LaRoomyApi.onLoop();
}