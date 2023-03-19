#ifndef UNLOCKCONTROL_PIN_STORAGE_CONTROLLER_H
#define UNLOCKCONTROL_PIN_STORAGE_CONTROLLER_H

#include "nRF_FlashStorage.h"

typedef struct _UNLOCKCONTROL_STORAGE_DATA {
    uint8_t isValid;
    char ucPin[11];
}UNLOCKCONTROL_STORAGE_DATA, *PUNLOCKCONTROL_STORAGE_DATA;

class UnlockControlPinStorageController {
public:
    ~UnlockControlPinStorageController();

    /**
     * @brief Get the Singleton Instance object of the UnlockControlPinStorageController
     * 
     * @return UnlockControlPinStorageController* 
     */
    static UnlockControlPinStorageController* GetInstance();    

    /**
     * @brief This methods loads the pin from flash storage, or returns the default pin '12345' if no pin was saved before.
     * 
     * @return String! - The Pin
     */
    String loadPin();

    /**
     * @brief This method saves the pin to flash storage.
     * 
     * @param pin The pin to save
     * @return bool - indicates if the operation succeeded or not
     */
    bool savePin(const String& pin);

private:
    static bool unlockControlPinStorageControllerInstanceCreated;
    static UnlockControlPinStorageController* ucpInstance;

    UnlockControlPinStorageController();
};

/**
 * @brief Access the Pin-Storage helper API. This class takes responibility for the flash storage of the pin.
 *          Please note that this is a singleton instance.
 * 
 */
#define PinStorageController    (*UnlockControlPinStorageController::GetInstance())

#endif // UNLOCKCONTROL_PIN_STORAGE_CONTROLLER_H