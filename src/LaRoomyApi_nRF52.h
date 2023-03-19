
#ifndef ARDUINOBLE_LAROOMY_NRF_H
#define ARDUINOBLE_LAROOMY_NRF_H

#define LAROOMY_API_VERSION     "1.0.0"

/*  The LaRoomy Framework is made to quickly implement Bluetooth remote control capabilities
 *  to your project. It works in conjunction with the LaRoomy App
 * 
 *  Read the full documentation: https://api.laroomy.com/
 * 
 * Copyright 2023 Hans Philipp Zimmermann
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "common.h"

#define PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER   0x01
#define PROPERTY_ELEMENT_FLAG_IS_DISABLED       0x02

typedef unsigned int cID;

#define INSERT_FIRST    ((unsigned int)(0x3253))
#define INSERT_LAST     ((unsigned int)(0x32C0))

#define INVALID_ELEMENT_INDEX   257
#define INVALID_PROPERTY_STATE  257
#define ID_DEVICE_MAIN_PAGE     16211

#ifndef OUT_MTU_SIZE
#define OUT_MTU_SIZE    20
#endif

class DeviceProperty;
class DevicePropertyGroup;

class Button;
class Switch;
class LevelSelector;
class LevelIndicator;
class TextDisplay;
class OptionSelector;
class RGBSelector;
class ExtendedLevelSelector;
class TimeSelector;
class TimeFrameSelector;
class DateSelector;
class UnlockControl;
class NavigatorControl;
class BarGraph;
class LineGraph;
class StringInterrogator;
class TextListPresenter;

class RGBSelectorState;
class ExtendedLevelSelectorState;
class TimeSelectorState;
class TimeFrameSelectorState;
class DateSelectorState;
class UnlockControlState;
class NavigatorState;
class BarGraphState;
class LineGraphState;
class StringInterrogatorState;
class TextListPresenterState;

class BarData;
class LineGraphDataPoints;

/**
 * @brief Point - Single coordinate definition (x/y)
 * 
 */
typedef struct _POINT {
    _POINT(){}
    _POINT(float x, float y){
        this->x = x;
        this->y = y;
    }
    _POINT(const _POINT& p){
        this->x = p.x;
        this->y = p.y;
    }
    float x;
    float y;

    _POINT& operator=(const _POINT& p){
        this->x = p.x;
        this->y = p.y;
        return *this;
    }
    bool operator==(const _POINT& p){
        if(this->x == p.x && this->y == p.y){
            return true;
        }
        else {
            return false;
        }
    }
    bool operator!=(const _POINT& p){
        return (*this == p) ? false : true;
    }
}POINT, *LPPOINT;

/**
 * @brief Color - Definition of a simple 24bit RGB color value
 * 
 */
typedef struct _COLOR {
    unsigned int redPart;
    unsigned int greenPart;
    unsigned int bluePart;

    _COLOR()
    :redPart(0), greenPart(0), bluePart(0) {}

    _COLOR(unsigned int r, unsigned int g, unsigned int b){
        this->redPart = r;
        this->greenPart = g;
        this->bluePart = b;
    }

    _COLOR(const _COLOR& c){
        this->redPart = c.redPart;
        this->greenPart = c.greenPart;
        this->bluePart = c.bluePart;
    }

    _COLOR(const char* str){
        this->fromString(str);
    }

    _COLOR& operator= (const _COLOR& c){
        this->redPart = c.redPart;
        this->greenPart = c.greenPart;
        this->bluePart = c.bluePart;
        return *this;
    }

    _COLOR& operator= (const char* str){
        this->fromString(str);
        return *this;
    }

    bool operator== (const _COLOR& c){
        if((c.redPart == this->redPart)&&(c.greenPart == this->greenPart)&&(c.bluePart == this->bluePart)){
            return true;
        }
        return false;
    }

    bool operator!= (const _COLOR& c){
        return !(*this == c);
    }

    /**
     * @brief Converts the color in a string with the format '#rrggbb'.
     * 
     * @return String 
     */
    String toString(){
        if((this->redPart < 256)&&(this->greenPart < 256)&&(this->bluePart < 256)){
            String col = "#";
            char tc[3];
            tc[2] = '\0';
            Convert::u8BitValueToHexTwoCharBuffer((uint8_t)this->redPart, tc);
            col += tc;
            Convert::u8BitValueToHexTwoCharBuffer((uint8_t)this->greenPart, tc);
            col += tc;
            Convert::u8BitValueToHexTwoCharBuffer((uint8_t)this->bluePart, tc);
            col += tc;
            return col;
        }
        else {
            return "unset";
        }
    }

    /**
     * @brief Converts a string with the format '#rrggbb' into a color object, if the format
     * is wrong, the object is set to col_not_set
     * 
     * @param s The String to convert
     */
    void fromString(const String& s){
        if(s.length() > 0 && s.length() <= 8){
            this->redPart = Convert::x2CharHexValueToU8BitValue(s.charAt(1), s.charAt(2));
            this->greenPart = Convert::x2CharHexValueToU8BitValue(s.charAt(3), s.charAt(4));
            this->bluePart = Convert::x2CharHexValueToU8BitValue(s.charAt(5), s.charAt(6));
        }
        else {
            this->redPart = 301;
            this->greenPart = 302;
            this->bluePart = 303;
        }
    }

    /**
     * @brief Checks if the rgb values represent a valid color
     * 
     * @return bool (valid or not)
     */
    bool isValidColor(){
        return ((this->redPart < 256)&&(this->greenPart < 256)&&(this->bluePart < 256)) ? true : false;
    }
} COLOR, *PCOLOR;

// unset marker for the color struct (values out of bounds for 24bit)
const COLOR col_not_set = {301, 302, 303};

typedef struct _UIMODEDATA {
    bool isNightMode;
    unsigned int dummy;
} UIMODEDATA, *PUIMODEDATA;

/**
 * @brief Callback handler for remote events (state changes & notifications)
 * 
 */
class ILaroomyAppCallback {
public:
    virtual void onConnectionStateChanged(bool newState){};
    virtual void onButtonPressed(cID buttonID){}
    virtual void onSwitchStateChanged(cID switchID, bool newState){}
    virtual void onLevelSelectorValueChanged(cID levelSelectorID, unsigned int newValue){}
    virtual void onOptionSelectorIndexChanged(cID optionSelectorID, unsigned int newIndex){}
    virtual void onRGBSelectorStateChanged(cID rgbSelectorID, const RGBSelectorState& state){}
    virtual void onExtendedLevelSelectorStateChanged(cID extendedLevelSelectorID, const ExtendedLevelSelectorState& state){}
    virtual void onTimeSelectorStateChanged(cID timeSelectorID, const TimeSelectorState& state){}
    virtual void onTimeFrameSelectorStateChanged(cID timeFrameSelectorID, const TimeFrameSelectorState& state){}
    virtual void onDateSelectorStateChanged(cID dateSelectorID, const DateSelectorState& state){}
    virtual void onUnlockControlStateChanged(cID unlockControlID, const UnlockControlState& state){}
    virtual void onUnlockControlInvalidOperation(cID unlockControlID, UnlockControlInvalidOperation invalidOperation){}
    virtual void onNavigatorStateChanged(cID navigatorID, const NavigatorState& state){}
    virtual void onStringInterrogatorDataReceived(cID stringInterrogatorID, String& fieldOneContent, String& fieldTwoContent){}
    virtual BindingResponseType onBindingTransmissionReceived(BindingTransmissionTypes bType, const String& key){
        return BindingResponseType::BINDING_FAIL_NOT_IMPLEMENTED;
    }
    virtual void onPropertyLoadingComplete(PropertyLoadingType plt){}
    virtual void onFactoryResetRequest(){}
    virtual void onDeviceConnectionRestored(cID currentOpenedPropertyPageID){}
    virtual void onBackNavigation(){}
    virtual void onComplexPropertyPageInvoked(cID propertyID){}
    virtual void onTimeRequestResponse(unsigned int hours, unsigned int minutes, unsigned int seconds){}
    virtual void onDateRequestResponse(unsigned int day, unsigned int month, unsigned int year){}
    virtual void onLanguageRequestResponse(String langID){}
    virtual void onUIModeRequestResponse(PUIMODEDATA pData){}
};

/**
 * @brief Callback for supporting language depended property and group descriptions
 * 
 */
class IElementDescriptionCallback {
public:
    virtual void onPropertyDescriptionRequired(cID propertyID, const String& langID, String& description){}
    virtual void onGroupDescriptionRequired(cID groupID, const String& langID, String& description){}
};

/**
 * @brief Data-class for bluetooth reception control
 * 
 */
class TransmissionControl {
public:
    String data;
    bool tComplete = false;

    void reset(){
        data = "";
        tComplete = false;
    }

    void setData(const char* str){
        this->data = str;
        if(this->data.length() > 0){
            this->tComplete = true;
        }
    }
};

/**
 * @brief Implementation of the LaRoomy App functionality
 * 
 */
class LaRoomyAppImplementation
{
public:   
    ~LaRoomyAppImplementation();

