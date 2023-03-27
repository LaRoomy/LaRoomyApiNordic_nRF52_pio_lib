#include <LaRoomyApi_nRF52.h>

// Check out the full documentation at: https://api.laroomy.com/
/*
    NOTE:   In a productive system it would be better to use an external I²C RealTimeClock with backup battery
            for the case of a power-loss. But for demonstration purposes a soft rtc should do the job.
*/

// define the pin names
#define LED_1 2
#define LED_2 3

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define SP_TIME_SELECTOR_ID 1
#define SP_TIME_SELECTOR_LED_SWITCH_ID 2
#define SP_TIME_FRAME_SELECTOR_ID 3
#define SP_TIME_MONITOR_ID 4
#define SP_DATE_SELECTOR_ID 5

// rtc data
struct CURRENTDATETIME {
    // litte state-time extension
    STATETIME time = {0,0};
    int seconds = 0;
    int day = 1;
    int month = 1;
    int year = 2023;
} currentDateTime;

unsigned long clockTimer = 0;
bool timeValid = false;
bool dateValid = false;
void controlTime();

// timer params
unsigned long mTimer = 0;
const unsigned long mTimeOut = 990;
unsigned long tTimer = 0;
const unsigned long tTimeOut = 10000;
STATETIME oldTime;

// state params
bool propertyLoaded = false;

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

    // catch the property-loading complete notification
    void onPropertyLoadingComplete(PropertyLoadingType plt) override
    {
        // when properties are loaded the local time must be requested from the app to set up the rtc
        LaRoomyApi.sendTimeRequest();

        // set marker
        propertyLoaded = true;
    }

    // contol the LEDs with the switches
    void onSwitchStateChanged(cID switchID, bool newState) override
    {
        if (switchID == SP_TIME_SELECTOR_LED_SWITCH_ID)
        {
            digitalWrite(LED_1, newState ? HIGH : LOW);
        }
    }

    void onTimeSelectorStateChanged(cID timeSelectorID, const TimeSelectorState &state) override
    {
        // this is only a demonstration of the callback method, in this example we don't need it
        String newTime = "New Simple-Time received: ";
        newTime += state.hour;
        newTime += " : ";
        newTime += state.minute;
        newTime += "\r\n\r\n";
        Serial.print(newTime.c_str());
        Serial.println("-----------------------------------------");
    }

    void onDateSelectorStateChanged(cID dateSelectorID, const DateSelectorState& state) override
    {
        // this is only a demonstration of the callback method, in this example we don't need it
        Serial.println("DateSelector state has changed:");
        Serial.print("New Date:  day= ");
        Serial.print(state.day);
        Serial.print("  month= ");
        Serial.print(state.month);
        Serial.print("  year= ");
        Serial.println(state.year);
        Serial.println("-----------------------------------------");
    }

    void onTimeFrameSelectorStateChanged(cID timeFrameSelectorID, const TimeFrameSelectorState &state) override
    {
        // this is only a demonstration of the callback method, in this example we don't need it
        String newTimeFrame = "New Time-Frame from ";
        newTimeFrame += state.startTime.hour;
        newTimeFrame += " : ";
        newTimeFrame += state.startTime.minute;
        newTimeFrame += " to ";
        newTimeFrame += state.endTime.hour;
        newTimeFrame += " : ";
        newTimeFrame += state.endTime.minute;
        newTimeFrame += "\r\n\r\n";
        Serial.print(newTimeFrame.c_str());
        Serial.println("-----------------------------------------");
    }

    // catch the response to the time request
    void onTimeRequestResponse(unsigned int hours, unsigned int minutes, unsigned int seconds) override
    {
        // set the local time
        currentDateTime.seconds = seconds;
        currentDateTime.time.minute = minutes;
        currentDateTime.time.hour = hours;

        // declare time as valid
        timeValid = true;

        // next: send date request
        LaRoomyApi.sendDateRequest();
    }

    // catch the response to the date request
    void onDateRequestResponse(unsigned int day, unsigned int month, unsigned int year) override
    {
        // save the date
        currentDateTime.day = day;
        currentDateTime.month = month;
        currentDateTime.year = year;

        // declare the date as valid
        dateValid = true;
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
    LaRoomyApi.setBluetoothName("Nano 33 BLE");

    // set the device image
    LaRoomyApi.setDeviceImage(LaRoomyImages::TIME_AND_DATE_017);

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    // docu for the property classes: https://api.laroomy.com/p/property-classes.html

    // create the time selector
    TimeSelector timeSel;
    timeSel.timeSelectorID = SP_TIME_SELECTOR_ID;
    timeSel.timeSelectorDescription = "Select Disable Time";
    timeSel.imageID = LaRoomyImages::TIME_012;

    // set initial disable time
    timeSel.timeSelectorState.hour = 18;
    timeSel.timeSelectorState.minute = 30;

    // add the time selector
    LaRoomyApi.addDeviceProperty(timeSel);

    // create a switch to control the time-selector indicator LED
    Switch timeSelectorLEDSwitch;
    timeSelectorLEDSwitch.switchID = SP_TIME_SELECTOR_LED_SWITCH_ID;
    timeSelectorLEDSwitch.switchDescription = "Time Selector LED";
    timeSelectorLEDSwitch.imageID = LaRoomyImages::LED_008;
    // add the switch
    LaRoomyApi.addDeviceProperty(timeSelectorLEDSwitch);

    // create the timeFrame selector
    TimeFrameSelector timeFrameSel;
    timeFrameSel.timeFrameSelectorID = SP_TIME_FRAME_SELECTOR_ID;
    timeFrameSel.timeFrameSelectorDescription = "Select Enable Timeframe";
    timeFrameSel.imageID = LaRoomyImages::TIME_FRAME_013;

    // set an initial time-frame
    timeFrameSel.timeFrameSelectorState.startTime = STATETIME(17, 0);
    timeFrameSel.timeFrameSelectorState.endTime = STATETIME(20, 0);

    // add the time-frame selector
    LaRoomyApi.addDeviceProperty(timeFrameSel);

    // create a textDisplay property to monitor the time on the device for evaluation
    TextDisplay timeMonitor;
    timeMonitor.textDisplayID = SP_TIME_MONITOR_ID;
    timeMonitor.imageID = LaRoomyImages::CLOCK_RELOAD_015;
    timeMonitor.textToDisplay = "DateTime not set!";
    // add the textDisplay
    LaRoomyApi.addDeviceProperty(timeMonitor);

    // create a date selector
    DateSelector dateSelector;
    dateSelector.dateSelectorDescription = "Select a Date";
    dateSelector.dateSelectorID = SP_DATE_SELECTOR_ID;
    dateSelector.imageID = LaRoomyImages::DATE_016;
    dateSelector.dateSelectorState.day = 1;
    dateSelector.dateSelectorState.month = 2;
    dateSelector.dateSelectorState.year = 2023;
    // add the dateSelector
    LaRoomyApi.addDeviceProperty(dateSelector);

    // on the end, call run to apply the setup and to start bluetooth advertising
    LaRoomyApi.run();
    
    // update timer
    mTimer = millis();
    tTimer = millis();
    clockTimer = millis();
}

