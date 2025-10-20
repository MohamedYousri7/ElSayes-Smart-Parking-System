#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

// -------- Configuration --------
#define NUM_LEDS 5
#define DATA_PIN 5

// IR sensor pins
#define IR_PIN_1 25
#define IR_PIN_2 33
#define IR_PIN_3 32
#define IR_PIN_4 35
#define IR_PIN_5 34

// WiFi credentials
const char* ssid = "Pioneer";
const char* password = "P@ION24@EER";

// Supabase details
const char* supabase_url = "https://ydcclknvaziivylayjrd.supabase.co";
const char* supabase_api_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InlkY2Nsa252YXppaXZ5bGF5anJkIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NDgxODQ0OTgsImV4cCI6MjA2Mzc2MDQ5OH0.S-TgjclBVPOo3SOD0zAkARIPU8advwS8XiesthEE424";

// LED array
CRGB leds[NUM_LEDS];
bool lastOccupied[NUM_LEDS] = {false, false, false, false, false};

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// IR pins array
const int IR_PINS[NUM_LEDS] = {IR_PIN_1, IR_PIN_2, IR_PIN_3, IR_PIN_4, IR_PIN_5};

struct SlotStatus {
  bool is_reserved;
};

SlotStatus slotStates[NUM_LEDS];

void resetAllOccupancy() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String endpoint = String(supabase_url) + "/rest/v1/slots?is_occupied=eq.true"; // Only update slots marked as occupied
  String payload = "{\"is_occupied\": false}";

  http.begin(endpoint);
  http.addHeader("apikey", supabase_api_key);
  http.addHeader("Authorization", "Bearer " + String(supabase_api_key));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=representation");

  int httpResponseCode = http.PATCH(payload);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("All slots reset to unoccupied: " + response);
  } else {
    Serial.println("Reset error: " + String(httpResponseCode));
  }

  http.end();
}


void sendPatchToSupabase(int slotId, bool isOccupied, bool clearReservation = false) {
  if (WiFi.status() != WL_CONNECTED) return;

  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm* timeInfo = gmtime((time_t*)&epochTime);
  char isoTimestamp[25];
  strftime(isoTimestamp, sizeof(isoTimestamp), "%Y-%m-%dT%H:%M:%SZ", timeInfo);

  HTTPClient http;
  String endpoint = String(supabase_url) + "/rest/v1/slots?id=eq." + String(slotId);
  String payload = "{\"is_occupied\": " + String(isOccupied ? "true" : "false") +
                   ", \"last_updated\": \"" + String(isoTimestamp) + "\"";
  if (clearReservation) payload += ", \"is_reserved\": false";
  payload += "}";

  http.begin(endpoint);
  http.addHeader("apikey", supabase_api_key);
  http.addHeader("Authorization", "Bearer " + String(supabase_api_key));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=representation");

  int httpResponseCode = http.PATCH(payload);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("PATCH response (" + String(slotId) + "): " + response);
  } else {
    Serial.println("PATCH error: " + String(httpResponseCode));
  }
  http.end();
}

void fetchReservationStates() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String endpoint = String(supabase_url) + "/rest/v1/slots?select=id,is_reserved";
  http.begin(endpoint);
  http.addHeader("apikey", supabase_api_key);
  http.addHeader("Authorization", "Bearer " + String(supabase_api_key));

  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      for (JsonObject obj : doc.as<JsonArray>()) {
        int id = obj["id"];
        if (id >= 1 && id <= NUM_LEDS) {
          slotStates[id - 1].is_reserved = obj["is_reserved"];
        }
      }
    } else {
      Serial.println("JSON parse error.");
    }
  } else {
    Serial.println("GET error: " + String(httpResponseCode));
  }
  http.end();
}

void updateSlot(int index, bool isOccupied) {
  bool wasOccupied = lastOccupied[index];
  bool isReserved = slotStates[index].is_reserved;

  // Determine LED color
  if (isOccupied && !isReserved) {
    leds[index] = CRGB::Green;
  } else if (!isOccupied && isReserved) {
    leds[index] = CRGB::Orange;
  } else if (isOccupied && isReserved) {
    leds[index] = CRGB::Red;
    // Mark reservation as fulfilled
    sendPatchToSupabase(index + 1, true, true);
  } else if (!isOccupied && !isReserved) {
    leds[index] = CRGB::Red;
  }

  // Send PATCH only if occupancy changed
  if (isOccupied != wasOccupied) {
    sendPatchToSupabase(index + 1, isOccupied);
    lastOccupied[index] = isOccupied;
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  timeClient.begin();
  timeClient.update();

  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(IR_PINS[i], INPUT);
  }
  resetAllOccupancy();
}

unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 5000; // fetch reservation status every 5s

void loop() {

  if (millis() - lastFetchTime > fetchInterval) {
    fetchReservationStates();
    lastFetchTime = millis();
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    bool isOccupied = !digitalRead(IR_PINS[i]);
    updateSlot(i, isOccupied);
  }

  FastLED.show();
  delay(500);
}
