# Project Brief: REMOTE_BLE

## Project Overview
A secure ESP32-based BLE remote control device for door access control. The device acts as a wireless keyboard that sends encrypted commands to a central hub to open doors.

## Core Functionality
- **Physical Button Interface**: Single button with short press and long press detection
- **BLE Communication**: Wireless connection to a hub device using Bluetooth Low Energy
- **Secure Command Transmission**: AES-128 encrypted commands sent to hub
- **State Management**: Finite State Machine (FSM) for reliable operation flow
- **Visual Feedback**: LED indicators for connection status and activity

## User Interaction Flow
1. User long-presses button (2+ seconds) to wake device and start BLE advertising
2. Device waits for hub connection (20 second timeout)
3. Once connected, user short-presses button to send encrypted door open command
4. Device returns to sleep after successful transmission or 10 seconds of inactivity
5. Automatic disconnection after period of inactivity to save power

## Key Requirements
- **Security**: All door commands must be AES-128 encrypted
- **Reliability**: FSM ensures predictable state transitions and timeout handling
- **Power Efficiency**: Device sleeps when idle, advertising only when needed
- **User Feedback**: LED patterns indicate current state to user
- **Robustness**: Handles connection failures, timeouts, and edge cases gracefully

## Technical Platform
- **Hardware**: ESP32 DOIT DevKit v1
- **Framework**: Arduino framework via PlatformIO
- **Primary Language**: C++
- **Architecture**: Component-based with custom libraries

## Success Criteria
- User can reliably connect and send door open commands
- All commands are properly encrypted before transmission
- Device provides clear visual feedback of its state
- Power consumption minimized through sleep states
- System recovers gracefully from errors and timeouts
