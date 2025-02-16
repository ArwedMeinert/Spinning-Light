#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AM2302-Sensor.h>
#include <Encoder.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <FunctionalInterrupt.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <BLE2902.h>



struct Config {
    uint32_t magicNumber = 0xA5A5A5A5;  // Identifier for validity
    int ftp = 0;
    float brightness = 0.5;
    char bt_name[20]="";
    char bt_adress[20] = "00:00:00:00"; // Changed to char array for compatibility
    int num_pixel = 10;
    bool temperature_sensor = true;
    bool first_start = true;
    float z1_percentage = 0.6;
    float z2_percentage = 0.75;
    float z3_percentage = 0.9;
    float z4_percentage = 1.05;
    float z5_percentage = 1.2;
    float z6_percentage = 1.3;
    float filter_constant = 0.1;
    float spark_constant = 0.6;
};


Config configDefault; // Default configuration
Config configData;    // Active configuration
Config configDataBackup; // Backup for changes




// GUI
enum Mode { NORMAL, SELECT, ADJUST,BTSCAN };
Mode currentMode = NORMAL;
int selectedVariable = 0;    // Index of the currently selected variable
bool inAdvancedTab = false;  // Track if the advanced tab is selected
int scrollOffset = 0;        // Tracks the starting index of the visible menu items
int mainPage=0;
const int MAX_VISIBLE_ITEMS = 4; // Number of items to show at a time
const uint8_t checkMark[] = {
  0x3C, 0x42, 0x42, 0x42, 0x3C // Custom checkmark bitmap (5x5)
};

const uint8_t crossMark[] = {
  0x42, 0x42, 0x42, 0x42, 0x42 // Custom cross bitmap (5x5)
};


// Encoder
Encoder myEnc(10, 13);
long oldPosition = -999;
bool isButtonPressed = false;
long lastUpdateMillis = 0;
const int ENCODER_STEP_THRESHOLD = 6;  // Set the number of steps needed before an action is taken

// Neopixel
#define PIN        5 // On Trinket or Gemma, suggest changing this to 1
CRGB* leds = nullptr;
uint8_t currentPixelCount = 0;
//Colours
CRGB off=CRGB(0,0,0);
CRGB z1_colour=CRGB(255,255,255); //white
CRGB z2_colour=CRGB(0,0,255); //blue
CRGB z3_colour=CRGB(0,255,0); //green
CRGB z4_colour=CRGB(255,255,0); // yellow
CRGB z5_colour=CRGB(255,130,0); //orange
CRGB z6_colour=CRGB(255,0,0); // red
CRGB z7_colour=CRGB(255,0,255); //purple
float zone1_middle;
float zone2_middle;
float zone3_middle;
float zone4_middle;
float zone5_middle;
float zone6_middle;
float zone7_start;
float power;
float prevPower;

// OLED
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C address for the display
#define SDA_PIN 21
#define SCL_PIN 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// AM2302 Sensor
AM2302::AM2302_Sensor am2302(17); // Replace '0' with the correct pin connected to the sensor
int status = -1;

//BTLE
BLEClient* client;
BLEScan* pBLEScan;
int scanTime=8;
int deviceCount = 0;
const int MAX_DEVICES = 10;
bool connected=false;
BLEAdvertisedDevice foundDevices[MAX_DEVICES]; // Use pointers to manage memory
static BLEUUID serviceUUID("1818");  // Power Service UUID (16-bit)
static BLEUUID charUUID("2A63");
bool startup=true;

void notifyCallback(BLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool isNotify) {
  if (length >= 4) {
    // Parse the power value (Watts)
    uint16_t power_measurement = data[2] | (data[3] << 8);
    Serial.print("Power: ");
    Serial.print(power_measurement);
    prevPower=power;
    power=power_measurement;
    Serial.println(" W");
  }
}


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
  if (deviceCount < MAX_DEVICES && advertisedDevice.getName() != "" && advertisedDevice.isAdvertisingService(serviceUUID)) {
    foundDevices[deviceCount] = advertisedDevice; // Store by value
    deviceCount++;
  }
}
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {}

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

void ARDUINO_ISR_ATTR handleKey() {
  //detachInterrupt(36);
    isButtonPressed = true;
}

