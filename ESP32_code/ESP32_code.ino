#include <HX711.h>
#include <LiquidCrystal.h>
#include <SPIFFS.h>
#include <Keypad.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define WIFI_CREDENTIALS_FILE "/wifi.txt"

const char* serverName = "http://192.168.124.124:3000/data";  // Replace with your server's IP and port
HTTPClient http;



// Pins and other Definations
// Define connections for LCD 
#define rs_pin  4
#define en_pin  5
#define d4_pin  18
#define d5_pin  19
#define d6_pin  21
#define d7_pin  22
#define DAC_PIN 25 // Use GPIO 25 for DAC output given to BLA pin of LCD


// Define connections for HX711
#define DT_PIN 15       // Data pin connected to GPIO 15
#define SCK_PIN 13     // Clock pin connected to GPIO 13


// Define connections for Calibration and Tare Push buttons
#define CALIBRATION_BUTTON_PIN 34
#define TARE_BUTTON_PIN 35


// Define DHT pin and type
#define DHTPIN 14         // Data pin connected to GPIO 14
#define DHTTYPE DHT11     // Define sensor type as DHT11


// Define the rows and columns for the keypad
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns

// Define the symbols on the keypad buttons
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'.', '0', '#', 'D'}
};

//Define connections for keypad using available pins
byte rowPins[ROWS] = {0, 16, 17, 23}; // row pins connected to GPIO 2, 16, 17, 23
byte colPins[COLS] = {26, 27, 32, 33}; // column pins connected to GPIO 26, 27, 32, 33


// Initialisation of LCD, DHT Sensor and Keypad
LiquidCrystal lcd(rs_pin, en_pin, d4_pin, d5_pin, d6_pin, d7_pin);  // LCD pins as configured
HX711 scale;    // Create an instance of the HX711 class
DHT dht(DHTPIN, DHTTYPE);   //// Initialize DHT sensor
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);    // Initialize the Keypad


// Variable declrations
// Variable to store the entered number as a string
String numberBuffer = "";

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000; // 1 seconds update interval to prevent flooding in DB

// Calibration factor variable, start with an estimated value
float calibration_factor = 397.92; // Adjust this value after calibration
byte avg_over_times = 10;
bool is_calibrating = false;
bool is_SPIFFS_mounted = false;

unsigned long lastWifiAttempt = 0; // Timer for WiFi reconnection attempts
const unsigned long wifiReconnectInterval = 10000; // Interval for reconnection in milliseconds

const char* calibrationFilePath = "/calibration.txt"; // SPIFFS path for calibration Storage in ESP memory



// Function Prototypes
void initSPIFFS();
void tare_scale();
float get_key_input();
void calibrate();
void saveCalibrationFactor(float calibration_factor);
void loadCalibrationFactor();
void display_data(float weight,float temperature, float humidity);
void update_DB(float weight,float temperature, float humidity);
bool readWiFiCredentials(String &ssid, String &password);
bool writeWiFiCredentials(const String &ssid, const String &password) ;





