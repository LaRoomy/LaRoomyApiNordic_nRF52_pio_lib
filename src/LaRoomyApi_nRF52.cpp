#include "LaRoomyApi_nRF52.h"

LaRoomyAppImplementation::~LaRoomyAppImplementation(){
    laRoomyAppImplInstanceCreated = false;
    this->ble_terminate();
}

bool LaRoomyAppImplementation::laRoomyAppImplInstanceCreated = false;
LaRoomyAppImplementation* LaRoomyAppImplementation::hInstance = nullptr;

void LaRoomyAppImplementation::begin(){
    this->hasBegun = true;
}

void LaRoomyAppImplementation::run(){
    this->ble_start();
}

void LaRoomyAppImplementation::end(){
    delete this;
}

void LaRoomyAppImplementation::onLoop(){
    BLE.poll();
    // *****************************
    if(this->tmc.tComplete){
        if(this->is_monitor_enabled){
            Serial.print("Data received:  ");
            Serial.println(this->tmc.data);
            Serial.print("\r\n");
        }

        switch(tmc.data.charAt(0)){
            case '1':// property request
                this->onPropertyRequest(tmc.data);
                break;
            case '2':// group request
                this->onGroupRequest(tmc.data);
                break;
            case '3':// property state request
                this->onPropertyStateRequest(tmc.data);
                break;
            case '4':// property execution command
                this->onPropertyExecutionCommand(tmc.data);
                break;
            case '5':// notification / command
                this->onNotificationTransmission(tmc.data);
                break;
            case '6':// binding transmission
                this->onBindingTransmission(tmc.data);
                break;
            case '7':
                this->onInitRequest();
                break;
            default:
                // unhandled transmission data
                if(this->is_monitor_enabled){
                    Serial.print("WARNING - unhandled transmission data: ");
                    Serial.print(tmc.data);
                    Serial.print("\r\n");
                }
                break;
        }        
        this->tmc.reset();
    }
}

void LaRoomyAppImplementation::addDeviceProperty(const DeviceProperty& p){
    this->_addDeviceProperty(p, true);
}

void LaRoomyAppImplementation::_addDeviceProperty(const DeviceProperty& p, bool sendCommand){

    if(this->is_monitor_enabled){
        if((p.propertyID == 0) || (p.propertyID == ID_DEVICE_MAIN_PAGE)){
            Serial.println("ERROR: Invalid property ID detected. Value 0 and 16211 are reserved for internal usage!");
            return;
        }
        else {
            // verify the propery ID (no double IDs!)
            for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
                if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == p.propertyID){
                    if(is_monitor_enabled){
                        Serial.println("ERROR while inserting property: Property-ID already exists.");
                    }
                    return;
                }
            }
        }
    }
    this->deviceProperties.AddItem(p);
    this->initializeComplexPropertyState(p);

    if(this->is_connected && sendCommand){
        // this is an add operation at runtime, so send an insert command (insert at the end)
        auto tStr = 
            this->deviceProperties.getObjectCoreReferenceAt(
                this->deviceProperties.GetCount() - 1
                )->toTransmissionString(
                    TransmissionSubType::INSERT,
                    this->deviceProperties.GetCount() - 1
                );
        this->sendData(tStr);
    }
}

void LaRoomyAppImplementation::addDevicePropertyGroup(const DevicePropertyGroup& g){
    // verify the group ID (no double IDs!)
    for(unsigned int i = 0; i < this->devicePropertyGroups.GetCount(); i++){
        if(this->devicePropertyGroups.getObjectCoreReferenceAt(i)->groupID == g.groupID){
            if(is_monitor_enabled){
                Serial.println("ERROR while inserting group: Group-ID already exists.");
            }
            return;
        }
    }
    // only add if the group contains properties
    if(g.propertyList.GetCount() > 0){
        // the count value is the next index
        unsigned int nextGroupIndex = this->devicePropertyGroups.GetCount();

        // add the groupID (index) and the flag value to each of the properties inside the group
        // + add the properties to the property list
        for(unsigned int i = 0; i < g.propertyList.GetCount(); i++){
            auto prop = g.propertyList.GetAt(i);
            prop.groupIndex = nextGroupIndex;
            prop.relatedGroupID = g.groupID;
            prop.flags |= PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER;
            this->_addDeviceProperty(prop, false);
        }
        // at the group itself
        this->devicePropertyGroups.AddItem(g);
        // clear the property collection in the group element (no longer needed, but keep the amount)
        if(nextGroupIndex < this->devicePropertyGroups.GetCount()){
            // update prop count
            this->devicePropertyGroups.getObjectCoreReferenceAt(nextGroupIndex)->propertyCount =
                this->devicePropertyGroups.getObjectCoreReferenceAt(nextGroupIndex)->propertyList.GetCount();
            // delete properties
            this->devicePropertyGroups.getObjectCoreReferenceAt(nextGroupIndex)->propertyList.Clear();
        }

        if(this->is_connected){
            // this is an add operation at runtime!
            // -> inserting groups are not supported, so invalidate the whole properties, to force the app to reload
            this->sendPropertyReloadCommand();
        }
    }
}

void LaRoomyAppImplementation::insertProperty(cID insertAfter, const DeviceProperty& p){

    // verify the propery ID (no double IDs!)
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == p.propertyID){
            if(is_monitor_enabled){
                Serial.println("Error while inserting property: Property-ID already exists.");
            }
            return;
        }
    }

    if(insertAfter == INSERT_FIRST){
        // insert on the top of the list
        this->deviceProperties.InsertAt(0, p);
        this->initializeComplexPropertyState(p);

        if(this->is_connected){
            // send insert command
            this->sendData(
                this->deviceProperties.getObjectCoreReferenceAt(0)->toTransmissionString(
                    TransmissionSubType::INSERT,
                    this->propertyIndexFromPropertyID(p.propertyID)
                )
            );
        }
    }
    else if(insertAfter == INSERT_LAST){
        // insert at the end of the list
        this->addDeviceProperty(p);
    }
    else {
        bool found = false;
        // search for insert position
        for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
            if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == insertAfter){
                // insert
                this->deviceProperties.InsertAt(i + 1, p);
                // send command when applicable
                if(this->is_connected){
                    this->sendData(
                        this->deviceProperties.getObjectCoreReferenceAt(i + 1)->toTransmissionString(
                            TransmissionSubType::INSERT,
                            this->propertyIndexFromPropertyID(p.propertyID)
                        )
                    );
                }
                found = true;
                break;
            }
        }
        if(!found && this->is_monitor_enabled){
            Serial.println("InsertProperty: error: ID to insert after not found");
        }
    }
}

void LaRoomyAppImplementation::insertPropertyInGroup(cID insertAfter, cID groupID, const DeviceProperty& p){

    // verify the propery ID (no double IDs!)
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == p.propertyID){
            if(is_monitor_enabled){
                Serial.println("Error while inserting property in group: Property-ID already exists.");
            }
            return;
        }
    }

    unsigned int groupIndex = INVALID_ELEMENT_INDEX;
    bool exists = false;

    // first look if the group exists and what's the index
    for(unsigned int i = 0; i < this->devicePropertyGroups.GetCount(); i++){
        if(this->devicePropertyGroups.getObjectCoreReferenceAt(i)->groupID == groupID){
            exists = true;
            groupIndex = i;
            break;
        }
    }

    if(exists){
        // ivalue copy
        auto pp = p;
        // save group ID
        pp.relatedGroupID = groupID;
        // find insert point and insert
        if(insertAfter == INSERT_FIRST){
            // look for the first property inside the group
            for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
                if(this->deviceProperties.getObjectCoreReferenceAt(i)->groupIndex == groupIndex){
                    // make sure to mark the property as group member
                    pp.groupIndex = groupIndex;
                    pp.flags |= PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER;
                    // insert the property
                    this->deviceProperties.InsertAt(i, pp);
                    // initialize the state
                    this->initializeComplexPropertyState(pp);
                    // if this happens at app-runtime, notify app
                    if(this->is_connected){
                        // send insert command
                        this->sendData(
                            this->deviceProperties.getObjectCoreReferenceAt(i)->toTransmissionString(
                                TransmissionSubType::INSERT,
                                this->propertyIndexFromPropertyID(pp.propertyID)
                            )
                        );
                    }
                    break;
                }
            }
        }
        else if(insertAfter == INSERT_LAST){
            // look for the last property inside the group
            for(int i = (this->deviceProperties.GetCount() - 1); i >= 0; i--){
                if(this->deviceProperties.getObjectCoreReferenceAt(i)->groupIndex == groupIndex){
                    // make sure to mark the property as group member
                    pp.groupIndex = groupIndex;
                    pp.flags |= PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER;
                    // insert the property
                    this->deviceProperties.InsertAt(i + 1, pp);
                    // initialize the state
                    this->initializeComplexPropertyState(pp);
                    // if this happens at app-runtime, notify app
                    if(this->is_connected){
                        // send insert command
                        this->sendData(
                            this->deviceProperties.getObjectCoreReferenceAt(i + 1)->toTransmissionString(
                                TransmissionSubType::INSERT,
                                this->propertyIndexFromPropertyID(pp.propertyID)
                            )
                        );
                    }
                    break;
                }
            }
        }
        else {
            bool found = false;
            // look for the property with the insertAfter-ID
            for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
                if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == insertAfter){
                    // make sure to mark the property as group member
                    pp.groupIndex = groupIndex;
                    pp.flags |= PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER;
                    // insert after the element with the insert after ID
                    this->deviceProperties.InsertAt(i + 1, pp);
                    // send command when applicable
                    if(this->is_connected){
                        this->sendData(
                            this->deviceProperties.getObjectCoreReferenceAt(i + 1)->toTransmissionString(
                                TransmissionSubType::INSERT,
                                this->propertyIndexFromPropertyID(pp.propertyID)
                            )
                        );
                    }
                    found = true;
                    break;
                }
            }
            if(!found && this->is_monitor_enabled){
                Serial.println("InsertPropertyInGroup: error: ID to insert after not found");
            }
        }
    }
}

bool LaRoomyAppImplementation::checkIfPropertyExist(cID propertyID){
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++)
    {
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == propertyID)
        {
            return true;
        }
    }
    return false;
}

void LaRoomyAppImplementation::setDeviceBindingAuthenticationRequired(bool required){
    if(!this->auto_handle_binding){
        this->deviceBindingAuthenticationRequired = required;
    }
}

void LaRoomyAppImplementation::setDeviceImage(LaRoomyImages img){
    this->deviceImageID = (unsigned int)img;
}

void LaRoomyAppImplementation::enableInternalBindingHandler(bool enable){
    this->auto_handle_binding = enable;
    if(enable){
        auto bindC = BindingController::GetInstance();
        if(bindC != nullptr){
            this->deviceBindingAuthenticationRequired = bindC->isBindingRequired();
        }
    }
}

unsigned int LaRoomyAppImplementation::getSimplePropertyState(cID pID){
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == pID){
            return this->deviceProperties.getObjectCoreReferenceAt(i)->propertyState;
        }
    }
    return INVALID_PROPERTY_STATE;
}

RGBSelectorState LaRoomyAppImplementation::getRGBSelectorState(cID rgbSelectorID){
    for(unsigned int i = 0; i < this->rgbStates.GetCount(); i++){
        if(this->rgbStates.getObjectCoreReferenceAt(i)->associatedPropertyID == rgbSelectorID){
            return this->rgbStates.GetAt(i);
        }
    }
    return RGBSelectorState();
}

ExtendedLevelSelectorState LaRoomyAppImplementation::getExtendedLevelSelectorState(cID exLevelSelectID){
    for(unsigned int i = 0; i < this->extendedLevelStates.GetCount(); i++){
        if(this->extendedLevelStates.getObjectCoreReferenceAt(i)->associatedPropertyID == exLevelSelectID){
            return this->extendedLevelStates.GetAt(i);
        }
    }
    return ExtendedLevelSelectorState();
}

TimeSelectorState LaRoomyAppImplementation::getTimeSelectorState(cID timeSelectorID){
    for(unsigned int i = 0; i < this->timeSelectorStates.GetCount(); i++){
        if(this->timeSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == timeSelectorID){
            return this->timeSelectorStates.GetAt(i);
        }
    }
    return TimeSelectorState();
}

TimeFrameSelectorState LaRoomyAppImplementation::getTimeFrameSelectorState(cID timeFrameSelectorID){
    for(unsigned int i = 0; i < this->timeFrameSelectorStates.GetCount(); i++){
        if(this->timeFrameSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == timeFrameSelectorID){
            return this->timeFrameSelectorStates.GetAt(i);
        }
    }
    return TimeFrameSelectorState();
}

