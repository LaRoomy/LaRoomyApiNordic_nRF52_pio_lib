#include "LaRoomyApi_nRF52.h"

// RGB State *******************************************************************************

String RGBSelectorState::toStateString(unsigned int propertyIndex, TransmissionSubType t) {

    char twoBuffer[3];
    twoBuffer[2] = '\0';

    char transmissionBuffer[22];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = 'd';// 13 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set on/off value
    transmissionBuffer[8] = (this->isOn) ? '1' : '0';

    // set flags
    Convert::u8BitValueToHexTwoCharBuffer(this->flags, twoBuffer);
    transmissionBuffer[9] = twoBuffer[0];
    transmissionBuffer[10] = twoBuffer[1];

    // set transition value
    Convert::u8BitValueToHexTwoCharBuffer(this->colorTransitionProgram, twoBuffer);
    transmissionBuffer[11] = twoBuffer[0];
    transmissionBuffer[12] = twoBuffer[1];

    // set red value
    Convert::u8BitValueToHexTwoCharBuffer(this->redValue, twoBuffer);
    transmissionBuffer[13] = twoBuffer[0];
    transmissionBuffer[14] = twoBuffer[1];
    // set green value
    Convert::u8BitValueToHexTwoCharBuffer(this->greenValue, twoBuffer);
    transmissionBuffer[15] = twoBuffer[0];
    transmissionBuffer[16] = twoBuffer[1];
    // set blue value
    Convert::u8BitValueToHexTwoCharBuffer(this->blueValue, twoBuffer);
    transmissionBuffer[17] = twoBuffer[0];
    transmissionBuffer[18] = twoBuffer[1];

    // set transition type
    transmissionBuffer[19] = (this->transitionType == RGBTransitionType::SOFT_TRANSITION) ? '0' : '1';

    // set delimiter and terminator
    transmissionBuffer[20] = '\r';
    transmissionBuffer[21] = '\0';

    return String(transmissionBuffer);
}

void RGBSelectorState::fromExecutionString(const String& data){
    if(data.length() >= 19){
        // on/off value
        this->isOn = (data.charAt(8) == '0') ? false : true;

        // flags
        this->flags =
            Convert::x2CharHexValueToU8BitValue(
                data.charAt(9),
                data.charAt(10)
            );

        // transition prog
        this->colorTransitionProgram = (RGBColorTransitionProgram)
            Convert::x2CharHexValueToU8BitValue(
                data.charAt(11), 
                data.charAt(12)
            );

        // red
        this->redValue =
            Convert::x2CharHexValueToU8BitValue(
                data.charAt(13),
                data.charAt(14)
            );

        // green
        this->greenValue =
            Convert::x2CharHexValueToU8BitValue(
                data.charAt(15),
                data.charAt(16)
            );

        // blue
        this->blueValue =
            Convert::x2CharHexValueToU8BitValue(
                data.charAt(17),
                data.charAt(18)
            );

        // transition type
        this->transitionType = (data.charAt(19) == '0') ? RGBTransitionType::SOFT_TRANSITION : RGBTransitionType::HARD_TRANSITION;
    }
}

RGBSelectorState& RGBSelectorState::operator= (const RGBSelectorState& state){
    this->copy(state);
    return *this;
}

RGBSelectorState& RGBSelectorState::operator= (const COLOR& col)
{
    this->colorTransitionProgram = RGBColorTransitionProgram::RCTP_NO_TRANSITION;
    this->redValue = col.redPart;
    this->greenValue = col.greenPart;
    this->blueValue = col.bluePart;
    return *this;
}

bool RGBSelectorState::operator== (const RGBSelectorState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->isOn == state.isOn)&&(this->flags == state.flags)
        &&(this->colorTransitionProgram == state.colorTransitionProgram)
        &&(this->redValue == state.redValue)&&(this->greenValue == state.greenValue)
        &&(this->blueValue == state.blueValue)&&(this->transitionType == state.transitionType)){
        return true;
    }
    else {
        return false;
    }
}

bool RGBSelectorState::operator!= (const RGBSelectorState& state){
    return (*this == state) ? false : true;
}

void RGBSelectorState::copy(const RGBSelectorState& s){
    this->associatedPropertyID = s.associatedPropertyID;
    this->isOn = s.isOn;
    this->flags = s.flags;
    this->colorTransitionProgram = s.colorTransitionProgram;
    this->redValue = s.redValue;
    this->greenValue = s.greenValue;
    this->blueValue = s.blueValue;
    this->transitionType = s.transitionType;
}

// Extended Level Selector State *************************************************************************************************

