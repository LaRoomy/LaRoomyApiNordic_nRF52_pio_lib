#include "LaRoomyApi_nRF52.h"

//________________________
// Button Section Start

Button::Button(){}

Button::Button(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto b = pLr->getProperty(propertyID);
            *this = b;
        }
        else {
            this->buttonDescriptor = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

Button::Button(const Button &b)
{
    this->buttonDescriptor = b.buttonDescriptor;
    this->buttonText = b.buttonText;
    this->imageID = b.imageID;
    this->buttonID = b.buttonID;
    this->isEnabled = b.isEnabled;
}

Button::Button(const DeviceProperty &dp)
{
    this->imageID = dp.imageID;
    this->buttonID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::BUTTON){
        this->buttonDescriptor = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
}

Button& Button::operator=(const Button& b){
    this->buttonDescriptor = b.buttonDescriptor;
    this->buttonText = b.buttonText;
    this->imageID = b.imageID;
    this->buttonID = b.buttonID;
    this->isEnabled = b.isEnabled;
    return *this;
}

Button& Button::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    this->buttonID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::BUTTON){
        this->buttonDescriptor = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
    return *this;
}

void Button::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
    }
}

String Button::toDevicePropertyDescriptor()
{
    String d = this->buttonDescriptor;
    d += ";;";
    d += this->buttonText;
    return d;
}

void Button::split(const String &descriptor)
{
    auto l = descriptor.length();
    bool sec = false;
    if (l >= 4)
    {
        this->buttonDescriptor = "";
        this->buttonText = "";

        unsigned int i = 0;
        unsigned int nextValidIndex = 0;

        while (i < l)
        {
            auto c = descriptor.charAt(i);

            if (c == ';')
            {
                if ((i < (l - 1)) && (descriptor.charAt(i + 1) == ';'))
                {
                    nextValidIndex = i + 2;
                    sec = true;
                }
            }
            if (sec)
            {
                if (i >= nextValidIndex)
                {
                    this->buttonText += c;
                }
            }
            else
            {
                this->buttonDescriptor += c;
            }
            i++;
        }
    }
    else
    {
        this->buttonDescriptor = descriptor;
    }
}

// Button Section End
//________________________
//
// Switch Section Start

Switch::Switch(){}

Switch::Switch(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto s = pLr->getProperty(propertyID);
            *this = s;
        }
        else {
            this->switchDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

Switch::Switch(const Switch &s)
{
    this->imageID = s.imageID;
    this->switchDescription = s.switchDescription;
    this->switchID = s.switchID;
    this->switchState = s.switchState;
    this->isEnabled = s.isEnabled;
}

Switch::Switch(const DeviceProperty &dp)
{
    this->imageID = dp.imageID;
    this->isEnabled = dp.isEnabled;
    this->switchID = dp.propertyID;
    if(dp.propertyType != PropertyType::SWITCH){
        this->switchDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->switchDescription = dp.descriptor;
    }
    this->switchState = (dp.propertyState != 0) ? true : false;
}

Switch& Switch::operator=(const Switch& s){
    this->imageID = s.imageID;
    this->switchDescription = s.switchDescription;
    this->switchID = s.switchID;
    this->switchState = s.switchState;
    this->isEnabled = s.isEnabled;
    return *this;
}

Switch& Switch::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    this->isEnabled = dp.isEnabled;
    this->switchID = dp.propertyID;
    if(dp.propertyType != PropertyType::SWITCH){
        this->switchDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->switchDescription = dp.descriptor;
    }
    this->switchState = (dp.propertyState != 0) ? true : false;
    return *this;
}

void Switch::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
    }
}

// Switch Section End
//________________________
//
// Level Selector Section Start

LevelSelector::LevelSelector(){}