void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);               // Initialize serial communication

  // Step 0 (Hardware Connections): Connect the LCD, HX711, DHT & Calibration and Tare button 
 
  // Initialize the DAC For LCD Backlight
  Serial.println("Initializing LCD...");
  pinMode(DAC_PIN, OUTPUT);
  analogWrite(DAC_PIN, 200); // Set to mid-level brightness (0-255)
  lcd.begin(16, 2);  // Set up the LCD's number of columns and rows.
  lcd.clear();
  lcd.print("Initializing...");

  //HX711 Connection Begin
  Serial.println("Initializing HX711...");
  scale.begin(DT_PIN, SCK_PIN); // Initialize the HX711

  //DHT Sensor Connection Begin
  dht.begin();           // Start the DHT sensor

  // delay(2000);
  // Check if Hx711 connected  
  int attempts = 0;
  const int maxAttempts = 10; // Limit the number of attempts to prevent infinite looping
  while (!scale.is_ready() && attempts < maxAttempts) {
    Serial.println("Error: HX711 not connected or not working correctly!");
    delay(1000);
    attempts++;
  }
  if (scale.is_ready()) {
    Serial.println("HX711 connected successfully.");
    initSPIFFS();
    lcd.clear();
    lcd.print("Initialized!!");

    // Setup calibration and tare buttons
    pinMode(CALIBRATION_BUTTON_PIN, INPUT_PULLUP);
    pinMode(TARE_BUTTON_PIN, INPUT_PULLUP);

    // Step 1: Calibration step
    if(is_SPIFFS_mounted){
      scale.tare(avg_over_times);
      loadCalibrationFactor();
      scale.set_scale(calibration_factor);
    } else {
      calibrate();
    }
    
    // Steo 2: Connect to wifi to send the reading data to DB
    String ssid, password;
    if (!readWiFiCredentials(ssid, password)) {
        Serial.println("Enter WiFi SSID:");
        while (Serial.available() == 0) {} // Wait for input
        ssid = Serial.readStringUntil('\n');
        ssid.trim();
        
        Serial.println("Enter WiFi Password:");
        while (Serial.available() == 0) {} // Wait for input
        password = Serial.readStringUntil('\n');
        password.trim();
        
        writeWiFiCredentials(ssid, password); // Store in SPIFFS
    }
    
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");

  } else {
    Serial.println("Failed to connect to HX711 after multiple attempts.");
    lcd.clear();
    lcd.print("Init Fail !!!");
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  wifiReconnect(); // Check and reconnect WiFi if disconnected

  // Step 5: Continuously read and print the weight
  if (scale.wait_ready_timeout(1000)) {

    // Check if the calibration button is pressed
    if (digitalRead(CALIBRATION_BUTTON_PIN) == HIGH && !is_calibrating) {
      is_calibrating = true;
      calibrate();
      is_calibrating = false;
    }
    // Check for tare button press
    if (digitalRead(TARE_BUTTON_PIN) == HIGH) {
      tare_scale();
    }


    float weight = scale.get_units(avg_over_times);
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    display_data(weight, temperature, humidity);
    
    if (WiFi.status() == WL_CONNECTED && millis() - lastUpdateTime >= updateInterval) {
      update_DB(weight,temperature,humidity); 
      lastUpdateTime = millis();  // Reset cooldown timer
      lcd.setCursor(15, 0);
      lcd.print("^");
    }
    
  } else {
    Serial.println("HX711 not found.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Failure");
  }


  // Delay before the next reading
  delay(500);

}






// function Definitions



// Function to initialize SPIFFS (for storage of calibration factor)
void initSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Error mounting SPIFFS.");
        is_SPIFFS_mounted = false;
    } else {
        Serial.println("SPIFFS mounted successfully.");
        is_SPIFFS_mounted = true;
    }
}

// Function to read WiFi credentials from SPIFFS
bool readWiFiCredentials(String &ssid, String &password) {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return false;
    }
    
    File file = SPIFFS.open(WIFI_CREDENTIALS_FILE, "r");
    if (!file) {
        Serial.println("Failed to open credentials file");
        return false;
    }
    
    ssid = file.readStringUntil('\n');
    password = file.readStringUntil('\n');
    ssid.trim();
    password.trim();
    
    file.close();
    return true;
}

// Function to write WiFi credentials to SPIFFS
bool writeWiFiCredentials(const String &ssid, const String &password) {
    File file = SPIFFS.open(WIFI_CREDENTIALS_FILE, "w");
    if (!file) {
        Serial.println("Failed to open credentials file for writing");
        return false;
    }
    
    file.println(ssid);
    file.println(password);
    file.close();
    return true;
}


// Function to Tare the Scale
void tare_scale() {
  Serial.println("Taring the scale... Remove any weight.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Taring...");
  lcd.setCursor(0, 1);
  lcd.print("Remove weight.");
  delay(2000);
  scale.tare(avg_over_times);
  Serial.println("Scale tared.");
  lcd.clear();
  lcd.print("Scale tared.");
  delay(1000); 
}



// Function to get the input from keypad
float get_key_input(){
  numberBuffer = "";
  lcd.clear();
  lcd.print("Enter wt(g): ");
  lcd.setCursor(0,1);
  bool flag = true;
  while(flag){ 
    char key = keypad.getKey(); // Check for key press

    if (key) { // If a key is pressed
      if (key >= '0' && key <= '9') {
        // If the key is a digit, add it to the buffer
        numberBuffer += key;
        Serial.print(key); // Display each digit as it's entered
        lcd.print(key);
      } else if (key == '.') {
        // Treat '*' as the decimal point
        if (numberBuffer.indexOf('.') == -1) { // Prevent multiple decimal points
          numberBuffer += ".";
          Serial.print(".");
          lcd.print(key);
        }
      } else if (key == '#') {
        // Treat '#' as the end of input
        Serial.println(); // Move to the next line
        Serial.print("Final number: ");
        float enteredNumber = numberBuffer.toFloat(); // Convert to float
        Serial.println(enteredNumber); // Display the floating-point number
        lcd.clear();
        lcd.print("wt(g) = ");
        lcd.setCursor(0,1);
        lcd.print(enteredNumber);
        return enteredNumber;
      } else {
        // Handle any other keys ('A', 'B', 'C', 'D') if necessary
        // Recalling the get_key_input function
        return get_key_input();
      }
    }
  }
}