DateSelectorState LaRoomyAppImplementation::getDateSelectorState(cID dateSelectorID){
    for(unsigned int i = 0; i < this->dateSelectorStates.GetCount(); i++){
        if(this->dateSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == dateSelectorID){
            return this->dateSelectorStates.GetAt(i);
        }
    }
    return DateSelectorState();
}

UnlockControlState LaRoomyAppImplementation::getUnlockControlState(cID unlockControlID){
    for(unsigned int i = 0; i < this->unlockControlStates.GetCount(); i++){
        if(this->unlockControlStates.getObjectCoreReferenceAt(i)->associatedPropertyID == unlockControlID){
            return this->unlockControlStates.GetAt(i);
        }
    }
    return UnlockControlState();
}

NavigatorState LaRoomyAppImplementation::getNavigatorState(cID navigatorID){
    for(unsigned int i = 0; i < this->navigatorStates.GetCount(); i++){
        if(this->navigatorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == navigatorID){
            return this->navigatorStates.GetAt(i);
        }
    }
    return NavigatorState();
}

BarGraphState LaRoomyAppImplementation::getBarGraphState(cID barGraphID){
    for(unsigned int i = 0; i < this->barGraphStates.GetCount(); i++){
        if(this->barGraphStates.getObjectCoreReferenceAt(i)->associatedPropertyID == barGraphID){
            return this->barGraphStates.GetAt(i);
        }
    }
    return BarGraphState();
}

LineGraphState LaRoomyAppImplementation::getLineGraphState(cID lineGraphID){
    for(unsigned int i = 0; i < this->lineGraphStates.GetCount(); i++){
        if(this->lineGraphStates.getObjectCoreReferenceAt(i)->associatedPropertyID == lineGraphID){
            return this->lineGraphStates.GetAt(i);
        }
    }
    return LineGraphState();
}

StringInterrogatorState LaRoomyAppImplementation::getStringInterrogatorState(cID stringInterrogatorID){
    for(unsigned int i = 0; i < this->stringInterrogatorStates.GetCount(); i++){
        if(this->stringInterrogatorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == stringInterrogatorID){
            this->stringInterrogatorStates.GetAt(i);
        }
    }
    return StringInterrogatorState();
}

TextListPresenterState LaRoomyAppImplementation::getTextListPresenterState(cID textListPresenterID){
    for(unsigned int i = 0; i < this->textListPresenterStates.GetCount(); i++){
        if(this->textListPresenterStates.getObjectCoreReferenceAt(i)->associatedPropertyID == textListPresenterID){
            return this->textListPresenterStates.GetAt(i);
        }
    }
    return TextListPresenterState();
}

void LaRoomyAppImplementation::updateDeviceProperty(const DeviceProperty& p){

    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == p.propertyID){
            // first check if the old element is part of a group
            unsigned int gIndex =
                (this->deviceProperties.getObjectCoreReferenceAt(i)->flags & PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER)
                ? this->deviceProperties.getObjectCoreReferenceAt(i)->groupIndex : INVALID_ELEMENT_INDEX;
            // replace property in collection
            this->deviceProperties.ReplaceAt(i, p);
            // if the old element was group-member, the new must be as well
            if(gIndex != INVALID_ELEMENT_INDEX){
                this->deviceProperties.getObjectCoreReferenceAt(i)->groupIndex = gIndex;
                this->deviceProperties.getObjectCoreReferenceAt(i)->flags |= PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER;
            }
            // if this happens at connect-time -> send command
            if(this->is_connected){
                // if the description callback is set, call it to get the descriptor
                if(this->pDescriptionCallback != nullptr){
                    this->pDescriptionCallback->onPropertyDescriptionRequired(p.propertyID, this->lastLangID, this->deviceProperties.getObjectCoreReferenceAt(i)->descriptor);
                }                
                auto updateTransmissionData =
                    this->deviceProperties.getObjectCoreReferenceAt(i)->toTransmissionString(TransmissionSubType::UPDATE, i);
                this->sendData(updateTransmissionData);
            }
            break;
        }
    }
}

DeviceProperty LaRoomyAppImplementation::getProperty(unsigned int propertyID){
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == propertyID){
            return this->deviceProperties.GetAt(i);
        }
    }
    return DeviceProperty();
}

void LaRoomyAppImplementation::removeProperty(cID propertyID){
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == propertyID){
            if(this->is_connected){
                // send remove command
                auto transmissionString =
                    this->deviceProperties.getObjectCoreReferenceAt(i)->toTransmissionString(TransmissionSubType::REMOVE, i);
                this->sendData(transmissionString);
            }
            bool rearrangeIndexes = false;

            if(this->deviceProperties.getObjectCoreReferenceAt(i)->flags & PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER){
                // if the property was part of a group and the property-count inside the group reaches zero -> delete the group
                for(unsigned int j = 0; j < this->devicePropertyGroups.GetCount(); j++){
                    // search the group
                    if(this->devicePropertyGroups.getObjectCoreReferenceAt(j)->groupID == this->deviceProperties.getObjectCoreReferenceAt(i)->relatedGroupID){
                        // decrease property count
                        this->devicePropertyGroups.getObjectCoreReferenceAt(j)->propertyCount--;
                        // if zero -> remove the group
                        if(this->devicePropertyGroups.getObjectCoreReferenceAt(j)->propertyCount == 0){
                            this->devicePropertyGroups.RemoveAt(j);
                            // schedule rearranging
                            rearrangeIndexes = true;
                        }
                        break;
                    }
                }
            }
            // remove the property
            this->deviceProperties.RemoveAt(i);

            if(rearrangeIndexes){
                // rearrange the group indexes of the remaining property elements
                this->rearrangeGroupIndexes();
            }
            break;
        }
    }
}

void LaRoomyAppImplementation::removePropertyGroup(cID groupID){
    // get the group index for the ID
    auto groupIndex = this->groupIndexFromGroupID(groupID);
    // remove the properties inside the group
    for(int i = (int)(this->deviceProperties.GetCount() - 1); i >= 0; i--){
        if(this->deviceProperties.getObjectCoreReferenceAt(((unsigned int)i))->flags & PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER) {
            if(this->deviceProperties.getObjectCoreReferenceAt(((unsigned int)i))->groupIndex == groupIndex){
                this->deviceProperties.RemoveAt(((unsigned int)i));
            }
        }
    }
    // re-arrange the group indexes of the remaining property elements
    if((groupIndex + 1) < this->devicePropertyGroups.GetCount()){
        this->rearrangeGroupIndexes();
    }
    // than remove the group itself and send command
    for(unsigned int i = 0; i < this->devicePropertyGroups.GetCount(); i++){
        if(this->devicePropertyGroups.getObjectCoreReferenceAt(i)->groupID == groupID){
            if(this->is_connected){
                // send remove command
                auto transmissionString =
                    this->devicePropertyGroups.getObjectCoreReferenceAt(i)->toTransmissionString(TransmissionSubType::REMOVE, i);
                this->sendData(transmissionString);
            }
            this->devicePropertyGroups.RemoveAt(i);
            break;
        }
    }
}

void LaRoomyAppImplementation::enableProperty(cID propertyID){
    // search if the property exists
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == propertyID){
            // set param
            this->deviceProperties.getObjectCoreReferenceAt(i)->isEnabled = true;
            // build transmission data
            char twoChar[2];
            String transmissionData = "17";// property definition + enable
            // property index
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(propertyID),
                twoChar
            );
            transmissionData += twoChar[0];
            transmissionData += twoChar[1];
            // data size (0) + flags (0) + delimiter (cr)
            transmissionData += "0000\r";
            // send transmission
            this->sendData(transmissionData);
            break;
        }
    }
}

void LaRoomyAppImplementation::disableProperty(cID propertyID){
    // search if the property exists
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == propertyID){
            // set param
            this->deviceProperties.getObjectCoreReferenceAt(i)->isEnabled = false;
            // build transmission data
            char twoChar[2];
            String transmissionData = "18";// property definition + disable
            // property index
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(propertyID),
                twoChar
            );
            transmissionData += twoChar[0];
            transmissionData += twoChar[1];
            // data size (0) + flags (0) + delimiter (cr)
            transmissionData += "0000\r";
            // send transmission
            this->sendData(transmissionData);
            break;
        }
    }
}

void LaRoomyAppImplementation::clearAllPropertiesAndGroups(){
    this->devicePropertyGroups.Clear();
    this->deviceProperties.Clear();
    this->rgbStates.Clear();
    this->extendedLevelStates.Clear();
    this->timeSelectorStates.Clear();
    this->timeFrameSelectorStates.Clear();
    this->navigatorStates.Clear();
    this->unlockControlStates.Clear();
    this->barGraphStates.Clear();
    this->lineGraphStates.Clear();
    this->stringInterrogatorStates.Clear();
    this->textListPresenterStates.Clear();
}

void LaRoomyAppImplementation::sendUserMessage(UserMessageType type, UserMessageHoldingPeriod period, const String& message){

    if(this->is_connected){
        char twoBuffer[3];
        twoBuffer[2] = '\0';
        String mNotification;

        if(message.length() < 242){
            // normal processing
            Convert::u8BitValueToHexTwoCharBuffer(message.length() + 3, twoBuffer);

            mNotification = "5300";
            mNotification += twoBuffer;
            mNotification += "001";
        }
        else {
            // oversize length header
            mNotification = "5300ff051";
        }
        mNotification += Convert::numToChar(type);
        mNotification += Convert::numToChar(period);
        mNotification += message;
        mNotification += '\r';

        this->sendData(mNotification);
    }
}

void LaRoomyAppImplementation::sendTimeRequest(){
    this->sendData("530002002\r\0");
}

void LaRoomyAppImplementation::sendDateRequest(){
    this->sendData("530002009\r\0");
}

void LaRoomyAppImplementation::sendPropertyReloadCommand(){
    this->sendData("530002003\r\0");
}

void LaRoomyAppImplementation::sendPropertyToCacheCommand(){
    this->sendData("530002004\r\0");
}

void LaRoomyAppImplementation::sendLanguageRequest(){
    this->sendData("530002005\r\0");
}

void LaRoomyAppImplementation::sendRefreshAllStatesCommand(){
    this->sendData("530002006\r\0");
}

void LaRoomyAppImplementation::sendNavBackToDeviceMainCommand(){
    this->sendData("530002007\r\0");
}

void LaRoomyAppImplementation::sendCloseDeviceCommand(){
    this->sendData("530002008\r\0");
}

void LaRoomyAppImplementation::sendUIModeInfoRequest(){
    this->sendData("53000200a\r\0");
}

void LaRoomyAppImplementation::addTextListPresenterElement(cID textListPresenterID, TextListPresenterElementType type, const String& elementText){

    char twoBuffer[2];
    char transmissionBuffer[12];
    bool overlength = (elementText.length() > 242) ? true : false;

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = '3';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(
        this->propertyIndexFromPropertyID(textListPresenterID),
        twoBuffer
    );
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size and flags
    if(overlength){
        transmissionBuffer[4] = 'f';
        transmissionBuffer[5] = 'f';
    }
    else {
        Convert::u8BitValueToHexTwoCharBuffer(
            elementText.length() + 4,
            twoBuffer
        );
        transmissionBuffer[4] = twoBuffer[0];
        transmissionBuffer[5] = twoBuffer[1];
    }
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = overlength ? '5' : '0';

    // set values
    transmissionBuffer[8] = '9';
    for(unsigned int i = 0; i < this->textListPresenterStates.GetCount(); i++){
        if(this->textListPresenterStates.getObjectCoreReferenceAt(i)->associatedPropertyID == textListPresenterID){
            transmissionBuffer[8] = (this->textListPresenterStates.getObjectCoreReferenceAt(i)->useBackgroundStack) ? '1' : '0';
            break;
        }
    }
    transmissionBuffer[9] = '1';// set action: add element

    switch(type){   // set message type
        case TextListPresenterElementType::DEFAULT_MESSAGE:
            transmissionBuffer[10] = '0';
            break;
        case TextListPresenterElementType::INFO_MESSAGE:
            transmissionBuffer[10] = '1';
            break;
        case TextListPresenterElementType::WARNING_MESSAGE:
            transmissionBuffer[10] = '2';
            break;
        case TextListPresenterElementType::ERROR_MESSAGE:
            transmissionBuffer[10] = '3';
            break;
        default:
            transmissionBuffer[10] = '0';
            break;
    }
    // set terminator
    transmissionBuffer[11] = '\0';

    // assemble transmission string
    String tm = transmissionBuffer;
    tm += elementText;
    tm += '\r';

    // send
    this->sendData(tm);
}

