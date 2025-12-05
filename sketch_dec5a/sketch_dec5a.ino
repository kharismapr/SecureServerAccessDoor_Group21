#define BLYNK_TEMPLATE_ID "TMPL6t6Qt2adf"
#define BLYNK_TEMPLATE_NAME "Smart Door"
#define BLYNK_AUTH_TOKEN "F71Ig8gxYu6zX5nGbkAJ_IvRtkln-Deh"
#define BLYNK_PRINT Serial 

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <NimBLEDevice.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Drowsy";
char pass[] = "sleepyhead";

#define PIN_DHT 15       
#define PIN_DOOR_LED 2   
#define PIN_RFID_SS 5
#define PIN_RFID_RST 4   

DHT dht(PIN_DHT, DHT11); 
MFRC522 mfrc522(PIN_RFID_SS, PIN_RFID_RST); 

SemaphoreHandle_t doorMutex; 
TaskHandle_t blynkTaskHandle;
TaskHandle_t rfidTaskHandle;
TaskHandle_t tempTaskHandle;

float currentTemp = 0.0;
const float TEMP_THRESHOLD = 30.0; 

// Buka Pintu
void openDoor(String source) {
  if (xSemaphoreTake(doorMutex, (TickType_t)100) == pdTRUE) {
    Serial.print("[DOOR] Opening via: "); Serial.println(source);
    digitalWrite(PIN_DOOR_LED, HIGH); 
    
    vTaskDelay(3000 / portTICK_PERIOD_MS); 
    
    digitalWrite(PIN_DOOR_LED, LOW);  
    Serial.println("[DOOR] Locked.");
    
    xSemaphoreGive(doorMutex); 
  } else {
    Serial.println("[DOOR] Busy! (Mutex Locked)");
  }
}

// RFID Sensor
void taskRFID(void *pvParameters) {
  for (;;) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      Serial.println("[RFID] Card Detected!");
      
      openDoor("RFID Reader");
      
      mfrc522.PICC_HaltA(); 
      mfrc522.PCD_StopCrypto1();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Sensor DHT11
void taskTemp(void *pvParameters) {
  dht.begin();
  for (;;) {
    float t = dht.readTemperature();
    if (!isnan(t)) {
      currentTemp = t;
      if (currentTemp > TEMP_THRESHOLD) {
        Blynk.logEvent("overheat", "Critical Temp!"); 
      }
      Blynk.virtualWrite(V1, currentTemp);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

// Blynk
void taskBlynk(void *pvParameters) {
  Blynk.begin(auth, ssid, pass);
  for (;;) {
    Blynk.run();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// BLE
class MyCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        if (value == "OPEN") {
             openDoor("BLE Admin");
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
                                         NIMBLE_PROPERTY::READ |
                                         NIMBLE_PROPERTY::WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  
  NimBLEAdvertisementData scanResponse;
  scanResponse.setName("ServerDoor"); 
  pAdvertising->setScanResponseData(scanResponse);
  
  pAdvertising->start();
  Serial.println("[BLE] Ready (NimBLE)");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DOOR_LED, OUTPUT);
  doorMutex = xSemaphoreCreateMutex();

  SPI.begin(); 
  mfrc522.PCD_Init();
  
  // Debug RFID Connection
  Serial.print("[INIT] RFID Firmware Version: ");
  mfrc522.PCD_DumpVersionToSerial(); // KALAU 0x0 or 0xFF, MASALAH WIRING
  
  // High sensitivity
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  Serial.println("[INIT] RFID Ready");

  setupBLE();

  // Mutex stuff
  xTaskCreate(taskRFID, "RFID_Task", 2048, NULL, 1, &rfidTaskHandle);
  xTaskCreate(taskTemp, "Temp_Task", 4096, NULL, 1, &tempTaskHandle);
  xTaskCreate(taskBlynk, "Blynk_Task", 6000, NULL, 2, &blynkTaskHandle);
}

void loop() {
}