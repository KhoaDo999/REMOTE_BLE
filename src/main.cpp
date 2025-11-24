#include <Arduino.h>
#include "SecureBleKeyboard.h"

// này là mã khóa, dùng để mã hóa và giải mã
char aesKey[] = "abcdefghijklmnop";
SecureBleKeyboard bleKeyboard;
const int BUTTON_PIN = 17;
const int LED_PIN = 0; // LED báo trạng thái
const int LED_CONNECT = 25;
void setup()
{
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // tắt LED lúc đầu
  pinMode(LED_CONNECT, OUTPUT);
  digitalWrite(LED_CONNECT, LOW);
  Serial.println("Starting Secure BLE Keyboard...");
  delay(1000);
  bleKeyboard.begin((unsigned char *)aesKey);
}

void loop()
{
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);
  bleKeyboard.handleButtonPress(buttonPressed);
  bleKeyboard.update();

  FSMState state = bleKeyboard.getFSM()->getCurrentState();
  static unsigned long lastBlinkTime = 0;
  static bool ledState = false;
  if (buttonPressed && state == STATE_SLEEP_IDLE)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  else if (state == STATE_WAIT_CONNECT)
  {
    if (millis() - lastBlinkTime > 500)
    { // nhấp nháy 0.5s
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlinkTime = millis();
    }
  }
  else if (state == STATE_SEND_KEY)
  {
    // Nhấn 1 lần -> LED sáng 1 lần trong 200ms
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
  }
  else
  {
    // Trạng thái khác -> LED tắt
    digitalWrite(LED_PIN, LOW);
  }
  if (state == STATE_CONNECTED)
  {
    digitalWrite(LED_CONNECT, HIGH);
  }
  else
  {
    digitalWrite(LED_CONNECT, LOW);
  }

  delay(50);
}
// xin chào, tôi đang test thử branch mới xem sao
// test thử hehehehe