void LaRoomyAppImplementation::clearTextListPresenterContent(cID textListPresenterID){

    char twoBuffer[2];
    char transmissionBuffer[13];

    // set transmission types
    transmissionBuffer[0] = '3';
    transmissionBuffer[1] = '3';

    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(
        this->propertyIndexFromPropertyID(textListPresenterID),
        twoBuffer
    );
    transmissionBuffer[2] = twoBuffer[0];
    transmissionBuffer[3] = twoBuffer[1];

    // set data size and flags
    transmissionBuffer[4] = '0';
    transmissionBuffer[5] = '4';// fixed in this transmission (4 bytes)
    transmissionBuffer[6] = '0';
    transmissionBuffer[7] = '0';

    // set values
    transmissionBuffer[8] = '9';
    for(unsigned int i = 0; i < this->textListPresenterStates.GetCount(); i++){
        if(this->textListPresenterStates.getObjectCoreReferenceAt(i)->associatedPropertyID == textListPresenterID){
            transmissionBuffer[8] = (this->textListPresenterStates.getObjectCoreReferenceAt(i)->useBackgroundStack) ? '1' : '0';
            break;
        }
    }
    transmissionBuffer[9] = '2';// set action: clear stack
    transmissionBuffer[10] = '0';// not used in this action

    // set terminator & delimiter
    transmissionBuffer[11] = '\r';
    transmissionBuffer[12] = '\0';

    // send
    this->sendData(transmissionBuffer);
}

void LaRoomyAppImplementation::connectHandler(BLEDevice central){
    auto pComp = LaRoomyAppImplementation::GetInstance();
    if(pComp != nullptr){
        pComp->is_connected = true;

        if(pComp->pLrCallback != nullptr){
            pComp->pLrCallback->onConnectionStateChanged(true);
        }
    }
}

void LaRoomyAppImplementation::disconnectHandler(BLEDevice central){
    auto pComp = LaRoomyAppImplementation::GetInstance();
    if(pComp != nullptr){
        pComp->is_connected = false;

        if(pComp->pLrCallback != nullptr){
            pComp->pLrCallback->onConnectionStateChanged(false);
        }
    }
}

void LaRoomyAppImplementation::rxCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic){

    auto pComp = LaRoomyAppImplementation::GetInstance();
    if(pComp != nullptr){
        auto vLen = characteristic.valueLength();

        if(vLen > 0){
            const char* data = (const char*)characteristic.value();

            char* securedData = new char[characteristic.valueLength() + 1];
            if(securedData != nullptr){

                for(int i = 0; i < characteristic.valueLength(); i++){
                    securedData[i] = data[i];
                }
                securedData[characteristic.valueLength()] = '\0';

                pComp->tmc.setData(securedData);

                delete [] securedData;
            }
        }
    }
}

void LaRoomyAppImplementation::onInitRequest(){

    // the init request comes in front of a property-loading-loop, so mark the property-loading incomplete
    this->propertyLoadingDone = false;

    // build init response string:
    String initString = "72000800";
    char twoBuffer[3];
    twoBuffer[2] = '\0';
    // property count
    Convert::u8BitValueToHexTwoCharBuffer(this->deviceProperties.GetCount(), twoBuffer);
    initString += twoBuffer;
    // group count
    Convert::u8BitValueToHexTwoCharBuffer(this->devicePropertyGroups.GetCount(), twoBuffer);
    initString += twoBuffer;
    // data caching permission
    initString += (this->cachingPermission ? '1' : '0');
    // device binding activity
    initString += (this->deviceBindingAuthenticationRequired ? '1' : '0');
    // stand-alone-property-mode
    initString += (this->isStandAloneMode && (this->deviceProperties.GetCount() == 1)) ? '1' : '0';
    // appendix (zeros + delimiter)
    initString += "000\r\0";

    // send it
    this->sendData(initString);
}

void LaRoomyAppImplementation::onPropertyRequest(const String& data){

    // get requested index
    auto index = Convert::x2CharHexValueToU8BitValue(data.charAt(2), data.charAt(3));

    // make sure the index is in range
    if(index < this->deviceProperties.GetCount()){

        // get reference to property element
        auto prop = this->deviceProperties.getObjectCoreReferenceAt(index);
        if(prop != nullptr){

            // generate property string
            if(this->pDescriptionCallback != nullptr){

                // get language id
                this->lastLangID = "";
                unsigned int i = 8;

                while(i < data.length()){
                    if((data.charAt(i) == '\r')||(data.charAt(i) == '\0')){
                        break;
                    }
                    else {
                        this->lastLangID += data.charAt(i);
                    }
                    i++;
                }

                // get descriptor
                this->pDescriptionCallback->onPropertyDescriptionRequired(prop->propertyID, this->lastLangID, prop->descriptor);
            }

            // send property definition
            this->sendData(
                prop->toTransmissionString(TransmissionSubType::RESPONSE, index)
            );
        }
    }
    else {
        // send error
        String errorString = "12";
        errorString += data.charAt(2);
        errorString += data.charAt(3);
        errorString += "0030\r\0";
        this->sendData(errorString);

        if(this->is_monitor_enabled){
            Serial.print("ERROR - invalid property request index: ");
            Serial.print(index);
            Serial.print("\r\n");
        }
    }
}

void LaRoomyAppImplementation::onGroupRequest(const String& data){

    // get requested index
    auto index = Convert::x2CharHexValueToU8BitValue(data.charAt(2), data.charAt(3));

    // make sure the index is in range
    if(index < this->devicePropertyGroups.GetCount()){

        // get ref to group element
        auto group = this->devicePropertyGroups.getObjectCoreReferenceAt(index);
        if(group != nullptr){

            // generate group string with callback
            if(this->pDescriptionCallback != nullptr){

                // get language id
                this->lastLangID = "";
                unsigned int i = 8;

                while(i < data.length()){
                    if((data.charAt(i) == '\r')||(data.charAt(i) == '\0')){
                        break;
                    }
                    else {
                        this->lastLangID += data.charAt(i);
                    }
                    i++;
                }

                // get descriptor
                this->pDescriptionCallback->onGroupDescriptionRequired(group->groupID, this->lastLangID, group->descriptor);
            }

            // send group definition
            this->sendData(
                group->toTransmissionString(TransmissionSubType::RESPONSE, index)
            );
        }
    }
    else {
        // send error
        String errorString = "22";
        errorString += data.charAt(2);
        errorString += data.charAt(3);
        errorString += "0030\r\0";
        this->sendData(errorString);

        if(this->is_monitor_enabled){
            Serial.print("ERROR - invalid group request index: ");
            Serial.print(index);
            Serial.print("\r\n");
        }
    }    
}

