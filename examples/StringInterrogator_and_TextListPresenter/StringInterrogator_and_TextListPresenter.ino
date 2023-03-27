#include <LaRoomyApi_nRF52.h>

// Check out the full documentation at: https://api.laroomy.com/

// define the pin names
#define HBUTTON_1 4
#define HBUTTON_2 5

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define SP_LOGIN_PROPERTY 1
#define SP_LOGDATA_PROPERTY 2

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

    // receive string-interrogator confirm button clicks
    void onStringInterrogatorDataReceived(cID stringInterrogatorID, String &fieldOneContent, String &fieldTwoContent) override
    {
        // if a login attempt is executed, this callback method is called with the entered contents
        // -> here we add it as a log message to demonstrate both complex properties
        String msg = "Login attempt occurred. Username: ";

        if (fieldOneContent.length() > 0)
        {
            msg += fieldOneContent;
        }
        else
        {
            msg += "<empty>";
        }
        msg += " / Password: ";

        if (fieldTwoContent.length() > 0)
        {
            msg += fieldTwoContent;
        }
        else
        {
            msg += "<empty>";
        }

        // add message to text-list-presenter stack
        LaRoomyApi.addTextListPresenterElement(SP_LOGDATA_PROPERTY, TextListPresenterElementType::WARNING_MESSAGE, msg);      

        // monitor
        Serial.println(msg);
    }

    void onPropertyLoadingComplete(PropertyLoadingType plt) override
    {
        LaRoomyApi.addTextListPresenterElement(SP_LOGDATA_PROPERTY, TextListPresenterElementType::DEFAULT_MESSAGE, "Hello user! This is a default-type message.");
    }

    // when the app was left without navigating back and the user goes back to it, this method is called
    void onDeviceConnectionRestored(cID currentOpenedPropertyPageID) override
    {
        // add as log message for demonstration
        LaRoomyApi.addTextListPresenterElement(SP_LOGDATA_PROPERTY, TextListPresenterElementType::INFO_MESSAGE, "Device connection restored!");
    }

    // detect when a complex property page is invoked
    void onComplexPropertyPageInvoked(cID propertyID) override
    {
        auto pLr = LaRoomyAppImplementation::GetInstance();
        if(pLr != nullptr)
        {
            String msg = "You navigated to complex property page with ID: ";
            msg += (unsigned int)propertyID;
            LaRoomyApi.addTextListPresenterElement(SP_LOGDATA_PROPERTY, TextListPresenterElementType::DEFAULT_MESSAGE, msg);
        }
    }

    // detect when the user navigated back to the device-main-page
    void onBackNavigation() override
    {
        // another way to send a textListPresenter element is to use the property itself:
        TextListPresenter tlp(SP_LOGDATA_PROPERTY);
        tlp.AddTextListPresenterElement(TextListPresenterElementType::DEFAULT_MESSAGE, "Back navigation occurred.");

        // same like this:

        // LaRoomyApi.addTextListPresenterElement(SP_LOGDATA_PROPERTY, TextListPresenterElementType::DEFAULT_MESSAGE, "Back navigation occurred.");
    }
};

void setup()
{
    // put your setup code here, to run once:

    // monitor output for evaluation
    Serial.begin(115200);

    pinMode(HBUTTON_1, INPUT);
    pinMode(HBUTTON_2, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED_BUILTIN, LOW);

    // begin - https://api.laroomy.com/p/laroomy-api-class.html
    LaRoomyApi.begin();

    // set the bluetooth name
    LaRoomyApi.setBluetoothName("Nano 33 Ble");

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    // create the string interrogator - https://api.laroomy.com/p/property-classes.html#laroomyApiRefMIDStringIG
    StringInterrogator si;
    si.imageID = LaRoomyImages::STRING_REQUEST_166;
    si.stringInterrogatorDescription = "Login";
    si.stringInterrogatorID = SP_LOGIN_PROPERTY;

    // define the field attributes for the string-interrogator

    // set the text of the confirm-button
    si.stringInterrogatorState.buttonText = "Confirm and go back!";

    // set the attributes for field 1
    si.stringInterrogatorState.fieldOneDescriptor = "Username";
    si.stringInterrogatorState.fieldOneHint = "Enter username here";
    si.stringInterrogatorState.fieldOneInputType = StringInterrogatorFieldInputType::SI_INPUT_TEXT; // this is the default

    // set the attributes for field 2
    si.stringInterrogatorState.fieldTwoDescriptor = "Password";
    si.stringInterrogatorState.fieldTwoHint = "Enter password here";
    si.stringInterrogatorState.fieldTwoInputType = StringInterrogatorFieldInputType::SI_INPUT_TEXT_PASSWORD;

    // add it
    LaRoomyApi.addDeviceProperty(si);

    // create the text-list-presenter - https://api.laroomy.com/p/property-classes.html#laroomyApiRefMIDTextListP
    TextListPresenter tlp;
    tlp.imageID = LaRoomyImages::SORT_LIST_168;
    tlp.textListPresenterDescription = "Show Log data";
    tlp.textListPresenterID = SP_LOGDATA_PROPERTY;

    // add it
    LaRoomyApi.addDeviceProperty(tlp);

    // at last, call run to apply the setup and start bluetooth advertising
    LaRoomyApi.run();
}

void loop()
{
    // put your main code here, to run repeatedly:

    // the 'onLoop' method must be implemented here to permanently check for incoming transmissions
    LaRoomyApi.onLoop();

    // first hardware button is pressed
    if (digitalRead(HBUTTON_1) == LOW)
    {
        delay(10);

        // add a message to the text-list-presenter - https://api.laroomy.com/p/laroomy-api-class.html#laroomyApiRefMAddTLElement
        LaRoomyApi.addTextListPresenterElement(SP_LOGDATA_PROPERTY, TextListPresenterElementType::INFO_MESSAGE, "This is an info-type message.");

        while (digitalRead(HBUTTON_1) == LOW)
            ;
        delay(50);
    }

    // second hardware button is pressed
    if (digitalRead(HBUTTON_2) == LOW)
    {
        delay(10);

        // add a message to the text-list-presenter - https://api.laroomy.com/p/laroomy-api-class.html#laroomyApiRefMAddTLElement
        LaRoomyApi.addTextListPresenterElement(SP_LOGDATA_PROPERTY, TextListPresenterElementType::ERROR_MESSAGE, "This is an error-type message.");
        
        while (digitalRead(HBUTTON_2) == LOW)
            ;
        delay(50);
    } 
}