String ExtendedLevelSelectorState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[3];
    twoBuffer[2] = '\0';

    char transmissionBuffer[25];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = 'f';// 15 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set on/off value
    transmissionBuffer[8] = (this->isOn) ? '1' : '0';

    char fourBuffer[4];

    // set level value
    Convert::s16BitValueToHex4CharBuffer(this->levelValue, fourBuffer);
    transmissionBuffer[9] = fourBuffer[0];
    transmissionBuffer[10] = fourBuffer[1];
    transmissionBuffer[11] = fourBuffer[2];
    transmissionBuffer[12] = fourBuffer[3];

    // set min value
    Convert::s16BitValueToHex4CharBuffer(this->minValue, fourBuffer);
    transmissionBuffer[13] = fourBuffer[0];
    transmissionBuffer[14] = fourBuffer[1];
    transmissionBuffer[15] = fourBuffer[2];
    transmissionBuffer[16] = fourBuffer[3];
   
    // set max value
    Convert::s16BitValueToHex4CharBuffer(this->maxValue, fourBuffer);
    transmissionBuffer[17] = fourBuffer[0];
    transmissionBuffer[18] = fourBuffer[1];
    transmissionBuffer[19] = fourBuffer[2];
    transmissionBuffer[20] = fourBuffer[3];

    // set flags
    uint8_t flags = 0;
    if(!this->showOnOffSwitch){
        flags |= ExLevelSelectorFlags::HIDE_ON_OFF_SWITCH;
    }
    if(this->transmitOnlyStartAndEndOfTracking){
        flags |= ExLevelSelectorFlags::TRANSMIT_ONLY_START_END_TRACKING;
    }
    Convert::u8BitValueToHexTwoCharBuffer(flags, twoBuffer);
    transmissionBuffer[21] = twoBuffer[0];
    transmissionBuffer[22] = twoBuffer[1];

    // set delimiter and terminator
    transmissionBuffer[23] = '\r';
    transmissionBuffer[24] = '\0';

    return String(transmissionBuffer);
}

void ExtendedLevelSelectorState::fromExecutionString(const String& data){
    if(data.length() >= 13){

        // check start/end flag
        if(data.charAt(7) == '3'){
            this->trackingType = ExLevelTrackingType::START;
        }
        else if(data.charAt(7) == '4'){
            this->trackingType = ExLevelTrackingType::END;
        }
        else {
            this->trackingType = ExLevelTrackingType::INTERTRACK;
        }

        // on/off value
        this->isOn = (data.charAt(8) == '0') ? false : true;

        // level value
        this->levelValue =
            Convert::x4CharHexValueToSigned16BitValue(
                data.charAt(9),
                data.charAt(10),
                data.charAt(11),
                data.charAt(12)
            );

        // min value
        /*this->minValue = 
            Convert::x4CharHexValueToSigned16BitValue(
                data.charAt(13),
                data.charAt(14),
                data.charAt(15),
                data.charAt(16)
            );
        // max value
        this->maxValue =
            Convert::x4CharHexValueToSigned16BitValue(
                data.charAt(17),
                data.charAt(18),
                data.charAt(19),
                data.charAt(20)
            );*/
    }
}

void ExtendedLevelSelectorState::fromDataHolderString(const String& data){
    if(data.length() >= 23){
        // on/off value
        this->isOn = (data.charAt(8) == '0') ? false : true;

        // level value
        this->levelValue =
            Convert::x4CharHexValueToSigned16BitValue(
                data.charAt(9),
                data.charAt(10),
                data.charAt(11),
                data.charAt(12)
            );

        // min value
        this->minValue = 
            Convert::x4CharHexValueToSigned16BitValue(
                data.charAt(13),
                data.charAt(14),
                data.charAt(15),
                data.charAt(16)
            );
        // max value
        this->maxValue =
            Convert::x4CharHexValueToSigned16BitValue(
                data.charAt(17),
                data.charAt(18),
                data.charAt(19),
                data.charAt(20)
            );

        // flags
        uint8_t flags = Convert::x2CharHexValueToU8BitValue(data.charAt(21), data.charAt(22));
        if(flags & ExLevelSelectorFlags::HIDE_ON_OFF_SWITCH){
            this->showOnOffSwitch = false;
        }
        else {
            this->showOnOffSwitch = true;
        }
        if(flags & ExLevelSelectorFlags::TRANSMIT_ONLY_START_END_TRACKING){
            this->transmitOnlyStartAndEndOfTracking = true;
        }
        else {
            this->transmitOnlyStartAndEndOfTracking = false;
        }
    }
}

ExtendedLevelSelectorState& ExtendedLevelSelectorState::operator=(const ExtendedLevelSelectorState& state){
    this->copy(state);
    return *this;
}

bool ExtendedLevelSelectorState::operator==(const ExtendedLevelSelectorState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->isOn == state.isOn)&&(this->levelValue == state.levelValue)){
        return true;
    }
    else {
        return false;
    }
}

bool ExtendedLevelSelectorState::operator!=(const ExtendedLevelSelectorState& state){
    return (*this == state) ? false : true;
}

void ExtendedLevelSelectorState::copy(const ExtendedLevelSelectorState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->isOn = state.isOn;
    this->levelValue = state.levelValue;
    this->maxValue = state.maxValue;
    this->minValue = state.minValue;
    this->showOnOffSwitch = state.showOnOffSwitch;
    this->transmitOnlyStartAndEndOfTracking = state.transmitOnlyStartAndEndOfTracking;
    this->trackingType = state.trackingType;
}