void LaRoomyAppImplementation::onPropertyStateRequest(const String& data){

    unsigned int pIndex = Convert::x2CharHexValueToU8BitValue(
        data.charAt(2),
        data.charAt(3)
    );

    if(pIndex < this->deviceProperties.GetCount()){

        auto pType = this->propertyTypeFromPropertyIndex(pIndex);
        auto pID = this->propertyIDFromPropertyIndex(pIndex);

        if(pType < PropertyType::RGB_SELECTOR){
            // simple property state request
            String response = "32";
            char twoBuffer[3];
            twoBuffer[2] = '\0';
            response += data.charAt(2);
            response += data.charAt(3);
            response += "0300";
            Convert::u8BitValueToHexTwoCharBuffer(this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyState, twoBuffer);
            response += twoBuffer;
            response += "\r\0";

            this->sendData(response);
        }
        else {
            // complex property state request
            String response;

            switch (pType)
            {
            case PropertyType::RGB_SELECTOR:
                for(unsigned int i = 0; i < this->rgbStates.GetCount(); i++){
                    if(pID == this->rgbStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->rgbStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::EX_LEVEL_SELECTOR:
                for(unsigned int i = 0; i < this->extendedLevelStates.GetCount(); i++){
                    if(pID == this->extendedLevelStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->extendedLevelStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::TIME_SELECTOR:
                for(unsigned int i = 0; i < this->timeSelectorStates.GetCount(); i++){
                    if(pID == this->timeSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->timeSelectorStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::TIME_FRAME_SELECTOR:
                for(unsigned int i = 0; i < this->timeFrameSelectorStates.GetCount(); i++){
                    if(pID == this->timeFrameSelectorStates.getObjectCoreReferenceAt(0)->associatedPropertyID){
                        response = this->timeFrameSelectorStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::DATE_SELECTOR:
                for(unsigned int i = 0; i < this->dateSelectorStates.GetCount(); i++){
                    if(pID == this->dateSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->dateSelectorStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::UNLOCK_CONTROL:
                for(unsigned int i = 0; i < this->unlockControlStates.GetCount(); i++){
                    if(pID == this->unlockControlStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->unlockControlStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::NAVIGATOR:
                for(unsigned int i = 0; i < this->navigatorStates.GetCount(); i++){
                    if(pID == this->navigatorStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->navigatorStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::BAR_GRAPH:
                for(unsigned int i = 0; i < this->barGraphStates.GetCount(); i++){
                    if(pID == this->barGraphStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->barGraphStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::LINE_GRAPH:
                for(unsigned int i = 0; i < this->lineGraphStates.GetCount(); i++){
                    if(pID == this->lineGraphStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->lineGraphStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::STRING_INTERROGATOR:
                for(unsigned int i = 0; i < this->stringInterrogatorStates.GetCount(); i++){
                    if(pID == this->stringInterrogatorStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->stringInterrogatorStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            case PropertyType::TEXT_LIST_PRESENTER:
                for(unsigned int i = 0; i < this->textListPresenterStates.GetCount(); i++){
                    if(pID == this->textListPresenterStates.getObjectCoreReferenceAt(i)->associatedPropertyID){
                        response = this->textListPresenterStates.getObjectCoreReferenceAt(i)->toStateString(pIndex, TransmissionSubType::RESPONSE);
                        break;
                    }
                }
                break;
            default:
                if(this->is_monitor_enabled){
                    Serial.println("onPropertyStateRequest: Error unknown property type");
                }
                break;
            }
            // send response
            if(response.length() > 0){
                this->sendData(response);
            }
        }
    }
}

void LaRoomyAppImplementation::onPropertyExecutionCommand(const String& data){
    // get element index
    auto pIndex = Convert::x2CharHexValueToU8BitValue(data.charAt(2), data.charAt(3));
    // make sure the index is in bounds
    if(pIndex < this->deviceProperties.GetCount()){
        // get element
        auto propertyElement = this->deviceProperties.getObjectCoreReferenceAt(pIndex);

        switch(propertyElement->propertyType){
            case PropertyType::BUTTON:
                // invoke button callback event
                if(this->pLrCallback != nullptr){
                    this->pLrCallback->onButtonPressed(propertyElement->propertyID);
                }
                break;
            case PropertyType::SWITCH:
                // save data to property object
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyState = (data.charAt(9) == '1') ? 1 : 0;
                // invoke switch callback event
                if(this->pLrCallback != nullptr){
                    this->pLrCallback->onSwitchStateChanged(
                        propertyElement->propertyID,
                        (data.charAt(9) == '1') ? true : false
                    );
                }
                break;
            case PropertyType::LEVEL_SELECTOR:
                // save data to property object
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyState = 
                    Convert::x2CharHexValueToU8BitValue(
                        data.charAt(8),
                        data.charAt(9)
                    );
                // invoke level selector callback event
                if(this->pLrCallback != nullptr){
                    this->pLrCallback->onLevelSelectorValueChanged(
                        propertyElement->propertyID,
                        this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyState
                    );
                }
                break;
            case PropertyType::LEVEL_INDICATOR:
                // this type has no execution!
                break;
            case PropertyType::TEXT_DISPLAY:
                // this type has no execution!
                break;
            case PropertyType::OPTION_SELECTOR:
                // save data to property object
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyState = 
                    Convert::x2CharHexValueToU8BitValue(
                        data.charAt(8),
                        data.charAt(9)
                    );
                // invoke option selector callback event
                if(this->pLrCallback != nullptr){
                    this->pLrCallback->onOptionSelectorIndexChanged(
                        propertyElement->propertyID,
                        this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyState
                    );
                }
                break;
            case PropertyType::RGB_SELECTOR:
                {
                    // acquire data
                    RGBSelectorState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // update state in collection
                    this->_updateRGBState(s, false);
                    // invoke rgb state callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onRGBSelectorStateChanged(s.associatedPropertyID, s);
                    }
                }
                break;
            case PropertyType::EX_LEVEL_SELECTOR:
                {
                    // acquire data
                    ExtendedLevelSelectorState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // update state in collection (partial)
                    this->_updateExLevelStateFromExecutionCommand(s);
                    // invoke callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onExtendedLevelSelectorStateChanged(s.associatedPropertyID, s);
                    }
                }
                break;
            case PropertyType::TIME_SELECTOR:
                {
                    // acquire data
                    TimeSelectorState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // update state in collection
                    this->_updateTimeSelectorState(s, false);
                    // invoke callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onTimeSelectorStateChanged(s.associatedPropertyID, s);
                    }
                }
                break;
            case PropertyType::TIME_FRAME_SELECTOR:
                {
                    // acquire data
                    TimeFrameSelectorState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // update state in collection
                    this->_updateTimeFrameSelectorState(s, false);
                    // invoke callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onTimeFrameSelectorStateChanged(s.associatedPropertyID, s);
                    }
                }
                break;
            case PropertyType::DATE_SELECTOR:
                {
                    // acquire data
                    DateSelectorState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // update state in collection
                    this->_updateDateSelectorState(s, false);
                    // invoke callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onDateSelectorStateChanged(s.associatedPropertyID, s);
                    }
                }
                break;
            case PropertyType::UNLOCK_CONTROL:
                {
                    // acquire data
                    UnlockControlState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // check if the pin was valid and only update if applicable
                    if(this->checkUnlockControlPin(s)){
                        // make sure the flag value is zero (to indicate success)
                        s.flags = 0;
                        // update the state in the collection and send an UI-Update
                        this->_updateUnlockControlState(s, true);

                        // invoke callback
                        if(this->pLrCallback != nullptr){
                            this->pLrCallback->onUnlockControlStateChanged(s.associatedPropertyID, s);
                        }                        
                    }
                }
                break;
            case PropertyType::NAVIGATOR:
                {
                    // acquire data
                    NavigatorState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // update state in collection
                    this->_updateNavigatorState(s, false);
                    // invoke callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onNavigatorStateChanged(s.associatedPropertyID, s);
                    }
                }
                break;
            case PropertyType::BAR_GRAPH:
                // this type has no execution!
                break;
            case PropertyType::LINE_GRAPH:
                // this type has no execution!
                break;
            case PropertyType::STRING_INTERROGATOR:
                {
                    // acquire data
                    StringInterrogatorState s;
                    s.fromExecutionString(data);
                    s.associatedPropertyID = this->deviceProperties.getObjectCoreReferenceAt(pIndex)->propertyID;
                    // do not update the state in collection, the transmission is a "one-shot"
                    // invoke callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onStringInterrogatorDataReceived(s.associatedPropertyID, s.fieldOneContent, s.fieldTwoContent);
                    }
                }
                break;
            case PropertyType::TEXT_LIST_PRESENTER:
                // this type has no execution
                break;
            default:
                if(this->is_monitor_enabled){
                    Serial.print("onPropertyExecutionCommand: Invalid property type detected: ");
                    Serial.print(propertyElement->propertyType);
                    Serial.print("\r\n");
                }
                break;
        }
    }
    else {
        if(this->is_monitor_enabled){
            Serial.print("onPropertyExecutionCommand: Invalid property index detected: ");
            Serial.print(pIndex);
            Serial.print("\r\n");
        }
    }
}

void LaRoomyAppImplementation::onBindingTransmission(const String& data){
    if(data.length() >= 9){

        String passKey;
        // record passkey (if there is one)
        if(data.length() > 10){
            for(unsigned int i = 10; i < data.length(); i++){
                if((data.charAt(i) == '\r')||(data.charAt(i) == '\0')){
                    break;
                }
                passKey += data.charAt(i);
            }
        }

        if(data.charAt(8) == '0'){
            // release binding command
            if(this->auto_handle_binding){
                auto bindC = BindingController::GetInstance();
                if(bindC != nullptr){
                    auto result = bindC->handleBindingTransmission(BindingTransmissionTypes::B_RELEASE, passKey);
                    this->sendBindingResponse(result);
                    if(result == BindingResponseType::BINDING_RELEASE_SUCCESS){
                        this->deviceBindingAuthenticationRequired = false;
                    }
                }
            }
            else {
                if(this->pLrCallback != nullptr){
                    auto result =
                        this->pLrCallback->onBindingTransmissionReceived(BindingTransmissionTypes::B_RELEASE, passKey);
                    this->sendBindingResponse(result);
                    if(result == BindingResponseType::BINDING_RELEASE_SUCCESS){
                        this->deviceBindingAuthenticationRequired = false;
                    }
                }
            }
        }
        else if(data.charAt(8) == '1'){
            // enable binding command
            if(this->auto_handle_binding){
                auto bindC = BindingController::GetInstance();
                if(bindC != nullptr){
                    auto result = bindC->handleBindingTransmission(BindingTransmissionTypes::B_ENABLE, passKey);
                    this->sendBindingResponse(result);
                    if(result == BindingResponseType::BINDING_ENABLE_SUCCESS){
                        this->deviceBindingAuthenticationRequired = true;
                    }
                }
            }
            else {
                if(this->pLrCallback != nullptr){
                    auto result =
                        this->pLrCallback->onBindingTransmissionReceived(BindingTransmissionTypes::B_ENABLE, passKey);
                    this->sendBindingResponse(result);
                    if(result == BindingResponseType::BINDING_ENABLE_SUCCESS){
                        this->deviceBindingAuthenticationRequired = true;
                    }
                }
            }
        }
        else if(data.charAt(8) == '2'){
            // binding auth request
            if(this->auto_handle_binding){
                auto bindC = BindingController::GetInstance();
                if(bindC != nullptr){
                    auto result = bindC->handleBindingTransmission(BindingTransmissionTypes::B_AUTH_REQUEST, passKey);
                    this->sendBindingResponse(result);
                }
            }
            else {
                if(this->pLrCallback != nullptr){
                    auto result =
                        this->pLrCallback->onBindingTransmissionReceived(BindingTransmissionTypes::B_AUTH_REQUEST, passKey);
                    this->sendBindingResponse(result);
                }
            }
        }
    }
}

void LaRoomyAppImplementation::onNotificationTransmission(const String& data){
    if(data.length() >= 9){
        switch(data.charAt(8)){
            case '1':// property loading complete notification
                this->propertyLoadingDone = true;
                
                if(this->pLrCallback != nullptr){
                    if(data.length() >= 10){
                        if(data.charAt(9) == '0'){
                            this->pLrCallback->onPropertyLoadingComplete(PropertyLoadingType::LOADED_FROM_DEVICE);
                        }
                        else {
                            this->pLrCallback->onPropertyLoadingComplete(PropertyLoadingType::LOADED_FROM_CACHE);
                        }
                    }
                }
                break;
            case '2':// time request response command
                if(this->pLrCallback != nullptr){
                    if(data.length() >= 15){
                        auto hours = Convert::x2CharHexValueToU8BitValue(data.charAt(9), data.charAt(10));
                        auto minutes = Convert::x2CharHexValueToU8BitValue(data.charAt(11), data.charAt(12));
                        auto seconds = Convert::x2CharHexValueToU8BitValue(data.charAt(13), data.charAt(14));
                        this->pLrCallback->onTimeRequestResponse(hours, minutes, seconds);
                    }
                }
                break;
            case '3':// property sub page invoked notification
                {
                    auto propIndex = Convert::x2CharHexValueToU8BitValue(data.charAt(2), data.charAt(3));
                    auto propID = this->propertyIDFromPropertyIndex(propIndex);
                    this->currentPropertyPageID = propID;

                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onComplexPropertyPageInvoked((cID)propID);
                    }
                }
                break;
            case '4':// navigated back to device main page notification
                this->currentPropertyPageID = ID_DEVICE_MAIN_PAGE;
                
                if(this->pLrCallback != nullptr){
                    this->pLrCallback->onBackNavigation();
                }
                break;
            case '5':// language request response command
                if(this->pLrCallback != nullptr){
                    if(data.length() >= 10){
                        String langID;
                        unsigned int counter = 9;
                        while(counter < data.length()){
                            auto c = data.charAt(counter);
                            if((c == '\r') || (c == '\0')){
                                break;
                            }
                            else {
                                langID += c;
                            }
                        }
                        this->pLrCallback->onLanguageRequestResponse(langID);
                    }
                }
                break;
            case '6':// factory reset request command
                if(this->pLrCallback != nullptr){
                    this->pLrCallback->onFactoryResetRequest();
                }
                break;
            case '7':// device reconnected notification
                if(this->pLrCallback != nullptr){
                    if(data.length() >= 11){
                        auto propIndex = Convert::x2CharHexValueToU8BitValue(
                                data.charAt(9),
                                data.charAt(10)
                            );

                        this->pLrCallback->onDeviceConnectionRestored(
                            this->propertyIDFromPropertyIndex(propIndex)
                        );
                    }
                }
                if(this->auto_refresh_states){
                    this->sendRefreshAllStatesCommand();
                }
                break;
            case '9':// date request response
                if(this->pLrCallback != nullptr){
                    if(data.length() >= 17){
                        auto day = Convert::x2CharHexValueToU8BitValue(data.charAt(9), data.charAt(10));
                        auto month = Convert::x2CharHexValueToU8BitValue(data.charAt(11), data.charAt(12));
                        auto year = Convert::x4CharHexValueToUnsigned16BitValue(data.charAt(13), data.charAt(14), data.charAt(15), data.charAt(16));
                        this->pLrCallback->onDateRequestResponse(day, month, year);
                    }
                }
                break;
            case 'a':// UI-Mode request response
                if(this->pLrCallback != nullptr){
                    if(data.length() >= 10){
                        UIMODEDATA uiData;
                        uiData.dummy = 0;
                        uiData.isNightMode = data.charAt(9) == '0' ? false : true;
                        this->pLrCallback->onUIModeRequestResponse(&uiData);
                    }
                }
                break;
            default:
                if(this->is_monitor_enabled){
                    Serial.print("onNotificationTransmission: unknown notification type character: ");
                    Serial.print(data.charAt(8));
                    Serial.print("\r\n");
                }
                break;
        }
    }
}

void LaRoomyAppImplementation::updateSimplePropertyState(cID propertyID, unsigned int value){

    // search for property
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == propertyID){
            
            // save the state internal
            this->deviceProperties.getObjectCoreReferenceAt(i)->propertyState = value;

            // send update transmissions if conditions are fulfilled
            if(this->is_connected && this->propertyLoadingDone){
                char twoStr[2];
                char stateUpdate[12];

                stateUpdate[0] = '3';
                stateUpdate[1] = '4';

                // set property index
                Convert::u8BitValueToHexTwoCharBuffer(i, twoStr);
                stateUpdate[2] = twoStr[0];
                stateUpdate[3] = twoStr[1];

                // set data size (fixed in this transmission)
                stateUpdate[4] = '0';
                stateUpdate[5] = '3';

                // flags
                stateUpdate[6] = '0';
                stateUpdate[7] = '3'; // 3 byte payload

                // set state
                Convert::u8BitValueToHexTwoCharBuffer(value, twoStr);
                stateUpdate[8] = twoStr[0];
                stateUpdate[9] = twoStr[1];

                // set delimiter and terminator
                stateUpdate[10] = '\r';
                stateUpdate[11] = '\0';

                this->sendData(stateUpdate);
            }
            break;
        }
    }
}

void LaRoomyAppImplementation::updateRGBState(cID rgbSelectorID, RGBSelectorState& state){
    state.associatedPropertyID = rgbSelectorID;
    this->_updateRGBState(state, true);
}
void LaRoomyAppImplementation::updateExLevelState(cID extendedLevelSelectorID, ExtendedLevelSelectorState& state){
    state.associatedPropertyID = extendedLevelSelectorID;
    this->_updateExLevelState(state, true);
}
void LaRoomyAppImplementation::updateTimeSelectorState(cID timeSelectorID, TimeSelectorState& state){
    state.associatedPropertyID = timeSelectorID;
    this->_updateTimeSelectorState(state, true);
}
void LaRoomyAppImplementation::updateTimeFrameSelectorState(cID timeFrameSelectorID, TimeFrameSelectorState& state){
    state.associatedPropertyID = timeFrameSelectorID;
    this->_updateTimeFrameSelectorState(state, true);
}
void LaRoomyAppImplementation::updateDateSelectorState(cID dateSelectorID, DateSelectorState& state){
    state.associatedPropertyID = dateSelectorID;
    this->_updateDateSelectorState(state, true);
}
void LaRoomyAppImplementation::updateUnlockControlState(cID unlockControlID, UnlockControlState& state){
    state.associatedPropertyID = unlockControlID;
    this->_updateUnlockControlState(state, true);
}
void LaRoomyAppImplementation::updateNavigatorState(cID navigatorID, NavigatorState& state){
    state.associatedPropertyID = navigatorID;
    this->_updateNavigatorState(state, true);
}
void LaRoomyAppImplementation::updateBarGraphState(cID barGraphID, BarGraphState& state){
    state.associatedPropertyID = barGraphID;
    this->_updateBarGraphState(state, true);
}
void LaRoomyAppImplementation::updateLineGraphState(cID lineGraphID, LineGraphState& state){
    state.associatedPropertyID = lineGraphID;
    this->_updateLineGraphState(state, true);
}

void LaRoomyAppImplementation::updateStringInterrogatorState(cID stringInterrogatorID, StringInterrogatorState& state){
    state.associatedPropertyID = stringInterrogatorID;
    this->_updateStringInterrogatorState(state, true);
}

void LaRoomyAppImplementation::updateTextListPresenterState(cID textListPresenterID, TextListPresenterState& state){
    state.associatedPropertyID = textListPresenterID;
    this->_updateTextListPresenterState(state, true);
}

void LaRoomyAppImplementation::barGraphFastDataPipeSetSingleBarValue(cID barGraphID, unsigned int barIndex, float barValue){

    if(this->is_connected){ // only do the job if it's worth it

        // build payload data
        char payLoadData[30] = {'\0'};
        sprintf(payLoadData, "%i::%.3f;;\r", barIndex, barValue);

        char transmissionData[9];

        // transmission header entry
        transmissionData[0] = '8';
        transmissionData[1] = '0';
        // barGraph ID
        char twoChar[2];
        Convert::u8BitValueToHexTwoCharBuffer(
            this->propertyIndexFromPropertyID(barGraphID),
            twoChar
        );
        transmissionData[2] = twoChar[0];
        transmissionData[3] = twoChar[1];
        // payload data size
        Convert::u8BitValueToHexTwoCharBuffer((uint8_t)strlen(payLoadData), twoChar);
        transmissionData[4] = twoChar[0];
        transmissionData[5] = twoChar[1];
        // flags (0)
        transmissionData[6] = '0';
        transmissionData[7] = '0';
        // delimiter
        transmissionData[8] = '\0';

        // build final transmission string
        String dataToSend = transmissionData;
        dataToSend += payLoadData;
        
        // send
        this->sendData(dataToSend);
    }
}

void LaRoomyAppImplementation::barGraphFastDataPipeSetAllBarValues(cID barGraphID, itemCollection<BarData>& bData){

    if(this->is_connected){ // only do the job if it is worth it
        if(bData.GetCount() > 0){

            String payLoadData;

            for(unsigned int i = 0; i < bData.GetCount(); i++){
                // build payload data
                char singleBarData[30] = {'\0'};
                sprintf(singleBarData, "%i::%.3f;;", i , bData.getObjectCoreReferenceAt(i)->barValue);
                // add payload data
                payLoadData += singleBarData;
            }
            payLoadData += '\r';

            bool oversize = (payLoadData.length() > 245) ? true : false;
        
            char transmissionData[9];

            // transmission header entry
            transmissionData[0] = '8';
            transmissionData[1] = '0';
            // barGraph ID
            char twoChar[2];
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(barGraphID), 
                twoChar
            );
            transmissionData[2] = twoChar[0];
            transmissionData[3] = twoChar[1];
            // payload data size
            if(oversize){
                transmissionData[4] = 'f';
                transmissionData[5] = 'f';
            }
            else {
                Convert::u8BitValueToHexTwoCharBuffer(payLoadData.length(), twoChar);
                transmissionData[4] = twoChar[0];
                transmissionData[5] = twoChar[1];
            }
            // flags
            transmissionData[6] = '0';
            transmissionData[7] = oversize ? '5' : '0';
            // delimiter
            transmissionData[8] = '\0';

            // build final transmission string
            String dataToSend = transmissionData;
            dataToSend += payLoadData;
            
            // send
            this->sendData(dataToSend);
        }
    }
}

void LaRoomyAppImplementation::lineGraphFastDataPipeResetDataPoints(cID lineGraphID, LineGraphDataPoints& lData){
    if(this->is_connected){ // only do the job if it is worth it
        if(lData.count() > 0){
            // get payload
            String payLoadData = lData.toString();
            payLoadData += '\r';

            bool oversize = (payLoadData.length() > 245) ? true : false;

            char transmissionData[9];

            // transmission header entry
            transmissionData[0] = '8';
            transmissionData[1] = '0';
            // lineGraph ID
            char twoChar[2];
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(lineGraphID), 
                twoChar
            );
            transmissionData[2] = twoChar[0];
            transmissionData[3] = twoChar[1];
            // payload data size
            if(oversize){
                transmissionData[4] = 'f';
                transmissionData[5] = 'f';
            }
            else {
                Convert::u8BitValueToHexTwoCharBuffer(payLoadData.length(), twoChar);
                transmissionData[4] = twoChar[0];
                transmissionData[5] = twoChar[1];
            }
            // flags
            transmissionData[6] = '0';
            transmissionData[7] = oversize ? '5' : '0';
            // delimiter
            transmissionData[8] = '\0';

            // build final transmission string
            String dataToSend = transmissionData;
            dataToSend += payLoadData;

            // send
            this->sendData(dataToSend);
        }
    }
}

void LaRoomyAppImplementation::lineGraphFastDataPipeAddDataPoints(cID lineGraphID, LineGraphDataPoints& lData){
    if(this->is_connected){ // only do the job if it is worth it
        if(lData.count() > 0){
            // create payload
            String payLoadData = "padd;";
            payLoadData += lData.toString();
            payLoadData += '\r';

            bool oversize = (payLoadData.length() > 245) ? true : false;

            char transmissionData[9];

            // transmission header entry
            transmissionData[0] = '8';
            transmissionData[1] = '0';
            // lineGraph ID
            char twoChar[2];
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(lineGraphID), 
                twoChar
            );
            transmissionData[2] = twoChar[0];
            transmissionData[3] = twoChar[1];
            // payload data size
            if(oversize){
                transmissionData[4] = 'f';
                transmissionData[5] = 'f';
            }
            else {
                Convert::u8BitValueToHexTwoCharBuffer(payLoadData.length(), twoChar);
                transmissionData[4] = twoChar[0];
                transmissionData[5] = twoChar[1];
            }
            // flags
            transmissionData[6] = '0';
            transmissionData[7] = oversize ? '5' : '0';
            // delimiter
            transmissionData[8] = '\0';

            // build final transmission string
            String dataToSend = transmissionData;
            dataToSend += payLoadData;

            // send
            this->sendData(dataToSend);
        }
    }
}

void LaRoomyAppImplementation::lineGraphFastDataPipeAddPoint(cID lineGraphID, LPPOINT pPoint){
    if(this->is_connected){ // only do the job if it is worth it
        if(pPoint != nullptr){

            char pointString[80] = {'\0'};
            sprintf(pointString, "%g:%g;\r", pPoint->x, pPoint->y);

            // create payload
            String payLoadData = "padd;";
            payLoadData += pointString;
            payLoadData += '\r';

            char transmissionData[9];

            // transmission header entry
            transmissionData[0] = '8';
            transmissionData[1] = '0';
            // lineGraph ID
            char twoChar[2];
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(lineGraphID), 
                twoChar
            );
            transmissionData[2] = twoChar[0];
            transmissionData[3] = twoChar[1];
            // payload data size
            Convert::u8BitValueToHexTwoCharBuffer(
                payLoadData.length() + 1,
                twoChar
            );
            transmissionData[4] = twoChar[0];
            transmissionData[5] = twoChar[1];
            // flags (0)
            transmissionData[6] = '0';
            transmissionData[7] = '0';
            // delimiter
            transmissionData[8] = '\0';

            // build final transmission string
            String dataToSend = transmissionData;
            dataToSend += payLoadData;

            // send
            this->sendData(dataToSend);
        }
    }
}

void LaRoomyAppImplementation::lineGraphFastDataPipeAddDataPoints(cID lineGraphID, LineGraphDataPoints& lData, float shifter, LineGraphGridShiftDirection dir){
    if(this->is_connected){ // only do the job if it is worth it
        if(lData.count() > 0){
            // create payload
            String payLoadData = "padd;";
            char axis, sign;
            switch (dir)
            {
            case LineGraphGridShiftDirection::LGS_SHIFT_X_AXIS_MINUS:
                axis = 'x';
                sign = '-';
                break;
            case LineGraphGridShiftDirection::LGS_SHIFT_X_AXIS_PLUS:
                axis = 'x';
                sign = '+';
                break;
            case LineGraphGridShiftDirection::LGS_SHIFT_Y_AXIS_MINUS:
                axis = 'y';
                sign = '-';
                break;
            case LineGraphGridShiftDirection::LGS_SHIFT_Y_AXIS_PLUS:
                axis = 'y';
                sign = '+';
                break;            
            default:
                axis = 'e';
                sign = 'e';
                break;
            }

            char shiftString[56] = {'\0'};
            sprintf(shiftString, "%csc%c:%g;", axis, sign, shifter);
            payLoadData += shiftString;

            payLoadData += lData.toString();
            payLoadData += '\r';

            bool oversize = (payLoadData.length() > 245) ? true : false;

            char transmissionData[9];

            // transmission header entry
            transmissionData[0] = '8';
            transmissionData[1] = '0';
            // lineGraph ID
            char twoChar[2];
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(lineGraphID), 
                twoChar
            );
            transmissionData[2] = twoChar[0];
            transmissionData[3] = twoChar[1];
            // payload data size
            if(oversize){
                transmissionData[4] = 'f';
                transmissionData[5] = 'f';
            }
            else {
                Convert::u8BitValueToHexTwoCharBuffer(payLoadData.length(), twoChar);
                transmissionData[4] = twoChar[0];
                transmissionData[5] = twoChar[1];
            }
            // flags
            transmissionData[6] = '0';
            transmissionData[7] = oversize ? '5' : '0';
            // delimiter
            transmissionData[8] = '\0';

            // build final transmission string
            String dataToSend = transmissionData;
            dataToSend += payLoadData;

            // send
            this->sendData(dataToSend);
        }
    }
}

void LaRoomyAppImplementation::lineGraphFastDataPipeAddPoint(cID lineGraphID, LPPOINT pPoint, float shifter, LineGraphGridShiftDirection dir){
    if(this->is_connected){ // only do the job if it is worth it
        if(pPoint != nullptr){
            // create payload
            String payLoadData = "padd;";
            char axis, sign;
            switch (dir)
            {
            case LineGraphGridShiftDirection::LGS_SHIFT_X_AXIS_MINUS:
                axis = 'x';
                sign = '-';
                break;
            case LineGraphGridShiftDirection::LGS_SHIFT_X_AXIS_PLUS:
                axis = 'x';
                sign = '+';
                break;
            case LineGraphGridShiftDirection::LGS_SHIFT_Y_AXIS_MINUS:
                axis = 'y';
                sign = '-';
                break;
            case LineGraphGridShiftDirection::LGS_SHIFT_Y_AXIS_PLUS:
                axis = 'y';
                sign = '+';
                break;            
            default:
                axis = 'e';
                sign = 'e';
                break;
            }

            char shiftString[56] = {'\0'};
            sprintf(shiftString, "%csc%c:%g;", axis, sign, shifter);
            payLoadData += shiftString;
            char pointString[80] = {'\0'};
            sprintf(pointString, "%g:%g;\r", pPoint->x, pPoint->y);
            payLoadData += pointString;

            char transmissionData[9];

            // transmission header entry
            transmissionData[0] = '8';
            transmissionData[1] = '0';
            // lineGraph ID
            char twoChar[2];
            Convert::u8BitValueToHexTwoCharBuffer(
                this->propertyIndexFromPropertyID(lineGraphID), 
                twoChar
            );
            transmissionData[2] = twoChar[0];
            transmissionData[3] = twoChar[1];
            // payload data size
            Convert::u8BitValueToHexTwoCharBuffer(payLoadData.length(), twoChar);
            transmissionData[4] = twoChar[0];
            transmissionData[5] = twoChar[1];
            // flags (0)
            transmissionData[6] = '0';
            transmissionData[7] = '0';
            // delimiter
            transmissionData[8] = '\0';

            // build final transmission string
            String dataToSend = transmissionData;
            dataToSend += payLoadData;

            // send
            this->sendData(dataToSend);
        }
    }
}

void LaRoomyAppImplementation::sendData(const String& data){

    if(!this->is_connected || data.length() == 0){
        // not connected -> skip execution
        return;
    }
    // check if the data must be sent in fragments
    if(data.length() > OUT_MTU_SIZE){
        // the data must be sent in fragments
        String header = "";
        String payLoad = "";
        String cFragment = "";

        if(this->is_monitor_enabled){
            Serial.print("SendData:  ");
            Serial.println(data.c_str());
            Serial.println("Data larger than MTU, splitting data in fragments!");
        }

        itemCollection<String> fragments;

        // set the fragment entry character
        switch(data.charAt(0)){
            case '1':
                header += 'a';
                break;
            case '2':
                header += 'b';
                break;
            case '3':
                header += 'c';
                break;
            case '4':
                header += 'd';
                break;
            case '5':
                header += 'e';
                break;
            case '6':
                header += 'f';
                break;
            case '8':
                header += '9';
                break;
            default:
                header += 'E';
                break;
        }

        for(int i = 1; i < 8; i++){
            header += data.charAt(i);
        }

        for(unsigned int i = 8; i < data.length(); i++){
            char c = data.charAt(i);
            if((c == '\r')||(c == '\0')){
                break;
            }
            else {
                payLoad += c;
            }
        }

        cFragment = header;

        for(unsigned int i = 0; i < payLoad.length(); i++){
            if(cFragment.length() < 18){
                cFragment += payLoad.charAt(i);
            }
            else {
                cFragment += payLoad.charAt(i);
                cFragment += "\r";

                if(i == (payLoad.length() - 1)){
                    // last fragment
                    if(data.charAt(7) == '5'){
                        cFragment.setCharAt(7, '7');
                    }
                }                

                if(this->is_monitor_enabled){
                    Serial.print("Generated data fragment: ");
                    Serial.println(cFragment.c_str());
                }

                fragments.AddItem(cFragment);
                cFragment = header;
            }
        }
        if(cFragment.length() > header.length()){
            cFragment += "\r\0";

            // last fragment
            if(data.charAt(7) == '5'){
                cFragment.setCharAt(7, '7');
            }            

            if(this->is_monitor_enabled){
                Serial.print("Generated data fragment: ");
                Serial.println(cFragment.c_str());
            }

            fragments.AddItem(cFragment);
        }

        for(unsigned int i = 0; i < fragments.GetCount(); i++){
            delay(40);
            this->pTxCharacteristic->writeValue(
                fragments.getObjectCoreReferenceAt(i)->c_str()
            );
        }
    }
    else {
        // monitor
        if(this->is_monitor_enabled){
            Serial.print("Sending data: ");
            Serial.println(data.c_str());
        }
        // send data
        this->pTxCharacteristic->writeValue(data.c_str());
    }
}

void LaRoomyAppImplementation::ble_start(){

    this->applyDeviceImageIDToBluetoothName();

    BLE.begin();
    BLE.setLocalName(this->bluetoothName.c_str());
    BLE.setDeviceName(this->bluetoothName.c_str());


    this->pService = new BLEService(this->serviceUUID.c_str());
    if(this->pService != nullptr){

        BLE.setAdvertisedService(*this->pService);

        this->pTxCharacteristic =
            new BLECharacteristic(this->txCharacteristicUUID.c_str(), BLERead | BLEWrite | BLENotify, 255, false);
        
        this->pRxCharacteristic = 
            new BLECharacteristic(this->rxCharacteristicUUID.c_str(), BLERead | BLEWrite | BLENotify, 255, false);

        if(this->pTxCharacteristic != nullptr && this->pRxCharacteristic != nullptr){

            this->pRxCharacteristic->setEventHandler(BLEWritten, LaRoomyAppImplementation::rxCharacteristicWritten);

            this->pService->addCharacteristic(*this->pTxCharacteristic);
            this->pService->addCharacteristic(*this->pRxCharacteristic);           

            BLE.addService(*this->pService);

            BLE.setEventHandler(BLEConnected, LaRoomyAppImplementation::connectHandler);
            BLE.setEventHandler(BLEDisconnected, LaRoomyAppImplementation::disconnectHandler);

            BLE.advertise();
        }
    }
}

void LaRoomyAppImplementation::ble_restart(){
    this->ble_terminate();
    this->ble_start();
}

void LaRoomyAppImplementation::ble_terminate(){

    BLE.stopAdvertise();
    delay(100);

    BLE.end();
    delay(100);

    if(this->pService != nullptr){
        delete this->pService;
        this->pService = nullptr;
    }
    if(this->pTxCharacteristic != nullptr){
        delete this->pTxCharacteristic;
        this->pTxCharacteristic = nullptr;
    }
    if(this->pRxCharacteristic != nullptr){
        delete this->pRxCharacteristic;
        this->pRxCharacteristic = nullptr;
    }
}

unsigned int LaRoomyAppImplementation::propertyIndexFromPropertyID(unsigned int pId){
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == pId){
            return i;
        }
    }
    return INVALID_ELEMENT_INDEX;
}

unsigned int LaRoomyAppImplementation::propertyTypeFromPropertyIndex(unsigned int propertyIndex){
    if(propertyIndex < this->deviceProperties.GetCount()){
        return this->deviceProperties.getObjectCoreReferenceAt(propertyIndex)->propertyType;
    }
    return PropertyType::PTYPE_INVALID;
}

unsigned int LaRoomyAppImplementation::propertyIDFromPropertyIndex(unsigned int propertyIndex){
    if(propertyIndex < this->deviceProperties.GetCount()){
        return this->deviceProperties.getObjectCoreReferenceAt(propertyIndex)->propertyID;
    }
    return 0;
}

unsigned int LaRoomyAppImplementation::groupIndexFromGroupID(cID groupID){
    for(unsigned int i = 0; i < this->devicePropertyGroups.GetCount(); i++){
        if(this->devicePropertyGroups.getObjectCoreReferenceAt(i)->groupID == groupID){
            return i;
        }
    }
    return INVALID_ELEMENT_INDEX;
}

bool LaRoomyAppImplementation::validatePropertyID(cID pID){
    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->propertyID == pID){
            return true;
        }
    }
    return false;
}

void LaRoomyAppImplementation::applyDeviceImageIDToBluetoothName(){

    if(this->bluetoothName.length() > 26){
        String tempName;
        for(unsigned int i = 0; i < this->bluetoothName.length(); i++){
            if(i < 26){
                tempName += this->bluetoothName.charAt(i);
            } else {
                break;
            }
        }
        this->bluetoothName = tempName;
    }
    if(this->deviceImageID > 0 && this->deviceImageID < 256){
        char tb[3];
        tb[2] = '\0';
        this->bluetoothName += '_';       
        Convert::u8BitValueToHexTwoCharBuffer(this->deviceImageID, tb);
        this->bluetoothName += tb;
    }
}

void LaRoomyAppImplementation::rearrangeGroupIndexes(){

    int curIndex = -1;
    cID curID = 0;

    for(unsigned int i = 0; i < this->deviceProperties.GetCount(); i++){
        if(this->deviceProperties.getObjectCoreReferenceAt(i)->flags & PROPERTY_ELEMENT_FLAG_IS_GROUP_MEMBER){
            if(this->deviceProperties.getObjectCoreReferenceAt(i)->relatedGroupID != curID){
                curID = this->deviceProperties.getObjectCoreReferenceAt(i)->relatedGroupID;
                curIndex++;
            }
            if(curIndex == -1){
                // error, groupID was set to zero (not allowed)
                break;
            }
            this->deviceProperties.getObjectCoreReferenceAt(i)->groupIndex = (unsigned int)curIndex;
        }
    }
}

void LaRoomyAppImplementation::initializeComplexPropertyState(const DeviceProperty& p){

    auto pIndex = this->propertyIndexFromPropertyID(p.propertyID);

    // initialize complex property states if required
    if(p.propertyType > PropertyType::OPTION_SELECTOR){
        switch (p.propertyType)
        {
        case PropertyType::RGB_SELECTOR:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set the state from the definition
                this->initRGBStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the initial state defintion
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultRGBState(p.propertyID);
            }
            break;
        case PropertyType::EX_LEVEL_SELECTOR:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set the state from the definition
                this->initExLevelStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the definition
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultExLevelState(p.propertyID);
            }
            break;
        case PropertyType::TIME_SELECTOR:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set state from definition
                this->initTimeSelectorStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the definition
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultTimeSelectorState(p.propertyID);
            }
            break;
        case PropertyType::TIME_FRAME_SELECTOR:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set state from defintion string
                this->initTimeFrameSelectorStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the definition
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultTimeFrameSelectorState(p.propertyID);
            }
            break;
        case PropertyType::DATE_SELECTOR:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set state from definition string
                this->initDateSelectorStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the definition
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultDateSelectorState(p.propertyID);
            }
            break;
        case PropertyType::UNLOCK_CONTROL:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set state from definition string
                this->initUnlockControlStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the definition
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultUnlockControlState(p.propertyID);
            }
            break;
        case PropertyType::NAVIGATOR:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set state from definition string
                this->initNavigatorStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the definition
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultNavigatorState(p.propertyID);
            }
            break;
        case PropertyType::BAR_GRAPH:
            // check if there is an initial state definition
            if(p.barGraphStateHolder != nullptr){
                // set state from pointer
                this->initBarGraphStateFromInitialStatePointer(p.propertyID, p.barGraphStateHolder);
                // clear the state holder
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->clearStateHolder();
            }
            else {
                // insert default state
                this->initDefaultBarGraphState(p.propertyID);
            }
            break;
        case PropertyType::LINE_GRAPH:
            // check if there is an initial state defintion
            if(p.lineGraphStateHolder != nullptr){
                // set state from pointer
                this->initLineGraphStateFromInitialStatePointer(p.propertyID, p.lineGraphStateHolder);
                // clear the state holder
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->clearStateHolder();
            }
            else {
                // init default state
                this->initDefaultLineGraphState(p.propertyID);
            }
            break;
        case PropertyType::STRING_INTERROGATOR:
            // check if there is an initial state definition
            if(p.stringInterrogatorStateHolder != nullptr){
                // set state from pointer
                this->initStringInterrogatorStateFromInitialStatePointer(p.propertyID, p.stringInterrogatorStateHolder);
                // clear state holder
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->clearStateHolder();
            }
            else {
                // init default state
                this->initDefaultStringInterrogatorState(p.propertyID);
            }
            break;
        case PropertyType::TEXT_LIST_PRESENTER:
            // check if there is an initial state definition
            if(p.initialStateDefinition.length() > 0){
                // set state from definition string
                this->initTextListPresenterStateFromInitialStateString(p.propertyID, p.initialStateDefinition);
                // clear the definition
                this->deviceProperties.getObjectCoreReferenceAt(pIndex)->initialStateDefinition = "";
            }
            else {
                // insert default state
                this->initDefaultTextListPresenterState(p.propertyID);
            }
            break;
        default:
            break;
        }
    }
}

