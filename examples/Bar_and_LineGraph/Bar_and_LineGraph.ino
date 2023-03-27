#include <LaRoomyApi_nRF52.h>

// Check out the full documentation at: https://api.laroomy.com/

// define the pin names
#define HBUTTON_1 4
#define POTI_1 A0

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define SP_BAR_GRAPH_LIVE 1
#define SP_LINE_GRAPH_LIVE 2
#define SP_BAR_GRAPH_STATIC 3
#define SP_LINE_GRAPH_STATIC 4

#define GP_LIVE_GROUP_ID 1
#define GP_STATIC_GROUP_ID 2

// control params
bool barGraphLiveOutputActive = false;
bool lineGraphLiveOutputActive = false;
cID currentPropertyPage = 0;
unsigned int staticBarGraphOutputIndex = 0;
unsigned int staticLineGraphOutputIndex = 0;

// timer params
long mTimer = 0;
long mTimeOut = 50;

// adc holder
int adc_val = 0;

// lineGraph params
int currentXPos = 80;

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

            barGraphLiveOutputActive = false;
            lineGraphLiveOutputActive = false;
        }
    }

    // detect when a complex property page is invoked
    void onComplexPropertyPageInvoked(cID propertyID) override
    {
        currentPropertyPage = propertyID;

        if (propertyID == SP_BAR_GRAPH_LIVE)
        {
            barGraphLiveOutputActive = true;
        }
        else if (propertyID == SP_LINE_GRAPH_LIVE)
        {
            lineGraphLiveOutputActive = true;
        }
    }

    // detect when the user navigated back to the device-main-page
    void onBackNavigation() override
    {
        // reset params
        barGraphLiveOutputActive = false;
        lineGraphLiveOutputActive = false;
        currentPropertyPage = 0;
        currentXPos = 80;
        staticBarGraphOutputIndex = 0;
        staticLineGraphOutputIndex = 0;
    }

    // detect when the device connection was restored from suspended state
    void onDeviceConnectionRestored(cID currentOpenedPropertyPageID) override
    {
        currentPropertyPage = currentOpenedPropertyPageID;

        if (currentOpenedPropertyPageID == SP_BAR_GRAPH_LIVE)
        {
            barGraphLiveOutputActive = true;
        }
        else if (currentOpenedPropertyPageID == SP_LINE_GRAPH_LIVE)
        {
            lineGraphLiveOutputActive = true;
        }
        else
        {
            this->onBackNavigation();
        }
    }
};

