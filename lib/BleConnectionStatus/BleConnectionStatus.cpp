#include "BleConnectionStatus.h"
#include <BLE2902.h>


BleConnectionStatus::BleConnectionStatus()
{
}

void BleConnectionStatus::onConnect(BLEServer *pServer)
{
    this->connected = true; // đánh dâu đã knoi
    if (this->inputKeyboard != nullptr)
    {
        BLE2902 *desc = (BLE2902 *)this->inputKeyboard->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));//UUID 0x2902 là Client Characteristic Configuration Descriptor (CCCD).
        if (desc != nullptr)// cccd là cách chuẩn của BLE để client nhận thông báo khi characteristic thay đổi.
        {
            desc->setNotifications(true);
        }
    }
}

void BleConnectionStatus::onDisconnect(BLEServer *pServer)
{
    this->connected = false;
    if (this->inputKeyboard != nullptr)
    {
        BLE2902 *desc = (BLE2902 *)this->inputKeyboard->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        if (desc != nullptr)
        {
            desc->setNotifications(false);
        }
    }
    
}