bool connecting_bt(){
  display.clearDisplay();
  display.setTextSize(2.5);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0,0) ;
  display.print("Connecting");
  display.setCursor(0,int(SCREEN_HEIGHT/2) );
  display.print("Device");
  display.display();
  BLEAddress deviceAddress(configData.bt_adress); // Replace with your device's MAC address
    Serial.println(deviceAddress.toString());
  client = BLEDevice::createClient();
  client->disconnect();
  client->setClientCallbacks(new MyClientCallback());
  delay(200);
  if (!client->connect(deviceAddress)) {
    Serial.println("Failed to connect to the power trainer.");
    
    return false;
  }
  Serial.println("Connected to power trainer!");
// Access the Power Service
  display.clearDisplay();
  display.setTextSize(2.5);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0,0) ;
  display.print("Connecting");
  display.setCursor(0,int(SCREEN_HEIGHT/2) );
  display.print("Power Service");
  display.display();
  BLERemoteService *powerService = client->getService(serviceUUID);
if (powerService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    client->disconnect();
    return false;
  }

// Step 3: Access the Power Measurement Characteristic from the service
display.clearDisplay();
display.setTextSize(2.5);             // Normal 1:1 pixel scale
display.setTextColor(SSD1306_WHITE);  // Draw white text
display.setCursor(0, 0);
display.print("Access");
display.setCursor(0, int(SCREEN_HEIGHT / 2));
display.print("Power");
display.display();

BLERemoteCharacteristic* powerCharacteristic = powerService->getCharacteristic(charUUID);
if (powerCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    client->disconnect();
    return false;
  }
// Register the notify callback
powerCharacteristic->registerForNotify(notifyCallback);
  
Serial.println("Subscribed to power updates.");
return true;
}





void calculateZones(){
float ftp=configData.ftp;
zone1_middle=(ftp*configData.z1_percentage)/2;
zone2_middle=(ftp*configData.z1_percentage)+((ftp*configData.z2_percentage)-(ftp*configData.z1_percentage))/2;
zone3_middle=(ftp*configData.z2_percentage)+((ftp*configData.z3_percentage)-(ftp*configData.z2_percentage))/2;
zone4_middle=(ftp*configData.z3_percentage)+((ftp*configData.z4_percentage)-(ftp*configData.z3_percentage))/2;
zone5_middle=(ftp*configData.z4_percentage)+((ftp*configData.z5_percentage)-(ftp*configData.z4_percentage))/2;
zone6_middle=(ftp*configData.z5_percentage)+((ftp*configData.z6_percentage)-(ftp*configData.z5_percentage))/2;
zone7_start=(ftp*configData.z6_percentage)+((ftp*configData.z6_percentage)-(ftp*configData.z5_percentage))/2;
}

CRGB blendColors(CRGB color1, CRGB color2, float ratio) {
    // Clamp ratio to the range 0.0 to 1.0
    ratio = constrain(ratio, 0.0, 1.0);

    // Linearly interpolate each RGB component
    return CRGB(
        (1 - ratio) * color1.r + ratio * color2.r,
        (1 - ratio) * color1.g + ratio * color2.g,
        (1 - ratio) * color1.b + ratio * color2.b
    );
}

void showPower(){
  bool rising=(power>prevPower);
  CRGB newColor;
  float filterConst=configData.filter_constant;
  float sparkConst=configData.spark_constant;

if (power<zone1_middle){
  newColor=blendColors(off,z1_colour,1-(zone1_middle-power)/zone1_middle);
} else if (power>zone1_middle && power <zone2_middle){
   newColor=blendColors(z1_colour,z2_colour,1-(zone2_middle-power)/(zone2_middle-zone1_middle));
}else if (power>zone2_middle && power <zone3_middle){
   newColor=blendColors(z2_colour,z3_colour,1-(zone3_middle-power)/(zone3_middle-zone2_middle));
}else if (power>zone3_middle && power <zone4_middle){
   newColor=blendColors(z3_colour,z4_colour,1-(zone4_middle-power)/(zone4_middle-zone3_middle));
}else if (power>zone4_middle && power <zone5_middle){
   newColor=blendColors(z4_colour,z5_colour,1-(zone5_middle-power)/(zone5_middle-zone4_middle));
}else if (power>zone5_middle && power <zone6_middle){
   newColor=blendColors(z5_colour,z6_colour,1-(zone6_middle-power)/(zone6_middle-zone5_middle));
}else if (power>zone6_middle){
   newColor=blendColors(z6_colour,z7_colour,1-(zone7_start-power)/(zone7_start-zone6_middle));
}else if (power>zone7_start){
   newColor=z7_colour;
}


if (rising) {
    for (int i = 0; i < currentPixelCount; i++) {
        leds[i] = blendColors(leds[i], newColor, filterConst * (currentPixelCount - i) / currentPixelCount);
    }
} else {
    for (int i = 0; i < currentPixelCount; i++) {
        leds[i] = blendColors(leds[i], newColor, (filterConst * i+1) / currentPixelCount);
    }
}

if (power > zone6_middle) {
    // Calculate the upper limit for randomizing the number of pixels
    float powerAboveThreshold = power - zone6_middle;
    int maxRandomPixels = map(powerAboveThreshold, 0, configData.ftp * 3, 1, currentPixelCount) * sparkConst+0.5; 

    // Generate a random number of pixels to change
    int numRandomPixels = random(0, maxRandomPixels + 1); // Ensure at least 1 pixel changes

    for (int i = 0; i < numRandomPixels; i++) {
        // Choose a random pixel index
        int randomIndex = random(0, currentPixelCount);
        
        // Set the pixel to either red or yellow
        if (random(0, 2) == 0) {
            leds[randomIndex] = CRGB::Red;
        } else {
            leds[randomIndex] = CRGB::Yellow;
        }
    }
}
FastLED.show();
}