LevelSelector::LevelSelector(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto ls = pLr->getProperty(propertyID);
            *this = ls;
        }
        else {
            this->levelSelectorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

LevelSelector::LevelSelector(const LevelSelector &ls)
{
    this->level = ls.level;
    this->imageID = ls.imageID;
    this->levelSelectorDescription = ls.levelSelectorDescription;
    this->levelSelectorID = ls.levelSelectorID;
    this->isEnabled = ls.isEnabled;
}

LevelSelector::LevelSelector(const DeviceProperty &dp)
{
    this->level = dp.propertyState;
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::LEVEL_SELECTOR){
        this->levelSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->levelSelectorDescription = dp.descriptor;
    }
    this->levelSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

LevelSelector& LevelSelector::operator=(const LevelSelector& ls){
    this->level = ls.level;
    this->imageID = ls.imageID;
    this->levelSelectorDescription = ls.levelSelectorDescription;
    this->levelSelectorID = ls.levelSelectorID;
    this->isEnabled = ls.isEnabled;
    return *this;
}

LevelSelector& LevelSelector::operator=(const DeviceProperty& dp){
    this->level = dp.propertyState;
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::LEVEL_SELECTOR){
        this->levelSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->levelSelectorDescription = dp.descriptor;
    }
    this->levelSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void LevelSelector::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
    }
}

// Level Selector Section End
//_______________________________________
//
// Level Indicator Section Start

LevelIndicator::LevelIndicator(){}

LevelIndicator::LevelIndicator(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto li = pLr->getProperty(propertyID);
            *this = li;
        }
        else {
            this->levelIndicatorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

LevelIndicator::LevelIndicator(const LevelIndicator &li)
{
    this->level = li.level;
    this->imageID = li.imageID;
    this->levelIndicatorDescription = li.levelIndicatorDescription;
    this->levelIndicatorID = li.levelIndicatorID;
    this->isEnabled = li.isEnabled;
    this->valueColor = li.valueColor;
}

LevelIndicator::LevelIndicator(const DeviceProperty &dp)
{
    this->level = dp.propertyState;
    this->imageID = dp.imageID;
    this->levelIndicatorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::LEVEL_INDICATOR){
        this->levelIndicatorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
}

LevelIndicator& LevelIndicator::operator=(const LevelIndicator& li){
    this->level = li.level;
    this->imageID = li.imageID;
    this->levelIndicatorDescription = li.levelIndicatorDescription;
    this->levelIndicatorID = li.levelIndicatorID;
    this->isEnabled = li.isEnabled;
    this->valueColor = li.valueColor;
    return *this;
}

LevelIndicator& LevelIndicator::operator=(const DeviceProperty& dp){
    this->level = dp.propertyState;
    this->imageID = dp.imageID;
    this->levelIndicatorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::LEVEL_INDICATOR){
        this->levelIndicatorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
    return *this;
}

void LevelIndicator::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
    }
}

String LevelIndicator::toPropertyDescriptor(){
    if(this->valueColor.isValidColor()){
        String desc = this->levelIndicatorDescription;
        desc += ";;";
        desc += this->valueColor.toString();
        return desc;
    }
    else {
        return this->levelIndicatorDescription;
    }
}

void LevelIndicator::split(const String &descriptor)
{
    auto l = descriptor.length();
    bool sec = false;
    if (l >= 9)
    {
        this->levelIndicatorDescription = "";
        String colString;

        unsigned int i = 0;
        unsigned int nextValidIndex = 0;

        while (i < l)
        {
            auto c = descriptor.charAt(i);

            if (c == ';')
            {
                if ((i < (l - 1)) && (descriptor.charAt(i + 1) == ';'))
                {
                    nextValidIndex = i + 2;
                    sec = true;
                }
            }
            if (sec)
            {
                if (i >= nextValidIndex)
                {
                    colString += c;
                }
            }
            else
            {
                this->levelIndicatorDescription += c;
            }
            i++;
        }
        this->valueColor.fromString(colString);
    }
    else
    {
        this->levelIndicatorDescription = descriptor;
    }
}

// Level Indicator Section End
//_______________________________________
//
// Text Display Section Start

TextDisplay::TextDisplay(){}

TextDisplay::TextDisplay(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto td = pLr->getProperty(propertyID);
            *this = td;
        }
        else {
            this->textToDisplay = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

TextDisplay::TextDisplay(const TextDisplay &td)
{
    this->textDisplayID = td.textDisplayID;
    this->textToDisplay = td.textToDisplay;
    this->imageID = td.imageID;
    this->isEnabled = td.isEnabled;
    this->colorRect = td.colorRect;
}

TextDisplay::TextDisplay(const DeviceProperty &dp)
{
    this->textDisplayID = dp.propertyID;
    this->imageID = dp.imageID;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::TEXT_DISPLAY){
        this->textToDisplay = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
}

TextDisplay& TextDisplay::operator=(const TextDisplay& td){
    this->textDisplayID = td.textDisplayID;
    this->textToDisplay = td.textToDisplay;
    this->imageID = td.imageID;
    this->isEnabled = td.isEnabled;
    this->colorRect = td.colorRect;
    return *this;
}

TextDisplay& TextDisplay::operator=(const DeviceProperty& dp){
    this->textDisplayID = dp.propertyID;
    this->imageID = dp.imageID;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::TEXT_DISPLAY){
        this->textToDisplay = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
    return *this;
}

void TextDisplay::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
    }
}