// Time selector state ***************************************************************************************************************

String TimeSelectorState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[3];
    twoBuffer[2] = '\0';

    char transmissionBuffer[14];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '5';// 5 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set hour value
    Convert::u8BitValueToHexTwoCharBuffer(this->hour, twoBuffer);
    transmissionBuffer[8] = twoBuffer[0];
    transmissionBuffer[9] = twoBuffer[1];

    // set minute value
    Convert::u8BitValueToHexTwoCharBuffer(this->minute, twoBuffer);
    transmissionBuffer[10] = twoBuffer[0];
    transmissionBuffer[11] = twoBuffer[1];

    // set delimiter and terminator
    transmissionBuffer[12] = '\r';
    transmissionBuffer[13] = '\0';

    return String(transmissionBuffer);
}

void TimeSelectorState::fromExecutionString(const String& data){
    if(data.length() >= 12){
        // hour
        this->hour = Convert::x2CharHexValueToU8BitValue(
            data.charAt(8),
            data.charAt(9)
        );
        // minute
        this->minute = Convert::x2CharHexValueToU8BitValue(
            data.charAt(10),
            data.charAt(11)
        );
    }
}

STATETIME TimeSelectorState::toStateTime(){
    STATETIME time;
    time.hour = this->hour;
    time.minute = this->minute;
    return time;
}

TimeSelectorState& TimeSelectorState::operator=(const TimeSelectorState& state){
    this->copy(state);
    return *this;
}

bool TimeSelectorState::operator==(const TimeSelectorState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->hour == state.hour)&&(this->minute == state.minute)){
        return true;
    }
    else {
        return false;
    }
}

bool TimeSelectorState::operator!=(const TimeSelectorState& state){
    return (*this == state) ? false : true;
}

void TimeSelectorState::copy(const TimeSelectorState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->hour = state.hour;
    this->minute = state.minute;
}

// Timeframe selector state ***********************************************************************************

String TimeFrameSelectorState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[2];
    char transmissionBuffer[18];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '9';// 9 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set start-hour value
    Convert::u8BitValueToHexTwoCharBuffer(this->startTime.hour, twoBuffer);
    transmissionBuffer[8] = twoBuffer[0];
    transmissionBuffer[9] = twoBuffer[1];

    // set start-minute value
    Convert::u8BitValueToHexTwoCharBuffer(this->startTime.minute, twoBuffer);
    transmissionBuffer[10] = twoBuffer[0];
    transmissionBuffer[11] = twoBuffer[1];

    // set end-hour value
    Convert::u8BitValueToHexTwoCharBuffer(this->endTime.hour, twoBuffer);
    transmissionBuffer[12] = twoBuffer[0];
    transmissionBuffer[13] = twoBuffer[1];

    // set end-minute value
    Convert::u8BitValueToHexTwoCharBuffer(this->endTime.minute, twoBuffer);
    transmissionBuffer[14] = twoBuffer[0];
    transmissionBuffer[15] = twoBuffer[1];

    // set delimiter and terminator
    transmissionBuffer[16] = '\r';
    transmissionBuffer[17] = '\0';

    return String(transmissionBuffer);
}

void TimeFrameSelectorState::fromExecutionString(const String& data){
    if(data.length() >= 16){
        // start time
        this->startTime.hour = Convert::x2CharHexValueToU8BitValue(
            data.charAt(8),
            data.charAt(9)
        );
        this->startTime.minute = Convert::x2CharHexValueToU8BitValue(
            data.charAt(10),
            data.charAt(11)
        );
        // end time
        this->endTime.hour = Convert::x2CharHexValueToU8BitValue(
            data.charAt(12),
            data.charAt(13)
        );
        this->endTime.minute = Convert::x2CharHexValueToU8BitValue(
            data.charAt(14),
            data.charAt(15)
        );
    }
}

bool TimeFrameSelectorState::checkIfTimeIsInFrame(const STATETIME& pTime){

    // at first, 3 possibilites
    // 1. start-time is lower end-time (time-frame within one day)
    // 2. end-time is lower start-time (time-frame from one day to the other)
    // 3. start-time equals the end-time (makes no sense but could happen)

    if(this->startTime < this->endTime){
        // time-frame within one day
        if((pTime < this->startTime)||(pTime >= this->endTime)){
            // outside frame
            return false;
        }
        else {
            // inside frame
            return true;
        }
    }
    else if (this->startTime > this->endTime){
        // time-frame from one day to the other
        if((pTime < this->startTime)&&(pTime >= this->endTime)){     // equi: if((pTime > this->startTime)||(pTime < this->endTime)) ??
            // outside frame
            return false;
        }
        else {
            // inside frame
            return true;
        }
    }
    else {
        // start-time equals the end-time (actually this makes no sense, but it must be handled)
        if(pTime == this->startTime){
            return true;
        }
        else {
            return false;
        }
    }
}