void saveConfiguration() {
    // Write the configuration to EEPROM
    int address = 0;
    EEPROM.writeBytes(address, &configData, sizeof(Config));
    EEPROM.commit();
    Serial.println("Configuration saved to EEPROM.");
}

void loadConfiguration() {
    int address = 0;
    Config tempConfig;

    // Read the configuration from EEPROM
    EEPROM.readBytes(address, &tempConfig, sizeof(Config));

    // Check for a valid magic number
    if (tempConfig.magicNumber == 0xA5A5A5A5) {
        configData = tempConfig;
        Serial.println("Loaded configuration from EEPROM.");
    } else {
        Serial.println("No valid configuration found. Initializing defaults...");
        initializeDefaultConfig();
    }

    // Backup the loaded configuration
    configDataBackup = configData;
}

void initializeDefaultConfig() {
    configData = configDefault; // Load default settings
    saveConfiguration();        // Save to EEPROM
}

void writeBT( int x_pos, int y_pos) {
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(x_pos, y_pos);     // Start at specified position


  // Display BT address and BT name from configData

  display.println("Addr:");
  display.setTextSize(1);
  display.println(configData.bt_adress);  // Display BT address
display.setTextSize(2);
  display.println("Name:");
  display.setTextSize(1);
  display.println(configData.bt_name);  // Display BT name
  // Display Bluetooth connection status symbol in top-right corner

  display.setCursor(SCREEN_WIDTH - 24, 0); // Position it at top-right corner (16 pixels for the symbol size)

  if (connected) {
    display.print("C");
  } else {
    display.print("NC");
  }
}


void writeData(float temperature, float humidity, int x_pos, int y_pos) {
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(x_pos, y_pos);     // Start at specified position
if (configData.temperature_sensor){
  // Display temperature and humidity
  display.print("T:");
  display.println(String(temperature, 1));
display.setCursor(0, 16);
  display.print("H:");
  display.print(String(humidity, 0));
  display.println(" %");

  // Display BT address and BT name from configData
  y_pos += 32; // Move down for BT info (adjust position as needed)
  display.setTextSize(1);  // Smaller text size for BT address and name
  display.setCursor(x_pos, y_pos);

  display.print("Addr:");
  display.println(configData.bt_adress);  // Display BT address

  y_pos += 15;  // Adjust position for BT name
  display.setCursor(x_pos, y_pos);

  display.print("Name:");
  display.println(configData.bt_name);  // Display BT name
  // Display Bluetooth connection status symbol in top-right corner
} else {
  display.println("Temperature Sensor");
  display.println("tuned Off");
}
  display.setCursor(SCREEN_WIDTH - 24, 0); // Position it at top-right corner (16 pixels for the symbol size)

  if (connected) {
    display.print("C");
  } else {
    display.print("NC");
  }
}
void writePower(int x_pos, int y_pos) {
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(x_pos, y_pos);     // Start at specified position
  // Display temperature and humidity
  display.println("Power:");
display.setCursor(0, 16);
  display.print(String(power));
  display.println(" W");

  display.setCursor(SCREEN_WIDTH - 24, 0); // Position it at top-right corner (16 pixels for the symbol size)

  if (connected) {
    display.print("C");
  } else {
    display.print("NC");
  }
}
void initializeFastLED() {
    // Cleanup the previous LED array if it exists
    if (leds != nullptr) {
        delete[] leds;
        leds = nullptr;
    }

    // Allocate new LED array based on configData.num_pixel
    currentPixelCount = configData.num_pixel;
    leds = new CRGB[currentPixelCount];

    // Set up FastLED with the new configuration
    FastLED.addLeds<WS2812, PIN, GRB>(leds, currentPixelCount);
    FastLED.setBrightness(int(configData.brightness * 255));

    // Clear LEDs on initialization
    FastLED.clear();
    FastLED.show();
}



