#ifndef BINDING_CONTROLLER_H
#define BINDING_CONTROLLER_H

#include "nRF_FlashStorage.h"
#include "constDef.h"

typedef struct _BINDING_STORAGE_DATA {
    uint8_t isValid;
    uint8_t isRequired;
    char bndKey[11];
}BINDING_STORAGE_DATA, *PBINDING_STORAGE_DATA;

/**
 * @brief Controller Class for the authentication, enable and release -process of the device-binding.
 *          This class takes also responsibility for storing the underlying data to flash.
 */
class BindingController {
public:
    ~BindingController();

    /**
     * @brief Get the Singleton-Instance object of the BindingController class
     * 
     * @return BindingController* 
     */
    static BindingController* GetInstance();    

    /**
     * @brief Binding control handler.
     * This method must be implemented as the response to the callback method 'onBindingTransmissionReceived' in the class derived from 'ILaroomyAppCallback'.
     * It takes the formal parameter from this callback-method as input parameter, processes the storing of the binding data and returns the appropriate result (BindingResponseType).
     * 
     * @param bType The BindingTransmissionType
     * @param key The binding key
     * @return BindingResponseType 
     */
    BindingResponseType handleBindingTransmission(BindingTransmissionTypes bType, const String& key);

    /**
     * @brief Get the reqired status of the device binding. The return value must be considered as invalid before Init() was called.
     * 
     * @return Boolean - The required status
     */
    inline bool isBindingRequired(){
        return this->bRequired;
    }

private:
    bool bRequired = false;
    String bKey;

    static bool bindingControllerInstanceCreated;
    static BindingController* bcInstance;    

    BindingController();

    void init();
};

/**
 * @brief Access the binding controller helper API for the authentication, enable and release -process of the device-binding.
 *        This class takes also responsibility for storing the underlying data to flash.
 * 
 */
#define DeviceBindingController (*BindingController::GetInstance())

#endif