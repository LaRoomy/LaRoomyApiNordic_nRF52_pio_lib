#ifndef NRF_FLASH_STORAGE
#define NRF_FLASH_STORAGE

#include <Arduino.h>
#include <fds.h>

class FlashStorageManager
{
public:
    static FlashStorageManager *GetInstance();

    ~FlashStorageManager();

    void Release()
    {
        delete this;
    }

    bool Init();
    bool write(void *data, unsigned int size);
    bool read(void *data, unsigned int size);
    bool selectFileKey(uint16_t key);
    bool selectDataKey(uint16_t key);
    bool deleteData();
    bool deleteFile();
    bool cleanUp();

    ret_code_t getLastResult()
    {
        return this->lastResult;
    }

private:
    uint16_t fileKey = 0x0;   // invalid file key
    uint16_t recordKey = 0x0; // invalid record key
    bool operationComplete = false;
    ret_code_t lastResult = FDS_SUCCESS;
    static bool flashStorageManagerInstanceCreated;
    static FlashStorageManager *fsmInstance;

    FlashStorageManager();
    static void fdsEventHandler(fds_evt_t const *fdsEvent);
};

#endif // NRF_FLASH_STORAGE