void drawPixels() {
  FastLED.clear();

  // Map pixel indices to a power value between zone1_middle and zone6_middle.
  // (Make sure calculateZones() has already been called so that these values are set.)
  for (int i = 0; i < currentPixelCount; i++) {
    // Compute a value along the power range. The first pixel will correspond to zone1_middle,
    // the last pixel to zone6_middle.
    float mappedValue = zone1_middle + (zone7_start) * (i / float(currentPixelCount - 1));
    CRGB color;

    // Decide which zone segment this mapped value falls into and blend accordingly.
    if (mappedValue <= zone2_middle) {
      // Between zone 1 and zone 2.
      float ratio = (mappedValue - zone1_middle) / (zone2_middle - zone1_middle);
      color = blendColors(z1_colour, z2_colour, ratio);
    } else if (mappedValue <= zone3_middle) {
      // Between zone 2 and zone 3.
      float ratio = (mappedValue - zone2_middle) / (zone3_middle - zone2_middle);
      color = blendColors(z2_colour, z3_colour, ratio);
    } else if (mappedValue <= zone4_middle) {
      // Between zone 3 and zone 4.
      float ratio = (mappedValue - zone3_middle) / (zone4_middle - zone3_middle);
      color = blendColors(z3_colour, z4_colour, ratio);
    } else if (mappedValue <= zone5_middle) {
      // Between zone 4 and zone 5.
      float ratio = (mappedValue - zone5_middle) / (zone6_middle - zone5_middle);
      color = blendColors(z4_colour, z5_colour, ratio);
    }  else if (mappedValue <= zone6_middle){
    // Between zone 4 and zone 5.
      float ratio = (mappedValue - zone6_middle) / (zone7_start - zone6_middle);
      color = blendColors(z6_colour, z7_colour, ratio);
    }else {
      color = z7_colour;
    }

    leds[i] = color;
    Serial.println(i);
  }
  FastLED.show();
}




void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (!inAdvancedTab) {
    const char *menuItems[] = {"Normal Mode", "FTP", "Brightness", "BT Address", "Advanced Settings"};
    for (int i = 0; i < 5; i++) {
      if (i == selectedVariable) display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      else display.setTextColor(SSD1306_WHITE);

      display.setCursor(0, i * 10);
      display.print(menuItems[i]);
    }
  } else {
    const char *advMenuItems[] = {
      "Back", "Num Pixels", "Temp Sensor", "Z1 Percentage",
      "Z2 Percentage", "Z3 Percentage", "Z4 Percentage",
      "Z5 Percentage", "Z6 Percentage", "Filter Const",
      "Spark Const", "Reset"
    };

    for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
      int index = i + scrollOffset;
      if (index >= 12) break;

      if (index == selectedVariable) display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      else display.setTextColor(SSD1306_WHITE);

      display.setCursor(0, i * 10);
      display.print(advMenuItems[index]);
    }
  }

  display.display();
}