String TextDisplay::toPropertyDescriptor(){
    if(this->colorRect.isValidColor()){
        String desc = this->textToDisplay;
        desc += ";;";
        desc += this->colorRect.toString();
        return desc;
    }
    else {
        return this->textToDisplay;
    }
}

void TextDisplay::split(const String &descriptor)
{
    auto l = descriptor.length();
    bool sec = false;
    if (l >= 9)
    {
        this->textToDisplay = "";
        String colString;

        unsigned int i = 0;
        unsigned int nextValidIndex = 0;

        while (i < l)
        {
            auto c = descriptor.charAt(i);

            if (c == ';')
            {
                if ((i < (l - 1)) && (descriptor.charAt(i + 1) == ';'))
                {
                    nextValidIndex = i + 2;
                    sec = true;
                }
            }
            if (sec)
            {
                if (i >= nextValidIndex)
                {
                    colString += c;
                }
            }
            else
            {
                this->textToDisplay += c;
            }
            i++;
        }
        this->colorRect.fromString(colString);
    }
    else
    {
        this->textToDisplay = descriptor;
    }
}

// Text Display Section End
//_______________________________________
//
// Option Selector Section Start

OptionSelector::OptionSelector(){}

OptionSelector::OptionSelector(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto os = pLr->getProperty(propertyID);
            *this = os;
        }
        else {
            this->optionSelectorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

OptionSelector::OptionSelector(const OptionSelector &os)
{
    this->imageID = os.imageID;
    this->optionList = os.optionList;
    this->optionSelectorDescription = os.optionSelectorDescription;
    this->optionSelectorID = os.optionSelectorID;
    this->selectedIndex = os.selectedIndex;
    this->isEnabled = os.isEnabled;
}

OptionSelector::OptionSelector(const DeviceProperty &dp)
{
    this->imageID = dp.imageID;
    this->optionSelectorID = dp.propertyID;
    this->selectedIndex = dp.propertyState;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::OPTION_SELECTOR){
        this->optionSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
}

void OptionSelector::removeOption(const String& option){
    for(unsigned int i = 0; i < this->optionList.GetCount(); i++){
        if(*this->optionList.getObjectCoreReferenceAt(i) == option){
            this->optionList.RemoveAt(i);
            break;
        }
    }
}

void OptionSelector::removeOption(unsigned int index){
    this->optionList.RemoveAt(index);
}

OptionSelector& OptionSelector::operator=(const OptionSelector& os){
    this->imageID = os.imageID;
    this->optionList = os.optionList;
    this->optionSelectorDescription = os.optionSelectorDescription;
    this->optionSelectorID = os.optionSelectorID;
    this->selectedIndex = os.selectedIndex;
    this->isEnabled = os.isEnabled;
    return *this;
}

OptionSelector& OptionSelector::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    this->optionSelectorID = dp.propertyID;
    this->selectedIndex = dp.propertyState;
    this->isEnabled = dp.isEnabled;
    if(dp.propertyType != PropertyType::OPTION_SELECTOR){
        this->optionSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->split(dp.descriptor);
    }
    return *this;
}

void OptionSelector::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
    }
}

String OptionSelector::toDevicePropertyDescriptor()
{
    String dc = this->optionSelectorDescription;
    for (unsigned int i = 0; i < this->optionList.GetCount(); i++)
    {
        dc += ";;";
        dc += this->optionList.GetAt(i);      
    }
    return dc;
}

