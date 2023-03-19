#include "LaRoomyApi_nRF52.h"

RGBControl::RGBControl(pin_size_t RedPin, pin_size_t GreenPin, pin_size_t BluePin)
    : redPin(RedPin), greenPin(GreenPin), bluePin(BluePin)
{}

RGBControl::~RGBControl(){
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
}

void RGBControl::begin()
{
    // config pins
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);

    // create default color selection
    this->colorSelection.AddItem(Colors::Red);
    this->colorSelection.AddItem(Colors::Green);
    this->colorSelection.AddItem(Colors::Blue);
    this->colorSelection.AddItem(Colors::Magenta);
    this->colorSelection.AddItem(Colors::Cyan);
    this->colorSelection.AddItem(Colors::Yellow);
    this->colorSelection.AddItem(Colors::Pink);
    this->colorSelection.AddItem(Colors::Orange);
    this->colorSelection.AddItem(Colors::Purple);
    this->colorSelection.AddItem(Colors::White);

    // init timer values
    this->fadeTimer = millis();
    this->progTimer = millis();

    // mark as begun
    this->hasBegun = true;
}

void RGBControl::end(){
    this->hasBegun = false;
    this->tProgram = RGBColorTransitionProgram::RCTP_NO_TRANSITION;
    this->redValue = 0;
    this->greenValue = 0;
    this->blueValue = 0;
    this->currentRedValue = 0;
    this->currentGreenValue = 0;
    this->currentBlueValue = 0;
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
}

void RGBControl::off(){
    if(this->tProgram != RGBColorTransitionProgram::RCTP_NO_TRANSITION){
        this->tProgram = RGBColorTransitionProgram::RCTP_NO_TRANSITION;
        this->fadeTimeOut = this->tmSaver;
    }
    this->redValue = 0;
    this->greenValue = 0;
    this->blueValue = 0;
}

void RGBControl::applyStateChange(const RGBSelectorState& state){
    if(state.isOn){
        if(state.colorTransitionProgram != RGBColorTransitionProgram::RCTP_NO_TRANSITION){
            this->changeRGBProgram(state.colorTransitionProgram, state.transitionType);
        }
        else {
            this->changeRGBColor(state.redValue, state.greenValue, state.blueValue, state.transitionType);
        }
    }
    else {
        this->off();
    }
}

void RGBControl::changeRGBColor(uint8_t red, uint8_t green, uint8_t blue, RGBTransitionType tType){

    if(this->tProgram != RGBColorTransitionProgram::RCTP_NO_TRANSITION){
        this->fadeTimeOut = this->tmSaver;
        this->tProgram = RGBColorTransitionProgram::RCTP_NO_TRANSITION;
    }
    this->redValue = red;
    this->greenValue = green;
    this->blueValue = blue;
    this->hardTrans = (tType == RGBTransitionType::HARD_TRANSITION) ? true : false;
}

void RGBControl::changeRGBProgram(RGBColorTransitionProgram program, RGBTransitionType tType){

    this->hardTrans = tType == RGBTransitionType::HARD_TRANSITION ? true : false;

    if((program == RGBColorTransitionProgram::RCTP_NO_TRANSITION) && (this->tProgram != RGBColorTransitionProgram::RCTP_NO_TRANSITION)){

        // disable program (only if it wasn't disabled!)
        this->fadeTimeOut = this->tmSaver;
        this->tProgram = program;


    } else {
        if(program != RGBColorTransitionProgram::RCTP_NO_TRANSITION){
            // enable program
            if(this->tProgram == RGBColorTransitionProgram::RCTP_NO_TRANSITION){
                // save the old timeOut value only if there is no program running
                this->tmSaver = this->fadeTimeOut;
            }
            // if the current condition is off, set the first color immediately
            if((this->redValue == 0 && this->greenValue == 0 && this->blueValue == 0)
               ||(this->tProgram == RGBColorTransitionProgram::RCTP_NO_TRANSITION)) {
                if(this->colorSelection.GetCount() > 0){
                    this->currentColorIndex = 0;
                    this->redValue = this->colorSelection.getObjectCoreReferenceAt(0)->redPart;
                    this->greenValue = this->colorSelection.getObjectCoreReferenceAt(0)->greenPart;
                    this->blueValue = this->colorSelection.getObjectCoreReferenceAt(0)->bluePart;
                }
            }

            this->tProgram = program;

            switch (program)
            {
            case RGBColorTransitionProgram::RCTP_SLOW_TRANSITION:
                this->progTimeOut = 8000;
                this->fadeTimeOut = 22;
                break;
            case RGBColorTransitionProgram::RCTP_SEMISLOW_TRANSITION:
                this->progTimeOut = 6000;
                this->fadeTimeOut = 14;
                break;
            case RGBColorTransitionProgram::RCTP_MEDIUM_TRANSITION:
                this->progTimeOut = 4000;
                this->fadeTimeOut = 8;
                break;
            case RGBColorTransitionProgram::RCTP_SEMIFAST_TRANSITION:
                this->progTimeOut = 2000;
                this->fadeTimeOut = 4;
                break;
            case RGBColorTransitionProgram::RCTP_FAST_TRANSITION:
                this->progTimeOut = 1000;
                this->fadeTimeOut = 1;
                break;
            default:
                break;
            }
        }
    }
}

