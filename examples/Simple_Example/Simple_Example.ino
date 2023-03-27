#include <LaRoomyApi_nRF52.h>

// Check out the full documentation at: https://api.laroomy.com/

// define the pin names
#define LED_1 2
#define LED_2 3
#define HBUTTON_1 4
#define HBUTTON_2 5

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define MY_FIRST_BUTTON_ID 1
#define MY_SECOND_BUTTON_ID 2
#define MY_FIRST_SWITCH_ID 3
#define MY_SECOND_SWITCH_ID 4

#define GROUP_ONE_ID 1

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
    };

    // receive button pressed events
    void onButtonPressed(cID buttonID) override
    {
        if (buttonID == MY_FIRST_BUTTON_ID)
        {
            Serial.println("First button was pressed!");

            auto pLr = LaRoomyAppImplementation::GetInstance();
            if (pLr != nullptr)
            {
                // here the button pressed event is used to demonstrate how to update a property at runtime
                // -> change the image and the text of the button

                // first get the current property data
                Button b1(MY_FIRST_BUTTON_ID);
                /*
                    NOTE: By calling the constructor of a property class with the ID of the property, the object is automatically
                            instantiated with the predefined property data (if the property was added before).
                */
                if (b1.imageID == LaRoomyImages::CIRCLE_EMPTY_148)
                {
                    // update the desired fields
                    b1.imageID = LaRoomyImages::CIRCLE_CHECKED_149;
                    b1.buttonText = "Uncheck it!";
                }
                else
                {
                    // update the desired fields
                    b1.imageID = LaRoomyImages::CIRCLE_EMPTY_148;
                    b1.buttonText = "Check it!";
                }
                // then update the property (if the device is connected, an update transmission is sent)
                b1.update();
            }
        }
        else if (buttonID == MY_SECOND_BUTTON_ID)
        {
            Serial.println("Second button was pressed!");

            // TODO: add an action for the second button
        }
    }

    // receive switch state changes
    void onSwitchStateChanged(cID switchID, bool newState) override
    {
        if (switchID == MY_FIRST_SWITCH_ID)
        {
            // set the state of the first LED
            if (newState)
            {
                digitalWrite(LED_1, HIGH);
            }
            else
            {
                digitalWrite(LED_1, LOW);
            }
        }
        if (switchID == MY_SECOND_SWITCH_ID)
        {
            // set the state of the second LED
            if (newState)
            {
                digitalWrite(LED_2, HIGH);
            }
            else
            {
                digitalWrite(LED_2, LOW);
            }
        }
    }
};

void setup()
{
    // put your setup code here, to run once:

    // monitor output for evaluation
    Serial.begin(115200);

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(HBUTTON_1, INPUT);
    pinMode(HBUTTON_2, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    // begin - https://api.laroomy.com/p/laroomy-api-class.html
    LaRoomyApi.begin();

    // set the bluetooth name
    LaRoomyApi.setBluetoothName("Nano 33 Ble");

    // set device image
    LaRoomyApi.setDeviceImage(LaRoomyImages::LIGHT_SETUP_005);

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    // docu for property classes: https://api.laroomy.com/p/property-classes.html

    //_________________________________________________________
    // at first we add two simple buttons as single properties:

    // declare the first button property
    Button b1;
    b1.buttonDescriptor = "First Button";
    b1.buttonText = "Check it";
    b1.imageID = LaRoomyImages::CIRCLE_EMPTY_148;
    b1.buttonID = MY_FIRST_BUTTON_ID;

    // add the first button property
    LaRoomyApi.addDeviceProperty(b1);

    // declare the second button property
    Button b2;
    b2.buttonDescriptor = "Second Button";
    b2.buttonText = "Press it!";
    b2.imageID = LaRoomyImages::MICROCHIP_128;
    b2.buttonID = MY_SECOND_BUTTON_ID;

    // add the second button property
    LaRoomyApi.addDeviceProperty(b2);

    //____________________________________________________________________________
    // now we add two simple switches but now arranged inside of a property group:

    // first declare the group
    DevicePropertyGroup group1;
    group1.descriptor = "Switches";
    group1.groupID = GROUP_ONE_ID;
    group1.imageID = LaRoomyImages::GROUP_180;

    // declare the first switch
    Switch sw1;
    sw1.imageID = LaRoomyImages::LED_008;
    sw1.switchDescription = "LED 1";
    sw1.switchID = MY_FIRST_SWITCH_ID;
    sw1.switchState = OFF;

    // add the first switch to the group
    group1.addDeviceProperty(sw1);

    // declare the second switch
    Switch sw2;
    sw2.imageID = LaRoomyImages::LED_008;
    sw2.switchDescription = "LED 2";
    sw2.switchID = MY_SECOND_SWITCH_ID;
    sw2.switchState = OFF;

    // add the second switch to the group
    group1.addDeviceProperty(sw2);

    // at last, add the group
    LaRoomyApi.addDevicePropertyGroup(group1);

    // on the end, call run to apply the setup and start bluetooth functionality
    LaRoomyApi.run();
}

void loop()
{
    // put your main code here, to run repeatedly:

    // the 'onLoop' method must be implemented here to permanently check for incoming transmissions
    LaRoomyApi.onLoop();

    /*
      another important point is that if we have properties which states can change by remote side and
      device side, there is the possibility that the state(s) changes while the app is connected. In this case
      it is important to update the state, otherwise this breaks the hardware - app state alignment.
      for example: if a LED is switched of by hardware button the state of the appropriate switch in the app
      remains in the ON-state.
      https://api.laroomy.com/2023/03/keep-property-states-updated.html
      To prevent this, update the internal state every time the regarding hardware element changes:
    */

    // first hardware button is pressed
    if (digitalRead(HBUTTON_1) == LOW)
    {
        delay(10);

        // first get the current internal state
        auto firstSwitchState = LaRoomyApi.getProperty(MY_FIRST_SWITCH_ID).propertyState;
        if (firstSwitchState > 0)
        {
            // the state was ON, so set it to OFF and update
            digitalWrite(LED_1, LOW);
            LaRoomyApi.updateSimplePropertyState(MY_FIRST_SWITCH_ID, OFF);
        }
        else
        {
            // the state was OFF, so set it to ON and update
            digitalWrite(LED_1, HIGH);
            LaRoomyApi.updateSimplePropertyState(MY_FIRST_SWITCH_ID, ON);
        }
        

        while (digitalRead(HBUTTON_1) == LOW)
            ;
        delay(50);
    }

    // second hardware button is pressed
    if (digitalRead(HBUTTON_2) == LOW)
    {
        delay(10);

        // first get the current internal state
        auto secondSwitchState = LaRoomyApi.getProperty(MY_SECOND_SWITCH_ID).propertyState;
        if (secondSwitchState > 0)
        {
            // the state was ON, so set it to OFF and update
            digitalWrite(LED_2, LOW);
            LaRoomyApi.updateSimplePropertyState(MY_SECOND_SWITCH_ID, OFF);
        }
        else
        {
            // the state was OFF, so set it to ON and update
            digitalWrite(LED_2, HIGH);
            LaRoomyApi.updateSimplePropertyState(MY_SECOND_SWITCH_ID, ON);
        }

        while (digitalRead(HBUTTON_2) == LOW)
            ;
        delay(50);
    }
}