void OptionSelector::split(const String &descriptor)
{
    auto l = descriptor.length();
    bool sec = false;
    if (l >= 4)
    {
        this->optionSelectorDescription = "";
        this->optionList.Clear();
        String option;

        unsigned int i = 0;
        unsigned int nextValidIndex = 0;

        while (i < l)
        {
            auto c = descriptor.charAt(i);

            if (c == ';')
            {
                if ((i < (l - 1)) && (descriptor.charAt(i + 1) == ';'))
                {
                    nextValidIndex = i + 2;
                    if (sec)
                    {
                        this->optionList += option;
                        option = "";
                    }
                    else
                    {
                        sec = true;
                    }
                }
            }
            if (sec)
            {
                if (i >= nextValidIndex)
                {
                    option += c;
                }
            }
            else
            {
                this->optionSelectorDescription += c;
            }
            i++;
        }
        if(option != *this->optionList.getObjectCoreReferenceAt(this->optionList.GetCount() - 1)){
            this->optionList.AddItem(option);
        }
    }
    else
    {
        this->optionSelectorDescription = descriptor;
    }
}

// Option Selector Section End
//_______________________________________
//
// RGB Selector Section Start

RGBSelector::RGBSelector(){}

RGBSelector::RGBSelector(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto rs = pLr->getProperty(propertyID);
            *this = rs;
        }
        else {
            this->rgbSelectorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

RGBSelector::RGBSelector(const RGBSelector& rs){
    this->imageID = rs.imageID;
    this->rgbSelectorDescription = rs.rgbSelectorDescription;
    this->rgbSelectorID = rs.rgbSelectorID;
    this->rgbState = rs.rgbState;
    this->isEnabled = rs.isEnabled;
}

RGBSelector::RGBSelector(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::RGB_SELECTOR){
        this->rgbSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->rgbSelectorDescription = dp.descriptor;
        this->rgbState = LaRoomyAppImplementation::GetInstance()->getRGBSelectorState(dp.propertyID);
    }   
    this->rgbSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

RGBSelector& RGBSelector::operator=(const RGBSelector& rs){
    this->imageID = rs.imageID;
    this->rgbSelectorDescription = rs.rgbSelectorDescription;
    this->rgbSelectorID = rs.rgbSelectorID;
    this->rgbState = rs.rgbState;
    this->isEnabled = rs.isEnabled;
    return *this;
}

RGBSelector& RGBSelector::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::RGB_SELECTOR){
        this->rgbSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->rgbSelectorDescription = dp.descriptor;
        this->rgbState = LaRoomyAppImplementation::GetInstance()->getRGBSelectorState(dp.propertyID);
    }   
    this->rgbSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void RGBSelector::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getRGBSelectorState(this->rgbSelectorID) != this->rgbState){
            pLr->updateRGBState(this->rgbSelectorID, this->rgbState);
        }
    }
}

// RGB Selector Section End
//_______________________________________
//
// Extended Level Selector Section Start

ExtendedLevelSelector::ExtendedLevelSelector(){}

ExtendedLevelSelector::ExtendedLevelSelector(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto els = pLr->getProperty(propertyID);
            *this = els;
        }
        else {
            this->extendedLevelSelectorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

ExtendedLevelSelector::ExtendedLevelSelector(const ExtendedLevelSelector& els){
    this->imageID = els.imageID;
    this->extendedLevelSelectorDescription = els.extendedLevelSelectorDescription;
    this->extendedLevelSelectorID = els.extendedLevelSelectorID;
    this->exLevelState = els.exLevelState;
    this->isEnabled = els.isEnabled;
}

ExtendedLevelSelector::ExtendedLevelSelector(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::EX_LEVEL_SELECTOR){
        this->extendedLevelSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->extendedLevelSelectorDescription = dp.descriptor;
        this->exLevelState = LaRoomyAppImplementation::GetInstance()->getExtendedLevelSelectorState(dp.propertyID);
    }
    this->extendedLevelSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

ExtendedLevelSelector& ExtendedLevelSelector::operator=(const ExtendedLevelSelector& els){
    this->imageID = els.imageID;
    this->extendedLevelSelectorDescription = els.extendedLevelSelectorDescription;
    this->extendedLevelSelectorID = els.extendedLevelSelectorID;
    this->exLevelState = els.exLevelState;
    this->isEnabled = els.isEnabled;
    return *this;
}

ExtendedLevelSelector& ExtendedLevelSelector::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::EX_LEVEL_SELECTOR){
        this->extendedLevelSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->extendedLevelSelectorDescription = dp.descriptor;
        this->exLevelState = LaRoomyAppImplementation::GetInstance()->getExtendedLevelSelectorState(dp.propertyID);
    }
    this->extendedLevelSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void ExtendedLevelSelector::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getExtendedLevelSelectorState(this->extendedLevelSelectorID) != this->exLevelState){
            pLr->updateExLevelState(this->extendedLevelSelectorID, this->exLevelState);
        }
    }
}