void adjustVariable() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  
  // Display different menu depending on whether we're in the advanced tab
  if (!inAdvancedTab) {
    const char *menuItems[] = {"Back", "FTP", "Brightness", "BT Address", "Advanced Settings"};
    display.println(menuItems[selectedVariable]);
  } else {
    const char *advVars[] = {
      "Back", "Num Pixels", "Temp Sensor", "Z1 Percentage", 
      "Z2 Percentage", "Z3 Percentage", "Z4 Percentage", 
      "Z5 Percentage", "Z6 Percentage", "Filter Const", 
      "Spark Const", "Reset"
    };
    display.println(advVars[selectedVariable]);
  }

  display.setCursor(0, 20);
  display.print("Value: ");
  
  if (!inAdvancedTab) {
    // Handle basic variables
    if (selectedVariable == 1) {
      display.print(configData.ftp);
    } else if (selectedVariable == 2) {
      display.print(configData.brightness, 2);
    } else if (selectedVariable == 3) {
      scanAndDisplayDevices();
    }
  } else {
    // Handle advanced variables
    if (selectedVariable == 1) {
      display.print(configData.num_pixel);
    } else if (selectedVariable == 2) {
      display.print(configData.temperature_sensor ? "true" : "false");
    } else if (selectedVariable >= 3 && selectedVariable <= 8) {
      // For Z1 to Z6 Percentage
      float *floatVars = &configData.z1_percentage;
      display.print(floatVars[selectedVariable - 3], 2);
    } else if (selectedVariable == 9) {
      display.print(configData.filter_constant, 2);
    } else if (selectedVariable == 10) {
      display.print(configData.spark_constant, 2);
    }
  }

  display.display();
}

void adjustAdvancedVariable(int index, int direction) {
 if (index == 10) {
    return;
  } else if (index == 0) {
    configData.num_pixel += direction;
    configData.num_pixel = max(0, configData.num_pixel); // Ensure non-negative
  } else if (index == 1) {
    configData.temperature_sensor = !configData.temperature_sensor; // Toggle boolean
  } else {
    float *floatVars = &configData.z1_percentage;
    floatVars[index - 2] += direction * 0.01; // Adjust percentages/constants
    floatVars[index - 2] = constrain(floatVars[index - 2], 0.0, 1.0); // Constrain
  }
  
  adjustVariable(); // Refresh the menu with updated values
}

void handleEncoderInput() {
  static long lastProcessedPosition = 0;
  long currentPosition = myEnc.read();

  if (abs(currentPosition - lastProcessedPosition) >= ENCODER_STEP_THRESHOLD) {
    int direction = (currentPosition > lastProcessedPosition) ? 1 : -1;
    lastProcessedPosition = currentPosition;

    if (currentMode==NORMAL){
      mainPage+=direction;
      if (mainPage>2){
        mainPage=0;
      } else if (mainPage<0){
        mainPage=2;
      }
    }
    if (currentMode == SELECT) {
      int menuSize = inAdvancedTab ? 12 : 5; // Advanced menu includes "Back"
      
      if (currentMode != BTSCAN) { // Prevent changes while scanning
        selectedVariable += direction;

        // Handle wrapping
        if (selectedVariable < 0) selectedVariable = menuSize - 1;
        else if (selectedVariable >= menuSize) selectedVariable = 0;

        // Handle scrolling
        if (inAdvancedTab) {
          if (selectedVariable < scrollOffset) {
            scrollOffset = selectedVariable;
          } else if (selectedVariable >= scrollOffset + MAX_VISIBLE_ITEMS) {
            scrollOffset = selectedVariable - MAX_VISIBLE_ITEMS + 1;
          }
        }

        drawMenu();
      }
    } else if (currentMode == ADJUST) {
      // Adjust values for the selected variable
      if (!inAdvancedTab) {
        if (selectedVariable == 1) {  // FTP
          configData.ftp += direction * 5;
        } else if (selectedVariable == 2) {  // Brightness
          configData.brightness += direction * 0.05;
          configData.brightness = constrain(configData.brightness, 0.0, 1.0); // Keep brightness between 0 and 1
          FastLED.setBrightness(int(configData.brightness * 255));
          drawPixels();
        }
        adjustVariable();
      } else {
        if (selectedVariable == 0) {
          // If "Back" is selected, return to the basic menu
          inAdvancedTab = false;
          selectedVariable = 4; // Set focus to "Advanced Settings" in the basic menu
        } else {
          // Adjust advanced variables
          adjustAdvancedVariable(selectedVariable - 1, direction);
        }
        // Ensure mode stays in ADJUST
        currentMode = ADJUST;  
      }
    } else if (currentMode == BTSCAN) {
      // If in BT scan mode, move through the devices
      selectedVariable += direction;

      if (selectedVariable < 0) selectedVariable = deviceCount;
      else if (selectedVariable >= deviceCount) selectedVariable = 0;

      drawScanResults();  // Update the display with the new selection
    }
  }
}



