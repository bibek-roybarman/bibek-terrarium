/*
  Terrarium Microclimate & Irrigation Controller
  Board: ESP32-S3 DevKitC-1
  
  Pin Mapping:
  - DHT22 (Data)            : GPIO 4
  - Soil Moisture (Analog)  : GPIO 1 (ADC)
  - OLED SDA                : GPIO 8
  - OLED SCL                : GPIO 9
  - Blue LED (Pump Relay)   : GPIO 15
  - Red LED (Warning)       : GPIO 16
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- Pin Definitions ---
#define SOIL_PIN        1
#define DHT_PIN         4
#define OLED_SDA        8
#define OLED_SCL        9
#define PUMP_PIN        15 // Blue LED (Stand-in for actual water pump relay)
#define WARNING_PIN     16 // Red LED

// --- Sensor Configuration ---
#define DHTTYPE         DHT22
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C // Standard I2C address for 128x64 OLEDs

// --- Calibration & Thresholds ---
// Capacitive sensors read HIGH when dry, LOW when wet. 
// You MUST calibrate these values for your specific dirt/water!
const int DRY_ADC_VALUE = 3200; // Raw ADC reading when completely dry
const int WET_ADC_VALUE = 1200; // Raw ADC reading when submerged in water

const int MOISTURE_LOW_THRESHOLD = 30;  // Turn pump ON at 30%
const int MOISTURE_HIGH_THRESHOLD = 50; // Turn pump OFF at 50%

// --- Timing Constants (in milliseconds) ---
const unsigned long SENSOR_INTERVAL = 5000;      // Read sensors every 5 seconds
const unsigned long PUMP_MAX_RUNTIME = 10000;    // Failsafe: Max 10 seconds of pumping

// --- Global Objects & State Variables ---
DHT dht(DHT_PIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastSensorReadTime = 0;
unsigned long pumpStartTime = 0;
bool isPumping = false;

float currentTemp = 0.0;
float currentHum = 0.0;
int currentMoisturePercent = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Terrarium Controller...");

  // Initialize Pins
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(WARNING_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);    // Ensure pump is OFF
  digitalWrite(WARNING_PIN, LOW); // Ensure warning is OFF

  // Initialize DHT22
  dht.begin();

  // Initialize I2C for OLED on custom pins
  Wire.begin(OLED_SDA, OLED_SCL);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 OLED allocation failed"));
    // Blink Red LED to indicate screen failure
    while(true) {
      digitalWrite(WARNING_PIN, HIGH); delay(500);
      digitalWrite(WARNING_PIN, LOW); delay(500);
    }
  }

  // Boot Screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.println("Terrarium OS");
  display.setCursor(10, 40);
  display.println("Initializing...");
  display.display();
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. NON-BLOCKING SENSOR READ & DISPLAY UPDATE (Every 5 Seconds)
  if (currentMillis - lastSensorReadTime >= SENSOR_INTERVAL) {
    lastSensorReadTime = currentMillis;
    readSensors();
    updateDisplay();
    printToSerial();
  }

  // 2. HYSTERESIS CONTROL LOGIC & FAILSAFES (Runs continuously)
  handleIrrigation(currentMillis);
}

// --- Helper Functions ---

void readSensors() {
  // Read DHT22
  currentTemp = dht.readTemperature();
  currentHum = dht.readHumidity();

  // Check if DHT read failed
  if (isnan(currentTemp) || isnan(currentHum)) {
    Serial.println("Failed to read from DHT sensor!");
    // Keep previous values to prevent control logic crashes
  }

  // Read Soil Moisture (Analog)
  int rawSoilADC = analogRead(SOIL_PIN);
  
  // Map ADC value to a 0-100 percentage.
  // Note: map() math is inverted because DRY = High ADC, WET = Low ADC
  currentMoisturePercent = map(rawSoilADC, DRY_ADC_VALUE, WET_ADC_VALUE, 0, 100);
  
  // Constrain to 0-100 bounds just in case ADC fluctuates past calibration limits
  currentMoisturePercent = constrain(currentMoisturePercent, 0, 100);
}

void handleIrrigation(unsigned long currentMillis) {
  // Turn ON logic (Hysteresis low bound)
  if (!isPumping && currentMoisturePercent <= MOISTURE_LOW_THRESHOLD) {
    Serial.println("Moisture critically low! Triggering Pump.");
    isPumping = true;
    pumpStartTime = currentMillis;
    digitalWrite(PUMP_PIN, HIGH);     // Turn on Blue LED (Pump)
    digitalWrite(WARNING_PIN, HIGH);  // Turn on Red LED (Warning)
    updateDisplay(); // Force immediate screen update
  }

  // Turn OFF logic (Hysteresis high bound OR Safety Timeout)
  if (isPumping) {
    bool reachedTarget = (currentMoisturePercent >= MOISTURE_HIGH_THRESHOLD);
    bool timeoutReached = (currentMillis - pumpStartTime >= PUMP_MAX_RUNTIME);

    if (reachedTarget || timeoutReached) {
      if (timeoutReached && !reachedTarget) {
        Serial.println("SAFETY TIMEOUT: Pump ran too long! Shutting down.");
      } else {
        Serial.println("Target moisture reached. Pump off.");
      }
      
      isPumping = false;
      digitalWrite(PUMP_PIN, LOW); // Turn off Blue LED (Pump)
      updateDisplay(); // Force immediate screen update
    }
  }

  // Red LED stays on as a warning if moisture is below low threshold, 
  // even if pump is resting or off due to a safety timeout.
  if (currentMoisturePercent <= MOISTURE_LOW_THRESHOLD) {
    digitalWrite(WARNING_PIN, HIGH);
  } else {
    digitalWrite(WARNING_PIN, LOW);
  }
}

void updateDisplay() {
  display.clearDisplay();
  
  // Row 1: Temperature & Humidity
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(currentTemp, 1); // 1 decimal place
  display.print(" C");

  display.setCursor(0, 12);
  display.print("Hum:  ");
  display.print(currentHum, 1);
  display.print(" %");

  // Row 2: Soil Moisture (Large Text)
  display.setCursor(0, 30);
  display.print("Soil Moisture:");
  display.setTextSize(2);
  display.setCursor(0, 42);
  display.print(currentMoisturePercent);
  display.print("%");

  // Indicator if pumping
  if (isPumping) {
    display.setTextSize(1);
    display.setCursor(80, 50);
    display.print("[PUMP]");
  }

  display.display();
}

void printToSerial() {
  Serial.print("Temp: "); Serial.print(currentTemp); Serial.print("C | ");
  Serial.print("Humidity: "); Serial.print(currentHum); Serial.print("% | ");
  Serial.print("Soil: "); Serial.print(currentMoisturePercent); Serial.print("% | ");
  Serial.print("Pump Status: "); Serial.println(isPumping ? "ON" : "OFF");
}