#include <LaRoomyApi_nRF52.h>

// Check out the full documentation at: https://api.laroomy.com/

/*
    This is a brief example how to implement an extern device-binding functionality.

        NOTE:
            The most convenient way to use the device binding is to activate the internal processing by calling
            the method 'enableInternalBindingHandler(true)' of the LaRoomyAppImplementation class. If this is
            done, no callbacks are fired on binding transmissions. When some custom processing is necessary or
            the whole process should be custom, binding must be handled externally. In this example the
            DeviceBindingController is used to implement the functionality.
            https://api.laroomy.com/2023/02/ways-to-implement-device-binding.html
*/

// define the pin names
#define LED_1 2
#define LED_2 3

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define SP_EXAMPLE_BUTTON_ONE 1
#define SP_EXAMPLE_BUTTON_TWO 2

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

    // receive button pressed events
    void onButtonPressed(cID buttonID) override
    {
        // check which button was pressed and execute an action, respectively
        if (buttonID == SP_EXAMPLE_BUTTON_ONE)
        {
            // toggle led1
            digitalWrite(LED_1, (digitalRead(LED_1) == LOW) ? HIGH : LOW);
        }
        else if (buttonID == SP_EXAMPLE_BUTTON_TWO)
        {
            // toggle led2
            digitalWrite(LED_2, (digitalRead(LED_2) == LOW) ? HIGH : LOW);
        }
    }

    // handle binding transmissions (here we use the binding-controller which implements the whole functionality)
    BindingResponseType onBindingTransmissionReceived(BindingTransmissionTypes bType, const String &key) override
    {
        /*
            When implementing the device-binding this callback-method is invoked on three cases:

             - binding is enabled by the user
             - binding is released by the user
             - an authentication request is done during the connection process

            All cases must be handled and the appropriate value must be returned. (BindingResponseType)
            If this method is not overwritten and auto-handling of the device-binding is set to false,
            the app reports that device binding is not supported.

            Here the DeviceBindingContoller is used to implement the whole functionality.
            https://api.laroomy.com/p/helper-classes.html#laroomyApiRefMIIDBCtrler
        */
       return DeviceBindingController.handleBindingTransmission(bType, key);
    }
};

void setup()
{
    // put your setup code here, to run once:

    // monitor output for evaluation
    Serial.begin(115200);

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    // begin - https://api.laroomy.com/p/laroomy-api-class.html
    LaRoomyApi.begin();

    // set the bluetooth name
    LaRoomyApi.setBluetoothName("Nano 33 Ble");

    // set device image
    LaRoomyApi.setDeviceImage(LaRoomyImages::CONJUNCTION_187);

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    /**********************************************************************************
    START >> BINDING-RELATED INIT SECTION *********************************************/
        
    /* IMPORTANT:
            Here we use binding, so when the user of the remote device enables binding, the key is
            saved to flash. If the power was lost or the device reboots for any other reason, the
            data must be restored from flash and the saved binding-enabled-parameter value must
            be set. Here the DeviceBindingController is used to save and load the data.
            https://api.laroomy.com/p/helper-classes.html#laroomyApiRefMIIDBCtrler
    */

    LaRoomyApi.setDeviceBindingAuthenticationRequired(
        DeviceBindingController.isBindingRequired()
    );

    /* END >> BINDING-RELATED SECTION *************************************************
    **********************************************************************************/

    // two simple buttons are inserted for demonstration purposes

    // create button one
    Button b1;
    b1.buttonDescriptor = "LED 1 control";
    b1.buttonText = "Toggle";
    b1.buttonID = SP_EXAMPLE_BUTTON_ONE;
    b1.imageID = LaRoomyImages::LED_008;
    // add it
    LaRoomyApi.addDeviceProperty(b1);

    // create button two
    Button b2;
    b2.buttonDescriptor = "LED 2 control";
    b2.buttonText = "Toggle";
    b2.buttonID = SP_EXAMPLE_BUTTON_TWO;
    b2.imageID = LaRoomyImages::LED_008;
    // add it
    LaRoomyApi.addDeviceProperty(b2);

    // call run to apply the setup and to start bluetooth advertising
    LaRoomyApi.run(); 
}

void loop()
{
    // put your main code here, to run repeatedly:

    // handle onLoop
    LaRoomyApi.onLoop();
}