void handleButtonPress() {
  bool changed;
  
  if (isButtonPressed && millis() - lastUpdateMillis > 500) {
    isButtonPressed = false;
    lastUpdateMillis = millis();
    Serial.println("Button");
    if (currentMode == NORMAL) {
      configDataBackup = configData;
      currentMode = SELECT;
      selectedVariable = 0;
      drawMenu();
    } else if (currentMode == SELECT) {
      if (!inAdvancedTab && selectedVariable == 0) {
        currentMode = NORMAL;
        if (configData.first_start){
          configData.first_start = false;
        }
        if (hasConfigChanged()){
          saveConfiguration();
          calculateZones();
        }
        Serial.println(changed);
      } else if (selectedVariable == 4) { // Advanced Settings tab
        inAdvancedTab = true;
        selectedVariable = 0;
        drawMenu();
      } else if (inAdvancedTab && selectedVariable == 0) {
        // "Back" is selected in advanced settings: exit advanced menu
        inAdvancedTab = false;
        selectedVariable = 4;  // Move focus to "Advanced Settings" or another item
        currentMode = SELECT;  // Return to SELECT mode
        drawMenu();  // Refresh the screen
      } else if (!inAdvancedTab && selectedVariable == 3) {
        client->disconnect();
        currentMode = BTSCAN;  // Switch to BTSCAN mode
        deviceCount=0;
        scanAndDisplayDevices();  // Start scanning for devices
      }
      else if (inAdvancedTab && selectedVariable == 11) {
        // Reset selected in advanced settings: reset values
        resetConfig();
        inAdvancedTab = false;
        selectedVariable = 0;
        currentMode = NORMAL;  // Return to SELECT mode
      } else {
        currentMode = ADJUST;
        adjustVariable();  // Call this function to update the selected variable value
      }
    } else if (currentMode == ADJUST) {
      currentMode = SELECT;
      drawMenu();  // Return to SELECT mode and refresh the menu
    } else if (currentMode == BTSCAN) {
      // Handle button press while in BTSCAN mode
      if (selectedVariable == 0) {
        currentMode = SELECT;  // Return to SELECT mode
        selectedVariable = 3;  // Keep the focus on "BT Address"
        drawMenu();  // Refresh the screen
      }
      if (selectedVariable >= 0 && selectedVariable < deviceCount+1) {
        // Store the selected device's address

        strncpy(configData.bt_name, foundDevices[selectedVariable-1].getName().c_str(), sizeof(configData.bt_name) - 1);
        configData.bt_name[sizeof(configData.bt_name) - 1] = '\0'; // Ensure null termination


        strncpy(configData.bt_adress, foundDevices[selectedVariable-1].getAddress().toString().c_str(), sizeof(configData.bt_adress) - 1);
        configData.bt_adress[sizeof(configData.bt_adress) - 1] = '\0'; // Ensure null termination
        for (int i = 0; i < MAX_DEVICES; i++) {
        foundDevices[i] = BLEAdvertisedDevice();  // Reset to default state
        }
        if (hasConfigChanged()){
        saveConfiguration();
        }
        connected=connecting_bt();
        currentMode = SELECT;  // Return to SELECT mode
        selectedVariable = 3;  // Keep the focus on "BT Address"
        deviceCount=0;
        drawMenu();  // Refresh the screen
        }

    }
  }
  

}

void resetConfig() {
  configData=configDefault;
  // After reset, exit Advanced tab and return to basic menu
  inAdvancedTab = false;
  Mode currentMode = NORMAL;
  selectedVariable = 0;  // Index of the currently selected variable
  drawMenu();  // Redraw the menu after resetting
}

bool hasConfigChanged() {
  return memcmp(&configData, &configDataBackup, sizeof(configData)) != 0;
}


void scanAndDisplayDevices() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Scanning...");
  display.display();
pBLEScan->clearResults();
  // Start BLE scan for the set duration
  pBLEScan->start(scanTime, false);

  // Wait for scan to finish
  pBLEScan->stop();
  // After scanning is finished, display the results
  drawScanResults();
}