TimeFrameSelectorState& TimeFrameSelectorState::operator=(const TimeFrameSelectorState& state){
    this->copy(state);
    return *this;
}

bool TimeFrameSelectorState::operator==(const TimeFrameSelectorState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->startTime == state.startTime)
        &&(this->endTime == state.endTime)){
            return true;
    }
    else {
        return false;
    }
}

bool TimeFrameSelectorState::operator!=(const TimeFrameSelectorState& state){
    return (*this == state) ? false : true;
}

void TimeFrameSelectorState::copy(const TimeFrameSelectorState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->startTime = state.startTime;
    this->endTime = state.endTime;
}

// Date Selector State ************************************************************************

DateSelectorState& DateSelectorState::operator=(const DateSelectorState& state)
{
    this->copy(state);
    return *this;
}

bool DateSelectorState::operator==(const DateSelectorState& state)
{
    if((this->day == state.day)&&(this->month == state.month)&&(this->year == state.year)){
        return true;
    } else {
        return false;
    }
}

bool DateSelectorState::operator!=(const DateSelectorState& state)
{
    return (*this == state) ? false : true;
}

String DateSelectorState::toStateString(unsigned int propertyIndex, TransmissionSubType t)
{
    char twoBuffer[3];
    twoBuffer[2] = '\0';

    char transmissionBuffer[18];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '9'; // 9 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set day value
    Convert::u8BitValueToHexTwoCharBuffer(this->day, twoBuffer);
    transmissionBuffer[8] = twoBuffer[0];
    transmissionBuffer[9] = twoBuffer[1];

    // set month value
    Convert::u8BitValueToHexTwoCharBuffer(this->month, twoBuffer);
    transmissionBuffer[10] = twoBuffer[0];
    transmissionBuffer[11] = twoBuffer[1];

    // set year value
    char fourBuffer[5];
    fourBuffer[4] = '\0';
    Convert::s16BitValueToHex4CharBuffer(this->year, fourBuffer);
    transmissionBuffer[12] = fourBuffer[0];
    transmissionBuffer[13] = fourBuffer[1];
    transmissionBuffer[14] = fourBuffer[2];
    transmissionBuffer[15] = fourBuffer[3];

    // set delimiter and terminator
    transmissionBuffer[16] = '\r';
    transmissionBuffer[17] = '\0';

    return String(transmissionBuffer);
}

void DateSelectorState::fromExecutionString(String data)
{
    if (data.length() >= 16)
    {
        // day
        this->day = Convert::x2CharHexValueToU8BitValue(
            data.charAt(8),
            data.charAt(9));
        // month
        this->month = Convert::x2CharHexValueToU8BitValue(
            data.charAt(10),
            data.charAt(11));
        // year
        this->year = Convert::x4CharHexValueToUnsigned16BitValue(
            data.charAt(12),
            data.charAt(13),
            data.charAt(14),
            data.charAt(15)
        );
    }
}

void DateSelectorState::copy(const DateSelectorState& state)
{
    this->associatedPropertyID = state.associatedPropertyID;
    this->day = state.day;
    this->month = state.month;
    this->year = state.year;
}

// Unlock control state ***********************************************************************

String UnlockControlState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[2];
    char transmissionBuffer[14];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '5';// 5 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set unlock value
    transmissionBuffer[8] = this->unlocked ? '2' : '1';

    // set mode value
    transmissionBuffer[9] = (this->mode == UnlockControlModes::PIN_CHANGE_MODE) ? '1' : '0';

    // set flag value
    Convert::u8BitValueToHexTwoCharBuffer(this->flags, twoBuffer);
    transmissionBuffer[10] = twoBuffer[0];
    transmissionBuffer[11] = twoBuffer[1];

    // set delimiter and terminator
    transmissionBuffer[12] = '\r';
    transmissionBuffer[13] = '\0';

    return String(transmissionBuffer);
}

String UnlockControlState::toDataHolderString(){

    char twoBuffer[2];
    char transmissionBuffer[13];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = '0';

    // set property index
    transmissionBuffer[2] = '0';
    transmissionBuffer[3] = '0';

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '0';
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set unlock value
    transmissionBuffer[8] = this->unlocked ? '2' : '1';

    // set mode value
    transmissionBuffer[9] = (this->mode == UnlockControlModes::PIN_CHANGE_MODE) ? '1' : '0';

    // set flag value
    Convert::u8BitValueToHexTwoCharBuffer(this->flags, twoBuffer);
    transmissionBuffer[10] = twoBuffer[0];
    transmissionBuffer[11] = twoBuffer[1];
    transmissionBuffer[12] = '\0';

    String dataHolderBuffer = transmissionBuffer;
    dataHolderBuffer += this->pin;
    dataHolderBuffer += "\r\0";// set delimiter and terminator

    return dataHolderBuffer;
}