void loop()
{
    // put your main code here, to run repeatedly:

    // the 'onLoop' method must be implemented here to permanently check for incoming transmissions
    LaRoomyApi.onLoop();

    // check the current time every second
    if (timeValid && dateValid && propertyLoaded)
    {
        if (millis() > (mTimeOut + mTimer))
        {
            // reset timer value
            mTimer = millis();

            /*
                at first check if the time equals the simple-time
            */
            auto simpleTimeSelectorTime =
                LaRoomyApi.getTimeSelectorState(SP_TIME_SELECTOR_ID).toStateTime();

            if (simpleTimeSelectorTime == currentDateTime.time)
            {
                /*
                    the time is equal, so disable LED (if necessary)
                */
                if (digitalRead(LED_1) == HIGH)
                {
                    digitalWrite(LED_1, LOW);

                    // IMPORTANT: update the internal state to keep hardware <> app-state alignment
                    LaRoomyApi.updateSimplePropertyState(SP_TIME_SELECTOR_LED_SWITCH_ID, 0);
                }
            }
            /*
                now check if the time is in the given time-frame
            */
            auto timeFrame =
                LaRoomyApi.getTimeFrameSelectorState(SP_TIME_FRAME_SELECTOR_ID);

            if (timeFrame.checkIfTimeIsInFrame(currentDateTime.time))
            {
                /*
                    the time is in frame, make sure the second led is active
                */
                if (digitalRead(LED_2) == LOW)
                {
                    digitalWrite(LED_2, HIGH);
                }
            }
            else
            {
                /*
                    the time is not in frame so disable the led
                */
                if (digitalRead(LED_2) == HIGH)
                {
                    digitalWrite(LED_2, LOW);
                }
            }

            /*
                update the textDisplay property to monitor the time on the device,
                but do it only if a change occured
            */
            if (currentDateTime.time != oldTime)
            {
                // build the string
                String timeString = "Time:  ";
                timeString += currentDateTime.time.hour;
                timeString += ':';
                if (currentDateTime.time.minute < 10)
                {
                    timeString += '0';
                }
                timeString += currentDateTime.time.minute;

                timeString += "  Date: ";
                timeString += currentDateTime.day;
                timeString += '/';
                timeString += currentDateTime.month;
                timeString += '/';
                timeString += currentDateTime.year;

                TextDisplay timeDisplay(SP_TIME_MONITOR_ID);
                // NOTE: invoking the constructor of a property class with the ID will automatically retrieve the
                //       original data (if the property was added before)
                timeDisplay.textToDisplay = timeString;
                timeDisplay.update();

                oldTime = currentDateTime.time;
            }         
        }
    }
    else
    {
        /*
            make sure the current time is set, sometimes a transmission is not recognized,
            but the device time is important data for this example, so we have to guarantee its reception
        */
        if (millis() > (tTimeOut + tTimer))
        {
            // reset timer
            tTimer = millis();

            if(propertyLoaded){
                if(!timeValid){
                    // send time request
                    LaRoomyApi.sendTimeRequest();
                }
                else if(!dateValid){
                    // send date request
                    LaRoomyApi.sendDateRequest();
                }
            }
        }
    }
    // control the time
    controlTime();
}

void controlTime(){
    if(millis() > (clockTimer + ((unsigned long)(1000)))){
        // 1 sec elapsed

        // reset timer
        clockTimer = millis();

        // increase seconds and infer the rest
        currentDateTime.seconds++;

        if(currentDateTime.seconds >= 60){
            currentDateTime.seconds = 0;

            currentDateTime.time.minute++;

            if(currentDateTime.time.minute >= 60){
                currentDateTime.time.minute = 0;

                currentDateTime.time.hour++;

                if(currentDateTime.time.hour == 24){
                    currentDateTime.time.hour = 0;

                    currentDateTime.day++;

                    // to be continued...
                }
            }
        }      
    }
}