void LaRoomyAppImplementation::initDefaultRGBState(unsigned int propertyID){
    RGBSelectorState rgbState;
    rgbState.associatedPropertyID = propertyID;
    this->rgbStates.AddItem(rgbState);
}

void LaRoomyAppImplementation::initRGBStateFromInitialStateString(cID propertyID, const String &iss){
    RGBSelectorState rgbState;
    rgbState.fromExecutionString(iss);
    rgbState.associatedPropertyID = propertyID;
    this->rgbStates.AddItem(rgbState);
}

void LaRoomyAppImplementation::initDefaultExLevelState(cID propertyID){
    ExtendedLevelSelectorState exLevelState;
    exLevelState.associatedPropertyID = propertyID;
    this->extendedLevelStates.AddItem(exLevelState);
}

void LaRoomyAppImplementation::initExLevelStateFromInitialStateString(cID propertyID, const String &iss){
    ExtendedLevelSelectorState els;
    els.fromDataHolderString(iss);
    els.associatedPropertyID = propertyID;
    this->extendedLevelStates.AddItem(els);
}

void LaRoomyAppImplementation::initDefaultTimeSelectorState(cID propertyID){
    TimeSelectorState timeSelectorState;
    timeSelectorState.associatedPropertyID = propertyID;
    this->timeSelectorStates.AddItem(timeSelectorState);
}