    /**
     * @brief Get the Singleton-Instance object of the LaRoomyAppImplementation class
     * 
     * @return LaRoomyAppImplementation* 
     */
    static LaRoomyAppImplementation* GetInstance(){
        if(laRoomyAppImplInstanceCreated){
            return hInstance;
        }
        else {
            hInstance = new LaRoomyAppImplementation();
            if(hInstance != nullptr){
                laRoomyAppImplInstanceCreated = true;
            }
            return hInstance;
        }
    }

    /**
     * @brief Start the API control (this method can be omitted - only provided for conformity)
     * 
     */
    void begin();

    /**
     * @brief Initialize the bluetooth functionality and start advertising 
     */
    void run();

    /**
     * @brief This method stops all bluetooth processes, deletes all property and property state data and resets
     * all control parameter
     */
    void end();    

    /**
     * @brief Handle events - must be implemented in the main loop
     * 
     */
    void onLoop();

    /**
     * @brief Add a device property element
     * 
     * @param p The property element
     */
    void addDeviceProperty(const DeviceProperty& p);

    /**
     * @brief Add a new group element. If the group element contains no deviceProperty objects, the element will be discarded
     * 
     * @param g The group to add
     */
    void addDevicePropertyGroup(const DevicePropertyGroup& g);

    /**
     * @brief Insert a new property element after the element with the given ID - if there is no element with the ID, the operation will be skipped
     * 
     * @param insertAfter The ID of the element before the insert point. To insert as first element use: INSERT_FIRST. To insert as last element use: INSERT_LAST.
     * @param p  The property element to insert.
     */
    void insertProperty(cID insertAfter, const DeviceProperty& p);

    /**
     * @brief Insert a new property in the group with the given group-ID after the element with the given insertAfter-ID. 
     * If there is no element with the ID, the operation will be skipped.
     * 
     * @param insertAfter The ID of the element before the insert point. To insert as first element use: INSERT_FIRST. To insert as last element use: INSERT_LAST.
     * @param groupID The ID of the group in which to insert.
     * @param p The property element to insert.
     */
    void insertPropertyInGroup(cID insertAfter, cID groupID, const DeviceProperty& p);

    /**
     * @brief Update an existing property. If the device is connected, an update transmission will be sent.
     * 
     * @param p The changed property element.
     */
    void updateDeviceProperty(const DeviceProperty& p);

    /**
     * @brief Get the Property object for the given ID. Note: the returned object is a copy. Changing it does not update the property.
     * For that purpose use the update method instead.
     * 
     * @param propertyID The ID of the desired property element.
     * @return DeviceProperty 
     */
    DeviceProperty getProperty(cID propertyID);

    // The property with the given ID will be deleted
    void removeProperty(cID propertyID);

    // The group with the given ID and all its properties will be deleted
    void removePropertyGroup(cID groupID);

    // The property with the given ID will be enabled. Further interactions by the user will be accepted.
    void enableProperty(cID propertyID);

    // The property with the given ID will be disabled. That means, it is grayed out and does not accept user input.
    void disableProperty(cID propertyID);
    
    // Removes all properties and their states
    // Removes all groups
    void clearAllPropertiesAndGroups();

    /**
     * @brief Send a message which will be displayed to the app-user
     * 
     * @param type Severity of the Notification (Info/Warning/Error)
     * @param period Holding period for the Notification
     * @param message The message to display
     *
    */
    void sendUserMessage(UserMessageType type, UserMessageHoldingPeriod period, const String& message);

    // Get the current local time from the app
    // Listen to the callback to catch the response with the time data
    void sendTimeRequest();

    // Get the current date from the app
    // Listen to the callback to catch the response with the date data
    void sendDateRequest();

    // Force the app to reload all properties, groups and states
    void sendPropertyReloadCommand();

    // Force the app to save the current state of the properties to cache
    void sendPropertyToCacheCommand();

    // Get the current app language from the app
    // Listen to the callback to catch the response
    void sendLanguageRequest();

    // Force the app to refresh all states by requesting from device
    void sendRefreshAllStatesCommand();

    // Force the back-navigation to the device main page (if a sub page was opened)
    void sendNavBackToDeviceMainCommand();

    // Force the App to quit the connection and release the device
    void sendCloseDeviceCommand();

    // Force the app to send UI Mode info
    void sendUIModeInfoRequest();

    // Add a new element to the textListPresenter with the given ID
    void addTextListPresenterElement(cID textListPresenterID, TextListPresenterElementType type, const String& elementText);
    
    // Clear all data in the textListPresenter dataHolder
    void clearTextListPresenterContent(cID textListPresenterID);

    /**
     * @brief Set the Callback Interface object to get state changes and notifications from the app
     * 
     * @param callback The subclass of the interface.
     */
    void setCallbackInterface(ILaroomyAppCallback* callback){
        this->pLrCallback = callback;
    }

    // subscribe this callback to support language dependend string resources
    void setDescriptionCallback(IElementDescriptionCallback* callback){
        this->pDescriptionCallback = callback;
    }

    // read-only property getter
    bool isConnected(){
        return this->is_connected;
    }

    /**
     * @brief Get the Current Opened Property Page ID (Navigational state). If it is the device-main-page, the return-value is: ID_DEVICE_MAIN_PAGE
     * 
     * @return cID 
     */
    cID getCurrentOpenedPropertyPageID(){
        return this->currentPropertyPageID;
    }

    /**
     * @brief Performs a lookup for the given property ID.
     * 
     * @param propertyID The ID of the property to search for
     * @return bool (true if the property exists or false if not)
     */
    bool checkIfPropertyExist(cID propertyID);    

    // allow/disallow property caching (default is: false)
    void setPropertyCachingPermission(bool cp){
        this->cachingPermission = cp;
    }

    // define if a binding authentication will be required on the next connection process
    void setDeviceBindingAuthenticationRequired(bool required);

    /**
     * @brief Enables or disables the log output on the serial object.
     *          (Don't miss to call Serial.begin(..) before enabling this)
     * 
     * @param state true to enable or false to disable
     */
    void setSerialMonitorEnabledState(bool state){
        this->is_monitor_enabled = state;
    }

    /**
     * @brief Set the Stand-Alone-Mode. This mode works only in conjunction with a single complex-type property.
     * When enabled, the device-main-page is bypassed and will not be shown. Instead the complex-property page will be opened directly.
     * If the property-count differs from 1 or the property is not a complex type, this setting has no effect.
     * 
     * @param enable boolean - true to enable and false to disable
     */
    void setStandAloneMode(bool enable){
        this->isStandAloneMode = enable;
    }

    /**
     * @brief Change the default service UUID */
    void setServiceUUID(const String& sUUID){
        this->serviceUUID = "";
        this->serviceUUID = sUUID;
    }

    /**
     * @brief Change the default TX characteristic UUID */
    void setTxCharacteristicUUID(const String& cUUID){
        this->txCharacteristicUUID = "";
        this->txCharacteristicUUID = cUUID;
    }

    /**
     * @brief Change the default RX characteristic UUID */
    void setRxCharacteristicUUID(String& cUUID){
        this->rxCharacteristicUUID = "";
        this->rxCharacteristicUUID = cUUID;
    }

    /**
     * @brief Set the bluetooth name */
    void setBluetoothName(const String& bName){
        this->bluetoothName = bName;
    }
    
    /**
     * @brief Set the bluetooth name */
    void setBluetoothName(const char* bName){
        this->bluetoothName = bName;
    }

    /**
     * @brief Set the Image for the device, shown in the list on the Main Page of the App. This will append an
     * underscore to the device-name and the hexadecimal index of the selected image. The trailing sequence will
     * be removed on the app side and transformed to the image.
     * 
     * @param img The Image ID
     */
    void setDeviceImage(LaRoomyImages img);

    /**
     * @brief enables or disables the automatic state refresh loop which is triggered
     * by a 'device-reconnected' notification.
     * 
     * @param enable true to enable, false to disable (the default is true)
     */
    void enableAutoRefreshStates(bool enable){
        this->auto_refresh_states = enable;
    }

    /**
     * @brief If the internal binding handler is activated, all binding functions are handled internally.
     *  This means also that no callbacks are fired on binding events.
     * 
     * @param enable type: bool (enables or disables the handler)
     */
    void enableInternalBindingHandler(bool enable);    

    /**
     * @brief Get the current simple property state
     * 
     * @param pID The ID of the property
     * @return The Property State (uInt)
     */
    unsigned int getSimplePropertyState(cID pID);

    /**
     * @brief Get the RGB Selector State
     * 
     * @param rgbSelectorID The ID of the RGBSelector Property
     * @return RGBSelectorState 
     */
    RGBSelectorState getRGBSelectorState(cID rgbSelectorID);

    /**
     * @brief Get the Extended Level Selector State
     * 
     * @param exLevelSelectID The ID of the Extended Level Selector Property
     * @return ExtendedLevelSelectorState 
     */
    ExtendedLevelSelectorState getExtendedLevelSelectorState(cID exLevelSelectID);

    /**
     * @brief Get the Time Selector State
     * 
     * @param timeSelctorID The ID of the TimeSelector Property
     * @return TimeSelectorState 
     */
    TimeSelectorState getTimeSelectorState(cID timeSelectorID);

    /**
     * @brief Get the Time Frame Selector State
     * 
     * @param timeFrameSelectorID The ID of the TimeFrameSelector Property
     * @return TimeFrameSelectorState
     */
    TimeFrameSelectorState getTimeFrameSelectorState(cID timeFrameSelectorID);

