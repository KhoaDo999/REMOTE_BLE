# Product Context

## Why This Project Exists
This project creates a secure wireless door access control solution using ESP32 hardware. It addresses the need for a reliable, encrypted, and user-friendly way to remotely trigger door opening mechanisms through BLE communication.

## Problems It Solves

### 1. Secure Door Access
- Traditional wireless door remotes often lack encryption
- This solution implements AES-128 encryption to prevent unauthorized access
- Encrypted commands ensure that intercepted BLE packets cannot be replayed or decoded

### 2. User-Friendly Operation
- Simple single-button interface eliminates complexity
- Clear LED feedback keeps users informed of device state
- Predictable behavior through FSM state management

### 3. Power Efficiency
- Device sleeps when idle to conserve battery
- BLE advertising only when actively needed
- Automatic disconnection after inactivity periods

### 4. Reliability
- Timeout mechanisms prevent hanging states
- Connection failure handling ensures graceful recovery
- FSM prevents invalid state transitions

## How It Works

### User Experience
1. **Wake Up**: User long-presses button (2+ seconds)
   - LED starts blinking (500ms intervals)
   - Device begins BLE advertising
   - Waiting for hub connection

2. **Connect**: Hub detects and connects to device
   - LED on pin 25 lights up solid (connected indicator)
   - Device ready to send commands

3. **Activate**: User short-presses button
   - LED on pin 0 flashes briefly (200ms)
   - Encrypted "OPEN_DOOR_ESP32!" command sent to hub
   - Device returns to sleep state after successful send

4. **Timeout/Idle Handling**:
   - 20 second timeout if hub doesn't connect
   - 10 second auto-disconnect if no activity while connected
   - Device returns to sleep state, stops advertising

### LED Feedback System
- **Pin 0 (Activity LED)**:
  - Solid: Button held during sleep (waking up)
  - Blinking (500ms): Waiting for connection
  - Brief flash (200ms): Command sent
  - Off: All other states

- **Pin 25 (Connection LED)**:
  - On: Connected to hub
  - Off: Not connected

### Security Model
- Pre-shared AES-128 key ("abcdefghijklmnop" in current implementation)
- All door commands encrypted before transmission
- 16-byte binary encrypted payload sent via BLE notify
- Hub must have matching key to decrypt commands

## User Experience Goals

### Simplicity
- One button does everything
- No configuration required by end user
- Intuitive LED feedback

### Reliability
- Always returns to known good state
- Handles all error conditions gracefully
- Predictable timeout behavior

### Security
- All commands encrypted
- No plaintext transmission
- Secure key-based authentication

### Responsiveness
- Quick button response
- Immediate visual feedback
- Fast BLE connection establishment

## Target Use Cases
- Residential door access control
- Office/workplace entry systems
- Garage door openers
- Any remote actuation requiring security and reliability