void LaRoomyAppImplementation::initTimeSelectorStateFromInitialStateString(cID propertyID, const String &iss){
    TimeSelectorState ts;
    ts.fromExecutionString(iss);
    ts.associatedPropertyID = propertyID;
    this->timeSelectorStates.AddItem(ts);
}

void LaRoomyAppImplementation::initDefaultTimeFrameSelectorState(cID propertyID){
    TimeFrameSelectorState tfss;
    tfss.associatedPropertyID = propertyID;
    this->timeFrameSelectorStates.AddItem(tfss);
}

void LaRoomyAppImplementation::initTimeFrameSelectorStateFromInitialStateString(cID propertyID, const String &iss){
    TimeFrameSelectorState tfss;
    tfss.fromExecutionString(iss);
    tfss.associatedPropertyID = propertyID;
    this->timeFrameSelectorStates.AddItem(tfss);
}

void LaRoomyAppImplementation::initDefaultDateSelectorState(cID propertyID){
    DateSelectorState dss;
    dss.associatedPropertyID = propertyID;
    this->dateSelectorStates.AddItem(dss);
}

void LaRoomyAppImplementation::initDateSelectorStateFromInitialStateString(cID propertyID, const String & iss){
    DateSelectorState dss;
    dss.fromExecutionString(iss);
    dss.associatedPropertyID = propertyID;
    this->dateSelectorStates.AddItem(dss);
}