void UnlockControlState::fromExecutionString(const String& data){
    if(data.length() >= 12){
        // unlocked state
        this->unlocked = (data.charAt(8) == '2') ? true : false;
        // mode
        this->mode = (data.charAt(9) == '1') ? UnlockControlModes::PIN_CHANGE_MODE : UnlockControlModes::UNLOCK_MODE;
        // flags
        this->flags = Convert::x2CharHexValueToU8BitValue(
            data.charAt(10),
            data.charAt(11)
        );
        // pin
        this->pin = "";// reset
        if(data.length() > 12){
            for(unsigned int i = 12; i < data.length(); i++){
                if(data.charAt(i) == '\r'){
                    break;
                }
                else {
                    this->pin += data.charAt(i);
                }
            }
        }
    }
}

UnlockControlState& UnlockControlState::operator=(const UnlockControlState& state){
    this->copy(state);
    return *this;
}

bool UnlockControlState::operator==(const UnlockControlState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->mode == state.mode)&&(this->flags == state.flags)
        &&(this->unlocked == state.unlocked)&&(this->pin == state.pin)){
            return true;
    }
    else {
        return false;
    }
}

bool UnlockControlState::operator!=(const UnlockControlState& state){
    return (*this == state) ? false : true;
}

void UnlockControlState::copy(const UnlockControlState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->flags = state.flags;
    this->mode = state.mode;
    this->unlocked = state.unlocked;
    this->pin = state.pin;
}

// Navigator state **********************************************************************************************************

void NavigatorState::setButtonVisibility(bool up, bool right, bool down, bool left, bool mid){
    this->visibilityFlags = 0;
    if(up){
        this->visibilityFlags |= NavigatorButtonTypes::UP_BUTTON;
    }
    if(right){
        this->visibilityFlags |= NavigatorButtonTypes::RIGHT_BUTTON;
    }
    if(down){
        this->visibilityFlags |= NavigatorButtonTypes::DOWN_BUTTON;
    }
    if(left){
        this->visibilityFlags |= NavigatorButtonTypes::LEFT_BUTTON;
    }
    if(mid){
        this->visibilityFlags |= NavigatorButtonTypes::MID_BUTTON;
    }
}

NavigatorState& NavigatorState::operator=(const NavigatorState& state){
    this->copy(state);
    return *this;
}

bool NavigatorState::operator==(const NavigatorState& state){
    if((this->visibilityFlags == state.visibilityFlags)&&(this->associatedPropertyID == state.associatedPropertyID)){
        return true;
    }
    else {
        return false;
    }
}

bool NavigatorState::operator!=(const NavigatorState& state){
    return (*this == state) ? false : true;
}

String NavigatorState::toStateString(unsigned int propertyIndex, TransmissionSubType t){
    
    char twoBuffer[2];
    char transmissionBuffer[16];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '7';// 7 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set button visibility values
    transmissionBuffer[8] = (this->visibilityFlags & NavigatorButtonTypes::UP_BUTTON)     ? '1' : '0';
    transmissionBuffer[9] = (this->visibilityFlags & NavigatorButtonTypes::RIGHT_BUTTON)  ? '1' : '0';
    transmissionBuffer[10] = (this->visibilityFlags & NavigatorButtonTypes::DOWN_BUTTON)  ? '1' : '0';
    transmissionBuffer[11] = (this->visibilityFlags & NavigatorButtonTypes::LEFT_BUTTON)  ? '1' : '0';
    transmissionBuffer[12] = (this->visibilityFlags & NavigatorButtonTypes::MID_BUTTON)   ? '1' : '0';

    // set type value (unused in state transmission)
    transmissionBuffer[13] = '0';


    // set delimiter and terminator
    transmissionBuffer[14] = '\r';
    transmissionBuffer[15] = '\0';

    return String(transmissionBuffer);
}

void NavigatorState::fromExecutionString(const String& data){
    if(data.length() >= 14){

        this->buttonType = 0;

        // up button
        if(data.charAt(8) == '1'){
            this->buttonType |= NavigatorButtonTypes::UP_BUTTON;
        }
        // right button
        if(data.charAt(9) == '1'){
            this->buttonType |= NavigatorButtonTypes::RIGHT_BUTTON;
        }
        // down button
        if(data.charAt(10) == '1'){
            this->buttonType |= NavigatorButtonTypes::DOWN_BUTTON;
        }
        // left button
        if(data.charAt(11) == '1'){
            this->buttonType |= NavigatorButtonTypes::LEFT_BUTTON;
        }
        // mid button
        if(data.charAt(12) == '1'){
            this->buttonType |= NavigatorButtonTypes::MID_BUTTON;
        }
        // action
        this->actionType = Convert::charToNum(
            data.charAt(13)
        );

    }
}

void NavigatorState::copy(const NavigatorState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->visibilityFlags = state.visibilityFlags;
    this->actionType = state.actionType;
    this->buttonType = state.buttonType;
}

// BarGraph state ****************************************************************************