// Function for Calibration Process
void calibrate() {
  float known_weight = 0.0;
  lcd.clear();
  // delay(2000);
  tare_scale();
  lcd.setCursor(0,0);
  lcd.print("enter wt(g):");
  Serial.println("Enter known weight in grams:");
  lcd.setCursor(0, 1);
  
  // Flag for successful weight input
  bool weight_entered = false;
  
  // Loop to get weight input either from Serial Monitor or other input
  while (!weight_entered) {
    if (Serial.available() > 0) {  // Check if Serial input is available
      known_weight = Serial.parseFloat();  // Read the entered float value
      if (known_weight > 0) {
        weight_entered = true;  // Set flag if valid weight is entered
      } else {
        Serial.println("Please enter a positive weight.");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Pos. Weight Req.");
        delay(1000);  // Brief delay for user clarity
      }
    } else {
      // Alternative input method (if Serial is unavailable)
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Calibrating...");
      delay(500);
      
      // get input through keypad
      known_weight = get_key_input();
      if (known_weight > 0) {
        weight_entered = true;
      }
    }
  }

  // Calibration process
  delay(2000);
  long raw_value = scale.get_units(avg_over_times);
  calibration_factor = raw_value / known_weight;  // Calculate factor

  Serial.print("Calibrated with factor (before storing): ");
  Serial.println(calibration_factor);

  // Save and reload calibration factor
  saveCalibrationFactor(calibration_factor);
  scale.set_scale(calibration_factor);
  loadCalibrationFactor();

  Serial.print("Calibrated with factor: ");
  Serial.println(calibration_factor);
  lcd.clear();
  lcd.print("Calibrated");
}


// Function to save the latest calibration factor in the file
void saveCalibrationFactor(float calibration_factor) {
  if (is_SPIFFS_mounted) {
    File file = SPIFFS.open(calibrationFilePath, FILE_WRITE);
    if (file) {
      file.println(calibration_factor);
      file.close();
      Serial.println("Calibration factor saved.");
    }
    else {
      Serial.println("Error opening file for writing.");
    }
  }
}


// Function to load the last calibration factor from file
void loadCalibrationFactor() {
  if (is_SPIFFS_mounted) {
    File file = SPIFFS.open(calibrationFilePath, FILE_READ);
    if (file) {
      calibration_factor = file.readStringUntil('\n').toFloat();
      file.close();
      if(calibration_factor==0.0){
        calibrate();
      }
      else{
        Serial.println("Loaded latest calibration factor: " + String(calibration_factor));
      }
    } else {
      Serial.println("No saved calibration factor found.");
      calibrate();
    }
  }
}


// Function to Display the weight on LCD
void display_data(float weight, float temperature, float humidity){
  lcd.clear();
  lcd.setCursor(0,0);         // Move to the first row
  if (weight>=1000.0){
    int kg = int(weight) / 1000;     
    int g = int(weight) % 1000;
    lcd.print(String(kg)+"."+String(g)+" kg");
    Serial.println(String(kg)+"."+String(g)+" kg");
  }
  else{
    lcd.print(String(weight)+" g");
    Serial.println(String(weight)+" g");
  }
  lcd.setCursor(0,1);         // Move to the second row
  lcd.print(String(temperature)+" C ; ");
  lcd.print(String(humidity)+"%");
}



// update data on DB
void update_DB(float weight,float temperature, float humidity){
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(serverName);
    // Specify content-type header
    http.addHeader("Content-Type", "application/json");
    // Prepare data to send
    String httpRequestData = "{\"weight\":" + String(weight) + 
                             ",\"temperature\":" + String(temperature) + 
                             ",\"humidity\":" + String(humidity) + "}";
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    // Check response from the server
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);   // HTTP response code
      Serial.println(response);           // Server response
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}