    /**
     * @brief Get the Date Selector State
     * 
     * @param dateSelectorID The ID of the dateSelector Property
     * @return DateSelectorState 
     */
    DateSelectorState getDateSelectorState(cID dateSelectorID);

    /**
     * @brief Get the Unlock Control State
     * 
     * @param unlockControlID The ID of the UnlockControl Property
     * @return UnlockControlState 
     */
    UnlockControlState getUnlockControlState(cID unlockControlID);

    /**
     * @brief Get the Navigator State
     * 
     * @param navigatorID The ID of the Navigator Property
     * @return NavigatorState 
     */
    NavigatorState getNavigatorState(cID navigatorID);

    /**
     * @brief Get the Bar Graph State
     * 
     * @param barGraphID The ID of the BarGraph Property
     * @return BarGraphState 
     */
    BarGraphState getBarGraphState(cID barGraphID);

    /**
     * @brief Get the Line Graph State
     * 
     * @param lineGraphID The ID of the LineGraph Property
     * @return LineGraphState 
     */
    LineGraphState getLineGraphState(cID lineGraphID);

    /**
     * @brief Get the String Interrogator State
     * 
     * @param stringInterrogatorID The ID of the StringInterrogator Property
     * @return StringInterrogatorState 
     */
    StringInterrogatorState getStringInterrogatorState(cID stringInterrogatorID);

    /**
     * @brief Get the Text List Presenter State
     * 
     * @param textListPresenterID The ID of the TextListPresenter Property
     * @return TextListPresenterState 
     */
    TextListPresenterState getTextListPresenterState(cID textListPresenterID);

    /**
     * @brief Update the state of simple-state property.
     * NOTE: If the device is connected a property-state-update transmission will be sent.
     * 
     * @param propertyID The ID of the property to update.
     * @param value The state-value.
     */
    void updateSimplePropertyState(cID propertyID, unsigned int value);
    
    /**
     * @brief Update the state of the specified RGB-Selector.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param rgbSelectorID The ID of the RGB-Selector to update.
     * @param state The new state. (Type: RGBSelectorState)
     */
    void updateRGBState(cID rgbSelectorID, RGBSelectorState& state);

    /**
     * @brief Update the state of the specified Extended-Level-Selector.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param extendedLevelSelectorID The ID of the Extended-Level-Selector to update.
     * @param state The new state. (Type: ExtendedLevelSelectorState)
     */
    void updateExLevelState(cID extendedLevelSelectorID, ExtendedLevelSelectorState& state);

    /**
     * @brief Update the state of the specified Time-Selector.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param timeSelectorID The ID of the Time-Selector to update.
     * @param state The new state. (Type: TimeSelectorState)
     */
    void updateTimeSelectorState(cID timeSelectorID, TimeSelectorState& state);

    /**
     * @brief Update the state of the specified Time-Frame-Selector.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param timeFrameSelectorID The ID of the Time-Frame-Selector to update.
     * @param state The new state. (Type: TimeFrameSelectorState)
     */
    void updateTimeFrameSelectorState(cID timeFrameSelectorID, TimeFrameSelectorState& state);

    /**
     * @brief Update the state of the specified Date-Selector.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param dateSelectorID The ID of the Date-Selector to update.
     * @param state The new state. (Type: DateSelectorState)
     */
    void updateDateSelectorState(cID dateSelectorID, DateSelectorState& state);

    /**
     * @brief Update the state of the specified Unlock-Control property.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param unlockControlID The ID of the Unlock-Control-Property.
     * @param state The new state. (Type: UnlockControlState)
     */
    void updateUnlockControlState(cID unlockControlID, UnlockControlState& state);

    /**
     * @brief Update the state of the specified Navigator.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param navigatorID The ID of the Navigator.
     * @param state The new state. (Type: NavigatorState)
     */
    void updateNavigatorState(cID navigatorID, NavigatorState& state);

    /**
     * @brief Update the state of the specified BarGraph.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * This is a slow method!: To set successive data more efficient use the fast-data-pipe methods:
     * "BarGraph_FastDataPipe_SetSingleBarValue" / "BarGraph_FastDataPipe_SetAllBarValues".
     * 
     * @param barGraphID The ID of the BarGraph to update.
     * @param state The new state. (Type: BarGraphState)
     */
    void updateBarGraphState(cID barGraphID, BarGraphState& state);

    /**
     * @brief Update the state of the specified LineGraph.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * This is a slow method!: To set successive data more efficient use the fast-data-pipe methods:
     * "LineGraph_FastDataPipe_AddPoint" / "LineGraph_FastDataPipe_AddDataPoints" / "LineGraph_FastDataPipe_ResetDataPoints"
     * 
     * @param lineGraphID The ID of the LineGraph to update.
     * @param state The new state. (Type: LineGraphState)
     */
    void updateLineGraphState(cID lineGraphID, LineGraphState& state);

    /**
     * @brief Update the state of the specified String-Interrogator.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param stringInterrogatorID The ID of the String-Interrogator to update.
     * @param state The new state. (Type: StringInterrogatorState)
     */
    void updateStringInterrogatorState(cID stringInterrogatorID, StringInterrogatorState& state);

    /**
     * @brief Update the state of the specified Text-List-Presenter.
     * NOTE: If the device is connected a complex-state-update transmission will be sent.
     * 
     * @param textListPresenterID The ID of the Text-List-Presenter to update.
     * @param state The new state. (Type: TextListPresenterState)
     */
    void updateTextListPresenterState(cID textListPresenterID, TextListPresenterState& state);

    /**
     * @brief Sets the value of a bar in the specified barGraph property using the fast-data pipe bypass transmission.
     * This only makes sense if the specified barGraph property page is opened. Use the notification callback to detect when a
     * complex property page is opened or closed. A data stream should only be active if the receiving page is invoked, otherwise
     * this could interfere or block other transmissions.
     * 
     * @param barGraphID The ID of the barGraph property
     * @param barIndex The index of the bar inside the barGraph property
     * @param barValue The value to set. Type: <float>
     */
    void barGraphFastDataPipeSetSingleBarValue(cID barGraphID, unsigned int barIndex, float barValue);

    /**
     * @brief Sets the value of all bars in the specified barGraph property using the fast-data pipe bypass transmission.
     * This only makes sense if the specified barGraph property page is opened. Use the notification callback to detect when a
     * complex property page is opened or closed. A data stream should only be active if the receiving page is invoked, otherwise
     * this could interfere or block other transmissions.
     * 
     * @param barGraphID The ID of the barGraph property
     * @param bData The bar-data to set. Type: <BarData> in an <itemCollection>
     */
    void barGraphFastDataPipeSetAllBarValues(cID barGraphID, itemCollection<BarData>& bData);

    /**
     * @brief Resets the whole line-data in the specified lineGraph using the fast-data pipe bypass transmission.
     * This only makes sense if the specified lineGraph property page is opened. Use the notification callback to detect when a
     * complex property page is opened or closed. A data stream should only be active if the receiving page is invoked, otherwise
     * this could interfere or block other transmissions.
     *      
     * @param lineGraphID The ID of the lineGraph property
     * @param lData The line-data to set. Type: <LineGraphDataPoints>
     */
    void lineGraphFastDataPipeResetDataPoints(cID lineGraphID, LineGraphDataPoints& lData);

    /**
     * @brief Adds new data-points to the specified lineGraphs line-data using the fast-data-pipe bypass transmission.
     * This only makes sense if the specified lineGraph property page is opened. Use the notification callback to detect when a
     * complex property page is opened or closed. A data stream should only be active if the receiving page is invoked, otherwise
     * this could interfere or block other transmissions. The lineGraph scope will be shifted in the given direction by the corresponding value. 
     * 
     * @param lineGraphID The ID of the lineGraph property
     * @param lData The line-data to set. Type: <LineGraphDataPoints>
     * @param shifter (optional) The value to be shifted. Type: <float>
     * @param dir (optional) The direction in which to shift. Type: <LineGraphGridShiftDirection>
     */
    void lineGraphFastDataPipeAddDataPoints(cID lineGraphID, LineGraphDataPoints& lData, float shifter, LineGraphGridShiftDirection dir); 

    /**
     * @brief Adds new data-points to the specified lineGraphs line-data using the fast-data-pipe bypass transmission.
     * This only makes sense if the specified lineGraph property page is opened. Use the notification callback to detect when a
     * complex property page is opened or closed. A data stream should only be active if the receiving page is invoked, otherwise
     * this could interfere or block other transmissions.
     * 
     * @param lineGraphID The ID of the lineGraph property
     * @param lData The line-data to set. Type: <LineGraphDataPoints>
     */
    void lineGraphFastDataPipeAddDataPoints(cID lineGraphID, LineGraphDataPoints& lData);