BarGraphState&  BarGraphState::operator=(const BarGraphState& state){
    this->copy(state);
    return *this;
}

bool BarGraphState::operator==(const BarGraphState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->fixedMaximumValue == state.fixedMaximumValue)
        && (this->useFixedMaximumValue == state.useFixedMaximumValue)&&(this->useValueAsBarDescriptor == state.useValueAsBarDescriptor)
        && (this->barDataList.GetCount() == state.barDataList.GetCount()))
    {
        for(unsigned int i = 0; i < this->barDataList.GetCount(); i++){
            if((this->barDataList.getObjectCoreReferenceAt(i)->barName != state.barDataList.GetAt(i).barName)
                || (this->barDataList.getObjectCoreReferenceAt(i)->barValue != state.barDataList.GetAt(i).barValue))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool BarGraphState::operator!=(const BarGraphState& state){
    return (*this == state) ? false : true;
}

String BarGraphState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[2];
    char transmissionBuffer[12];
    String ads;

    for(unsigned int i = 0; i < this->barDataList.GetCount(); i++){

        String barString;
        barString += Convert::numToChar(i);

        if(this->barDataList.getObjectCoreReferenceAt(i)->barName.length() == 0){
            this->barDataList.getObjectCoreReferenceAt(i)->barName = "_";
        }

        barString = barString + "::" + this->barDataList.getObjectCoreReferenceAt(i)->barName + "::" + this->barDataList.getObjectCoreReferenceAt(i)->barValue + ";;";
        ads += barString;
    }

    if(this->useFixedMaximumValue){
        ads += "9::_::";
        ads += this->fixedMaximumValue;
    }

    bool oversize = (ads.length() > 239) ? true : false;

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size
    if(oversize){
        transmissionBuffer[4] = 'f';
        transmissionBuffer[5] = 'f';
    }
    else {
        unsigned int payLoadSize = 4 + ads.length();
        Convert::u8BitValueToHexTwoCharBuffer(payLoadSize, twoBuffer);
        transmissionBuffer[4] = twoBuffer[0];
        transmissionBuffer[5] = twoBuffer[1];
    }

    // flags
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = oversize ? '5' : '0';

    // set bg flags
    unsigned int flags = 0;
    if(this->useValueAsBarDescriptor){
        flags |= BarGraphFlags::USE_VALUE_AS_BARDESCRIPTOR;
    }
    if(this->useFixedMaximumValue){
        flags |= BarGraphFlags::USE_FIXED_MAXIMUM_VALUE;
    }
    Convert::u8BitValueToHexTwoCharBuffer(flags, twoBuffer);
    transmissionBuffer[8] = twoBuffer[0];
    transmissionBuffer[9] = twoBuffer[1];

    // set number of bars
    transmissionBuffer[10] = Convert::numToChar(this->barDataList.GetCount());//Convert::numToChar(this->numberOfBars);
    transmissionBuffer[11] = '\0';

    // merge transmissionbuffer and ads
    String stateString(transmissionBuffer);
    stateString += ads;
    stateString += "\r\0";

    return stateString;
}

void BarGraphState::copy(const BarGraphState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->barDataList = state.barDataList;
    //this->numberOfBars = state.numberOfBars;
    this->useFixedMaximumValue = state.useFixedMaximumValue;
    this->useValueAsBarDescriptor = state.useValueAsBarDescriptor;
    this->fixedMaximumValue = state.fixedMaximumValue;
}

// LineGraph state: *********************************************************************************

String LineGraphDataPoints::toString(){
    String pp;
    for(unsigned int i = 0; i < this->points.GetCount(); i++){
        char buffer[48] = {'\0'};
        sprintf(buffer, "%g:%g;", this->points.getObjectCoreReferenceAt(i)->x, this->points.getObjectCoreReferenceAt(i)->y);
        pp += buffer;
    }
    return pp;
}

LineGraphState& LineGraphState::operator=(const LineGraphState& state){
    this->copy(state);
    return *this;
}

bool LineGraphState::operator==(const LineGraphState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->drawGridLines == state.drawGridLines)
        &&(this->drawAxisValues == state.drawAxisValues)&&(this->xIntersection == state.xIntersection)
        &&(this->xMaxValue == state.xMaxValue)&&(this->xMinValue == state.xMinValue)&&(this->yIntersection == state.yIntersection)
        &&(this->yMaxValue == state.yMaxValue)&&(this->yMinValue == state.yMinValue)&&(this->lineGraphPoints == state.lineGraphPoints))
    {
        return true;
    }
    else {
        return false;
    }
}

bool LineGraphState::operator!=(const LineGraphState& state){
    return (*this == state) ? false : true;
}

