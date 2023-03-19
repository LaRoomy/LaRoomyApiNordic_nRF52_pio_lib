#include "nRF_FlashStorage.h"

FlashStorageManager* FlashStorageManager::GetInstance()
{
    if (flashStorageManagerInstanceCreated)
    {
        return fsmInstance;
    }
    else
    {
        fsmInstance = new FlashStorageManager();
        if (fsmInstance != nullptr)
        {
            flashStorageManagerInstanceCreated = true;
        }
        return fsmInstance;
    }
}

FlashStorageManager::FlashStorageManager() {}

FlashStorageManager::~FlashStorageManager()
{
    flashStorageManagerInstanceCreated = false;
    fsmInstance = nullptr;
}

bool FlashStorageManager::flashStorageManagerInstanceCreated = false;
FlashStorageManager *FlashStorageManager::fsmInstance = nullptr;

bool FlashStorageManager::Init()
{
    auto res = fds_register(FlashStorageManager::fdsEventHandler);
    if (res == FDS_SUCCESS)
    {
        res = fds_init();
        if (res == FDS_SUCCESS)
        {
            while (!this->operationComplete)
                ;
            if (this->lastResult == FDS_SUCCESS)
            {
                return true;
            }
        }
    }
    return false;
}

bool FlashStorageManager::write(void *data, unsigned int size)
{
    fds_record_t record;
    fds_record_desc_t recordDesc;
    fds_find_token_t findToken;
    ret_code_t result = FDS_SUCCESS;

    uint8_t __attribute__((aligned(4))) buffer[48];

    // zero token
    memset(&findToken, 0x0, sizeof(fds_find_token_t));

    unsigned int const len = size < sizeof(buffer) ? size : sizeof(buffer);
    memset(buffer, 0x0, sizeof(buffer));
    memcpy(buffer, data, len);

    record.file_id = this->fileKey;
    record.key = this->recordKey;
    record.data.p_data = buffer;
    record.data.length_words = (len + 3) / 4;

    result = fds_record_find(this->fileKey, this->recordKey, &recordDesc, &findToken);
    if (result == FDS_SUCCESS)
    {
        result = fds_record_update(&recordDesc, &record);
    }
    else
    {
        result = fds_record_write(&recordDesc, &record);
    }

    // wait for callback
    while (!this->operationComplete)
        ;

    return (this->lastResult == FDS_SUCCESS) ? true : false;
}

bool FlashStorageManager::read(void *data, unsigned int size)
{
    fds_flash_record_t flashRecord;
    fds_record_desc_t recordDesc;
    fds_find_token_t findToken;
    ret_code_t result = FDS_ERR_NOT_FOUND;

    // zero token
    memset(&findToken, 0x0, sizeof(fds_find_token_t));

    while (fds_record_find(this->fileKey, this->recordKey, &recordDesc, &findToken) == FDS_SUCCESS)
    {
        result = fds_record_open(&recordDesc, &flashRecord);
        if (result != FDS_SUCCESS)
        {
            this->lastResult = result;
            return false;
        }

        memcpy(data, flashRecord.p_data, size);

        result = fds_record_close(&recordDesc);
    }
    this->lastResult = result;
    return (result == FDS_SUCCESS) ? true : false;
}

bool FlashStorageManager::selectFileKey(uint16_t key)
{
    if (key >= 0x0001 && key < 0xbfff)
    {
        this->fileKey = key;
        return true;
    }
    else
    {
        this->lastResult = FDS_ERR_INVALID_ARG;
        return false;
    }
}

bool FlashStorageManager::selectDataKey(uint16_t key)
{
    if (key >= 0x0001 && key < 0xbfff)
    {
        this->recordKey = key;
        return true;
    }
    else
    {
        this->lastResult = FDS_ERR_INVALID_ARG;
        return false;
    }
}

bool FlashStorageManager::deleteData()
{
    fds_record_desc_t recordDesc;
    fds_find_token_t findToken;
    ret_code_t result = FDS_ERR_NOT_FOUND;

    memset(&findToken, 0x0, sizeof(fds_find_token_t));

    while (fds_record_find(this->fileKey, this->recordKey, &recordDesc, &findToken) == FDS_SUCCESS)
    {
        this->operationComplete = false;

        result = fds_record_delete(&recordDesc);
        if (result != FDS_SUCCESS)
        {
            this->operationComplete = true;
            this->lastResult = result;
            return false;
        }

        while (!this->operationComplete)
            ;
    }
    return (this->lastResult == FDS_SUCCESS) ? this->cleanUp() : false;
}

bool FlashStorageManager::deleteFile()
{
    ret_code_t result = FDS_ERR_NOT_FOUND;
    this->operationComplete = false;

    result = fds_file_delete(this->fileKey);
    if (result != FDS_SUCCESS)
    {
        this->operationComplete = true;
        this->lastResult = result;
        return false;
    }

    while (!this->operationComplete)
        ;

    return (this->lastResult == FDS_SUCCESS) ? this->cleanUp() : false;
}

bool FlashStorageManager::cleanUp()
{
    this->operationComplete = false;

    auto result = fds_gc();
    if(result != FDS_SUCCESS){
        this->operationComplete = true;
        this->lastResult = result;
        return false;
    }

    while(!this->operationComplete);

    return (this->lastResult == FDS_SUCCESS) ? true : false;
}

void FlashStorageManager::fdsEventHandler(fds_evt_t const *fdsEvent)
{        
    if (fsmInstance != nullptr)
    {
        // mark async operation as complete
        fsmInstance->operationComplete = true;
        // save the result
        fsmInstance->lastResult = fdsEvent->result;
    }

    // switch (fdsEvent->id)
    // {
    // case FDS_EVT_INIT:
    //     break;
    // case FDS_EVT_WRITE:
    //     break;
    // case FDS_EVT_UPDATE:
    //     break;
    // case FDS_EVT_DEL_RECORD:
    //     break;
    // case FDS_EVT_DEL_FILE:
    //     break;
    // case FDS_EVT_GC:
    //     break;
    // default:
    //     break;
    // }
}