// Extended Level Selector Section End
//_______________________________________
//
// Time Selector Section Start

TimeSelector::TimeSelector(){}

TimeSelector::TimeSelector(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto ts = pLr->getProperty(propertyID);
            *this = ts;
        }
        else {
            this->timeSelectorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

TimeSelector::TimeSelector(const TimeSelector& ts){
    this->imageID = ts.imageID;
    this->timeSelectorDescription = ts.timeSelectorDescription;
    this->timeSelectorID = ts.timeSelectorID;
    this->timeSelectorState = ts.timeSelectorState;
    this->isEnabled = ts.isEnabled;
}

TimeSelector::TimeSelector(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::TIME_SELECTOR){
        this->timeSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->timeSelectorDescription = dp.descriptor;
        this->timeSelectorState = LaRoomyAppImplementation::GetInstance()->getTimeSelectorState(dp.propertyID);
    }
    this->timeSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;   
}

TimeSelector& TimeSelector::operator=(const TimeSelector& ts){
    this->imageID = ts.imageID;
    this->timeSelectorDescription = ts.timeSelectorDescription;
    this->timeSelectorID = ts.timeSelectorID;
    this->timeSelectorState = ts.timeSelectorState;
    this->isEnabled = ts.isEnabled;
    return *this;
}

TimeSelector& TimeSelector::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::TIME_SELECTOR){
        this->timeSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->timeSelectorDescription = dp.descriptor;
        this->timeSelectorState = LaRoomyAppImplementation::GetInstance()->getTimeSelectorState(dp.propertyID);
    }
    this->timeSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;  
}

void TimeSelector::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getTimeSelectorState(this->timeSelectorID) != this->timeSelectorState){
            pLr->updateTimeSelectorState(this->timeSelectorID, this->timeSelectorState);
        }
    }
}

// Time Selector Section End
//_______________________________________
//
// TimeFrame Selector Section Start

TimeFrameSelector::TimeFrameSelector(){}

TimeFrameSelector::TimeFrameSelector(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto tfs = pLr->getProperty(propertyID);
            *this = tfs;
        }
        else {
            this->timeFrameSelectorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

TimeFrameSelector::TimeFrameSelector(const TimeFrameSelector& tfs){
    this->imageID = tfs.imageID;
    this->timeFrameSelectorDescription = tfs.timeFrameSelectorDescription;
    this->timeFrameSelectorID = tfs.timeFrameSelectorID;
    this->timeFrameSelectorState = tfs.timeFrameSelectorState;
    this->isEnabled = tfs.isEnabled;
}

TimeFrameSelector::TimeFrameSelector(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::TIME_FRAME_SELECTOR){
        this->timeFrameSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->timeFrameSelectorDescription = dp.descriptor;
        this->timeFrameSelectorState = LaRoomyAppImplementation::GetInstance()->getTimeFrameSelectorState(dp.propertyID);
    }
    this->timeFrameSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

TimeFrameSelector& TimeFrameSelector::operator=(const TimeFrameSelector& tfs){
    this->imageID = tfs.imageID;
    this->timeFrameSelectorDescription = tfs.timeFrameSelectorDescription;
    this->timeFrameSelectorID = tfs.timeFrameSelectorID;
    this->timeFrameSelectorState = tfs.timeFrameSelectorState;
    this->isEnabled = tfs.isEnabled;
    return *this;
}

TimeFrameSelector& TimeFrameSelector::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::TIME_FRAME_SELECTOR){
        this->timeFrameSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->timeFrameSelectorDescription = dp.descriptor;
        this->timeFrameSelectorState = LaRoomyAppImplementation::GetInstance()->getTimeFrameSelectorState(dp.propertyID);
    }
    this->timeFrameSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void TimeFrameSelector::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getTimeFrameSelectorState(this->timeFrameSelectorID) != this->timeFrameSelectorState){
            pLr->updateTimeFrameSelectorState(this->timeFrameSelectorID, this->timeFrameSelectorState);
        }
    }
}

// TimeFrame Selector Section End
//_______________________________________
//
// Date Selector Section Start

DateSelector::DateSelector(){}