String LineGraphState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[2];
    char transmissionBuffer[12];
    String ads;

    ads += String("xmin:") + this->xMinValue + ';';
    ads += String("xmax:") + this->xMaxValue + ';';
    ads += String("ymin:") + this->yMinValue + ';';
    ads += String("ymax:") + this->yMaxValue + ';';
    ads += String("xisc:") + this->xIntersection + ';';
    ads += String("yisc:") + this->yIntersection + ';';

    if(this->lineGraphPoints.count() > 0){
        ads += this->lineGraphPoints.toString();
    }

    bool oversize = (ads.length() > 239) ? true : false;

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size
    if(oversize){
        transmissionBuffer[4] = 'f';
        transmissionBuffer[5] = 'f';
    }
    else {
        unsigned int payLoadSize = 4 + ads.length();
        Convert::u8BitValueToHexTwoCharBuffer(payLoadSize, twoBuffer);
        transmissionBuffer[4] = twoBuffer[0];
        transmissionBuffer[5] = twoBuffer[1];
    }

    // flags
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = oversize ? '5' : '0';

    // set lg flags
    unsigned int flags = 0;
    if(this->drawAxisValues){
        flags |= LineGraphFlags::DRAW_AXIS_VALUES;
    }
    if(this->drawGridLines){
        flags |= LineGraphFlags::DRAW_GRID_LINES;
    }
    Convert::u8BitValueToHexTwoCharBuffer(flags, twoBuffer);
    transmissionBuffer[8] = twoBuffer[0];
    transmissionBuffer[9] = twoBuffer[1];

    // set transmission type value
    transmissionBuffer[10] = '0'; // override existing state
    transmissionBuffer[11] = '\0';

    // merge transmissionbuffer and ads
    String stateString(transmissionBuffer);
    stateString += ads;
    stateString += "\r\0";

    return stateString;
}

void LineGraphState::copy(const LineGraphState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->drawAxisValues = state.drawAxisValues;
    this->drawGridLines = state.drawGridLines;
    this->lineGraphPoints = state.lineGraphPoints;
    this->xIntersection = state.xIntersection;
    this->yIntersection = state.yIntersection;
    this->xMaxValue = state.xMaxValue;
    this->xMinValue = state.xMinValue;
    this->yMaxValue = state.yMaxValue;
    this->yMinValue = state.yMinValue;
}

StringInterrogatorState& StringInterrogatorState::operator=(const StringInterrogatorState& state){
    this->copy(state);
    return *this;
}

bool StringInterrogatorState::operator==(const StringInterrogatorState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->buttonText == state.buttonText)
        &&(this->fieldOneContent == state.fieldOneContent)&&(this->fieldOneDescriptor == state.fieldOneDescriptor)
        &&(this->fieldOneHint == state.fieldOneHint)&&(this->fieldOneInputType == state.fieldOneInputType)
        &&(this->fieldOneVisible == state.fieldOneVisible)&&(this->fieldTwoContent == state.fieldTwoContent)
        &&(this->fieldTwoDescriptor == state.fieldTwoDescriptor)&&(this->fieldTwoHint == state.fieldTwoHint)
        &&(this->fieldTwoInputType == state.fieldTwoInputType)&&(this->fieldTwoVisible == state.fieldTwoVisible)
        &&(this->navigateBackOnButtonPress == state.navigateBackOnButtonPress))
    {
        return true;
    }
    else {
        return false;
    }
}

bool StringInterrogatorState::operator!=(const StringInterrogatorState& state){
    return (*this == state) ? false : true;
}

String StringInterrogatorState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[2];
    char transmissionBuffer[14];

    String ads;
    // build the definition data string:
    if(this->buttonText.length() > 0){
        ads += "B::" + this->buttonText + ";;";
    }
    if(this->fieldOneDescriptor.length() > 0){
        ads += "F1::" + this->fieldOneDescriptor + ";;";
    }
    if(this->fieldTwoDescriptor.length() > 0){
        ads += "F2::" + this->fieldTwoDescriptor + ";;";
    }
    if(this->fieldOneHint.length() > 0){
        ads += "H1::" + this->fieldOneHint + ";;";
    }
    if(this->fieldTwoHint.length() > 0){
        ads += "H2::" + this->fieldTwoHint + ";;";
    }
    if(this->fieldOneContent.length() > 0){
        ads += "C1::" + this->fieldOneContent + ";;";
    }
    if(this->fieldTwoContent.length() > 0){
        ads += "C2::" + this->fieldTwoContent + ";;";
    }

    bool oversize = (ads.length() > 237) ? true : false;

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size
    if(oversize){
        transmissionBuffer[4] = 'f';
        transmissionBuffer[5] = 'f';
    }
    else {
        unsigned int payLoadSize = 4 + ads.length();
        Convert::u8BitValueToHexTwoCharBuffer(payLoadSize, twoBuffer);
        transmissionBuffer[4] = twoBuffer[0];
        transmissionBuffer[5] = twoBuffer[1];
    }

    // flags
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = oversize ? '5' : '0';
    
    // set visibilities
    if(this->fieldOneVisible && this->fieldTwoVisible){
        transmissionBuffer[8] = '0';
    }
    else if(this->fieldOneVisible && !this->fieldTwoVisible){
        transmissionBuffer[8] = '1';
    }
    else if(!this->fieldOneVisible && this->fieldTwoVisible){
        transmissionBuffer[8] = '2';
    }
    else {
        transmissionBuffer[8] = '9';
    }

    // set field 1 input type
    transmissionBuffer[9] = Convert::numToChar(this->fieldOneInputType);
    // set field 2 input type
    transmissionBuffer[10] = Convert::numToChar(this->fieldTwoInputType);
    // set button behavior
    transmissionBuffer[11] = this->navigateBackOnButtonPress ? '1' : '0';
    // set accept non-ascii
    transmissionBuffer[12] = this->acceptNonAsciiCharacters ? '1' : '0';
    // set delimiter
    transmissionBuffer[13] = '\0';

    // merge transmissionbuffer and ads
    String stateString(transmissionBuffer);
    stateString += ads;
    stateString += "\r\0";

    return stateString;
}