    /**
     * @brief Adds a new data-point to the specified lineGraphs line-data using the fast-data-pipe bypass transmission.
     * This only makes sense if the specified lineGraph property page is opened. Use the notification callback to detect when a
     * complex property page is opened or closed. A data stream should only be active if the receiving page is invoked, otherwise
     * this could interfere or block other transmissions. The lineGraph scope will be shifted in the given direction by the corresponding value. 
     * 
     * @param lineGraphID The ID of the lineGraph property
     * @param pPoint The point to set. Type: <LPPOINT>
     * @param shifter The shift distance. Type: <float>
     * @param dir The direction in which to shift. Type: <LineGraphGridShiftDirection>
     */
    void lineGraphFastDataPipeAddPoint(cID lineGraphID, LPPOINT pPoint, float shifter, LineGraphGridShiftDirection dir);

    /**
     * @brief Adds a new data-point to the specified lineGraphs line-data using the fast-data-pipe bypass transmission.
     * This only makes sense if the specified lineGraph property page is opened. Use the notification callback to detect when a
     * complex property page is opened or closed. A data stream should only be active if the receiving page is invoked, otherwise
     * this could interfere or block other transmissions.
     * 
     * @param lineGraphID The ID of the lineGraph property
     * @param pPoint The point to set. Type: <LPPOINT>
     */
    void lineGraphFastDataPipeAddPoint(cID lineGraphID, LPPOINT pPoint);

private:
    LaRoomyAppImplementation(){}

    // bluetooth name with default value
    String bluetoothName = "My BLE Device";

    // UUID's with default values
    String serviceUUID = "b47f725f-5fca-45b9-998f-f828388d044f";
    String txCharacteristicUUID = "124c0402-da98-4d2b-8492-e712f8036997";
    String rxCharacteristicUUID = "124c0402-da99-4d2b-8492-e712f8036997";

    // instance params
    static bool laRoomyAppImplInstanceCreated;
    static LaRoomyAppImplementation* hInstance;

    // private properties
    bool hasBegun = false;
    bool is_connected = false;
    bool is_monitor_enabled = false;
    bool auto_refresh_states = true;
    bool auto_handle_binding = false;
    bool isStandAloneMode = false;

    // BLE system objects
    BLEService *pService = nullptr;
    BLECharacteristic *pTxCharacteristic = nullptr;
    BLECharacteristic *pRxCharacteristic = nullptr;

    TransmissionControl tmc;
    String lastLangID = "en";
    unsigned int deviceImageID = 0;

    // callback for remote user events and descriptions
    ILaroomyAppCallback *pLrCallback = nullptr;
    IElementDescriptionCallback *pDescriptionCallback = nullptr;

    // config parameters
    bool cachingPermission = false;
    bool deviceBindingAuthenticationRequired = false;
    bool propertyLoadingDone = true;
    cID currentPropertyPageID = ID_DEVICE_MAIN_PAGE;

    // properties & groups
    itemCollection<DeviceProperty> deviceProperties;
    itemCollection<DevicePropertyGroup> devicePropertyGroups;

    // complex property states
    itemCollection<RGBSelectorState> rgbStates;
    itemCollection<ExtendedLevelSelectorState> extendedLevelStates;
    itemCollection<TimeSelectorState> timeSelectorStates;
    itemCollection<TimeFrameSelectorState> timeFrameSelectorStates;
    itemCollection<DateSelectorState> dateSelectorStates;
    itemCollection<UnlockControlState> unlockControlStates;
    itemCollection<NavigatorState> navigatorStates;
    itemCollection<BarGraphState> barGraphStates;
    itemCollection<LineGraphState> lineGraphStates;
    itemCollection<StringInterrogatorState> stringInterrogatorStates;
    itemCollection<TextListPresenterState> textListPresenterStates;

    // connect callback methods
    static void connectHandler(BLEDevice central);
    static void disconnectHandler(BLEDevice central);

    // characterisic callback method
    static void rxCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic);

    // ble send data method
    void sendData(const String& data);

    // private property add
    void _addDeviceProperty(const DeviceProperty& p, bool sendCommand);

    // state init methods
    void initializeComplexPropertyState(const DeviceProperty& p);
    void initDefaultRGBState(cID propertyID);
    void initRGBStateFromInitialStateString(cID propertyID, const String &iss);
    void initDefaultExLevelState(cID propertyID);
    void initExLevelStateFromInitialStateString(cID propertyID, const String &iss);
    void initDefaultTimeSelectorState(cID propertyID);
    void initTimeSelectorStateFromInitialStateString(cID properyID, const String &iss);
    void initDefaultTimeFrameSelectorState(cID propertyID);
    void initTimeFrameSelectorStateFromInitialStateString(cID propertyID, const String &iss);
    void initDefaultDateSelectorState(cID propertyID);
    void initDateSelectorStateFromInitialStateString(cID propertyID, const String & iss);
    void initDefaultUnlockControlState(cID propertyID);
    void initUnlockControlStateFromInitialStateString(cID propertyID, const String &iss);
    void initDefaultNavigatorState(cID propertyID);
    void initNavigatorStateFromInitialStateString(cID propertyID, const String &iss);
    void initDefaultBarGraphState(cID propertyID);
    void initBarGraphStateFromInitialStatePointer(cID propertyID, BarGraphState* state);
    void initDefaultLineGraphState(cID propertyID);
    void initLineGraphStateFromInitialStatePointer(cID propertyID, LineGraphState* state);
    void initDefaultStringInterrogatorState(cID propertyID);
    void initStringInterrogatorStateFromInitialStatePointer(cID propertyID, StringInterrogatorState* state);
    void initDefaultTextListPresenterState(cID propertyID);
    void initTextListPresenterStateFromInitialStateString(cID propertyID, const String& iss);

    // transmission handler
    void onInitRequest();
    void onPropertyRequest(const String& data);
    void onGroupRequest(const String& data);
    void onPropertyStateRequest(const String& data);
    void onPropertyExecutionCommand(const String& data);
    void onBindingTransmission(const String& data);
    void onNotificationTransmission(const String& data);

    void ble_start();
    void ble_restart();
    void ble_terminate();

    // helper
    unsigned int propertyIndexFromPropertyID(unsigned int pId);
    unsigned int propertyTypeFromPropertyIndex(unsigned int propertyIndex);
    unsigned int propertyIDFromPropertyIndex(unsigned int propertyIndex);
    unsigned int groupIndexFromGroupID(cID groupID);
    bool validatePropertyID(cID pID);
    void applyDeviceImageIDToBluetoothName();
    void rearrangeGroupIndexes();
    
    // internal update methods
    void _updateRGBState(RGBSelectorState& state, bool send);
    void _updateExLevelState(ExtendedLevelSelectorState& state, bool send);
    void _updateExLevelStateFromExecutionCommand(ExtendedLevelSelectorState &s);
    void _updateTimeSelectorState(TimeSelectorState& state, bool send);
    void _updateTimeFrameSelectorState(TimeFrameSelectorState& state, bool send);
    void _updateDateSelectorState(DateSelectorState& state, bool send);
    void _updateUnlockControlState(UnlockControlState& state, bool send);
    void _updateNavigatorState(NavigatorState& state, bool send);
    void _updateBarGraphState(BarGraphState& state, bool send);
    void _updateLineGraphState(LineGraphState& state, bool send);
    void _updateStringInterrogatorState(StringInterrogatorState& state, bool send);
    void _updateTextListPresenterState(TextListPresenterState& state, bool send);

    void sendBindingResponse(BindingResponseType t);
    bool checkUnlockControlPin(UnlockControlState& state);
};

/**
 * @brief Access the LaRoomy API
 * 
 */
#define LaRoomyApi (*LaRoomyAppImplementation::GetInstance())

/**
 * @brief The device-property base class. Use this to construct a property or use a specific property-type-class
 * 
 */
class DeviceProperty : public ICollectable<DeviceProperty> {
    friend LaRoomyAppImplementation;
public:
    DeviceProperty(){}
    DeviceProperty(const DeviceProperty& p){
        this->copy(p);
    }
    DeviceProperty(Button &b);
    DeviceProperty(Switch &s);
    DeviceProperty(LevelSelector &ls);
    DeviceProperty(LevelIndicator &li);
    DeviceProperty(TextDisplay &td);
    DeviceProperty(OptionSelector &os);
    DeviceProperty(RGBSelector &rs);
    DeviceProperty(ExtendedLevelSelector &els);
    DeviceProperty(TimeSelector &ts);
    DeviceProperty(TimeFrameSelector &tfs);
    DeviceProperty(DateSelector &dss);
    DeviceProperty(UnlockControl &uc);
    DeviceProperty(NavigatorControl &nc);
    DeviceProperty(BarGraph &bg);
    DeviceProperty(LineGraph &lg);
    DeviceProperty(StringInterrogator& si);
    DeviceProperty(TextListPresenter& tlp);

    ~DeviceProperty();

    DeviceProperty& operator= (const DeviceProperty& p){
        this->copy(p);
        return *this;
    }

	bool operator== (const DeviceProperty& p){
        if((this->propertyType == p.propertyType) && (this->imageID == p.imageID) && (this->descriptor == p.descriptor)
            && (this->groupIndex == p.groupIndex) && (this->flags == p.flags)){
                return true;
            }
        else {
            return false;
        }
    }

    unsigned int propertyType = 0;
    uint8_t imageID = 0;
    uint8_t propertyState = 0;

    /** @brief NOTE: The property ID must be unique for a property element! And the property ID must not be zero!*/ 
    unsigned int propertyID = 0;

    bool isEnabled = true;

