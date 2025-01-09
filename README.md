# Universal Home Remote with ESP32

## Overview
This project aims to create a **universal home-remote** using the ESP32. The remote can control various appliances at home, including air conditioners, TVs, fans, and other devices that use **IR-based remotes** or **wireless remote control via the ESP-NOW protocol**.  

The setup includes:
- **IR-based remote functionality** for devices with infrared control.
- **ESP-NOW protocol** for wireless communication between ESP32 devices.

## Features
- **Wireless Remote Control**: Utilizes the ESP-NOW protocol to send and receive control signals between ESP32 devices.  
- **IR Remote Control**: Can send IR signals to control devices that use IR-based remotes (e.g., air conditioners, TVs, fans).  

## Libraries Used
The following libraries are essential for this project:  
1. [u8g2](https://github.com/olikraus/u8g2) – For handling graphical displays (OLED/monochrome).  
2. [ESP32Encoder](https://github.com/madhephaestus/ESP32Encoder) – For reading rotary encoders.  
3. [Bounce2](https://github.com/thomasfredericks/Bounce2) – For debouncing button inputs.  
4. [ESP-NOW](https://www.arduino.cc/en/Reference/WiFiEspNow) – For wireless communication between ESP32 devices.  
5. [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) – For sending and receiving IR signals.  

## How It Works
### IR-Based Remote
The ESP32 uses the **IRremoteESP8266** library to send IR signals based on the user's input. These signals replicate those from existing appliance remotes, allowing control over devices like air conditioners, TVs, etc.  

### ESP-NOW Wireless Remote
The ESP32 acts as a **transmitter** that communicates wirelessly with an **ESP-NOW-enabled receiver**. The receiver ESP32, programmed with the appropriate receiver code, executes commands based on the transmitted signals.  

## Setup and Installation
1. **Install Libraries**: Ensure the required libraries are installed in your PlatformIO environment.  
2. **Configure Transmitter ESP32**:  
   - Load the transmitter code on your ESP32 device.  
   - Connect any input devices (e.g., buttons, rotary encoders, or displays).  
3. **Configure Receiver ESP32**:  
   - Load the receiver code on another ESP32 device.  
   - Enable the ESP-NOW protocol.  
4. **IR Remote**: Test and calibrate the IR transmission using known IR codes of your appliances.  

## Applications
- Control air conditioners, TVs, fans, and other IR-based appliances.  
- Use the wireless remote feature to interact with appliances or devices that require remote commands.  

## Future Scope
- Add support for Bluetooth or Wi-Fi-based remote control.  
- Add support for RF based remote control

## Contributing
Feel free to contribute to this project! If you have suggestions or enhancements, create a pull request or open an issue.

---

**Happy hacking with ESP32!**🚀