DateSelector::DateSelector(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto ds = pLr->getProperty(propertyID);
            *this = ds;
        }
        else {
            this->dateSelectorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

DateSelector::DateSelector(const DateSelector& ds){
    this->dateSelectorDescription = ds.dateSelectorDescription;
    this->dateSelectorID = ds.dateSelectorID;
    this->dateSelectorState = ds.dateSelectorState;
    this->imageID = ds.imageID;
    this->isEnabled = ds.isEnabled;
}

DateSelector::DateSelector(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::DATE_SELECTOR){
        this->dateSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->dateSelectorDescription = dp.descriptor;
        this->dateSelectorState = LaRoomyAppImplementation::GetInstance()->getDateSelectorState(dp.propertyID);
    }
    this->dateSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

DateSelector& DateSelector::operator=(const DateSelector& ds){
    this->dateSelectorDescription = ds.dateSelectorDescription;
    this->dateSelectorID = ds.dateSelectorID;
    this->dateSelectorState = ds.dateSelectorState;
    this->imageID = ds.imageID;
    this->isEnabled = ds.isEnabled;
    return *this;
}

DateSelector& DateSelector::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::DATE_SELECTOR){
        this->dateSelectorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->dateSelectorDescription = dp.descriptor;
        this->dateSelectorState = LaRoomyAppImplementation::GetInstance()->getDateSelectorState(dp.propertyID);
    }
    this->dateSelectorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void DateSelector::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getDateSelectorState(this->dateSelectorID) != this->dateSelectorState){
            pLr->updateDateSelectorState(this->dateSelectorID, this->dateSelectorState);
        }
    }
}

// Date Selector Section End
//_______________________________________
//
// UnlockControl Section Start

UnlockControl::UnlockControl(){}

UnlockControl::UnlockControl(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto uc = pLr->getProperty(propertyID);
            *this = uc;
        }
        else {
            this->unlockControlDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

UnlockControl::UnlockControl(const UnlockControl& uc){
    this->imageID = uc.imageID;
    this->unlockControlDescription = uc.unlockControlDescription;
    this->unlockControlID = uc.unlockControlID;
    this->unlockControlState = uc.unlockControlState;
    this->isEnabled = uc.isEnabled;
}

UnlockControl::UnlockControl(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::UNLOCK_CONTROL){
        this->unlockControlDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->unlockControlDescription = dp.descriptor;
        this->unlockControlState = LaRoomyAppImplementation::GetInstance()->getUnlockControlState(dp.propertyID);
    }
    this->unlockControlID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

UnlockControl& UnlockControl::operator=(const UnlockControl& uc){
    this->imageID = uc.imageID;
    this->unlockControlDescription = uc.unlockControlDescription;
    this->unlockControlID = uc.unlockControlID;
    this->unlockControlState = uc.unlockControlState;
    this->isEnabled = uc.isEnabled;
    return *this;
}

UnlockControl& UnlockControl::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::UNLOCK_CONTROL){
        this->unlockControlDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->unlockControlDescription = dp.descriptor;
        this->unlockControlState = LaRoomyAppImplementation::GetInstance()->getUnlockControlState(dp.propertyID);
    }
    this->unlockControlID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void UnlockControl::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getUnlockControlState(this->unlockControlID) != this->unlockControlState){
            pLr->updateUnlockControlState(this->unlockControlID, this->unlockControlState);
        }
    }
}

// UnlockControl Section End
//_______________________________________
//
// NavigatorControl Section Start

NavigatorControl::NavigatorControl(){}

NavigatorControl::NavigatorControl(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto nc = pLr->getProperty(propertyID);
            *this = nc;
        }
        else {
            this->navigatorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

NavigatorControl::NavigatorControl(const NavigatorControl& nc){
    this->imageID = nc.imageID;
    this->navigatorDescription = nc.navigatorDescription;
    this->navigatorID = nc.navigatorID;
    this->navigatorState = nc.navigatorState;
    this->isEnabled = nc.isEnabled;
}

NavigatorControl::NavigatorControl(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::NAVIGATOR){
        this->navigatorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->navigatorDescription = dp.descriptor;
        this->navigatorState = LaRoomyAppImplementation::GetInstance()->getNavigatorState(dp.propertyID);
    }
    this->navigatorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

NavigatorControl& NavigatorControl::operator=(const NavigatorControl& nc){
    this->imageID = nc.imageID;
    this->navigatorDescription = nc.navigatorDescription;
    this->navigatorID = nc.navigatorID;
    this->navigatorState = nc.navigatorState;
    this->isEnabled = nc.isEnabled;
    return *this;
}

NavigatorControl& NavigatorControl::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::NAVIGATOR){
        this->navigatorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->navigatorDescription = dp.descriptor;
        this->navigatorState = LaRoomyAppImplementation::GetInstance()->getNavigatorState(dp.propertyID);
    }
    this->navigatorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void NavigatorControl::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getNavigatorState(this->navigatorID) != this->navigatorState){
            pLr->updateNavigatorState(this->navigatorID, this->navigatorState);
        }
    }
}