    String descriptor = "not set";

private:
    unsigned int groupIndex = 0;
    unsigned int flags = 0;
    cID relatedGroupID = 0;
    String initialStateDefinition;
    BarGraphState* barGraphStateHolder = nullptr;
    LineGraphState* lineGraphStateHolder = nullptr;
    StringInterrogatorState* stringInterrogatorStateHolder = nullptr;

    void updateFlags();
    String toTransmissionString(TransmissionSubType t, unsigned int propertyIndex);
    void copy(const DeviceProperty& p);
    void clearStateHolder();
};

/**
 * @brief The helper object to arrange properties in groups. Make sure to add properties to the group before adding the group.
 * 
 */
class DevicePropertyGroup : public ICollectable<DevicePropertyGroup> {
    friend LaRoomyAppImplementation;
public:
    DevicePropertyGroup(){}
    DevicePropertyGroup(const DevicePropertyGroup& g){
        this->copy(g);
    }

    DevicePropertyGroup& operator= (const DevicePropertyGroup& g){
        this->copy(g);
        return *this;
    }
    bool operator== (const DevicePropertyGroup& g){
        if((this->imageID == g.imageID) && (this->descriptor == g.descriptor) && (this->propertyList.GetCount() == g.propertyList.GetCount()) && (this->groupID == g.groupID)){
            return true;
        }
        else {
            return false;
        }
    }

    /**
     * @brief Add a deviceProperty to this group
     * 
     * @param p The property to add
     */
    void addDeviceProperty(const DeviceProperty& p){
        this->propertyList.AddItem(p);
    }

    String descriptor = "not set";

    unsigned int imageID = 0;
    cID groupID = 0;

private:
    itemCollection<DeviceProperty> propertyList;
    unsigned int propertyCount = 0;

    String toTransmissionString(TransmissionSubType t, unsigned int groupIndex);

    void copy(const DevicePropertyGroup& g){
        // this->descriptor = "";
        this->descriptor = g.descriptor;
        this->imageID = g.imageID;
        this->propertyList.Clear();
        this->propertyList = g.propertyList;
        this->groupID = g.groupID;
        this->propertyCount = g.propertyCount;
    }
};

/**
 * @brief Collection to arrange COLOR-Struct objects
 * 
 */
typedef itemCollection<COLOR> ColorCollection;

/**
 * @brief Helper-Class for the control of rgb output
 * 
 */
class RGBControl {
public:
    /**
     * @brief Construct a new RGBControl object
     * 
     * @param RedPin The output pin for the red value
     * @param GreenPin The output pin for the green value
     * @param BluePin The output pin for the blue valie
     */
    RGBControl(pin_size_t RedPin, pin_size_t GreenPin, pin_size_t BluePin);

    ~RGBControl();

    /**
     * @brief Start RGB control on the specified pins
     */
    void begin();

    /**
     * @brief End the RGB control on the specified pins
     * 
     */
    void end();

    /**
     * @brief Set the Fade Delay Value in milliseconds.
     * NOTE: The fading is generated by increasing or decreasing the current value until is reaches the desired value.
     * This value is the delay before the next single increasing or decreasing action will be taken. Example: when fading from OFF to full RED color, the
     * Red-Value 0 must be increased to 255. So this delay value is applied 255 times. If this value is set to 10 milliseconds, the fade takes up to 2,55 seconds.
     * The default value is 3 milliseconds, this makes 765 millis fading time.
     * 
     * @param fadeDelay The delay before the next increasing or decreasing action will be applied.
     */
    void setFadeDelayValue(unsigned int fadeDelay){
        this->fadeTimeOut = (unsigned long)fadeDelay;
    }

    /**
     * @brief Disable all output on the RGB pins.
     * 
     */
    void off();

    /**
     * @brief Set the Custom Color Selection for the color fading program. This must be a collection of COLOR struct values.
     * 
     * @param colorSel Type: itemCollection<COLOR>
     */
    void setCustomColorSelection(ColorCollection& colorSel);

    /**
     * @brief Set the output on the RGB pins directly from a RGBSelectorState class object
     * 
     * @param state Type: RGBSelectorState
     */
    void applyStateChange(const RGBSelectorState& state);

    /**
     * @brief Change the current output to the desired color value.
     * NOTE: If a program is active, it will be stopped.
     * 
     * @param red Type: uint8_t - The red color value
     * @param green Type: uint8_t - The green color value
     * @param blue Type: uint8_t - The blue color value
     * @param tType Type: RGBTransitionType Enum - Defines if a fading to the new value should occur or if the transition should happen immediately
     */
    void changeRGBColor(uint8_t red, uint8_t green, uint8_t blue, RGBTransitionType tType = RGBTransitionType::SOFT_TRANSITION);

    /**
     * @brief Change the current output to the desired color value.
     * NOTE: If a program is active, it will be stopped.
     * 
     * @param c Type Pointer to a COLOR struct value
     */
    void changeRGBColor(PCOLOR c){
        this->changeRGBColor(c->redPart, c->greenPart, c->bluePart);
    }

    /**
     * @brief Change the output of the RGB pins in relation to the selected program.
     * 
     * @param program Type: RGBTransitionProgram Enum - The desired program (slow to fast)
     * @param tType Type: RGBTransitionType Enum - Defines if a fading to the new value should occur or if the transition should happen immediately
     */
    void changeRGBProgram(RGBColorTransitionProgram program, RGBTransitionType tType);

    /**
     * @brief The method must be placed inside the main-loop to handle desired output and timing features
     * 
     */
    void onLoop();

private:
    uint8_t redValue = 0;
    uint8_t greenValue = 0;
    uint8_t blueValue = 0;

    uint8_t currentRedValue = 0;
    uint8_t currentGreenValue = 0;
    uint8_t currentBlueValue = 0;

    pin_size_t redPin = 0;
    pin_size_t greenPin = 0;
    pin_size_t bluePin = 0;
    
    bool hardTrans = false;
    bool hasBegun = false;
    unsigned int currentColorIndex = 0;

    ColorCollection colorSelection;

    unsigned long fadeTimer = 0;
    unsigned long fadeTimeOut = 3;
    unsigned long progTimer = 0;
    unsigned long progTimeOut = 20;
    unsigned long tmSaver = 0;

    RGBColorTransitionProgram tProgram = RGBColorTransitionProgram::RCTP_NO_TRANSITION;
};

/**
 * @brief Color value definitions for use without instantiation
 * 
 */
class Colors {
public:
    static COLOR Red;
    static COLOR Green;
    static COLOR Blue;
    static COLOR Cyan;
    static COLOR Magenta;
    static COLOR Orange;
    static COLOR Yellow;
    static COLOR White;
    static COLOR GreenYellow;
    static COLOR YellowGreen;
    static COLOR Purple;
    static COLOR Pink;
    static COLOR Mint;
    static COLOR Turquoise;
    static COLOR OrangeRed;
    static COLOR SeaGreen;
    static COLOR Gold;
    static COLOR PaleRed;

    static COLOR LightRed;
    static COLOR LightGreen;
    static COLOR LightBlue;
    static COLOR LightYellow;
    static COLOR LightPink;
    static COLOR LightCyan;
    static COLOR LightPurple;

    static COLOR DarkRed;
    static COLOR DarkGreen;
    static COLOR DarkBlue;
    static COLOR DarkYellow;
    static COLOR DarkPink;
    static COLOR DarkCyan;
    static COLOR DarkPurple;
};

/**
 * @brief The state of a rgb selector property
 * 
 */
class RGBSelectorState : ICollectable<RGBSelectorState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    RGBSelectorState(){};
    RGBSelectorState(const RGBSelectorState& state){
        this->copy(state);
    }
    RGBSelectorState(const COLOR& col){
        this->redValue = col.redPart;
        this->greenValue = col.greenPart;
        this->blueValue = col.bluePart;
    }    

    bool isOn = false;
    unsigned int flags = 0;
    RGBColorTransitionProgram colorTransitionProgram = RGBColorTransitionProgram::RCTP_NO_TRANSITION;
    unsigned int redValue = 255;
    unsigned int greenValue = 255;
    unsigned int blueValue = 255;
    RGBTransitionType transitionType = RGBTransitionType::SOFT_TRANSITION;

    RGBSelectorState& operator= (const RGBSelectorState& state);
    RGBSelectorState& operator= (const COLOR& col);
    bool operator== (const RGBSelectorState& state);
    bool operator!= (const RGBSelectorState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    void fromExecutionString(const String& data);
    void copy(const RGBSelectorState& s);
};

/**
 * @brief The state of a extended-level-selector property
 * 
 */
class ExtendedLevelSelectorState : public ICollectable<ExtendedLevelSelectorState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    ExtendedLevelSelectorState(){}
    ExtendedLevelSelectorState(const ExtendedLevelSelectorState& state){
        this->copy(state);
    }

    bool isOn = false;
    int16_t levelValue = 0;
    int16_t maxValue = 100;
    int16_t minValue = 0;
    bool showOnOffSwitch = true;
    bool transmitOnlyStartAndEndOfTracking = false;
    ExLevelTrackingType trackingType = ExLevelTrackingType::ELTT_UNUSED;

    ExtendedLevelSelectorState& operator=(const ExtendedLevelSelectorState& state);
    bool operator==(const ExtendedLevelSelectorState& state);
    bool operator!=(const ExtendedLevelSelectorState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    void fromDataHolderString(const String& data);

    void fromExecutionString(const String& data);
    void copy(const ExtendedLevelSelectorState& state);
};

