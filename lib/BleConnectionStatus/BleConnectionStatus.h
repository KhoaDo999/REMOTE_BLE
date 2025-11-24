#ifndef BLECONNECTIONSTATUS_H
#define BLECONNECTIONSTATUS_H

#include <BLEServer.h>
#include <BLECharacteristic.h>

class BleConnectionStatus : public BLEServerCallbacks
{
public:
    BleConnectionStatus();

    bool connected = false;
    BLECharacteristic *inputKeyboard = nullptr;

    void onConnect(BLEServer *pServer) override;
    void onDisconnect(BLEServer *pServer) override;

    //  Thêm hàm này
    void setInputKeyboard(BLECharacteristic *characteristic)
    {
        inputKeyboard = characteristic;
    }
};

#endif
