#include "BindingController.h"

const uint16_t BCSFS_FILEKEY = 0xbffc;
const uint16_t BCSFS_DATAKEY = 0xbffd;

BindingController::BindingController(){
    this->init();
}

BindingController::~BindingController(){
    bindingControllerInstanceCreated = false;
}

BindingController* BindingController::GetInstance(){
    if(bindingControllerInstanceCreated){
        return bcInstance;
    }
    else {
        bcInstance = new BindingController();
        if(bcInstance != nullptr){
            bindingControllerInstanceCreated = true;
        }
        return bcInstance;
    }
}

bool BindingController::bindingControllerInstanceCreated = false;
BindingController* BindingController::bcInstance = nullptr;

void BindingController::init(){

    this->bKey = "";

    // read data
    BINDING_STORAGE_DATA bData;
    memset(&bData, 0x0, sizeof(BINDING_STORAGE_DATA));

    auto fStorage = FlashStorageManager::GetInstance();
    if(fStorage != nullptr)
    {
        if(fStorage->Init())
        {
            fStorage->selectFileKey(BCSFS_FILEKEY);
            fStorage->selectDataKey(BCSFS_DATAKEY);

            if(fStorage->read(&bData, sizeof(BINDING_STORAGE_DATA)))
            {
                // validate data
                if(bData.isValid == 1)
                {
                    this->bRequired = (bData.isRequired == 1) ? true : false;
                    bData.bndKey[10] = '\0';
                    this->bKey = bData.bndKey;       
                }
            }
        }
    }
}

BindingResponseType BindingController::handleBindingTransmission(BindingTransmissionTypes bType, const String& key){

    switch (bType)
    {
        case BindingTransmissionTypes::B_AUTH_REQUEST:
            if(this->bKey == key){
                // the key is valid
                return BindingResponseType::BINDING_AUTHENTICATION_SUCCESS;               
            }
            else {
                // key is invalid, auth fail
                return BindingResponseType::BINDING_AUTHENTICATION_FAIL_WRONG_KEY;
            }
        case BindingTransmissionTypes::B_ENABLE:
            // save the key + set required to true
            if(key.length() > 0 && key.length() < 11){
                this->bKey = key;
                this->bRequired = true;

                // fill data struct
                BINDING_STORAGE_DATA bData;
                bData.isRequired = 1;
                bData.isValid = 1;
                strcpy(bData.bndKey, key.c_str());

                // write data to flash
                auto fStorage = FlashStorageManager::GetInstance();
                if(fStorage != nullptr)
                {
                    if(fStorage->Init())
                    {
                        fStorage->selectFileKey(BCSFS_FILEKEY);
                        fStorage->selectDataKey(BCSFS_DATAKEY);

                        fStorage->write(&bData, sizeof(BINDING_STORAGE_DATA));
                    }
                }

                // report success
                return BindingResponseType::BINDING_ENABLE_SUCCESS;
            }
            else {
                return BindingResponseType::BINDING_ERROR_UNKNOWN;
            }
        case BindingTransmissionTypes::B_RELEASE:
        {
            // delete key + set required to false
            this->bKey = "";
            this->bRequired = false;

            // fill data struct
            BINDING_STORAGE_DATA bData;
            bData.isRequired = 0;
            bData.isValid = 1;
            for(unsigned int i = 0; i < 11; i++){
                bData.bndKey[i] = '\0';
            }

            // overwrite flash
            auto fStorage = FlashStorageManager::GetInstance();
            if(fStorage != nullptr)
            {
                if(fStorage->Init())
                {
                    fStorage->selectFileKey(BCSFS_FILEKEY);
                    fStorage->selectDataKey(BCSFS_DATAKEY);

                    fStorage->write(&bData, sizeof(BINDING_STORAGE_DATA));
                }
            }

            // report release success
        }
            return BindingResponseType::BINDING_RELEASE_SUCCESS;
        default:
            return BindingResponseType::BINDING_ERROR_UNKNOWN_REQUEST;
    }
}