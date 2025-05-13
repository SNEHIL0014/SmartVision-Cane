# 🔍 SmartVision Cane (SVC) [https://drive.google.com/file/d/1S2Fh-44BxfqaYEFgEkCb_u9bTByvVzu0/view?usp=sharing]

An AI-powered smart cane using edge computing, computer vision, and IoT to enhance the mobility and safety of visually impaired individuals.

## 🧰 Hardware Components

The following hardware components were used to build the SmartVision Cane:

| Component                            | Description / Purpose                             |
|--------------------------------------|---------------------------------------------------|
| **Xilinx Kria KV260**                | Edge AI processing and computer vision            |
| **ESP8266 (NodeMCU)**                | Wi-Fi enabled microcontroller for sensor I/O      |
| **MPU6050**                          | Accelerometer + Gyroscope for fall detection      |
| **NEO-6M GPS Module**                | Real-time satellite positioning                   |
| **A7670C GSM Module**                | Cellular communication (SMS alerts)               |
| **USB Webcam**                       | Captures video input for AI model                 |
| **Speaker**                          | Audio output for navigation alerts                |
| **3.5mm Jack to Jack AUX Cable**     | Audio signal routing                              |
| **AUX to USB Converter**             | Enables USB microphone/speaker functionality      |
| **MicroSD Card**                     | Data storage for models/logs                      |
| **Push Button**                      | Manual SOS trigger                                |
| **Power Supply Adapter**             | Powers the entire system                          |
| **Jumper Wires**                     | Electrical connectivity between components        |
| **USB Data Cable**                   | Data transfer and device flashing                 |
---

## 🛠️ Software Stack

- Python 3.x
- TensorFlow Lite
- OpenCV
- pyttsx3 (TTS)
- Embedded C (ESP8266)
- NLP for navigation prompts

---

## 💡 How It Works

1. AI model detects obstacles using camera input.
2. NLP engine delivers voice alerts.
3. Fall detection triggers SOS with GPS via GSM.
4. Guardian can track real-time location.
5. All computation is handled on-device (no internet required).

---

🔄 System Workflow
graph TD
A[Start/Initialize] --> B[Verify All Component Operation]
B --> C{Error Detected?}
C -- Yes --> D[Alert User via Voice]
C -- No --> E[Look for Obstacle]
E -- Yes --> F[Voice Alert: "Vehicle"]
E -- No --> G[Track Live Location]
G --> H[Share to Guardians]
H --> I{SOS Button Pressed?}
I -- Yes --> J[Send Emergency Alert]
I -- No --> K[Move Ahead]
K --> A

---
💡 Key Features

🎯 Real-Time Obstacle Detection (YOLOv5)

🧭 Voice-Guided Navigation via NLP

📍 Live GPS Tracking using NEO-6M

🆘 Fall Detection with SOS alerts

🔊 Voice Alerts for detected objects

📶 Offline Processing with Edge AI

📱 Mobile App Support (future integration)

---

🧩 Flow Chart [https://drive.google.com/file/d/14UmJYK49MJjwZDECLBR8z1w-aU-XnhNO/view?usp=sharing]

Interaction between Kria KV260, sensors, GSM/GPS, and ESP32

---

⚡ Circuit Diagram [https://drive.google.com/file/d/1U-wmsbWvhk-P1RDY2-QXjNO1bjcSw4-R/view?usp=sharing]

Includes SIM800C, ESP32, GPS, Camera, Speaker, MPU6050, and Power Supply

---

⚙️ How It Works (Step-by-Step)

Startup: System performs hardware checks.

Obstacle Detection: Camera captures surroundings; AI detects objects.

Voice Feedback: Alerts the user about obstacles (e.g., “Vehicle ahead”).

Fall Detection: Sudden tilt/movement triggers MPU6050-based SOS.

GPS Transmission: Location is sent via GSM to pre-configured contacts.

Live Tracking: Guardians can track the user’s real-time location.

User-Controlled SOS: Manual button allows emergency alert at any time.

---

🧪 Technologies Used

Hardware: AMD Kria KV260, ESP32, SIM800C, GPS NEO-6M, MPU6050

Languages: Python, Embedded C

AI Models: YOLOv5, CNN

Software: Vitis AI, Vivado, Xilinx SDK

NLP: Text-to-speech conversion for voice navigation

Connectivity: GSM, GPS, Wi-Fi, BLE

---

🧭 Applications

✅ Navigation aid for visually impaired individuals

✅ Real-time GPS tracking for caregivers

✅ Fall detection for elderly users

✅ Indoor navigation using BLE beacons

✅ Public transport and smart city integration

---

🚀 Future Enhancements

🔲 LiDAR Integration for 3D mapping

✋ Gesture Recognition for contactless control

🏙️ Smart City API integration (traffic signals, public transport)

📱 Custom Mobile App for alerts and route setup

🔋 Energy Optimization for longer battery life

---

👥 Team & Contributions
AI & Vision: Model Training, YOLOv5 & CNN Integration

Hardware: Circuit Design, Sensor Integration

Software: Real-time Processing, NLP Feedback

Connectivity: GPS, GSM, ESP32 Programming

Research: Literature Review & Feasibility Study

---

🧠 Special thanks to visually impaired users who provided field-testing feedback across diverse environments.

---