void StringInterrogatorState::fromExecutionString(const String& data){
    // extract the content of the fields
    unsigned int counter = 8;
    auto sLen = data.length();
    this->fieldOneContent = "";
    this->fieldTwoContent = "";
    if(sLen > 8){
        while(counter < sLen){
            char cChar = data.charAt(counter);
            if(cChar == 'C'){
                // content entry
                counter++;
                if(data.charAt(counter) == '1'){
                    counter += 3;
                    // record content field 1
                    while(counter < sLen){
                        char c = data.charAt(counter);
                        if(c == ';'){
                            if((counter + 1) < sLen){
                                if(data.charAt(counter + 1) == ';'){
                                    counter ++;// the counter must be increased with 2, but after the break the counter will be increased by 1 too, so only increase by 1
                                    break;
                                }
                            }
                        }
                        else if(c == '\r'){
                            break;
                        }
                        else {
                            this->fieldOneContent += c;
                        }
                        counter++;
                    }
                }
                else if(data.charAt(counter) == '2'){
                    counter += 3;
                    // record content field 2
                    while(counter < sLen){
                        char c = data.charAt(counter);
                        if(c == ';'){
                            if((counter + 1) < sLen){
                                if(data.charAt(counter + 1) == ';'){
                                    counter++;// the counter must be increased with 2, but after the break the counter will be increased by 1 too, so only increase by 1
                                    break;
                                }
                            }
                        }
                        else if(c == '\r'){
                            break;
                        }
                        else {
                            this->fieldTwoContent += c;
                        }
                        counter++;
                    }
                }
            }
            counter++;
        }
    }
    if(this->fieldOneContent == "_"){
        this->fieldOneContent = "";
    }
    if(this->fieldTwoContent == "_"){
        this->fieldTwoContent = "";
    }
}


void StringInterrogatorState::copy(const StringInterrogatorState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->buttonText = state.buttonText;
    this->fieldOneContent = state.fieldOneContent;
    this->fieldOneDescriptor = state.fieldOneDescriptor;
    this->fieldOneHint = state.fieldOneHint;
    this->fieldOneInputType = state.fieldOneInputType;
    this->fieldOneVisible = state.fieldOneVisible;
    this->fieldTwoContent = state.fieldTwoContent;
    this->fieldTwoDescriptor = state.fieldTwoDescriptor;
    this->fieldTwoHint = state.fieldTwoHint;
    this->fieldTwoInputType = state.fieldTwoInputType;
    this->fieldTwoVisible = state.fieldTwoVisible;
    this->navigateBackOnButtonPress = state.navigateBackOnButtonPress;
    this->acceptNonAsciiCharacters = state.acceptNonAsciiCharacters;
}

String TextListPresenterState::toStateString(unsigned int propertyIndex, TransmissionSubType t){

    char twoBuffer[2];
    char transmissionBuffer[13];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = (t == TransmissionSubType::RESPONSE) ? '2' : '4';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size (fixed for this transmission) and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '4';// 4 bytes payload
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set values
    transmissionBuffer[8] = this->useBackgroundStack ? '1' : '0';
    transmissionBuffer[9] = '0';// action: param update transmission
    transmissionBuffer[10] = '0';// not used in state transmission

    // set delimiter and terminator
    transmissionBuffer[11] = '\r';
    transmissionBuffer[12] = '\0';

    return String(transmissionBuffer);
}

TextListPresenterState& TextListPresenterState::operator=(const TextListPresenterState& state){
    this->copy(state);
    return *this;
}

bool TextListPresenterState::operator==(const TextListPresenterState& state){
    if((this->associatedPropertyID == state.associatedPropertyID)&&(this->useBackgroundStack == state.useBackgroundStack)){
        return true;
    }
    else {
        return false;
    }
}

bool TextListPresenterState::operator!=(const TextListPresenterState& state){
    return (*this == state) ? false : true;
}

void TextListPresenterState::copy(const TextListPresenterState& state){
    this->associatedPropertyID = state.associatedPropertyID;
    this->useBackgroundStack = state.useBackgroundStack;
}