void setup()
{
    // put your setup code here, to run once:

    // monitor output for evaluation
    Serial.begin(115200);

    pinMode(HBUTTON_1, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(POTI_1, INPUT);

    digitalWrite(LED_BUILTIN, LOW);

    // begin - https://api.laroomy.com/p/laroomy-api-class.html
    LaRoomyApi.begin();

    // set the bluetooth name
    LaRoomyApi.setBluetoothName("Nano 33 Ble");

    // set the device image
    LaRoomyApi.setDeviceImage(LaRoomyImages::BAR_GRAPH_162);

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    // create the live group
    DevicePropertyGroup liveGroup;
    liveGroup.descriptor = "Live Output";
    liveGroup.groupID = GP_LIVE_GROUP_ID;
    liveGroup.imageID = LaRoomyImages::GROUP_180;

    // create the first bar-graph for the live output
    BarGraph barGraphLive;
    barGraphLive.barGraphDescription = "BarGraph Realtime Output";
    barGraphLive.barGraphID = SP_BAR_GRAPH_LIVE;
    barGraphLive.imageID =  LaRoomyImages::BAR_GRAPH_162;

    // BarGraphState docu: https://api.laroomy.com/p/property-state-classes.html#laroomyApiRefMIDBGState

    // define the bars, in this case we want 2 bars for demonstration, one displaying the measured value of the adc
    // and the other bar is the same inverted
    barGraphLive.barGraphState.addBar(
        BarData("ADC Value", 0));
    barGraphLive.barGraphState.addBar(
        BarData("Inverted", 1024));

    // define a fixed maximum value (by default, the barGraph automatically adapts the scale in relation to the highest value,
    // in this scenario we don't want that, because this would cause a jittering visual appearance)
    barGraphLive.barGraphState.useFixedMaximumValue = true;
    barGraphLive.barGraphState.fixedMaximumValue = (float)1024.0;

    // add the barGraph to the live-group
    liveGroup.addDeviceProperty(barGraphLive);

    // create the first line-graph for the live output
    LineGraph lineGraphLive;
    lineGraphLive.lineGraphDescription = "LineGraph Realtime Output";
    lineGraphLive.lineGraphID = SP_LINE_GRAPH_LIVE;
    lineGraphLive.imageID = LaRoomyImages::LINE_GRAPH_163;

    // define the lineGraph state - https://api.laroomy.com/p/property-state-classes.html#laroomyApiRefMIDLGState

    // first define the area of the coordinate system and the visual appearance, we want grid-lines and values on the axis
    lineGraphLive.lineGraphState.xMinValue = 0;
    lineGraphLive.lineGraphState.xMaxValue = 100;
    lineGraphLive.lineGraphState.yMinValue = 0;
    lineGraphLive.lineGraphState.yMaxValue = 1100;

    lineGraphLive.lineGraphState.xIntersection = 10;  // the grid intersection lines of the x-axis
    lineGraphLive.lineGraphState.yIntersection = 100; // the grid intersection lines of the y-axis

    lineGraphLive.lineGraphState.drawAxisValues = true;
    lineGraphLive.lineGraphState.drawGridLines = true;

    // add the lineGraph to the live-group
    liveGroup.addDeviceProperty(lineGraphLive);

    // add the group
    LaRoomyApi.addDevicePropertyGroup(liveGroup);

    // create the static-group
    DevicePropertyGroup staticGroup;
    staticGroup.descriptor = "Static Data";
    staticGroup.groupID = GP_STATIC_GROUP_ID;
    staticGroup.imageID = LaRoomyImages::GROUP_180;

    // create the static-barGraph
    BarGraph barGraphStatic;
    barGraphStatic.barGraphDescription = "BarGraph static data";
    barGraphStatic.barGraphID = SP_BAR_GRAPH_STATIC;
    barGraphStatic.imageID = LaRoomyImages::INDIFFERENT_BARS_176;

    // set the barGraph data
    barGraphStatic.barGraphState.addBar(BarData("Bar 1", 26));
    barGraphStatic.barGraphState.addBar(BarData("Bar 2", 34));
    barGraphStatic.barGraphState.addBar(BarData("Bar 3", 6));
    barGraphStatic.barGraphState.addBar(BarData("Bar 4", 13));
    barGraphStatic.barGraphState.addBar(BarData("Bar 5", 18)); // NOTE: the maximum amount of bars is 9

    // add the barGraph
    staticGroup.addDeviceProperty(barGraphStatic);

    // create the static lineGraph
    LineGraph lineGraphStatic;
    lineGraphStatic.lineGraphDescription = "LineGraph static data";
    lineGraphStatic.lineGraphID = SP_LINE_GRAPH_STATIC;
    lineGraphStatic.imageID = LaRoomyImages::HEARTRATE_182;

    /* > configure the static lineGraph
        - lets assume we want to display temperature average values for a day
    */
    lineGraphStatic.lineGraphState.drawGridLines = true;
    lineGraphStatic.lineGraphState.drawAxisValues = true;
    // x-axis takes the hours
    lineGraphStatic.lineGraphState.xMaxValue = 24;
    lineGraphStatic.lineGraphState.xMinValue = 0;
    lineGraphStatic.lineGraphState.xIntersection = 3;
    // y-axis takes the temperature
    lineGraphStatic.lineGraphState.yMaxValue = 30;
    lineGraphStatic.lineGraphState.yMinValue = -10;
    lineGraphStatic.lineGraphState.yIntersection = 5;

    // generate the line-graph-data-points (NOTE: this is only a demonstration and no representation of real values)
    for (uint8_t i = 0; i <= 24; i++)
    {
        lineGraphStatic.lineGraphState.lineGraphPoints.addPoint(
            POINT(i, -0.2 * ((i - 12) * (i - 12)) + 23));
    }

    // add the static lineGraph
    staticGroup.addDeviceProperty(lineGraphStatic);

    // add the static group
    LaRoomyApi.addDevicePropertyGroup(staticGroup);

    // on the end, call run to apply the setup
    LaRoomyApi.run(); 

    // init timer reference
    mTimer = millis();
}

void loop()
{
    // put your main code here, to run repeatedly:

    // the 'onLoop' method must be implemented here to permanently check for incoming transmissions
    LaRoomyApi.onLoop();

    // if a live-output Graph property page is opened, measure value and update element
    if (barGraphLiveOutputActive || lineGraphLiveOutputActive)
    {
        // measure analog input
        if (millis() > (unsigned long)(mTimeOut + mTimer))
        {
            /* NOTE:
                Do not update the parameter on every measure result, do only update when necessary and make sure there is a upper
                limit for update-transmissions. In this case the maximum update-transmission count is limited to 20 per second
                by a timer implementation.
                This assures that the transmission buffer will not be blocked by to many transmissions but ensures a fluent
                visible update of the app element!
            */

            // update timer reference
            mTimer = millis();
            // save the old value before measure
            auto old_adc = adc_val;
            // get the current value (as average)
            adc_val = 0;
            for (uint8_t i = 0; i < 5; i++)
            {
                adc_val += analogRead(POTI_1);
            }
            adc_val /= 5;

            if (barGraphLiveOutputActive)
            {
                // only update if the value has changed
                if (old_adc != adc_val)
                { //((adc_val < (old_adc - 10)) || (adc_val > (old_adc + 10))){
                    // if connected, update the app
                    if (LaRoomyApi.isConnected())
                    {

                        // create a collection to hold the data for the update
                        itemCollection<BarData> bData;
                        bData.AddItem(
                            BarData(adc_val));
                        bData.AddItem(
                            BarData((float)(1024 - adc_val)));

                        /*
                            There are two ways to update the bars, the normal property-state update and the fast-data-pipe.
                            The fast-data-pipe is designed especially for the successive update of elements. It bypasses some 'slow' functions
                            to achive a higher data upstream rate. NOTE: updating the state with the fast-data-pipe does not save the state, it is only
                            transmitted.
                        */
                        LaRoomyApi.barGraphFastDataPipeSetAllBarValues(SP_BAR_GRAPH_LIVE, bData);
                    }
                }
            }
            if (lineGraphLiveOutputActive)
            {
                /*
                    There are two ways to update the lineGraph, the normal property-state update and the fast-data-pipe.
                    The fast-data-pipe is designed especially for the successive update of elements. It bypasses some 'slow' functions
                    to achive a higher data upstream rate. NOTE: updating the state with the fast-data-pipe does not save the state, it is only
                    transmitted.
                */

                // define the new point for the coordinate system
                POINT pt;
                pt.x = currentXPos++; // increase the x-axis position by 1
                pt.y = adc_val;

                // add the point to the system and shift the x-axis scale to the left (1 scale point according to the new x value)
                LaRoomyApi.lineGraphFastDataPipeAddPoint(SP_LINE_GRAPH_LIVE, &pt, 1.0, LineGraphGridShiftDirection::LGS_SHIFT_X_AXIS_PLUS);
            }
        }
    }

    // hardware button is pressed
    if (digitalRead(HBUTTON_1) == LOW)
    {
        // if the appropriate complex property page is open, we manipulate the graph data of the static graphs
        if (currentPropertyPage == SP_BAR_GRAPH_STATIC)
        {
            auto bgState = BarGraphState();

            if (staticBarGraphOutputIndex == 0)
            {
                // get the current state and change the values
                bgState = LaRoomyApi.getBarGraphState(SP_BAR_GRAPH_STATIC);
                bgState.changeBarValueAt(0, 5);
                bgState.changeBarValueAt(1, 10);
                bgState.changeBarValueAt(2, 15);
                bgState.changeBarValueAt(3, 20);
                bgState.changeBarValueAt(4, 25);

                staticBarGraphOutputIndex++;
            }
            else if (staticBarGraphOutputIndex == 1)
            {
                // get the current state and change the values
                bgState = LaRoomyApi.getBarGraphState(SP_BAR_GRAPH_STATIC);
                bgState.changeBarValueAt(0, 10);
                bgState.changeBarValueAt(1, 20);
                bgState.changeBarValueAt(2, 25);
                bgState.changeBarValueAt(3, 20);
                bgState.changeBarValueAt(4, 5);

                staticBarGraphOutputIndex++;
            }
            else if (staticBarGraphOutputIndex == 2)
            {
                // get the current state and delete a bar
                bgState = LaRoomyApi.getBarGraphState(SP_BAR_GRAPH_STATIC);
                bgState.removeBarAt(4);

                staticBarGraphOutputIndex++;
            }
            else if (staticBarGraphOutputIndex == 3)
            {
                // swap the whole barGraph data

                // add two bars
                bgState.addBar(BarData("Val 1", 253));
                bgState.addBar(BarData("Val 2", 166));

                staticBarGraphOutputIndex++;
            }
            else if (staticBarGraphOutputIndex == 4)
            {
                // swap the whole barGraph data

                // the value is displayed beneath the bar
                bgState.useValueAsBarDescriptor = true;
                // add three bars
                bgState.addBar(BarData(26.5));
                bgState.addBar(BarData(3.2));
                bgState.addBar(BarData(19));

                staticBarGraphOutputIndex++;
            }
            else
            {
                // swap the whole barGraph data

                // add five bars
                bgState.addBar(BarData("1", 12566));
                bgState.addBar(BarData("2", 3258));
                bgState.addBar(BarData("3", 867));
                bgState.addBar(BarData("4", 5900));
                bgState.addBar(BarData("5", 8800));

                staticBarGraphOutputIndex = 0;
            }
            LaRoomyApi.updateBarGraphState(SP_BAR_GRAPH_STATIC, bgState);
        }
        else if (currentPropertyPage == SP_LINE_GRAPH_STATIC)
        {
            /*
                On the static lineGraph we perform no data change, we perform a scope-change to
                demonstrate the implementation of a zoom functionality (however a data change would also be possible)
            */
            auto lgState = LaRoomyApi.getLineGraphState(SP_LINE_GRAPH_STATIC);

            if (staticLineGraphOutputIndex == 0)
            {
                lgState.xMaxValue = 36;
                lgState.xMinValue = -12;
                lgState.yMinValue = -20;
                lgState.yMaxValue = 40;
                lgState.xIntersection = 4;
                lgState.yIntersection = 5;

                staticLineGraphOutputIndex++;
            }
            else if (staticLineGraphOutputIndex == 1)
            {
                lgState.xMaxValue = 22;
                lgState.xMinValue = 2;
                lgState.yMinValue = -2;
                lgState.yMaxValue = 38;
                lgState.xIntersection = 4;
                lgState.yIntersection = 5;

                staticLineGraphOutputIndex++;
            }
            else if (staticLineGraphOutputIndex == 2)
            {
                lgState.xMaxValue = 18;
                lgState.xMinValue = 6;
                lgState.yMinValue = 5;
                lgState.yMaxValue = 30;
                lgState.xIntersection = 2;
                lgState.yIntersection = 5;

                staticLineGraphOutputIndex++;
            }
            else
            {
                lgState.xMaxValue = 14;
                lgState.xMinValue = 10;
                lgState.yMinValue = 20;
                lgState.yMaxValue = 30;
                lgState.xIntersection = 2;
                lgState.yIntersection = 1;

                staticLineGraphOutputIndex = 0;
            }
            LaRoomyApi.updateLineGraphState(SP_LINE_GRAPH_STATIC, lgState);
        }
        while (digitalRead(HBUTTON_1) == LOW)
            ;
        delay(20);
    }
}