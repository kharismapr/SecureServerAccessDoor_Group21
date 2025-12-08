/*
  SMART DOOR LOCK ULTIMATE (LCD FIXED VERSION)
  Features: RFID + BLE (NimBLE) + WiFi + Google Sheets Log + LCD + RTOS
  Fixes: LCD initialization + I2C explicit init + Timing fixed
*/

// --- 1. SETTING BLYNK & WIFI ---
#define BLYNK_TEMPLATE_ID "TMPL6t6Qt2adf"
#define BLYNK_TEMPLATE_NAME "Smart Door"
#define BLYNK_AUTH_TOKEN "F71Ig8gxYu6zX5nGbkAJ_IvRtkln-Deh"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NimBLEDevice.h> 

char ssid[] = "iyeah"; 
char pass[] = "imaiot2023";
char auth[] = BLYNK_AUTH_TOKEN;

String GAS_URL = "https://script.google.com/macros/s/AKfycbyYRo2qSSc4vaLwUelyrSg4qRzgXy_aZtxuLINxRKdoxBi8YfZNgamiz0LHlgsrMpKt/exec"; 

#define PIN_DHT 15        
#define PIN_DOOR_LED 2    
#define PIN_RFID_SS 5
#define PIN_RFID_RST 4    

DHT dht(PIN_DHT, DHT11); 
MFRC522 mfrc522(PIN_RFID_SS, PIN_RFID_RST); 
LiquidCrystal_I2C lcd(0x27, 16, 2); 
SemaphoreHandle_t mutexBus; 

String authorizedUIDs[] = {"07a83825", "7af84e1b"}; 
int authorizedCount = 2;
float currentTemp = 0.0;
bool lcdReady = false;

void sendLogToGoogleSheets(String uid, String status) {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    String url = GAS_URL + "?uid=" + uid + "&status=" + status;
    
    http.begin(url);
    int httpCode = http.GET();
    http.end();
    Serial.print("[HTTP] Log Sent: "); Serial.println(httpCode);
  } else {
    Serial.println("[HTTP] Error: WiFi not connected");
  }
}

void openDoorTask(String source, String uid) {
  if (xSemaphoreTake(mutexBus, (TickType_t)100) == pdTRUE) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("ACCESS ALLOWED");
    lcd.setCursor(0, 1); lcd.print("Via: " + source); 
    
    digitalWrite(PIN_DOOR_LED, HIGH); 
    Blynk.virtualWrite(V0, 1);
    
    sendLogToGoogleSheets(uid, "Allowed");
    Blynk.virtualWrite(V2, "Open by " + source);

    xSemaphoreGive(mutexBus);
    
    vTaskDelay(3000 / portTICK_PERIOD_MS); 
    
    digitalWrite(PIN_DOOR_LED, LOW);
    Blynk.virtualWrite(V0, 0);
    
    if (xSemaphoreTake(mutexBus, (TickType_t)100) == pdTRUE) {
       lcd.clear(); 
       lcd.setCursor(0, 0); 
       lcd.print("Tap Your Card");
       xSemaphoreGive(mutexBus);
    }
  }
}

void denyAccessTask(String uid) {
  if (xSemaphoreTake(mutexBus, (TickType_t)100) == pdTRUE) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("ACCESS DENIED!");
    
    Blynk.logEvent("access_denied", "Unknown ID: " + uid);
    sendLogToGoogleSheets(uid, "Denied");
    
    xSemaphoreGive(mutexBus);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    if (xSemaphoreTake(mutexBus, (TickType_t)100) == pdTRUE) {
       lcd.clear(); 
       lcd.setCursor(0, 0); 
       lcd.print("Tap Your Card");
       xSemaphoreGive(mutexBus);
    }
  }
}

class MyCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue(); 
      if (value.length() > 0) {
        if (value == "OPEN") {
             Serial.println("[BLE] Command Received: OPEN");
             openDoorTask("BLE", "ADMIN_HP");
        }
      }
    }
};