void LaRoomyAppImplementation::initDefaultUnlockControlState(cID propertyID){
    UnlockControlState ucs;
    ucs.associatedPropertyID = propertyID;
    this->unlockControlStates.AddItem(ucs);
}

void LaRoomyAppImplementation::initUnlockControlStateFromInitialStateString(cID propertyID, const String &iss){
    UnlockControlState ucs;
    ucs.fromExecutionString(iss);
    ucs.associatedPropertyID = propertyID;
    this->unlockControlStates.AddItem(ucs);
}

void LaRoomyAppImplementation::initDefaultNavigatorState(cID propertyID){
    NavigatorState ngs;
    ngs.associatedPropertyID = propertyID;
    this->navigatorStates.AddItem(ngs);
}

void LaRoomyAppImplementation::initNavigatorStateFromInitialStateString(cID propertyID, const String &iss){
    NavigatorState nc;
    nc.fromExecutionString(iss);
    nc.associatedPropertyID = propertyID;
    this->navigatorStates.AddItem(nc);
}

void LaRoomyAppImplementation::initDefaultBarGraphState(cID propertyID){
    BarGraphState bgs;
    bgs.associatedPropertyID = propertyID;
    this->barGraphStates.AddItem(bgs);
}

void LaRoomyAppImplementation::initBarGraphStateFromInitialStatePointer(cID propertyID, BarGraphState* state){
    state->associatedPropertyID = propertyID;
    this->barGraphStates.AddItem(state);
}

void LaRoomyAppImplementation::initDefaultLineGraphState(cID propertyID){
    LineGraphState lgs;
    lgs.associatedPropertyID = propertyID;
    this->lineGraphStates.AddItem(lgs);
}

void LaRoomyAppImplementation::initLineGraphStateFromInitialStatePointer(cID propertyID, LineGraphState* state){
    state->associatedPropertyID = propertyID;
    this->lineGraphStates.AddItem(state);
}

void LaRoomyAppImplementation::initDefaultStringInterrogatorState(cID propertyID){
    StringInterrogatorState sis;
    sis.associatedPropertyID = propertyID;
    this->stringInterrogatorStates.AddItem(sis);
}

void LaRoomyAppImplementation::initStringInterrogatorStateFromInitialStatePointer(cID propertyID, StringInterrogatorState* state){
    state->associatedPropertyID = propertyID;
    this->stringInterrogatorStates.AddItem(state);
}

void LaRoomyAppImplementation::initDefaultTextListPresenterState(cID propertyID){
    TextListPresenterState tlps;
    tlps.associatedPropertyID = propertyID;
    this->textListPresenterStates.AddItem(tlps);
}

void LaRoomyAppImplementation::initTextListPresenterStateFromInitialStateString(cID propertyID, const String& iss){
    TextListPresenterState tlps;
    tlps.useBackgroundStack = iss.charAt(8) == '1' ? true : false;
    tlps.associatedPropertyID = propertyID;
    this->textListPresenterStates.AddItem(tlps);
}

