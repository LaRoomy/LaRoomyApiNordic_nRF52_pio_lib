#include "UnlockControlPinStorageController.h"

const uint16_t ULCSD_FILE_KEY = 0xbffa;
const uint16_t ULCSD_DATA_KEY = 0xbffb;

bool UnlockControlPinStorageController::unlockControlPinStorageControllerInstanceCreated = false;
UnlockControlPinStorageController* UnlockControlPinStorageController::ucpInstance = nullptr;

UnlockControlPinStorageController::UnlockControlPinStorageController()
{}

UnlockControlPinStorageController::~UnlockControlPinStorageController()
{
    unlockControlPinStorageControllerInstanceCreated = false;
}

UnlockControlPinStorageController* UnlockControlPinStorageController::GetInstance()
{
    if(unlockControlPinStorageControllerInstanceCreated)
    {
        return ucpInstance;
    }
    else
    {
        ucpInstance = new UnlockControlPinStorageController();
        if(ucpInstance != nullptr)
        {
            unlockControlPinStorageControllerInstanceCreated = true;
        }
        return ucpInstance;
    }
}

String UnlockControlPinStorageController::loadPin(){
    // default value
    String pin("12345");

    // load data
    UNLOCKCONTROL_STORAGE_DATA ucData;
    memset(&ucData, 0x0, sizeof(UNLOCKCONTROL_STORAGE_DATA));

    auto fStorage = FlashStorageManager::GetInstance();
    if(fStorage != nullptr)
    {
        if(fStorage->Init())
        {
            fStorage->selectFileKey(ULCSD_FILE_KEY);
            fStorage->selectDataKey(ULCSD_DATA_KEY);

            if(fStorage->read(&ucData, sizeof(UNLOCKCONTROL_STORAGE_DATA)))
            {
                // validate data
                if(ucData.isValid == 1)
                {
                    ucData.ucPin[10] = '\0';
                    pin = ucData.ucPin;
                }
            }
        }
    }
    return pin;
}

bool UnlockControlPinStorageController::savePin(const String& pin){

    // validate pin length
    if(pin.length() > 0 && pin.length() < 11){
        
        // set data to save
        UNLOCKCONTROL_STORAGE_DATA ucData;
        ucData.isValid = 1;
        strcpy(ucData.ucPin, pin.c_str());

        // save
        auto fStorage = FlashStorageManager::GetInstance();
        if(fStorage != nullptr)
        {
            if(fStorage->Init())
            {
                fStorage->selectFileKey(ULCSD_FILE_KEY);
                fStorage->selectDataKey(ULCSD_DATA_KEY);
                
                if(fStorage->write(&ucData, sizeof(UNLOCKCONTROL_STORAGE_DATA)))
                {
                    return true;
                }
            }
        }
    }
    return false;
}