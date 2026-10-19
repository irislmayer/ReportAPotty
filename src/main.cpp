#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFiS3.h>
#include <Wire.h>
#include <WiFiUdp.h> 

// Device Identity Settings
const char* DEVICE_ID = "All Gender Hall Bathroom";

// WiFi credentials
const char* ssid = "QuantumFiber4390";
const char* password = "adb4bf4f74eb66";

// Server and Port
const char* serverAddress = "192.168.0.71"; 
const int port = 8080; 

// Hardware Pins
const int BUTTON_PIN = 4;

// State Variables
int lastButtonState = HIGH;
unsigned long lastDisplayUpdateTime = 0;
bool showButtonMessage = false;
bool alertSentThisPress = false; 

// Double-click Timing Variables
unsigned long lastPressTime = 0;   
const int DOUBLE_CLICK_TIME = 250; 

// Create U8g2 object for SSD1306 128x64 using Hardware I2C on Wire1 (Qwiic Port)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

// Create the UDP object instance
WiFiUDP udpClient;

// Function Declarations
void checkButton();
void updateDisplay();
bool connectToWiFi();
void sendLocalNotification();

void setup() {
  Serial.begin(9600);
  delay(1000); 
  Serial.println("Initializing System (U8g2 UDP)...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  u8g2.setI2CAddress(0x3C * 2); 
  u8g2.setBusClock(100000);     
  u8g2.begin();
  Serial.println("[OK] U8g2 OLED Display Initialized");

  connectToWiFi();
}

void loop() {
  bool oldState = showButtonMessage;
  checkButton();

  // Instant display reactions
  if (oldState != showButtonMessage) {
    updateDisplay(); 
  }

  // Periodic visual refresh loop
  unsigned long currentMillis = millis();
  if (currentMillis - lastDisplayUpdateTime >= 1000) {
    lastDisplayUpdateTime = currentMillis;
    updateDisplay();
  }
}

bool connectToWiFi() {
  Serial.println("\n========== WiFi Connection Start ==========");
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr); 
  u8g2.drawStr(0, 12, "Connecting WiFi...");
  u8g2.sendBuffer();
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  u8g2.clearBuffer();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[SUCCESS] WiFi Connected!");
    u8g2.drawStr(0, 12, "WiFi Connected!");
    u8g2.drawStr(0, 28, "IP Address:");
    u8g2.drawStr(0, 44, WiFi.localIP().toString().c_str());
    u8g2.sendBuffer();
    delay(2000);
    
    // Boot up the UDP client 
    udpClient.begin(port);
    return true;
  } else {
    Serial.println("[FAILED] WiFi Connection Failed!");
    u8g2.drawStr(0, 12, "WiFi Failed!");
    u8g2.sendBuffer();
    delay(2000);
    return false;
  }
}

void checkButton() {
  int currentButtonState = digitalRead(BUTTON_PIN);
  
  if (currentButtonState != lastButtonState) {
    if (currentButtonState == HIGH) { 
      unsigned long currentMillis = millis();
      
      if (showButtonMessage) {
        if (currentMillis - lastPressTime < DOUBLE_CLICK_TIME) {
          Serial.println("Button Press: Double Click. Resetting to Idle.");
          Serial.flush(); // <-- To fix issues with data not showing in serial monitor
          showButtonMessage = false;
          alertSentThisPress = false; 
        }
      } else {
        Serial.println("Button Press: Single Click. Entering Alert State.");
        Serial.flush(); // <-- To fix issues with data not showing in serial monitor
        showButtonMessage = true;
        
        if (WiFi.status() == WL_CONNECTED && !alertSentThisPress) {
          sendLocalNotification();
          alertSentThisPress = true; 
        }
      }
      lastPressTime = currentMillis;
    }
    lastButtonState = currentButtonState;
    delay(50); 
  }
}

void sendLocalNotification() {
  Serial.println("Network Action: Firing Fire-and-Forget UDP Packet!");
  Serial.flush();
  
  udpClient.beginPacket(serverAddress, port);
  udpClient.print(DEVICE_ID); 
  udpClient.endPacket();
  
  Serial.println("Packet cleared tx queue.");
  Serial.flush();
}

void updateDisplay() {
  u8g2.clearBuffer(); 
  
  // ---- 1. Display Name of Bathroom ----
  u8g2.setFont(u8g2_font_profont10_tf); 
  u8g2.drawStr(3, 10, DEVICE_ID);    
  u8g2.drawHLine(0, 13, 128); 
  
  // ---- 2. Message Center ----
  if (showButtonMessage) {
    u8g2.setFont(u8g2_font_helvB10_tf); 
    u8g2.drawStr(0, 32, "Alert Sent!");
    u8g2.setFont(u8g2_font_profont12_tf);     
    u8g2.drawStr(0, 46, "Staff on the way!");
  } else {
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(0, 32, "Needs attention?");
    u8g2.setFont(u8g2_font_profont12_tf);
    u8g2.drawStr(0, 46, "Press button to alert");
    u8g2.drawStr(0, 56, "staff");
  }
  
  // ---- 3. CUSTOM WIFI ICON ----
  if (WiFi.status() == WL_CONNECTED) {
    u8g2.drawCircle(115, 60, 2, U8G2_DRAW_ALL);
    u8g2.drawCircle(115, 60, 6, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    u8g2.drawCircle(115, 60, 10, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
  } else {
    u8g2.drawLine(110, 50, 120, 60);
    u8g2.drawLine(120, 50, 110, 60);
  }
  u8g2.sendBuffer(); 
}