void LaRoomyAppImplementation::_updateRGBState(RGBSelectorState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->rgbStates.GetCount(); i++){
        if(this->rgbStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->rgbStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if the state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateExLevelState(ExtendedLevelSelectorState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->extendedLevelStates.GetCount(); i++){
        if(this->extendedLevelStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->extendedLevelStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateExLevelStateFromExecutionCommand(ExtendedLevelSelectorState &s){
    // search for state
    for(unsigned int i = 0; i < this->extendedLevelStates.GetCount(); i++){
        if(this->extendedLevelStates.getObjectCoreReferenceAt(i)->associatedPropertyID == s.associatedPropertyID){
            // only update the level and the on param (other values are not incluced in the execution command!)
            this->extendedLevelStates.getObjectCoreReferenceAt(i)->levelValue = s.levelValue;
            this->extendedLevelStates.getObjectCoreReferenceAt(i)->isOn = s.isOn;
        }
    }
}

void LaRoomyAppImplementation::_updateTimeSelectorState(TimeSelectorState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->timeSelectorStates.GetCount(); i++){
        if(this->timeSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->timeSelectorStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateTimeFrameSelectorState(TimeFrameSelectorState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->timeFrameSelectorStates.GetCount(); i++){
        if(this->timeFrameSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->timeFrameSelectorStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateDateSelectorState(DateSelectorState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->dateSelectorStates.GetCount(); i++){
        if(this->dateSelectorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->dateSelectorStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateUnlockControlState(UnlockControlState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->unlockControlStates.GetCount(); i++){
        if(this->unlockControlStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->unlockControlStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateNavigatorState(NavigatorState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->navigatorStates.GetCount(); i++){
        if(this->navigatorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->navigatorStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateBarGraphState(BarGraphState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->barGraphStates.GetCount(); i++){
        if(this->barGraphStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->barGraphStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateLineGraphState(LineGraphState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->lineGraphStates.GetCount(); i++){
        if(this->lineGraphStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->lineGraphStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateStringInterrogatorState(StringInterrogatorState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->stringInterrogatorStates.GetCount(); i++){
        if(this->stringInterrogatorStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->stringInterrogatorStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::_updateTextListPresenterState(TextListPresenterState& state, bool send){

    bool isValid = false;
    // search for state
    for(unsigned int i = 0; i < this->textListPresenterStates.GetCount(); i++){
        if(this->textListPresenterStates.getObjectCoreReferenceAt(i)->associatedPropertyID == state.associatedPropertyID){
            this->textListPresenterStates.ReplaceAt(i, state);
            isValid = true;
            break;
        }
    }
    // check if state was found
    if(isValid){
        // send update
        if(this->is_connected && send){
            this->sendData(
                state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                )
            );
        }
    }
}

void LaRoomyAppImplementation::sendBindingResponse(BindingResponseType t){
    switch(t){
        case BindingResponseType::BINDING_AUTHENTICATION_SUCCESS:
            this->sendData("6200040020\r\0");
            break;
        case BindingResponseType::BINDING_AUTHENTICATION_FAIL_WRONG_KEY:
            this->sendData("6200040021\r\0");
            break;
        case BindingResponseType::BINDING_ENABLE_SUCCESS:
            this->sendData("6200040010\r\0");
            break;
        case BindingResponseType::BINDING_ENABLE_FAIL_NOT_SUPPORTED:
            this->sendData("6200040012\r\0");
            break;
        case BindingResponseType::BINDING_RELEASE_SUCCESS:
            this->sendData("6200040000\r\0");
            break;
        case BindingResponseType::BINDING_RELEASE_FAIL:
            this->sendData("6200040003\r\0");
            break;
        case BindingResponseType::BINDING_FAIL_NOT_IMPLEMENTED:
            this->sendData("6200040032\r\0");
            break;
        default:
            // unknown error
            this->sendData("6200040004\r\0");
            break;
    }
}

bool LaRoomyAppImplementation::checkUnlockControlPin(UnlockControlState& state){

    // first validate the pID
    if(this->validatePropertyID(state.associatedPropertyID)){
        // then get the current state
        auto curState =
            this->getUnlockControlState(state.associatedPropertyID);

        if(state.mode == UnlockControlModes::UNLOCK_MODE){
            if(state.unlocked == false){
                // in the lock-transmission is no pin included, so to keep the old pin
                // the current pin must be set to the state object which is stored to the collection
                // otherwise the pin would be overridden with an empty string object
                state.pin = curState.pin;
                return true;
            }
            else {
                if(state.pin == curState.pin){
                    return true;
                }
                else {
                    // the pin is not valid - send error response
                    state.flags = 0x01;// unlock failed flag!
                    auto stateString = state.toStateString(
                        this->propertyIndexFromPropertyID(state.associatedPropertyID),
                        TransmissionSubType::UPDATE
                    );
                    this->sendData(stateString);// report to app

                    // notify via callback
                    if(this->pLrCallback != nullptr){
                        this->pLrCallback->onUnlockControlInvalidOperation(
                            state.associatedPropertyID,
                            UnlockControlInvalidOperation::UNLOCK_FAILED_WRONG_PIN
                        );
                    }
                    return false;
                }
            }
        }
        else if(state.mode == UnlockControlModes::PIN_CHANGE_MODE){
            // the old and the new pin must be in this string, so separate it
            String oldPin;
            String newPin;
            bool isOld = true;
            unsigned int nextValidIndex = 0;

            for(unsigned int i = 0; i < state.pin.length(); i++){
                if((state.pin.charAt(i) == ':')&&(isOld)){
                    nextValidIndex = i + 2;
                    isOld = false;
                }
                if(isOld){
                    oldPin += state.pin.charAt(i);
                }
                else {
                    if(i >= nextValidIndex){
                        newPin += state.pin.charAt(i);
                    }
                }
            }
            if(oldPin == curState.pin){
                // the old pin is valid, so set the new pin
                state.pin = newPin;
                return true;
            }
            else {
                // old pin is invalid - send error response
                state.flags = 0x02;// pin change failed flag
                auto stateString = state.toStateString(
                    this->propertyIndexFromPropertyID(state.associatedPropertyID),
                    TransmissionSubType::UPDATE
                );
                this->sendData(stateString);// report to app

                // notify via callback
                if(this->pLrCallback != nullptr){
                    this->pLrCallback->onUnlockControlInvalidOperation(
                        state.associatedPropertyID,
                        UnlockControlInvalidOperation::PIN_CHANGE_REJECTED_WRONG_PIN
                    );
                }
            }

        }
    }
    return false;
}

// *********************************************************************************************************

DeviceProperty::DeviceProperty(Button &b){
    this->propertyType = PropertyType::BUTTON;
    this->propertyID = b.buttonID;
    this->imageID = b.imageID;
    this->isEnabled = b.isEnabled;

    this->descriptor = b.buttonDescriptor;
    this->descriptor += ";;";
    this->descriptor += b.buttonText;
}

DeviceProperty::DeviceProperty(Switch &s){
    this->propertyType = PropertyType::SWITCH;
    this->propertyID = s.switchID;
    this->descriptor = s.switchDescription;
    this->propertyState = (s.switchState == true) ? 1 : 0;
    this->imageID = s.imageID;
    this->isEnabled = s.isEnabled;
}

DeviceProperty::DeviceProperty(LevelSelector &ls){
    this->propertyType = PropertyType::LEVEL_SELECTOR;
    this->propertyID = ls.levelSelectorID;
    this->imageID = ls.imageID;
    this->propertyState = ls.level;
    this->descriptor = ls.levelSelectorDescription;
    this->isEnabled = ls.isEnabled;
}

DeviceProperty::DeviceProperty(LevelIndicator &li){
    this->propertyType = PropertyType::LEVEL_INDICATOR;
    this->imageID = li.imageID;
    this->propertyID = li.levelIndicatorID;
    this->propertyState = li.level;
    this->descriptor = li.levelIndicatorDescription;
    this->isEnabled = li.isEnabled;
    if(li.valueColor.isValidColor()){
        this->descriptor += ";;";
        this->descriptor += li.valueColor.toString();
    }
}

DeviceProperty::DeviceProperty(TextDisplay &td){
    this->propertyType = PropertyType::TEXT_DISPLAY;
    this->imageID = td.imageID;
    this->descriptor = td.textToDisplay;
    this->propertyID = td.textDisplayID;
    this->isEnabled = td.isEnabled;
    if(td.colorRect.isValidColor()){
        this->descriptor += ";;";
        this->descriptor += td.colorRect.toString();
    }
}

DeviceProperty::DeviceProperty(OptionSelector &os){
    this->propertyType = PropertyType::OPTION_SELECTOR;
    this->propertyID = os.optionSelectorID;
    this->imageID = os.imageID;
    this->descriptor = os.toDevicePropertyDescriptor();
    this->propertyState = os.selectedIndex;
    this->isEnabled = os.isEnabled;
}

DeviceProperty::DeviceProperty(RGBSelector &rs){
    this->propertyType = PropertyType::RGB_SELECTOR;
    this->propertyID = rs.rgbSelectorID;
    this->imageID = rs.imageID;
    this->descriptor = rs.rgbSelectorDescription;
    this->initialStateDefinition = rs.rgbState.toStateString(0, TransmissionSubType::TST_NONE);
    this->isEnabled = rs.isEnabled;
}

DeviceProperty::DeviceProperty(ExtendedLevelSelector &els){
    this->propertyType = PropertyType::EX_LEVEL_SELECTOR;
    this->propertyID = els.extendedLevelSelectorID;
    this->imageID = els.imageID;
    this->descriptor = els.extendedLevelSelectorDescription;
    this->initialStateDefinition = els.exLevelState.toStateString(0, TransmissionSubType::TST_NONE);
    this->isEnabled = els.isEnabled;
}

DeviceProperty::DeviceProperty(TimeSelector &ts){
    this->propertyType = PropertyType::TIME_SELECTOR;
    this->propertyID = ts.timeSelectorID;
    this->imageID = ts.imageID;
    this->descriptor = ts.timeSelectorDescription;
    this->initialStateDefinition = ts.timeSelectorState.toStateString(0, TransmissionSubType::TST_NONE);
    this->isEnabled = ts.isEnabled;
}

DeviceProperty::DeviceProperty(TimeFrameSelector &tfs){
    this->propertyType = PropertyType::TIME_FRAME_SELECTOR;
    this->propertyID = tfs.timeFrameSelectorID;
    this->imageID = tfs.imageID;
    this->descriptor = tfs.timeFrameSelectorDescription;
    this->initialStateDefinition = tfs.timeFrameSelectorState.toStateString(0, TransmissionSubType::TST_NONE);
    this->isEnabled = tfs.isEnabled;
}

DeviceProperty::DeviceProperty(DateSelector &ds){
    this->propertyType = PropertyType::DATE_SELECTOR;
    this->propertyID = ds.dateSelectorID;
    this->imageID = ds.imageID;
    this->descriptor = ds.dateSelectorDescription;
    this->initialStateDefinition = ds.dateSelectorState.toStateString(0, TransmissionSubType::TST_NONE);
    this->isEnabled = ds.isEnabled;
}

DeviceProperty::DeviceProperty(UnlockControl &uc){
    this->propertyType = PropertyType::UNLOCK_CONTROL;
    this->propertyID = uc.unlockControlID;
    this->imageID = uc.imageID;
    this->descriptor = uc.unlockControlDescription;
    this->initialStateDefinition = uc.unlockControlState.toDataHolderString();
    this->isEnabled = uc.isEnabled;
}

DeviceProperty::DeviceProperty(NavigatorControl &nc){
    this->propertyType = PropertyType::NAVIGATOR;
    this->propertyID = nc.navigatorID;
    this->imageID = nc.imageID;
    this->descriptor = nc.navigatorDescription;
    this->initialStateDefinition = nc.navigatorState.toStateString(0, TransmissionSubType::TST_NONE);
    this->isEnabled = nc.isEnabled;
}

DeviceProperty::DeviceProperty(BarGraph &bg){
    this->propertyType = PropertyType::BAR_GRAPH;
    this->propertyID = bg.barGraphID;
    this->imageID = bg.imageID;
    this->descriptor = bg.barGraphDescription;
    this->isEnabled = bg.isEnabled;
    this->barGraphStateHolder = new BarGraphState(bg.barGraphState);
}

DeviceProperty::DeviceProperty(LineGraph &lg){
    this->propertyType = PropertyType::LINE_GRAPH;
    this->propertyID = lg.lineGraphID;
    this->imageID = lg.imageID;
    this->descriptor = lg.lineGraphDescription;
    this->isEnabled = lg.isEnabled;
    this->lineGraphStateHolder = new LineGraphState(lg.lineGraphState);
}

DeviceProperty::DeviceProperty(StringInterrogator& si){
    this->propertyType = PropertyType::STRING_INTERROGATOR;
    this->propertyID = si.stringInterrogatorID;
    this->imageID = si.imageID;
    this->descriptor = si.stringInterrogatorDescription;
    this->isEnabled = si.isEnabled;
    this->stringInterrogatorStateHolder = new StringInterrogatorState(si.stringInterrogatorState);
}

DeviceProperty::DeviceProperty(TextListPresenter& tlp){
    this->propertyType = PropertyType::TEXT_LIST_PRESENTER;
    this->propertyID = tlp.textListPresenterID;
    this->imageID = tlp.imageID;
    this->descriptor = tlp.textListPresenterDescription;
    this->initialStateDefinition = tlp.textListPresenterState.toStateString(0, TransmissionSubType::TST_NONE);
    this->isEnabled = tlp.isEnabled;
}

DeviceProperty::~DeviceProperty(){
    if(this->barGraphStateHolder != nullptr){
        delete this->barGraphStateHolder;
    }
    if(this->lineGraphStateHolder != nullptr){
        delete this->lineGraphStateHolder;
    }
    if(this->stringInterrogatorStateHolder != nullptr){
        delete this->stringInterrogatorStateHolder;
    }
}

void DeviceProperty::updateFlags(){
    if(!this->isEnabled){
        this->flags |= PROPERTY_ELEMENT_FLAG_IS_DISABLED;
    }
    else {
        this->flags &= (~PROPERTY_ELEMENT_FLAG_IS_DISABLED);
    }   
}

String DeviceProperty::toTransmissionString(TransmissionSubType t, unsigned int propertyIndex){

    char twoBuffer[3];
    twoBuffer[2] = '\0';

    // set the transmission type characters
    String tString = "1";
    switch (t)
    {
    case TransmissionSubType::REQUEST:
        tString += '1';
        break;
    case TransmissionSubType::RESPONSE:
        tString += '2';
        break;
    case TransmissionSubType::COMMAND:
        tString += '3';
        break;
    case TransmissionSubType::UPDATE:
        tString += '4';
        break;
    case TransmissionSubType::INSERT:
        tString += '5';
        break;
    case TransmissionSubType::REMOVE:
        tString += '6';
        break;
    default:
        tString += 'E';
        break;
    }

    // build the payload-data
    // *********************************
    String payLoadData;

    if(t != TransmissionSubType::REMOVE){   // NOTE: the remove command needs no payload data
        // property type
        Convert::u8BitValueToHexTwoCharBuffer(this->propertyType, twoBuffer);
        payLoadData += twoBuffer;
        // image ID
        Convert::u8BitValueToHexTwoCharBuffer(this->imageID, twoBuffer);
        payLoadData += twoBuffer;
        // groupIndex
        Convert::u8BitValueToHexTwoCharBuffer(this->groupIndex, twoBuffer);
        payLoadData += twoBuffer;
        // flag value
        this->updateFlags();
        Convert::u8BitValueToHexTwoCharBuffer(this->flags, twoBuffer);
        payLoadData += twoBuffer;
        // state value
        Convert::u8BitValueToHexTwoCharBuffer(this->propertyState, twoBuffer);
        payLoadData += twoBuffer;
        // descriptor
        payLoadData += this->descriptor;
    }
    // *******************************

    bool oversize = (payLoadData.length() > 242) ? true : false;
    
    // set property index
    Convert::u8BitValueToHexTwoCharBuffer(propertyIndex, twoBuffer);
    tString += twoBuffer;

    // set payload data-size
    if(oversize){
        tString += "ff";
    }
    else {
        Convert::u8BitValueToHexTwoCharBuffer(payLoadData.length() + 1, twoBuffer);
        tString += twoBuffer;
    }

    // set flags
    if(oversize){
        // set oversize flag
        tString += "05";
    }
    else {
        // set header flags to zero
        tString += "00";
    }
    
    if(payLoadData.length() > 0){
        // add payload data
        tString += payLoadData;
    }

    // add delimiter
    tString += "\r\0";

    return tString;
}

void DeviceProperty::copy(const DeviceProperty& p){
    this->propertyType = p.propertyType;
    this->imageID = p.imageID;
    this->propertyState = p.propertyState;
    this->descriptor = "";
    this->descriptor = p.descriptor;
    this->groupIndex = p.groupIndex;
    this->flags = p.flags;
    this->isEnabled = p.isEnabled;
    this->propertyID = p.propertyID;
    this->relatedGroupID = p.relatedGroupID;

    if(p.barGraphStateHolder != nullptr){
        if(this->barGraphStateHolder != nullptr){
            delete this->barGraphStateHolder;
        }
        this->barGraphStateHolder = new BarGraphState(*p.barGraphStateHolder);
    }
    if(p.lineGraphStateHolder != nullptr){
        if(this->lineGraphStateHolder != nullptr){
            delete this->lineGraphStateHolder;
        }
        this->lineGraphStateHolder = new LineGraphState(*p.lineGraphStateHolder);
    }
    if(p.stringInterrogatorStateHolder != nullptr){
        if(this->stringInterrogatorStateHolder != nullptr){
            delete this->stringInterrogatorStateHolder;
        }
        this->stringInterrogatorStateHolder = new StringInterrogatorState(*p.stringInterrogatorStateHolder);
    }      
}

void DeviceProperty::clearStateHolder(){
    if(this->barGraphStateHolder != nullptr){
        delete this->barGraphStateHolder;
        this->barGraphStateHolder = nullptr;
    }
    if(this->lineGraphStateHolder != nullptr){
        delete this->lineGraphStateHolder;
        this->lineGraphStateHolder = nullptr;
    }
    if(this->stringInterrogatorStateHolder != nullptr){
        delete this->stringInterrogatorStateHolder;
        this->stringInterrogatorStateHolder = nullptr;
    }
}

// *********************************************************************************************************

String DevicePropertyGroup::toTransmissionString(TransmissionSubType t, unsigned int groupIndex){

        char twoBuffer[3];
        twoBuffer[2] = '\0';

        // set the transmission type characters
        String tString = "2";
        switch (t)
        {
        case TransmissionSubType::REQUEST:
            tString += '1';
            break;
        case TransmissionSubType::RESPONSE:
            tString += '2';
            break;
        case TransmissionSubType::COMMAND:
            tString += '3';
            break;
        case TransmissionSubType::UPDATE:
            tString += '4';
            break;
        case TransmissionSubType::INSERT:
            tString += '5';
            break;
        case TransmissionSubType::REMOVE:
            tString += '6';
            break;
        default:
            tString += 'E';
            break;
        }

        String payLoadData;

        if(t != TransmissionSubType::REMOVE){   // NOTE: the remove command needs no payload data
            // member count
            Convert::u8BitValueToHexTwoCharBuffer(this->propertyCount, twoBuffer);
            payLoadData += twoBuffer;
            // image ID
            Convert::u8BitValueToHexTwoCharBuffer(this->imageID, twoBuffer);
            payLoadData += twoBuffer;
            // descriptor
            payLoadData += this->descriptor;
        }
        // *******************************

        // set group index
        Convert::u8BitValueToHexTwoCharBuffer(groupIndex, twoBuffer);
        tString += twoBuffer;
        // set payload data-size
        Convert::u8BitValueToHexTwoCharBuffer(payLoadData.length() + 1, twoBuffer);
        tString += twoBuffer;
        // set header flags to zero
        tString += "00";

        if(payLoadData.length() > 0){
            // add payload data
            tString += payLoadData;
        }

        // add delimiter
        tString += "\r\0";

        return tString;
    }
    