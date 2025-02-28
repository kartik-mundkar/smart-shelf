# smart-shelf
## Overview
Smart Shelf is an IoT-based system that measures weight, temperature, and humidity data using an ESP32 microcontroller and sends it to a backend server. The data is displayed on an LCD screen and can be accessed via a web-based frontend.

## Features
- **ESP32-Based System**: Uses an ESP32 microcontroller to collect weight, temperature, and humidity data.
- **WiFi Connectivity**: Sends real-time data to a Node.js-based backend.
- **LCD Display**: Shows live readings of weight, temperature, and humidity.
- **Database Storage**: Uses MongoDB to store sensor readings.
- **Web Interface**: A frontend web app displays real-time and historical data.

## Project Structure
```
SmartShelfProject/
│── Backend/              # Node.js backend with Express and MongoDB
│── Frontend/             # Web-based frontend (HTML, CSS, JS)
│── ESP32_code/           # Arduino code for ESP32
│── Components_details.pdf # Information Regarding Components
│── README.md             # Project guide (this file)
```

## Requirements
- **Hardware:**
  - ESP32-WROOM Module
  - HX711 Load Cell Amplifier
  - Load Cell
  - 16x2 LCD Display
  - DHT11 or DS18B20 Temperature Sensor
- **Software:**
  - Arduino IDE (for ESP32)
  - Node.js & npm
  - MongoDB
  - Any web browser

## Setup Instructions
### 1. ESP32 Code
- Install the required libraries: `WiFi.h`, `HTTPClient.h`, `DHT.h`, `HX711.h`.
- Upload `ESP32_code.ino` to ESP32.
- Store WiFi credentials in SPIFFS (`wifi.txt`).
- Ensure the ESP32 is connected to the same network as the backend server.

### 2. Backend Server
- Navigate to the `Backend/` folder:
  ```bash
  cd Backend
  npm install
  ```
- Set up a `.env` file with your MongoDB connection string:
  ```
  MONGO_URI=mongodb://your-mongodb-url
  PORT=3000
  ```
- Start the backend:
  ```bash
  node index.js
  ```

### 3. Frontend
- Open `Frontend/index.html` in a browser.
- The frontend fetches data from the backend and displays real-time updates.

## Usage
1. Power on the ESP32 module.
2. Sensor data is sent to the backend and stored in MongoDB.
3. Access the web interface to view real-time data.

## Future Improvements
- Add user authentication for data security.
- Implement real-time notifications for weight threshold alerts.
- Deploy the web app to a cloud platform for remote access.


