# Smart-Data-Logger-System-for-Agriculture-Applications

An interrupt-based smart data logging system for agricultural applications, utilizing digital interfaces, LED displays, and memory logging. Implemented wireless communication using Bluetooth and Xbee modules. Integrated user interfaces, sensor signals, and motor control for a comprehensive farming system. This project uses an AVR microcontroller to monitor temperature, moisture, water level, and battery status, and communicate these readings via USART to a remote user interface. The system also includes a simple motor control for water delivery based on moisture readings and an alert mechanism for low battery.

## Features
- **USART Communication:** Communicates sensor data to a remote user interface using USART0 and USART1.
- **CRC Error Checking:** Implements CRC3 and CRC11 error checking for reliable data transmission.
- **Memory Management:** Includes functions for memory dump and last entry retrieval.
- **Sleep Mode:** Utilizes sleep mode to save power when the system is idle.
- **LCD Display:** Displays sensor readings and alerts on an LCD screen.
- **Motor Control:** Controls a water pump motor based on moisture readings.
- **Keypad Input:** Accepts user input via a keypad.
- **Wireless Communication:** Utilizes Bluetooth and Xbee modules for wireless communication.

## Files
### `main_remote_sensor.c`
This file contains the main logic for the remote sensor system, including initialization of peripherals, USART communication, CRC error checking, memory management, and sensor data handling.

### `main_sensor_unit.c`
This file includes the code for the sensor unit, which reads sensor values, controls the LCD display, and manages the water pump motor based on moisture levels.

### `main_keypad.c`
This file includes the configuration and handling of a keypad input, which can be used for user interactions and settings adjustments.

## Functions
### Remote Sensor Functions (`main_remote_sensor.c`)
- **Sleep_and_Wait():** Puts the microcontroller into sleep mode to save power.
- **UserBufferOut(unsigned char msg):** Sends a message to the user via USART0.
- **MessageSensor(unsigned char msg):** Sends a message to the sensor via USART1.
- **MessageUser():** Sends a message from the system to the user.
- **CRC3(unsigned char dataInput):** Computes CRC3 for the given data input.
- **CRC3_Check():** Checks the CRC3 value of the current packet.
- **CRC11_Check(unsigned char commandInput, unsigned char dataInput):** Checks the CRC11 value for the given command and data inputs.
- **Repeat_Request():** Requests the sensor to repeat the last transmission.
- **DataPacketType():** Handles data packets received from the sensor.
- **CommandPacketType():** Handles command packets received from the sensor.
- **MemoryDump():** Dumps the memory contents to the user interface.
- **lastEntry():** Retrieves the last entry from the memory.
- **UserTrBufferInit():** Initializes the user transmit buffer.
- **SensorTrBufferInit():** Initializes the sensor transmit buffer.
- **Init_RemoteSensor():** Initializes the remote sensor.

### Sensor Unit Functions (`main_sensor_unit.c`)
- **Sleep_and_Wait():** Puts the microcontroller into sleep mode to save power.
- **UserBufferOut(unsigned char msg):** Sends a message to the user via USART0.
- **Configuration():** Configures the LCD display, ADC, USART, and timer.
- **water_motor():** Controls the water pump motor based on moisture readings.
- **inputs():** Reads the sensor inputs (temperature, moisture, water level, battery).
- **display():** Displays sensor readings and alerts on the LCD screen.
- **TIMER1_COMPA_vect():** Timer interrupt service routine for periodic tasks.

### Keypad Functions (`main_keypad.c`)
- **Sleep_and_Wait():** Puts the microcontroller into sleep mode to save power.
- **Configuration():** Configures the LCD display and keypad.

## Usage
1. **Compile the Code:**
   Ensure you have the necessary AVR development tools installed. Compile the code using your preferred method (e.g., AVR-GCC).
   
2. **Flash the Microcontroller:**
   Use an AVR programmer to flash the compiled hex files onto the microcontroller.
   
3. **Connect the Hardware:**
   - Connect the sensors (temperature, moisture, water level, battery) to the appropriate ADC pins.
   - Connect the LCD display to the appropriate data and control pins.
   - Connect the water pump motor to the appropriate control pin.
   - Connect the keypad to the appropriate pins.
   - Connect Bluetooth and Xbee modules for wireless communication.

4. **Power On:**
   Power on the system. The remote sensor unit will initialize and start reading sensor values, displaying them on the LCD, and communicating them to the user interface. 

5. **Simulate with Proteus:**
   Additionaly, You can use Proteus for simulation by importing 'Smart_Data_Logger_Sim.pdsprj'.

## Notes
- Ensure the microcontroller is properly configured for external memory if required.
- The watchdog timer is disabled in the provided code but can be enabled if needed.
- Modify the pin configurations as per your hardware setup.
