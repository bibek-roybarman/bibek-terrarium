<div align="center">
<!-- Optional: Replace this placeholder with a real photo of your terrarium -->
<img src="https://img.icons8.com/color/150/000000/terrarium.png" alt="Terrarium Logo" width="120" />
<h1>🌿 Terrarium Microclimate IoT Controller</h1>
<p>
<strong>An automated, closed-loop environmental control system built on the ESP32-S3.</strong>
</p>
<!-- Badges -->
<p>
<img src="https://img.shields.io/badge/Hardware-ESP32--S3-blue?style=for-the-badge&logo=espressif" alt="ESP32">
<img src="https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B" alt="C++">
<img src="https://img.shields.io/badge/IoT-ThingsBoard%20%7C%20Blynk-ff6600?style=for-the-badge" alt="IoT">
<img src="https://img.shields.io/badge/License-MIT-success?style=for-the-badge" alt="License">
</p>
<p>
<a href="#-about-the-project">About</a> •
<a href="#-features">Features</a> •
<a href="#-hardware-bom">Hardware</a> •
<a href="#-installation--setup">Setup</a> •
<a href="#-iot-dashboard">IoT Integration</a>
</p>
</div>
## 📖 About The Project
Maintaining the perfect microclimate for a bioactive terrarium requires constant vigilance. This project eliminates manual watering and monitoring by creating a **closed-loop feedback system**.
Powered by an ESP32-S3, the system constantly polls ambient temperature, humidity, and soil moisture. Using hysteresis logic, it intelligently triggers an isolated water pump to maintain the exact moisture threshold required by the ecosystem, while streaming real-time telemetry to a cloud dashboard.
## ✨ Features
 * 🌡️ **Precision Monitoring:** Real-time logging of Ambient Temp (^\circC) and Humidity (%) via the DHT22.
 * 💧 **Capacitive Soil Sensing:** Anti-corrosion analog moisture reading mapped to accurate relative percentages.
 * 🔄 **Hysteresis Irrigation:** Intelligent pump control prevents "fluttering" by using distinct Turn-On (30%) and Turn-Off (50%) bounds.
 * 🛡️ **Hardware Fail-Safes:** Hardcoded maximum continuous pump runtime (10s) prevents catastrophic flooding during a sensor failure.
 * 📺 **OLED Telemetry:** On-device 128x64 display for immediate system status at a glance.
 * ⚡ **Isolated Actuation:** Safely drives the 5V inductive pump load via a 1-channel relay, isolating high currents from the ESP32 logic pins.
## 🧰 Hardware (BOM)
| Component | Function | Operating Voltage | Connection |
|---|---|---|---|
| **ESP32-S3-DevKitC-1** | Main Microcontroller (MCU) | 3.3V Logic / 5V VIN | Brain |
| **DHT22** | Temp & Humidity Sensor | 3.3V | GPIO 4 (Add 10kΩ Pull-up) |
| **Capacitive Soil Sensor** | Analog Soil Moisture | 3.3V | GPIO 1 (ADC) |
| **5V 1-Channel Relay** | Pump Circuit Isolator | 5V Trigger | GPIO 15 |
| **5V Submersible Pump** | Irrigation Actuator | 5V DC | Relay NO & COM |
| **128x64 I2C OLED** | Local Display | 3.3V | SDA: 8, SCL: 9 |
> ⚠️ **Crucial Warning:** NEVER power the 5V water pump directly from the ESP32 GPIO pins. Always route the external 5V power supply through the Relay module to the pump.
> 
## 🔌 System Schematic
<div align="center">
<!-- Replace this with a real Fritzing or wiring diagram image link once you take one -->
<img src="https://via.placeholder.com/800x400.png?text=Wiring+Schematic+Placeholder" alt="Schematic" width="100%">
<p><em>System Architecture & Wiring Diagram</em></p>
</div>
## 🚀 Installation & Setup
### 1. Prerequisites
 * Arduino IDE installed.
 * ESP32 Board Manager URL added to Arduino IDE.
 * Required Libraries: DHT sensor library (Adafruit), Adafruit SSD1306, Adafruit GFX Library.
### 2. Clone the Repository
```bash
git clone https://github.com/yourusername/Terrarium-Controller.git
cd Terrarium-Controller

```
### 3. Sensor Calibration (Crucial)
Because every soil mix is different, you must calibrate the capacitive sensor:
 1. Deploy the sensor in completely **dry** soil and check the Serial Monitor for the raw ADC value.
 2. Submerge the sensor to the water line in a glass of water and note the new ADC value.
 3. Open terrarium_controller.ino and update the constants:
   ```cpp
   const int DRY_ADC_VALUE = 3200; // Update with your dry value
   const int WET_ADC_VALUE = 1200; // Update with your wet value
   
   ```
### 4. Upload
Connect the ESP32-S3 via USB Type-C, select the correct COM port, and upload the sketch.
## 🌐 IoT Dashboard
This project is built with IoT scalability in mind. The ESP32 utilizes Wi-Fi to push telemetry to the cloud.
<div align="center">
<!-- Replace with a screenshot of your Blynk or ThingsBoard dashboard -->
<img src="https://via.placeholder.com/800x400.png?text=IoT+Dashboard+Screenshot+Placeholder" alt="IoT Dashboard" width="100%">
</div>
**Recommended Platforms:**
 * **Blynk IoT:** Best for controlling the pump manually from your smartphone.
 * **ThingsBoard:** Best for generating long-term environmental charts and complex web automation rules.
## 📄 License
Distributed under the MIT License. See LICENSE for more information.
<div align="center">
<p>Built with 💧 and ☕ by <strong>[Your Name]</strong></p>
</div>