void RGBControl::setCustomColorSelection(ColorCollection& colorSel){
    // pause the program while exchanging the collection to avoid an access violation
    if(colorSel.GetCount() > 0){
        auto oldProg = this->tProgram;
        this->tProgram = RGBColorTransitionProgram::RCTP_NO_TRANSITION;
        this->currentColorIndex = 0;
        this->colorSelection = colorSel;
        this->tProgram = oldProg;
    }
}


void RGBControl::onLoop(){

    if(!hasBegun){
        return;
    }

    // handle program action
    if(millis() > (unsigned long)(this->progTimer + this->progTimeOut)){

        // reset timer reference
        this->progTimer = millis();

        // only execute if a program is selcted
        if(this->tProgram != RGBColorTransitionProgram::RCTP_NO_TRANSITION){

            // increase the color index
            this->currentColorIndex++;

            // reset the index if it is above scope
            if(this->currentColorIndex >= this->colorSelection.GetCount()){
                this->currentColorIndex = 0;
            }

            // make sure the color collection has content
            if(this->colorSelection.GetCount() > 0){
                this->redValue = this->colorSelection.getObjectCoreReferenceAt(this->currentColorIndex)->redPart;
                this->greenValue = this->colorSelection.getObjectCoreReferenceAt(this->currentColorIndex)->greenPart;
                this->blueValue = this->colorSelection.getObjectCoreReferenceAt(this->currentColorIndex)->bluePart;
            }
        }
    }


    // handle fading action
    if(millis() > (unsigned long)(this->fadeTimer + this->fadeTimeOut)){
        // reset timer reference
        this->fadeTimer = millis();

        // check if the red value must be changed
        if(this->redValue != this->currentRedValue){
            // if hard transition is requested, set the color direct
            if(this->hardTrans){
                this->currentRedValue = this->redValue;
                analogWrite(this->redPin, this->currentRedValue);
            }
            else {
                // no hard transition -> fade
                if(this->redValue > this->currentRedValue){
                    this->currentRedValue++;
                }
                else {
                    this->currentRedValue--;
                }
                analogWrite(this->redPin, this->currentRedValue);
            }
        }

        // check if the green value must be changed
        if(this->greenValue != this->currentGreenValue){
            // if hard transition is requested, set the color direct
            if(this->hardTrans){
                this->currentGreenValue = this->greenValue;
                analogWrite(greenPin, currentGreenValue);
            }
            else {
                // no hard transition -> fade
                if(this->greenValue > this->currentGreenValue){
                    this->currentGreenValue++;
                }
                else {
                    this->currentGreenValue--;
                }
                analogWrite(greenPin, currentGreenValue);
            }
        }

        // check if the blue value must be changed
        if(this->blueValue != this->currentBlueValue){
            // if hard transition is requested, set the color direct
            if(this->hardTrans){
                this->currentBlueValue = this->blueValue;
                analogWrite(this->bluePin, this->currentBlueValue);
            }
            else {
                // no hard transition -> fade
                if(this->blueValue > this->currentBlueValue){
                    this->currentBlueValue++;
                }
                else {
                    this->currentBlueValue--;
                }
                analogWrite(this->bluePin, this->currentBlueValue);
            }
        }
    }
}
