#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "time.h"

// ==== WiFi Credentials ====
const char* ssid = "OMAR";
const char* password = "OMAR2003$$"; 

// ==== Supabase Info ====
#define SUPABASE_URL "https://ydcclknvaziivylayjrd.supabase.co"
#define SUPABASE_BUCKET "entrance"
#define SUPABASE_API_KEY "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InlkY2Nsa252YXppaXZ5bGF5anJkIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTc0ODE4NDQ5OCwiZXhwIjoyMDYzNzYwNDk4fQ.tdxC5w8o9u0dNDvNvj8SUfVbL6rzOVoKmd0AHisNvm4"

// ==== NTP Settings ====
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3 * 3600  // UTC+3 = 3 hours * 3600 seconds
#define DAYLIGHT_OFFSET_SEC 0

// ==== Camera Pin Configuration ====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ==== WiFi Connection ====
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to WiFi!");
}

// ==== Time Sync ====
void initTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.print("Synchronizing time");
  while (time(nullptr) < 100000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime synchronized!");
}

// ==== Camera Initialization ====
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    ESP.restart();
  }
}

// ==== Check request from Supabase ====
bool shouldTakePhoto() {
  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/camera_request?id=eq.1&select=request";
  http.begin(url);
  http.addHeader("apikey", SUPABASE_API_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_API_KEY));

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Camera request response: " + response);
    http.end();
    return response.indexOf("true") != -1;
  } else {
    Serial.print("Failed to fetch camera request. HTTP error code: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

// ==== Set request to false ====
void markRequestFalse() {
  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/camera_request?id=eq.1";
  http.begin(url);
  http.addHeader("apikey", SUPABASE_API_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_API_KEY));
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.PATCH("[{\"request\": false}]");
  if (httpResponseCode > 0) {
    Serial.println("Marked request as false.");
  } else {
    Serial.println("Failed to mark request false.");
  }
  http.end();
}

// ==== Upload Image to Supabase ====
void uploadImageToSupabase(uint8_t* imageData, size_t length) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char timeStringBuff[40];
    strftime(timeStringBuff, sizeof(timeStringBuff), "photo_%Y-%m-%d_%H-%M-%S.jpg", timeinfo);
    String fileName = String(timeStringBuff);

    String fullURL = String(SUPABASE_URL) + "/storage/v1/object/" + SUPABASE_BUCKET + "/" + fileName;

    http.begin(fullURL);
    http.addHeader("Content-Type", "image/jpeg");
    http.addHeader("apikey", SUPABASE_API_KEY);
    http.addHeader("Authorization", "Bearer " + String(SUPABASE_API_KEY));
    http.addHeader("x-upsert", "true");

    int httpResponseCode = http.POST(imageData, length);
    if (httpResponseCode > 0) {
      Serial.println("✅ Image uploaded successfully: " + fileName);
    } else {
      Serial.print("❌ Upload failed: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

// ==== Flush Buffer & Take Fresh Photo ====
camera_fb_t* captureFreshPhoto() {
  for (int i = 0; i < 3; i++) {
    camera_fb_t* temp = esp_camera_fb_get();
    if (temp) {
      esp_camera_fb_return(temp);
      delay(100);
    }
  }
  return esp_camera_fb_get();
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);
  connectWiFi();
  initTime();
  initCamera();
  Serial.println("Setup complete.");
}

// ==== Main Loop ====
void loop() {
  Serial.println("Checking for camera request...");

  if (shouldTakePhoto()) {
    Serial.println("✅ Request is TRUE. Capturing photo...");

    camera_fb_t* fb = captureFreshPhoto();
    if (!fb) {
      Serial.println("❌ Camera capture failed");
      delay(5000);
      return;
    }

    uploadImageToSupabase(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    markRequestFalse();

  } else {
    Serial.println("⏸ No photo requested.");
  }

  delay(5000); // Check every 5 seconds
}