void drawScanResults() {
  display.clearDisplay();
  display.setTextSize(1);
  if (startup){
    currentMode=NORMAL;
    strncpy(configData.bt_name, configData.bt_name, sizeof(configData.bt_name) - 1);
    configData.bt_name[sizeof(configData.bt_name) - 1] = '\0'; // Ensure null termination

    strncpy(configData.bt_adress, configData.bt_adress, sizeof(configData.bt_adress) - 1);
    configData.bt_adress[sizeof(configData.bt_adress) - 1] = '\0'; // Ensure null termination
    return;
  }

  if (deviceCount == 0) {
    display.setCursor(0, 0);
    display.print("No devices found.");
    display.display();
    delay(2000); // Wait a bit before returning
    currentMode = SELECT; // Return to SELECT mode if no devices found
    selectedVariable = 3;  // Keep focus on BT Address
    drawMenu();
    return;
  }

  display.setCursor(0, 0);
  display.print("Devices found:");

  // First entry corresponds to "Back"
  if (selectedVariable == 0) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // Black text on white background for "Back"
    display.setCursor(0, 10);
    display.print("Back");

  } else {
    display.setTextColor(SSD1306_WHITE);  // Regular white text for "Back"
    display.setCursor(0, 10);
    display.print("Back");
  }

  // Display all the found devices
  for (int i = 0; i < deviceCount; i++) {
    int cursorY = 20 + (i * 10);  // Start from line 20 for devices

    if (i + 1 == selectedVariable) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // Highlight selected device
    } else {
      display.setTextColor(SSD1306_WHITE);  // Regular white text for unselected devices
    }

    display.setCursor(0, cursorY);
    display.print(i + 1); // Index
    display.print(": ");
    display.println(foundDevices[i].getName().c_str());  // Correct usage

    // Reset text color back to white after drawing the selected device
    if (i + 1 == selectedVariable) {
      display.setTextColor(SSD1306_WHITE);  // Restore white text
    }
  }

  display.display();
}








void setup() {
  Serial.begin(9600);
  BLEDevice::init("ESP32_BLE_Client");

  // Initialize BLE scan
  pBLEScan = BLEDevice::getScan();
  static MyAdvertisedDeviceCallbacks myDeviceCallbacks;
  pBLEScan->setAdvertisedDeviceCallbacks(&myDeviceCallbacks);
  pBLEScan->setActiveScan(true); // Active scan retrieves more data
  pBLEScan->clearResults();
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  pBLEScan->start(scanTime/2, false);     // Scan for 5 seconds
  pBLEScan->stop();  // Stop scanning
 
    
    Wire.begin(SDA_PIN, SCL_PIN);
    pinMode(26, INPUT_PULLUP);
    attachInterrupt(26, handleKey, RISING);
    
    // Initialize AM2302 Sensor
    configData.temperature_sensor=true;
    if (configData.temperature_sensor){
      am2302.begin();
    if (am2302.read() == 0) {
        Serial.println("AM2302 Sensor OK");
    } else {
        Serial.println("AM2302 Sensor failed to initialize");
        configData.temperature_sensor=false;
    }
    }else{
      mainPage=1;
    }

    // Initialize OLED Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Loop forever if display initialization fails
    }
    display.setRotation(2);
    display.clearDisplay();
    display.display();
     if (!EEPROM.begin(sizeof(Config))) {
        Serial.println("Failed to initialize EEPROM.");
        while (1) {
            delay(1000); // Infinite loop to halt the program
        }
    }

    // Load configuration
    loadConfiguration();
    // Setup additional logic for first-time configuration
    if (configData.first_start) {
        currentMode = SELECT;
        selectedVariable = 0;
        drawMenu(); // Open the menu for setup
        configData.first_start = false; // Mark as initialized
        //saveConfiguration(); // Save the updated state
    }

    Serial.println("Configuration ready.");
    initializeFastLED();
    calculateZones();
     //delay(10000); 
//scanAndDisplayDevices();
  //connected=connecting_bt();
}

float humidity=-999;
float temperature=-999;
long last_increase=0;
long last_try_connect=100000;


void loop() {
  if (startup){
    startup=false;
  }
  if (!connected && millis()-last_try_connect>200000){
    connected=connecting_bt();
    last_try_connect=millis();
  }
  handleEncoderInput();
  handleButtonPress();
  if (currentMode == NORMAL) {
    // Display temperature and humidity in NORMAL mode
    //noInterrupts();
    if (configData.temperature_sensor){
    status=am2302.read();
    if (status==0){
      humidity = am2302.get_Humidity();
      temperature = am2302.get_Temperature();
    }}
    display.clearDisplay();
    if (mainPage==0){
    writeData(temperature, humidity, 0, 0);
    }else if (mainPage==1){
      writePower(0,0);
    }else if (mainPage==2){
      writeBT(0,0);
    }else{
      mainPage=0;
    }
    display.display();
    showPower();
    
    //interrupts();
  }
}


