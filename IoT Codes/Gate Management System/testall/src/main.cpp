#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

#define IR_OUT 34
#define IR_IN 35

const int servo_Pin_out = 15;
const int servo_Pin_in = 4;

Servo servo_out;
Servo servo_in;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "Pioneer";
const char* password = "P@ION24@EER";

// Supabase details
const char* supabase_url = "https://ydcclknvaziivylayjrd.supabase.co";
const char* supabase_api_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InlkY2Nsa252YXppaXZ5bGF5anJkIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NDgxODQ0OTgsImV4cCI6MjA2Mzc2MDQ5OH0.S-TgjclBVPOo3SOD0zAkARIPU8advwS8XiesthEE424";

const char* table = "slots";

int lastIR_IN = HIGH;
int lastIR_OUT = HIGH;

void triggerEntryCameraRequest();
void triggerExitCameraRequest();
int getAvailableSlots();
void handleEntry();
void handleExit();
void displayAvailableSlots();

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  servo_out.attach(servo_Pin_out);
  servo_out.write(90); // Closed

  servo_in.attach(servo_Pin_in);
  servo_in.write(90); // Closed

  pinMode(IR_OUT, INPUT);
  pinMode(IR_IN, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
  displayAvailableSlots();  // Show default slot info
}

void loop() {
  int ir_in = digitalRead(IR_IN);
  int ir_out = digitalRead(IR_OUT);

  // Handle car entry
  if (ir_in == LOW && lastIR_IN == HIGH) {
    triggerEntryCameraRequest();
    int available = getAvailableSlots();
    if (available > 0) {
      handleEntry();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Garage Full!");
      delay(2000);
      displayAvailableSlots();
    }
  }

  // Handle car exit
  if (ir_out == LOW && lastIR_OUT == HIGH) {
    triggerExitCameraRequest();
    handleExit();
  }

  // Show default screen when idle
  if (ir_in == HIGH && ir_out == HIGH) {
    displayAvailableSlots();
    delay(1000);
  }

  lastIR_IN = ir_in;
  lastIR_OUT = ir_out;
}

// ========== DISPLAY SLOTS ==========

void displayAvailableSlots() {
  int available = getAvailableSlots();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slots: ");
  lcd.print(available);
}

// ========== CAMERA TRIGGER FUNCTIONS ==========

void triggerEntryCameraRequest() {
  HTTPClient http;
  String url = String(supabase_url) + "/rest/v1/camera_request?id=eq.1";

  http.begin(url);
  http.addHeader("apikey", supabase_api_key);
  http.addHeader("Authorization", "Bearer " + String(supabase_api_key));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=representation");

  String payload = "{\"request\": true}";

  int httpResponseCode = http.PATCH(payload);
  if (httpResponseCode > 0) {
    Serial.println("Entry camera request updated:");
    Serial.println(http.getString());
  } else {
    Serial.print("Failed to update entry camera request. HTTP error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void triggerExitCameraRequest() {
  HTTPClient http;
  String url = String(supabase_url) + "/rest/v1/camera_request?id=eq.2";

  http.begin(url);
  http.addHeader("apikey", supabase_api_key);
  http.addHeader("Authorization", "Bearer " + String(supabase_api_key));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=representation");

  String payload = "{\"request\": true}";

  int httpResponseCode = http.PATCH(payload);
  if (httpResponseCode > 0) {
    Serial.println("Exit camera request updated:");
    Serial.println(http.getString());
  } else {
    Serial.print("Failed to update exit camera request. HTTP error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// ========== GATE ACTIONS ==========

void handleEntry() {
  Serial.println("Car entering...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("El-Sayes");

  servo_in.write(180); // Open gate
  delay(2000);
  servo_in.write(90);  // Close gate

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gate Closed");
  delay(1000);
  displayAvailableSlots();
}

void handleExit() {
  Serial.println("Car exiting...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Goodbye!");

  servo_out.write(0); // Open gate
  delay(2000);
  servo_out.write(90); // Close gate

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gate Closed");
  delay(1000);
  displayAvailableSlots();
}

// ========== SLOT CHECK ==========

int getAvailableSlots() {
  HTTPClient http;
  String url = String(supabase_url) + "/rest/v1/" + table + "?select=is_occupied";
  http.begin(url);
  http.addHeader("apikey", supabase_api_key);
  http.addHeader("Authorization", "Bearer " + String(supabase_api_key));
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();
  int available = 0;

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      for (JsonObject slot : doc.as<JsonArray>()) {
        if (!slot["is_occupied"]) {
          available++;
        }
      }
    }
  } else {
    Serial.print("Error: ");
    Serial.println(httpCode);
  }

  http.end();
  return available;
}