// NavigatorControl Section End
//_______________________________________
//
// BarGraph Section Start

BarGraph::BarGraph(){}

BarGraph::BarGraph(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto bg = pLr->getProperty(propertyID);
            *this = bg;
        }
        else {
            this->barGraphDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

BarGraph::BarGraph(const BarGraph& bg){
    this->imageID = bg.imageID;
    this->barGraphDescription = bg.barGraphDescription;
    this->barGraphID = bg.barGraphID;
    this->barGraphState = bg.barGraphState;
    this->isEnabled = bg.isEnabled;
}

BarGraph::BarGraph(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::BAR_GRAPH){
        this->barGraphDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->barGraphDescription = dp.descriptor;
        this->barGraphState = LaRoomyAppImplementation::GetInstance()->getBarGraphState(dp.propertyID);
    }
    this->barGraphID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

BarGraph& BarGraph::operator=(const BarGraph& bg){
    this->imageID = bg.imageID;
    this->barGraphDescription = bg.barGraphDescription;
    this->barGraphID = bg.barGraphID;
    this->barGraphState = bg.barGraphState;
    this->isEnabled = bg.isEnabled;
    return *this;
}

BarGraph& BarGraph::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::BAR_GRAPH){
        this->barGraphDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->barGraphDescription = dp.descriptor;
        this->barGraphState = LaRoomyAppImplementation::GetInstance()->getBarGraphState(dp.propertyID);
    }
    this->barGraphID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void BarGraph::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getBarGraphState(this->barGraphID) != this->barGraphState){
            pLr->updateBarGraphState(this->barGraphID, this->barGraphState);
        }
    }
}

// BarGraph Section End
//_______________________________________
//
// LineGraph Section Start

LineGraph::LineGraph(){}

LineGraph::LineGraph(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto lg = pLr->getProperty(propertyID);
            *this = lg;
        }
        else {
            this->lineGraphDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

LineGraph::LineGraph(const LineGraph& lg){
    this->imageID = lg.imageID;
    this->lineGraphDescription = lg.lineGraphDescription;
    this->lineGraphID = lg.lineGraphID;
    this->lineGraphState = lg.lineGraphState;
    this->isEnabled = lg.isEnabled;
}

LineGraph::LineGraph(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::LINE_GRAPH){
        this->lineGraphDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->lineGraphDescription = dp.descriptor;
        this->lineGraphState = LaRoomyAppImplementation::GetInstance()->getLineGraphState(dp.propertyID);
    }
    this->lineGraphID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

LineGraph& LineGraph::operator=(const LineGraph& lg){
    this->imageID = lg.imageID;
    this->lineGraphDescription = lg.lineGraphDescription;
    this->lineGraphID = lg.lineGraphID;
    this->lineGraphState = lg.lineGraphState;
    this->isEnabled = lg.isEnabled;
    return *this;
}

LineGraph& LineGraph::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::LINE_GRAPH){
        this->lineGraphDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->lineGraphDescription = dp.descriptor;
        this->lineGraphState = LaRoomyAppImplementation::GetInstance()->getLineGraphState(dp.propertyID);
    }
    this->lineGraphID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void LineGraph::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getLineGraphState(this->lineGraphID) != this->lineGraphState){
            pLr->updateLineGraphState(this->lineGraphID, this->lineGraphState);
        }
    }
}

// LineGraph Section End
//_______________________________________
//
// StringInterrogator Section Start

StringInterrogator::StringInterrogator(){}

