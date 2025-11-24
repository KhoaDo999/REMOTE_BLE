
#ifndef SECURE_BLE_KEYBOARD_H
#define SECURE_BLE_KEYBOARD_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "ButtonFSM.h"
#include "AES128Encryptor.h"
#include "BleConnectionStatus.h"

// Giao tiếp và thao tác dữ liệu
#define UUID_SERVICE_DOOR "12345678-1234-1234-1234-123456789abc"
#define UUID_CHAR_COMMAND "abcdefab-cdef-abcd-efab-cdefabcdef12"

class SecureBleKeyboard
{
public:
    SecureBleKeyboard(std::string deviceName = "Secure BLE Keyboard",
                      std::string deviceManufbatteryLevelacturer = "ESP32",
                      uint8_t batteryLevel = 100);
    ~SecureBleKeyboard();

    void begin(const unsigned char *aesKey);
    void update();
    void handleButtonPress(bool pressed);
    void handleAck(bool received);
    bool isConnected() const;
    void setConnected(bool status);
    void startAdvertising();
    void stopAdvertising();
    void setSendingFlag(bool value) { isSending = value; }
    bool getSendingFlag() const { return isSending; }
    ButtonFSM *getFSM() { return fsm; }

private:
    std::string deviceName;
    std::string deviceManufacturer;
    uint8_t batteryLevel;
    uint16_t connId = 0xFFFF;
    bool bleStarted;
    bool creatingServer;

    AES128Encryptor *encryptor;
    ButtonFSM *fsm;
    BleConnectionStatus *connectionStatus;

    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCommandChar;
    bool isSending = false; // mới thêm 5/11
    static void taskServer(void *pvParameter);
    void sendEncryptedCommand(const String &command);
    friend class CommandCallback;
    friend class MyServerCallbacks;
};

// Callbacks
// class CommandCallback : public BLECharacteristicCallbacks
// {
// private:
//     SecureBleKeyboard *bleKeyboardInstance;

// public:
//     CommandCallback(SecureBleKeyboard *instance) : bleKeyboardInstance(instance) {}
//     void onWrite(BLECharacteristic *pCharacteristic) override
//     {
//         std::string rxValue = pCharacteristic->getValue();
//         if (!rxValue.empty())
//         {
//             Serial.print(" Received from hub: ");
//             Serial.println(rxValue.c_str());
//             if (rxValue == "ACK" && bleKeyboardInstance)
//                 bleKeyboardInstance->handleAck(true);
//         }
//     }
// };

class MyServerCallbacks : public BLEServerCallbacks
{
private:
    SecureBleKeyboard *bleKeyboardInstance;

public:
    MyServerCallbacks(SecureBleKeyboard *instance) : bleKeyboardInstance(instance) {}

    void onConnect(BLEServer *pServer) override
    {
        Serial.println(" Remote: Client connected!");
        if (bleKeyboardInstance)
        {
            bleKeyboardInstance->connId = pServer->getConnId();
            bleKeyboardInstance->setConnected(true);
            bleKeyboardInstance->connectionStatus->setInputKeyboard(bleKeyboardInstance->pCommandChar);
            bleKeyboardInstance->fsm->handleBLEEvent(true); // FSM nhận hub connect
        }
    }

    void onDisconnect(BLEServer *pServer) override
    {
        Serial.println(" Remote: Client disconnected!");
        if (bleKeyboardInstance)
        {
            bleKeyboardInstance->setConnected(false);
        }
    }
};

#endif