/**
 * @brief StateTime - Single definition of a point in time
 * 
 */
typedef struct _STATETIME {

    _STATETIME() {
        this->hour = 0;
        this->minute = 0;
    }

    _STATETIME(unsigned int h, unsigned int m) {
        this->hour = h;
        this->minute = m;
    }

    _STATETIME(const _STATETIME& t) {
        this->hour = t.hour;
        this->minute = t.minute;
    }

    unsigned int hour;
    unsigned int minute;

    _STATETIME& operator=(const _STATETIME& time) {
        this->hour = time.hour;
        this->minute = time.minute;
        return *this;
    }

    bool operator==(const _STATETIME& time) const {
        if ((this->hour == time.hour) && (this->minute == time.minute)) {
            return true;
        }
        else {
            return false;
        }
    }

    bool operator!=(const _STATETIME& time) const {
        return !(*this == time);
    }

    bool operator>(const _STATETIME& time) const {
        if (this->hour > time.hour) {
            return true;
        }
        else if ((this->hour == time.hour) && (this->minute > time.minute)) {
            return true;
        }
        else {
            return false;
        }
    }

    bool operator<(const _STATETIME& time) const {
        if (this->hour < time.hour) {
            return true;
        }
        else if ((this->hour == time.hour) && (this->minute < time.minute)) {
            return true;
        }
        else {
            return false;
        }
    }

    bool operator>=(const _STATETIME& time) const {
        if ((*this == time) || (*this > time)) {
            return true;
        }
        else {
            return false;
        }
    }

    bool operator<=(const _STATETIME& time) const {
        if ((*this == time) || (*this < time)) {
            return true;
        }
        else {
            return false;
        }
    }

} STATETIME, * LPSTATETIME;


/**
 * @brief The state of a time-selector property
 * 
 */
class TimeSelectorState : ICollectable<TimeSelectorState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    TimeSelectorState(){}
    TimeSelectorState(const TimeSelectorState& state){
        this->copy(state);
    }

    unsigned int hour = 0;
    unsigned int minute = 0;

    STATETIME toStateTime();
    TimeSelectorState& operator=(const TimeSelectorState& state);
    bool operator==(const TimeSelectorState& state);
    bool operator!=(const TimeSelectorState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    void fromExecutionString(const String& data);
    void copy(const TimeSelectorState& state);
};

/**
 * @brief The state of a time-frame-selector property
 * 
 */
class TimeFrameSelectorState : public ICollectable<TimeFrameSelectorState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    TimeFrameSelectorState(){}
    TimeFrameSelectorState(const TimeFrameSelectorState& state){
        this->copy(state);
    }

    STATETIME startTime = {0,0};
    STATETIME endTime = {0,0};

    bool checkIfTimeIsInFrame(const STATETIME& pTime);
    TimeFrameSelectorState& operator=(const TimeFrameSelectorState& state);
    bool operator==(const TimeFrameSelectorState& state);
    bool operator!=(const TimeFrameSelectorState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    void fromExecutionString(const String& data);
    void copy(const TimeFrameSelectorState& state);
};

/**
 * @brief The state of a date-selector property
 * 
*/
class DateSelectorState : public ICollectable<DateSelectorState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    DateSelectorState(){}
    DateSelectorState(const DateSelectorState& state){
        this->copy(state);
    }

    unsigned int day = 1;
    unsigned int month = 1;
    unsigned int year = 2023;

    DateSelectorState& operator=(const DateSelectorState& state);
    bool operator==(const DateSelectorState& state);
    bool operator!=(const DateSelectorState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    void fromExecutionString(String data);
    void copy(const DateSelectorState& state);

};

/**
 * @brief The state of a unlock-control property
 * 
 */
class UnlockControlState : public ICollectable<UnlockControlState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    UnlockControlState(){}
    UnlockControlState(const UnlockControlState& state){
        this->copy(state);
    }

    bool unlocked = false;
    unsigned int mode = 0;
    String pin = "not set";

    UnlockControlState& operator=(const UnlockControlState& state);
    bool operator==(const UnlockControlState& state);
    bool operator!=(const UnlockControlState& state);

private:
    cID associatedPropertyID = 0;
    unsigned int flags = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    String toDataHolderString();
    void fromExecutionString(const String& data);
    void copy(const UnlockControlState& state);
};

/**
 * @brief The state of a navigator-control property
 * 
 */
class NavigatorState : public ICollectable<NavigatorState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    NavigatorState(){}
    NavigatorState(const NavigatorState& state){
        this->copy(state);
    }

    unsigned int buttonType = NavigatorButtonTypes::NO_BUTTON;
    unsigned int actionType = NavigatorActionTypes::NAT_UNUSED;

    void setButtonVisibility(bool up, bool right, bool down, bool left, bool mid);

    NavigatorState& operator=(const NavigatorState& state);
    bool operator==(const NavigatorState& state);
    bool operator!=(const NavigatorState& state);

private:
    cID associatedPropertyID = 0;
    unsigned int visibilityFlags = 0x1F;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    void fromExecutionString(const String& data);
    void copy(const NavigatorState& state);
};

/**
 * @brief The data-model for a single bar in a barGraph property
 * 
 */
class BarData : public ICollectable<BarData> {
public:
    BarData(){}
    BarData(const BarData& bData){
        this->barName = bData.barName;
        this->barValue = bData.barValue;
    }
    BarData(const String& bar_name, float bar_value) {
        this->barName = bar_name;
        this->barValue = bar_value;
    }
    BarData(const char* bar_name, float bar_value){
        this->barName = bar_name;
        this->barValue = bar_value;
    }
    BarData(float bar_value){
        this->barValue = bar_value;
    }

    String barName = "_";
    float barValue = 0;

    BarData& operator=(const BarData& bData){
        this->barName = bData.barName;
        this->barValue = bData.barValue;
        return *this;
    }

    String toString(unsigned int barIndex){
        String bData;
        bData += Convert::numToChar(barIndex);
        bData += "::";
        if(this->barName.length() == 0){
            bData += '_';
        }
        else {
            bData += this->barName;
        }
        bData += "::";
        bData += this->barValue;
        bData += ";;";

        return bData;
    }
};

/**
 * @brief The state of a barGraph property
 * 
 */
class BarGraphState : public ICollectable<BarGraphState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    BarGraphState(){}
    BarGraphState(const BarGraphState& state){
        this->copy(state);
    }

    void addBar(const BarData& bData){
        this->barDataList.AddItem(bData);
    }
    void addBar(const char* barName, float barValue){
        auto bd = BarData(barName, barValue);
        this->addBar(bd);
    }
    void removeBarAt(unsigned int index){
        this->barDataList.RemoveAt(index);
    }
    void changeBarValueAt(unsigned int index, float newValue){
        if(index < this->barDataList.GetCount()){
            this->barDataList.getObjectCoreReferenceAt(index)->barValue = newValue;
        }
    }
    void changeBarDataAt(unsigned int index, const BarData& bd){
        this->barDataList.ReplaceAt(index, bd);
    }
    BarData getBarDataAt(unsigned int index){
        if(index < this->barDataList.GetCount()){
            this->barDataList.GetAt(index);
        }
        return BarData();
    }

    bool useValueAsBarDescriptor = false;
    bool useFixedMaximumValue = false;
    float fixedMaximumValue = 0;

    BarGraphState&  operator=(const BarGraphState& state);

    bool operator==(const BarGraphState& state);
    bool operator!=(const BarGraphState& state);

private:
    cID associatedPropertyID = 0;

    itemCollection<BarData> barDataList;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    // NOTE: fromExecutionString(..) not necessary -> this property type has no execution

    void copy(const BarGraphState& state);
};

/**
 * @brief The data for the line in a lineGraph property (Add data-points to generate a line)
 * 
 */
class LineGraphDataPoints {
public:
    LineGraphDataPoints(){}
    LineGraphDataPoints(const LineGraphDataPoints& dataPoints){
        this->points = dataPoints.points;
    }
    void addPoint(const POINT &p){
        this->points.AddItem(p);
    }
    void clear(){
        this->points.Clear();
    }
    LineGraphDataPoints& operator=(const LineGraphDataPoints& dataPoints){
        this->points = dataPoints.points;
        return *this;
    }
    String toString();

    unsigned int count(){
        return this->points.GetCount();
    }

