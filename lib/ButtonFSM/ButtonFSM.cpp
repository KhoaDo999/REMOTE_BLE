#include "ButtonFSM.h"

ButtonFSM::ButtonFSM() : currentState(STATE_SLEEP_IDLE), stateStartTime(0),// khởi tạo
                         pressStartTime(0), buttonPressed(false), lastActivityTime(0)
{
    sendKeyFlag = false;
    lastActivityTime = millis();
}

void ButtonFSM::begin() // khởi tạo là ở trang thái ngủ
{
    changeState(STATE_SLEEP_IDLE);
}

void ButtonFSM::changeState(FSMState newState)
{
    currentState = newState;
    stateStartTime = millis();

    Serial.print("State changed to: ");
    switch (currentState)
    {
    case STATE_SLEEP_IDLE:
        Serial.println("SLEEP/IDLE");
        break;
    case STATE_WAIT_CONNECT:
        Serial.println("WAIT_CONNECT");
        break;
    case STATE_CONNECTED:
        Serial.println("CONNECTED");
        break;
    case STATE_SEND_KEY:
        Serial.println("SEND_KEY");
        break;
    }
}

void ButtonFSM::update()
{
    unsigned long currentTime = millis();

    switch (currentState)
    {
    case STATE_WAIT_CONNECT: // kết nối lâu quá, cho ngắt luôn
        if (currentTime - stateStartTime > CONNECT_TIMEOUT)
        {
            handleEvent(EVENT_TIMEOUT);
        }
        break;
    case STATE_CONNECTED: // ở trạng thái connected, ko làm gì trong 10s thì sẽ về trạng thái sleep/idle. rồi từ đó ngắt kết nối luôn
        // Kiểm tra 10s không hoạt động -> tự ngắt
        if (currentTime - lastActivityTime > INACTIVITY_TIMEOUT)
        {
            Serial.println("Connected but do nothing");
            handleEvent(EVENT_BLE_FAIL); // hoặc tạo EVENT_INACTIVITY nếu muốn
        }
        break;

    case STATE_SEND_KEY: // gửi key nhưng mà phản hồi ack từ hub lại lâu quá thì sẽ cho ngắt luôn(ở đây đang giả lập sẽ nhận dc ACK luôn do hub chưa có chwusc nawg gửi ACK)
        if (currentTime - stateStartTime > ACK_TIMEOUT)
        {
            Serial.println("No ACK received or too late");
            handleEvent(EVENT_TIMEOUT);
        }
        break;

    default:
        break;
    }
}
// kiểm tra nút nhấn nhấn ngắn hay nhấn giữ
void ButtonFSM::handleButtonPress(bool pressed)
{
    if (pressed && !buttonPressed)// nhấn nút sẽ bắt đầu tính thời gian nhấn
    {
        pressStartTime = millis();
        buttonPressed = true;
        lastActivityTime = millis();
    }
    else if (!pressed && buttonPressed)// kiểm tra xem nhấn thường hay nhấn giữ
    {
        unsigned long pressDuration = millis() - pressStartTime;
        buttonPressed = false;
        lastActivityTime = millis();
        if (pressDuration < LONG_PRESS_TIME)
        {
            handleEvent(EVENT_SHORT_PRESS);
        }
        else
        {
            handleEvent(EVENT_LONG_PRESS);
        }
    }
}
// kiểm tra đã connect dc hay chưa
void ButtonFSM::handleBLEEvent(bool connected)
{
    lastActivityTime = millis();
    if (connected)
    {
        handleEvent(EVENT_HUB_CONNECT);
    }
    else
    {
        handleEvent(EVENT_BLE_FAIL);
    }
}
// nhận tín hiệu ack từ hub
void ButtonFSM::handleAck(bool received)
{
    lastActivityTime = millis();
    if (received)
    {
        handleEvent(EVENT_ACK_OK);
    }
    else
    {
        handleEvent(EVENT_TIMEOUT);
    }
}
// sự kiện
void ButtonFSM::handleEvent(ButtonEvent event)
{
    Serial.print("Handling event: ");
    switch (event)
    {
    case EVENT_NONE:
        Serial.println("NONE");
        break;
    case EVENT_SHORT_PRESS:
        Serial.println("SHORT_PRESS");
        break;
    case EVENT_LONG_PRESS:
        Serial.println("LONG_PRESS");
        break;
    case EVENT_BLE_FAIL:
        Serial.println("BLE_FAIL");
        break;
    case EVENT_HUB_CONNECT:
        Serial.println("HUB_CONNECT");
        break;
    case EVENT_ACK_OK:
        Serial.println("ACK_OK");
        break;
    case EVENT_TIMEOUT:
        Serial.println("TIMEOUT");
        break;
    }

    switch (currentState)
    {
    case STATE_SLEEP_IDLE:
        if (event == EVENT_LONG_PRESS)
        {
            changeState(STATE_WAIT_CONNECT);
        }
        break;

    case STATE_WAIT_CONNECT:
        if (event == EVENT_HUB_CONNECT)
        {
            changeState(STATE_CONNECTED);
            sendKeyFlag = false;
        }
        else if (event == EVENT_TIMEOUT || event == EVENT_BLE_FAIL)
        {
            changeState(STATE_SLEEP_IDLE);
        }
        break;

    case STATE_CONNECTED:
        if (event == EVENT_SHORT_PRESS)
        {
            sendKeyFlag = true;
            changeState(STATE_SEND_KEY);
        }
        else if (event == EVENT_BLE_FAIL)
        {
            changeState(STATE_SLEEP_IDLE);
        }
        break;

    case STATE_SEND_KEY:
        if (event == EVENT_ACK_OK)
        {
            sendKeyFlag = false;           // mới thêm
            changeState(STATE_SLEEP_IDLE); // thay cho STATE_CONNECTED
        }
        else if (event == EVENT_TIMEOUT)
        {
            sendKeyFlag = false;
            changeState(STATE_SLEEP_IDLE);
        }
        break;
    }
}
void ButtonFSM::setShouldSendKey(bool val)
{
    this->sendKeyFlag = val;
}
unsigned long ButtonFSM::getLastActivityTime() const
{
    return lastActivityTime;
}
