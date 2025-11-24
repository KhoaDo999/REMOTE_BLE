    #include "SecureBleKeyboard.h"

    // ==============================
    // Khởi tạo/ Hủy bỏ đối tượng
    // ==============================
    SecureBleKeyboard::SecureBleKeyboard(std::string deviceName,// Khởi tạo đối tượng
                                        std::string deviceManufacturer,
                                        uint8_t batteryLevel)
    {
        this->deviceName = deviceName;
        this->deviceManufacturer = deviceManufacturer;
        this->batteryLevel = batteryLevel;
        this->encryptor = nullptr;
        this->fsm = new ButtonFSM();
        this->connectionStatus = new BleConnectionStatus();
        this->bleStarted = false;
        this->creatingServer = false;
        this->pServer = nullptr;
        this->pService = nullptr;
        this->pCommandChar = nullptr;
    }

    SecureBleKeyboard::~SecureBleKeyboard()// Hủy bỏ đối tượng để tránh rò rỉ bộ nhớ trên heap (memory leak).
    {
        delete fsm;
        delete connectionStatus;
        delete encryptor;
    }

    // ==============================
    // kết nối/ko kết nối?
    // ==============================
    bool SecureBleKeyboard::isConnected() const
    {
        return connectionStatus ? connectionStatus->connected : false;// kiểm tra trạng thái kết nối ble
    }

    void SecureBleKeyboard::setConnected(bool status)
    {
        if (connectionStatus)
            connectionStatus->connected = status;
    }

    // ==============================
    // Begin
    // ==============================
    void SecureBleKeyboard::begin(const unsigned char *aesKey)// khởi động file mã hóa,fsm, tạo server ble bằng task rtos riêng để chạy // mà ko chặn loop chính
    {
        this->encryptor = new AES128Encryptor(aesKey);
        this->fsm->begin();
        Serial.println("SecureBleKeyboard initialized. Hold button to start BLE advertising.");

        if (!creatingServer)
        {
            creatingServer = true;
            xTaskCreate(this->taskServer, "BLE_Server_Task", 20000, (void *)this, 5, NULL);
            Serial.println("BLE advertising task started.");
        }
    }

    // ==============================
    // Update
    // ==============================
    void SecureBleKeyboard::update()// hàm này chạy liên tục trong loop, quản lý logic toàn hệ thống
    {
        // Cập nhật FSM
        this->fsm->update();
        FSMState state = fsm->getCurrentState();

        // ----------------------------
        // Quản lý BLE advertising
        // ----------------------------
        if (state == STATE_SLEEP_IDLE && bleStarted)
        {
            stopAdvertising(); // chỉ dừng quảng bá, sau khi nhấn nút mở cửa
        }
        if (state == STATE_SLEEP_IDLE && isConnected())// 10s sau khi mở cửa, ngắt kết nối
        {
            if (millis() - fsm->getLastActivityTime() > 10000) // 10s inactivity
            {
                Serial.println("Inactivity timeout: disconnecting BLE...");
                if (pServer && connId != 0xFFFF)
                {
                    pServer->disconnect(connId);
                    connId = 0xFFFF;
                }
                setConnected(false);
                fsm->handleBLEEvent(false); // FSM biết đã disconnect
            }
        }

        // ----------------------------
        // Xử lý gửi key
        // ----------------------------
        if (fsm->shouldSendKey() && isConnected()) // nếu đang kết nối và nhấn gửi key 
        {
            sendEncryptedCommand("OPEN_DOOR_ESP32!");
            fsm->setShouldSendKey(false);
            isSending = true;
            fsm->handleAck(true);
        }
        else
        {
            isSending = false;
        }
    }

    // ==============================
    // Gửi key mở cửa dc mã hóa
    // ==============================
    void SecureBleKeyboard::sendEncryptedCommand(const String &command)
    {
        if (!isConnected())
        {
            Serial.println(" BLE not connected — cannot send command.");
            return;
        }

        if (!encryptor)
        {
            Serial.println(" Encryptor not initialized!");
            return;
        }
        isSending = true; // đánh dấu đã gửi
        unsigned char input[16] = {0};// biến chuỗi cần gửi thành mảng 16 byte
        int len = command.length();
        if (len > 16)// nếu chuỗi lớn hơn 16 byte, cắt bớt đi. còn ko thì thêm 0x00 cho tròn 16 byte
            len = 16;
        for (int i = 0; i < len; i++)
            input[i] = command[i];

        // --- Mã hóa AES ECB ---
        unsigned char encrypted[16]; // mã hóa aes128
        encryptor->encrypt(input, encrypted);

        // --- Gửi dữ liệu nhị phân 16 byte ---
        pCommandChar->setValue(encrypted, 16); // pCommandChar là biến mà hub đang theo dõi, sau đó đặt giá trị độ lớn bằng 16byte và notify tức gửi đi
        pCommandChar->notify();

        // --- In ra Serial để debug (hiển thị HEX) ---
        Serial.print("Sending encrypted raw bytes: ");
        for (int i = 0; i < 16; i++)
        {
            if (encrypted[i] < 16) Serial.print("0");
            Serial.print(encrypted[i], HEX);// biểu diễn dưới dạng mã hex
        }
        Serial.println();
    }

    // ==============================
    // Nhấn nút
    // ==============================
    void SecureBleKeyboard::handleButtonPress(bool pressed) // Giao tiếp giữa nút nhấn / BLE ACK và FSM.
    {
        if (fsm)
            fsm->handleButtonPress(pressed);
    }

    void SecureBleKeyboard::handleAck(bool received)
    {
        if (fsm)
            fsm->handleAck(received);
    }

    // ==============================
    // Tạo BLE server
    // ==============================
    void SecureBleKeyboard::taskServer(void *pvParameter)// Task riêng để quản lý BLE Server gồm tạo ble, ble server+ callback, uuid với 2 mã định danh, quảng bá khi fsm yêu cầu
    // vòng lặp while giúp task ble luôn hoạt động đúng fsm 
    {
        SecureBleKeyboard *bleKeyboard = (SecureBleKeyboard *)pvParameter;

        BLEDevice::init(bleKeyboard->deviceName);
        bleKeyboard->pServer = BLEDevice::createServer();
        bleKeyboard->pServer->setCallbacks(new MyServerCallbacks(bleKeyboard));

        bleKeyboard->pService = bleKeyboard->pServer->createService(UUID_SERVICE_DOOR);// định danh đây là service điều kiển cửa
        bleKeyboard->pCommandChar = bleKeyboard->pService->createCharacteristic(
            UUID_CHAR_COMMAND,
            BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);// property đọc và viết 
        // gán characteristic vào connectionStatus để set notifications
        bleKeyboard->connectionStatus->setInputKeyboard(bleKeyboard->pCommandChar);
        bleKeyboard->pService->start();
        BLEAdvertising *pAdvertising = bleKeyboard->pServer->getAdvertising();
        pAdvertising->addServiceUUID(UUID_SERVICE_DOOR);
        bleKeyboard->bleStarted = false;  // chưa advertising lúc mới tạo task
        bleKeyboard->creatingServer = false;
        // -----------------------------
        // Vòng lặp quản lý advertising dựa trên FSM
        // ----------------------------- 
        while (true)
        {
            if (bleKeyboard->fsm->getCurrentState() == STATE_WAIT_CONNECT && !bleKeyboard->bleStarted)// NẾU ĐỢI KẾT NỐI THÌ QUẢNG BÁ BLE
            {
                bleKeyboard->pServer->getAdvertising()->start();
                bleKeyboard->bleStarted = true;
            }
            else if (bleKeyboard->fsm->getCurrentState() == STATE_SLEEP_IDLE && bleKeyboard->bleStarted) // NẾU SLEEP THÌ KO QUẢNG BÁ
            {
                bleKeyboard->pServer->getAdvertising()->stop();
                bleKeyboard->bleStarted = false;
            }
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }


    // ==============================
    // Quảng bá
    // ==============================
    void SecureBleKeyboard::startAdvertising() // bật tắt quảng bá để hub thấy or khi muốn ngắt kết nối
    {
        if (pServer)
        {
            BLEAdvertising *pAdvertising = pServer->getAdvertising();
            if (pAdvertising)
            {
                pAdvertising->start();
                bleStarted = true;
                Serial.println(" BLE advertising started!");
            }
        }
    }

    void SecureBleKeyboard::stopAdvertising()
    {
        if (pServer)
        {
            BLEAdvertising *pAdvertising = pServer->getAdvertising();
            if (pAdvertising)
            {
                pAdvertising->stop();
                Serial.println(" BLE advertising stopped due to timeout or idle state.");
            }
        }
        bleStarted = false;
    }