    bool operator==(const LineGraphDataPoints& lgdp){
        if(this->points.GetCount() == lgdp.points.GetCount()){
            for(unsigned int i = 0; i < this->points.GetCount(); i++){
                if(this->points.GetAt(i) != lgdp.points.GetAt(i)){
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool operator!=(const LineGraphDataPoints& lgdp){
        return (*this == lgdp) ? false : true;
    }

private:
    itemCollection<_POINT> points;
};

/**
 * @brief The state of a lineGraph property
 * 
 */
class LineGraphState : public ICollectable<LineGraphState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    LineGraphState(){}
    LineGraphState(const LineGraphState& state){
        this->copy(state);
    }

    bool drawGridLines = false;
    bool drawAxisValues = false;
    float xMinValue = -10;
    float xMaxValue = 10;
    float yMinValue = -10;
    float yMaxValue = 10;
    float xIntersection = 2;
    float yIntersection = 2;

    LineGraphDataPoints lineGraphPoints;

    LineGraphState& operator=(const LineGraphState& state);

    bool operator==(const LineGraphState& state);
    bool operator!=(const LineGraphState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    // NOTE: fromExecutionString(..) not necessary -> this property type has no execution

    void copy(const LineGraphState& state);
};

/**
 * @brief The state of a string-interrogator property
 * 
 */
class StringInterrogatorState : public ICollectable<StringInterrogatorState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    StringInterrogatorState(){}
    StringInterrogatorState(const StringInterrogatorState& state){
        this->copy(state);
    }
    bool fieldOneVisible = true;
    bool fieldTwoVisible = true;
    StringInterrogatorFieldInputType fieldOneInputType = StringInterrogatorFieldInputType::SI_INPUT_TEXT;
    StringInterrogatorFieldInputType fieldTwoInputType = StringInterrogatorFieldInputType::SI_INPUT_TEXT;
    bool navigateBackOnButtonPress = true;
    bool acceptNonAsciiCharacters = false;

    String buttonText;

    String fieldOneDescriptor;
    String fieldTwoDescriptor;
    String fieldOneHint;
    String fieldTwoHint;
    String fieldOneContent;
    String fieldTwoContent;

    StringInterrogatorState& operator=(const StringInterrogatorState& state);

    bool operator==(const StringInterrogatorState& state);
    bool operator!=(const StringInterrogatorState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    void fromExecutionString(const String& data);
    void copy(const StringInterrogatorState& state);
};

/**
 * @brief The state of a text-list-presenter property
 * 
 */
class TextListPresenterState : public ICollectable<TextListPresenterState> {
    friend LaRoomyAppImplementation;
    friend DeviceProperty;
public:
    TextListPresenterState(){}
    TextListPresenterState(const TextListPresenterState& state){
        this->copy(state);
    }
    bool useBackgroundStack = true;

    TextListPresenterState& operator=(const TextListPresenterState& state);

    bool operator==(const TextListPresenterState& state);
    bool operator!=(const TextListPresenterState& state);

private:
    cID associatedPropertyID = 0;

    String toStateString(unsigned int propertyIndex, TransmissionSubType t);
    // NOTE: fromExecutionString(..) not necessary -> this property type has no execution
    
    void copy(const TextListPresenterState& state);
};


// specific property types: *******************************************************************

#define PTYPE_CAST_ERROR "ptype-cast-error"
#define PINIT_PROP_NOT_FOUND_ERROR  "prop-not-found-error"

/**
 * @brief The Button property. A simple button will be displayed in the list on the device-main-page of the app.
 * You can define an image, the text inside the button and the text describing the button.
 * A button has no state. If the button is pressed in the app an event will be reported. To catch this event:
 * subclass the ILaroomyAppCallback and override the "onButtonPressed" method. Make sure the button-ID is unique, otherwise this could cause problems.
 */
class Button {
public:
    Button();
    Button(cID propertyID);
    Button(const Button& b);
    Button(const DeviceProperty& dp);

    String buttonText;
    String buttonDescriptor;
    unsigned int buttonID = 0;
    uint8_t imageID = 0;
    bool isEnabled = true;

    String toDevicePropertyDescriptor();

    Button& operator=(const Button& b);
    Button& operator=(const DeviceProperty& dp);

    void update();

private:
    void split(const String& descriptor);
};

/**
 * @brief The Switch property. A simple switch will be displayed in the list on the device-main page of the app.
 * You can define an image and the text for the description of the switch. A switch has a simple boolean state. If the switch is pressed and the 
 * switch-state changes, an event will be reported. To catch this event: subclass the ILaroomyAppCallback and override the "onSwichStateChanged" method.
 * Make sure the switch-ID is unique, otherwise this could cause problems.
 */
class Switch {
public:
    Switch();
    Switch(cID propertyID);
    Switch(const Switch& s);
    Switch(const DeviceProperty& dp);

    String switchDescription;
    unsigned int switchID = 0;
    uint8_t imageID = 0;
    bool switchState = false;
    bool isEnabled = true;

    Switch& operator=(const Switch& s);
    Switch& operator=(const DeviceProperty& dp);

    void update();
};
/**
 * @brief The level-Selector property. A simple slider will be displayed as popup if the list element is clicked.
 * You can define the text for the description of the element and the image. The simple level-selector has an 8bit state ( 0 = 0% to 255 = 100% ).
 * The value will be displayed as percentage value inside of the element.
 * If the slider position changes an event will be reported. To catch this event: subclass the ILaroomyAppCallback interface and override the "onLevelSelectorValueChanged" method.
 * Make sure the levelSelectorID is unique, otherwise this could cause problems.
 */
class LevelSelector {
public:
    LevelSelector();
    LevelSelector(cID propertyID);
    LevelSelector(const LevelSelector& ls);
    LevelSelector(const DeviceProperty& dp);

    String levelSelectorDescription;
    uint8_t level = 0;
    cID levelSelectorID = 0;
    uint8_t imageID = 0;
    bool isEnabled = true;

    LevelSelector& operator=(const LevelSelector& ls);
    LevelSelector& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The level-Indicator property. This element displayes a simple 8bit value converted to a percentage value.
 * You can define the image and the text of the value description. The level-Indicator has an 8bit state ( 0 = 0% to 255 = 100% ).
 * This property is not executable by the app user and has no callback event. Make sure the levelIndicatorID is unique, otherwise this could cause problems.
 */
class LevelIndicator {
public:
    LevelIndicator();
    LevelIndicator(cID propertyID);
    LevelIndicator(const LevelIndicator& li);
    LevelIndicator(const DeviceProperty& dp);

    String levelIndicatorDescription;
    uint8_t level = 0;
    uint8_t imageID = 0;
    cID levelIndicatorID = 0;
    bool isEnabled = true;
    COLOR valueColor = col_not_set;

    LevelIndicator& operator=(const LevelIndicator& li);
    LevelIndicator& operator=(const DeviceProperty& dp);

    void update();

    String toPropertyDescriptor();

private:
    void split(const String& descriptor);
};

/**
 * @brief The text-Display property. This element displayes text to the app-user.
 * You can define the element text and the image for the element. The text-Display element has no state.
 * This property is not executable by the app-user and has no callback event. Make sure the textDisplayID is unique, otherwise this could cause problems.
 */
class TextDisplay {
public:
    TextDisplay();
    TextDisplay(cID propertyID);
    TextDisplay(const TextDisplay& td);
    TextDisplay(const DeviceProperty& dp);

    String textToDisplay;
    uint8_t imageID = 0;
    cID textDisplayID = 0;
    bool isEnabled = true;
    COLOR colorRect = col_not_set;

    TextDisplay& operator=(const TextDisplay& td);
    TextDisplay& operator=(const DeviceProperty& dp);

    void update();

    String toPropertyDescriptor();

private:
    void split(const String& descriptor);
};

/**
 * @brief The option-Selector property. This element displays a popup with selectable options when the app-user clicks on the list element.
 * You can define the image and the text for the description of the element. Option text can be added with the appropriate member function.
 * The option-Selector property has a simple 8bit state. The value is the current selected option index. If the selected option is changed by the app user, an event will be reported.
 * To catch this event: subclass the ILaroomyAppCallback interface and override the "onOptionSelectorIndexChanged" method.
 * Make sure the optionSelectorID is unique, otherwise this could cause problems.
 */
class OptionSelector {
    friend DeviceProperty;
public:
    OptionSelector();
    OptionSelector(cID propertyID);
    OptionSelector(const OptionSelector& os);
    OptionSelector(const DeviceProperty& dp);

    String optionSelectorDescription;
    uint8_t imageID = 0;
    cID optionSelectorID = 0;
    uint8_t selectedIndex = 0;
    bool isEnabled = true;

    void addOption(String option){
        this->optionList.AddItem(option);
    }

    void clearAllOptions(){
        this->optionList.Clear();
    }

    void removeOption(const String& option);
    void removeOption(unsigned int index);

    unsigned int getOptionCount(){
        return this->optionList.GetCount();
    }

    OptionSelector& operator=(const OptionSelector& os);
    OptionSelector& operator=(const DeviceProperty& dp);

    void update();

    String toDevicePropertyDescriptor();

private:
    itemCollection<String> optionList;
    void split(const String& descriptor);
};

/**
 * @brief The RGB-Selector property. This element has a separate page. You can define the image and the text for the property-element.
 * If the user clicks on it, the app navigates to a separate page containing elements to set the rgb state. The state of the rgbSelector is
 * a separate dataType (object::RGBSelectorState). If the state is changed by the user, an event will be reported. To catch this event: subclass
 * the ILaroomyAppCallback interface and override the "onRGBSelectorStateChanged" method. Make sure the rgbSelectorID is unique, otherwise this could cause problems.
 */
class RGBSelector {
    friend DeviceProperty;
public:
    RGBSelector();
    RGBSelector(cID propertyID);
    RGBSelector(const RGBSelector& rs);
    RGBSelector(const DeviceProperty& dp);
    
    String rgbSelectorDescription;
    uint8_t imageID = 0;
    cID rgbSelectorID = 0;
    bool isEnabled = true;

    RGBSelectorState rgbState;

    RGBSelector& operator=(const RGBSelector& rs);
    RGBSelector& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The extended-level-Selector property. This element has a separate page. You can define the image and the text for the description of the property-element.
 * If the user clicks on it, the app navigates to a separate page with an extended slider and an on/off switch. Other than the simple-level-selector, the extended-level-selectors scope and behavior is adjustable.
 * The state is a separate dataType (object::ExtendedLevelSelectorState). If the state is changed by the user, an event will be reported. To catch this event: subclass the
 * ILaroomyAppCallback interface and override the "onExtendedLevelSelectorStateChanged" method. Make sure the extendedLevelSelectorID is unique, otherwise this could cause problems.
 */
class ExtendedLevelSelector {
public:
    ExtendedLevelSelector();
    ExtendedLevelSelector(cID propertyID);
    ExtendedLevelSelector(const ExtendedLevelSelector& els);
    ExtendedLevelSelector(const DeviceProperty& dp);

    String extendedLevelSelectorDescription;
    uint8_t imageID = 0;
    cID extendedLevelSelectorID = 0;
    bool isEnabled = true;

    ExtendedLevelSelectorState exLevelState;

    ExtendedLevelSelector& operator=(const ExtendedLevelSelector& els);
    ExtendedLevelSelector& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The time-selector property. This element has a separate page. You can define the image and the text for the description of the property-element.
 * If the user clicks on it, the app navigates to a separate page with a single timePicker, where the user can set up a time. The state is a separate dataType
 * (object:: TimeSelectorState). If the time is changed by the user, an event will be reported. To catch this event: subclass the ILaroomyAppCallback interface and
 * override the "onTimeSelectorStateChanged" method. Make sure the timeSelectorID is unique, otherwise this could cause problems.
 */
class TimeSelector {
public:
    TimeSelector();
    TimeSelector(cID propertyID);
    TimeSelector(const TimeSelector& ts);
    TimeSelector(const DeviceProperty& dp);

    String timeSelectorDescription;
    uint8_t imageID = 0;
    cID timeSelectorID = 0;
    bool isEnabled = true;

    TimeSelectorState timeSelectorState;

    TimeSelector& operator=(const TimeSelector& ts);
    TimeSelector& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The time-frame-selector property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with two timePickers, where the user can select a start and end time (frame). The state is
 * a separate dataType (object:TimeFrameSelectorState). If the time-frame is changed by the user an event will be reported. To catch this event:
 * subclass the ILaroomyAppCallback and override the "onTimeFrameSelectorStateChanged" method. Make sure the timeFrameSelectorID is unique, otherwise this could cause problems.
 */
class TimeFrameSelector {
public:
    TimeFrameSelector();
    TimeFrameSelector(cID propertyID);
    TimeFrameSelector(const TimeFrameSelector& tfs);
    TimeFrameSelector(const DeviceProperty& dp);

    String timeFrameSelectorDescription;
    uint8_t imageID = 0;
    cID timeFrameSelectorID = 0;
    bool isEnabled = true;

    TimeFrameSelectorState timeFrameSelectorState;

    TimeFrameSelector& operator=(const TimeFrameSelector& tfs);
    TimeFrameSelector& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The date-selector property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with a date-picker, where the user can select a date. The state is a separate dataType (object:DateSelectorState).
 * If the date is changed by the user an event will be reported. To catch this event: subclass the ILaroomyAppCallback and override the "onDateSelectorStateChanged" method.
 * Make sure the dateSelectorID is unique, otherwise this could cause problems.
 * 
 */
class DateSelector {
public:
    DateSelector();
    DateSelector(cID propertyID);
    DateSelector(const DateSelector& ds);
    DateSelector(const DeviceProperty& dp);

    String dateSelectorDescription;
    uint8_t imageID = 0;
    cID dateSelectorID = 0;
    bool isEnabled = true;

    DateSelectorState dateSelectorState;

    DateSelector& operator=(const DateSelector& ds);
    DateSelector& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The unlock-control property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with a number-keypad and a lock-image to indicate the lock-status. The state is a separate
 * dataType (object::UnlockControlState). If there is a unlock, lock or pin-change request by the user, an event will be reported. To catch this event:
 * subclass the ILaroomyAppCallback and override the "onUnlockControlStateChanged" method. Make sure the unlockControlID is unique, otherwise this could cause problems.
 */
class UnlockControl {
public:
    UnlockControl();
    UnlockControl(cID propertyID);
    UnlockControl(const UnlockControl& uc);
    UnlockControl(const DeviceProperty& dp);

    String unlockControlDescription;
    uint8_t imageID = 0;
    cID unlockControlID = 0;
    bool isEnabled = true;

    UnlockControlState unlockControlState;

    UnlockControl& operator=(const UnlockControl& uc);
    UnlockControl& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The navigator-control property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with navigation arrows. The state is a separate dataType (object::NavigatorState). If the user clicks
 * on one or more (multi-touch is accepted) navigator arrows, this is reported with an event. To catch this event: subclass the ILaroomyAppCallback and override the
 * "onNavigatorStateChanged" method. Make sure the navigatorID is unique, otherwise this could cause problems.
 */
class NavigatorControl {
public:
    NavigatorControl();
    NavigatorControl(cID propertyID);
    NavigatorControl(const NavigatorControl& nc);
    NavigatorControl(const DeviceProperty& dp);

    String navigatorDescription;
    uint8_t imageID = 0;
    cID navigatorID = 0;
    bool isEnabled = true;

    NavigatorState navigatorState;

    NavigatorControl& operator=(const NavigatorControl& nc);
    NavigatorControl& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The bar-graph property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with a predefined number of bars displaying values. The state is a separate dataType
 * (object::BarGraphState). The barGraph property is only a viewer, it is not executable hence there is no event for this property.
 * Make sure the barGraphID is unique, otherwise this could cause problems.
 * 
 */
class BarGraph {
public:
    BarGraph();
    BarGraph(cID propertyID);
    BarGraph(const BarGraph& bg);
    BarGraph(const DeviceProperty& dp);

    String barGraphDescription;
    uint8_t imageID = 0;
    cID barGraphID = 0;
    bool isEnabled = true;

    BarGraphState barGraphState;

    BarGraph& operator=(const BarGraph& bg);
    BarGraph& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The line-graph property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with a coordinate system displaying the a line based on the values set in the state.
 * The state is a separate dataType (object::LineGraphState). The lineGraph property is only a viewer, it is not executable hence there is no event for this property.
 * Make sure the lineGraphID is unique, otherwise this could cause problems.
 * 
 */
class LineGraph {
public:
    LineGraph();
    LineGraph(cID propertyID);
    LineGraph(const LineGraph& lg);
    LineGraph(const DeviceProperty& dp);

    String lineGraphDescription;
    uint8_t imageID = 0;
    cID lineGraphID = 0;
    bool isEnabled = true;

    LineGraphState lineGraphState;

    LineGraph& operator=(const LineGraph& lg);
    LineGraph& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The string-interrogator property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with up to two input boxes and a confirm button. The input type and the text-properties of these boxes and the button are adjustable.
 * The state is a separate dataType (object::StringInterrogatorState). If the user clicks the confirm button an event will be reported. To catch this event:
 * subclass the ILaroomyAppCallback and override the "onStringInterrogatorDataReceived" method. Make sure the stringInterrogatorID is unique, otherwise this could cause problems.
 */
class StringInterrogator {
public:
    StringInterrogator();
    StringInterrogator(cID propertyID);
    StringInterrogator(const StringInterrogator& si);
    StringInterrogator(const DeviceProperty& dp);

    String stringInterrogatorDescription;
    uint8_t imageID = 0;
    cID stringInterrogatorID = 0;
    bool isEnabled = true;

    StringInterrogatorState stringInterrogatorState;

    StringInterrogator& operator=(const StringInterrogator& si);
    StringInterrogator& operator=(const DeviceProperty& dp);

    void update();
};

/**
 * @brief The text-list-presenter property. This element has a separate page. You can define the image and the text for the description of the property element.
 * If the user clicks on it, the app navigates to a separate page with a list where strings can be displayed. The strings can have a different type, which affects
 * the visual appearence in the list. The state is a separate dataType (object::TextListPresenterState). The textListPresenter is only a viewer,
 * it is not executable hence there is no event for this property. Make sure textListPresenterID is unique, otherwise this could cause problems.
 */
class TextListPresenter {
public:
    TextListPresenter();
    TextListPresenter(cID propertyID);
    TextListPresenter(const TextListPresenter& tlp);
    TextListPresenter(const DeviceProperty& dp);

    String textListPresenterDescription;
    uint8_t imageID = 0;
    cID textListPresenterID = 0;
    bool isEnabled = true;

    TextListPresenterState textListPresenterState;

    void AddTextListPresenterElement(TextListPresenterElementType type, String elementText);
    void ClearTextListPresenterContent();

    TextListPresenter& operator=(const TextListPresenter& tlp);
    TextListPresenter& operator=(const DeviceProperty& dp);

    void update();
};

#endif // ARDUINOBLE_LAROOMY_NRF_H