StringInterrogator::StringInterrogator(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto si = pLr->getProperty(propertyID);
            *this = si;
        }
        else {
            this->stringInterrogatorDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

StringInterrogator::StringInterrogator(const StringInterrogator& si){
    this->imageID = si.imageID;
    this->stringInterrogatorDescription = si.stringInterrogatorDescription;
    this->stringInterrogatorID = si.stringInterrogatorID;
    this->stringInterrogatorState = si.stringInterrogatorState;
    this->isEnabled = si.isEnabled;
}

StringInterrogator::StringInterrogator(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::STRING_INTERROGATOR){
        this->stringInterrogatorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->stringInterrogatorDescription = dp.descriptor;
        this->stringInterrogatorState = LaRoomyAppImplementation::GetInstance()->getStringInterrogatorState(dp.propertyID);
    }
    this->stringInterrogatorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

StringInterrogator& StringInterrogator::operator=(const StringInterrogator& si){
    this->imageID = si.imageID;
    this->stringInterrogatorDescription = si.stringInterrogatorDescription;
    this->stringInterrogatorID = si.stringInterrogatorID;
    this->stringInterrogatorState = si.stringInterrogatorState;
    this->isEnabled = si.isEnabled;
    return *this;
}

StringInterrogator& StringInterrogator::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::STRING_INTERROGATOR){
        this->stringInterrogatorDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->stringInterrogatorDescription = dp.descriptor;
        this->stringInterrogatorState = LaRoomyAppImplementation::GetInstance()->getStringInterrogatorState(dp.propertyID);
    }
    this->stringInterrogatorID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void StringInterrogator::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getStringInterrogatorState(this->stringInterrogatorID) != this->stringInterrogatorState){
            pLr->updateStringInterrogatorState(this->stringInterrogatorID, this->stringInterrogatorState);
        }
    }
}

// StringInterrogator Section End
//_______________________________________
//
// TextListPresenter Section Start

TextListPresenter::TextListPresenter(){}

TextListPresenter::TextListPresenter(cID propertyID){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        if(pLr->checkIfPropertyExist(propertyID)){
            auto tlp = pLr->getProperty(propertyID);
            *this = tlp;
        }
        else {
            this->textListPresenterDescription = PINIT_PROP_NOT_FOUND_ERROR;
        }
    }
}

TextListPresenter::TextListPresenter(const TextListPresenter& tlp){
    this->textListPresenterDescription = tlp.textListPresenterDescription;
    this->imageID = tlp.imageID;
    this->textListPresenterID = tlp.textListPresenterID;
    this->textListPresenterState = tlp.textListPresenterState;
    this->isEnabled = tlp.isEnabled;
}

TextListPresenter::TextListPresenter(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::TEXT_LIST_PRESENTER){
        this->textListPresenterDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->textListPresenterDescription = dp.descriptor;
        this->textListPresenterState = LaRoomyAppImplementation::GetInstance()->getTextListPresenterState(dp.propertyID);
    }
    this->textListPresenterID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
}

void TextListPresenter::AddTextListPresenterElement(TextListPresenterElementType type, String elementText){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->addTextListPresenterElement(this->textListPresenterID, type, elementText);
    }
}

void TextListPresenter::ClearTextListPresenterContent(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->clearTextListPresenterContent(this->textListPresenterID);
    }
}

TextListPresenter& TextListPresenter::operator=(const TextListPresenter& tlp){
    this->textListPresenterDescription = tlp.textListPresenterDescription;
    this->imageID = tlp.imageID;
    this->textListPresenterID = tlp.textListPresenterID;
    this->textListPresenterState = tlp.textListPresenterState;
    this->isEnabled = tlp.isEnabled;
    return *this;
}

TextListPresenter& TextListPresenter::operator=(const DeviceProperty& dp){
    this->imageID = dp.imageID;
    if(dp.propertyType != PropertyType::TEXT_LIST_PRESENTER){
        this->textListPresenterDescription = PTYPE_CAST_ERROR;
    }
    else {
        this->textListPresenterDescription = dp.descriptor;
        this->textListPresenterState = LaRoomyAppImplementation::GetInstance()->getTextListPresenterState(dp.propertyID);
    }
    this->textListPresenterID = dp.propertyID;
    this->isEnabled = dp.isEnabled;
    return *this;
}

void TextListPresenter::update(){
    auto pLr = LaRoomyAppImplementation::GetInstance();
    if(pLr != nullptr){
        pLr->updateDeviceProperty(*this);
        if(pLr->getTextListPresenterState(this->textListPresenterID) != this->textListPresenterState){
            pLr->updateTextListPresenterState(this->textListPresenterID, this->textListPresenterState);
        }
    }
}
