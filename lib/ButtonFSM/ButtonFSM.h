#ifndef BUTTON_FSM_H
#define BUTTON_FSM_H

#include <Arduino.h>

enum FSMState
{
    STATE_SLEEP_IDLE,
    STATE_WAIT_CONNECT,
    STATE_CONNECTED,
    STATE_SEND_KEY
};

enum ButtonEvent
{
    EVENT_NONE,        // ko làm gì
    EVENT_SHORT_PRESS, // nhấn 1 lần
    EVENT_LONG_PRESS,  // nhấn giữ
    EVENT_BLE_FAIL,    // ko knoi ble dc
    EVENT_HUB_CONNECT, // kết nối ble thành công
    EVENT_ACK_OK,      // Nhận dc ACK thành công
    EVENT_TIMEOUT      // hết thời gian chờ
};

class ButtonFSM
{
private:
    FSMState currentState;
    unsigned long stateStartTime;
    unsigned long pressStartTime;
    unsigned long lastActivityTime;
    bool buttonPressed;
    bool sendKeyFlag;                                  // mới thêm 5/11
    static const unsigned long LONG_PRESS_TIME = 2000; // định nghĩa nhấn giữ trong 2s
    static const unsigned long CONNECT_TIMEOUT = 20000;
    static const unsigned long ACK_TIMEOUT = 10000;
    static const unsigned long INACTIVITY_TIMEOUT = 10000; // thời gian ko hoạt động trong vòng 10s

public:
    ButtonFSM();
    void begin();
    void update();
    void handleButtonPress(bool pressed);
    void handleBLEEvent(bool connected);
    void handleAck(bool received);
    unsigned long getLastActivityTime() const;
    FSMState getCurrentState() { return currentState; }
    bool shouldWakeUp() { return currentState != STATE_SLEEP_IDLE; }
    // bool shouldSendKey() { return currentState == STATE_SEND_KEY; }
    bool shouldSendKey() { return sendKeyFlag; }
    void setShouldSendKey(bool val); // mới thêm 5/11

private:
    void changeState(FSMState newState);
    void handleEvent(ButtonEvent event);
};

#endif