void setupBLE() {
  NimBLEDevice::init("ServerDoor");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); 
  NimBLEServer *pServer = NimBLEDevice::createServer();
  NimBLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                           "beb5483e-36e1-4688-b7f5-ea07361b26a8",
                                           NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
                                         );
  pCharacteristic->setCallbacks(new MyCallbacks());
  NimBLEDescriptor* pDescriptor = pCharacteristic->createDescriptor("2901");
  pDescriptor->setValue("Door Unlock Switch"); 

  pService->start();
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  
  NimBLEAdvertisementData scanResponse;
  scanResponse.setName("ServerDoor"); 
  pAdvertising->setScanResponseData(scanResponse);
  
  pAdvertising->start();
  Serial.println("[BLE] Bluetooth Ready!");
}

void taskRFID(void *pvParameters) {
  // Tunggu LCD ready
  while(!lcdReady) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  
  for (;;) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      String uidRaw = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidRaw += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uidRaw += String(mfrc522.uid.uidByte[i], HEX);
      }
      
      Serial.print("[RFID] Card detected: ");
      Serial.println(uidRaw);
      
      bool authorized = false;
      for (int i = 0; i < authorizedCount; i++) {
        if (uidRaw == authorizedUIDs[i]) authorized = true;
      }

      if (authorized) openDoorTask("RFID", uidRaw);
      else denyAccessTask(uidRaw);
      
      mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void taskTemp(void *pvParameters) {
  // Tunggu LCD ready
  while(!lcdReady) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  
  dht.begin();
  
  for (;;) {
    float t = dht.readTemperature();
    if (!isnan(t)) {
      currentTemp = t;
      Blynk.virtualWrite(V1, currentTemp);
      
      if (xSemaphoreTake(mutexBus, (TickType_t)50) == pdTRUE) {
         lcd.setCursor(0, 1); 
         lcd.print("Temp:");
         lcd.print(t, 1);
         lcd.print("C  ");
         xSemaphoreGive(mutexBus);
      }
      
      if (currentTemp > 30.0) {
        Blynk.logEvent("overheat");
        Serial.println("[ALERT] Temperature > 30C!");
      }
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void taskBlynk(void *pvParameters) {
  Blynk.begin(auth, ssid, pass);
  for (;;) {
    Blynk.run();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n=== SMART DOOR LOCK STARTING ===");
  
  pinMode(PIN_DOOR_LED, OUTPUT);
  digitalWrite(PIN_DOOR_LED, LOW);
  
  mutexBus = xSemaphoreCreateMutex();
  
  // IMPORTANT: Initialize I2C explicitly for ESP32
  Wire.begin(); // SDA=21, SCL=22
  delay(100);
  
  Serial.println("[LCD] Initializing...");
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  
  // Welcome message
  lcd.setCursor(0, 0); 
  lcd.print("Smart Door Lock");
  lcd.setCursor(0, 1); 
  lcd.print("Starting...");
  Serial.println("[LCD] Welcome message displayed");
  
  delay(2000); // Biar kebaca dulu
  
  // Initialize SPI for RFID
  Serial.println("[RFID] Initializing...");
  SPI.begin(); 
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  Serial.println("[RFID] Ready!");
  
  // Initialize BLE
  Serial.println("[BLE] Initializing...");
  setupBLE();
  
  // Ready state
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Tap Your Card");
  lcd.setCursor(0, 1); 
  lcd.print("Ready!");
  Serial.println("[LCD] Ready screen displayed");
  
  delay(1000);
  
  lcdReady = true; // Baru sekarang task lain boleh akses LCD
  
  // Start RTOS tasks
  Serial.println("[RTOS] Starting tasks...");
  xTaskCreate(taskRFID, "RFID", 8192, NULL, 1, NULL);
  xTaskCreate(taskTemp, "Temp", 4096, NULL, 1, NULL);
  xTaskCreate(taskBlynk, "Blynk", 8192, NULL, 2, NULL);
  
  Serial.println("=== ALL SYSTEMS READY ===\n");
}

void loop() {
  // Empty - semua dihandle oleh RTOS tasks
}