#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_camera.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "time.h"

// WiFi credentials
const char* ssid = "your ssid";
const char* password = "ssid password";

// Telegram bot details
String telegramBotToken = "YOUR_BOT_TOKEN";
String chatID = "YOUR_CHAT_ID";

// Flask server endpoint
const char* serverUrl = "http://your ip:5000/upload";

// PIR motion pin
#define PIR_PIN 13

// Camera pins (AI Thinker)
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

WebServer server(80);

void startCamera() {
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void sendToTelegram(camera_fb_t* fb, const String& timestamp) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + telegramBotToken + "/sendPhoto";

  if (https.begin(client, url)) {
    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + chatID + "\r\n";
    bodyStart += "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"caption\"\r\n\r\nMotion Detected at " + timestamp + "\r\n";
    bodyStart += "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"photo\"; filename=\"image.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    int totalLen = bodyStart.length() + fb->len + bodyEnd.length();
    uint8_t* payload = (uint8_t*)malloc(totalLen);
    if (!payload) {
      Serial.println("Not enough memory for payload");
      return;
    }

    memcpy(payload, bodyStart.c_str(), bodyStart.length());
    memcpy(payload + bodyStart.length(), fb->buf, fb->len);
    memcpy(payload + bodyStart.length() + fb->len, bodyEnd.c_str(), bodyEnd.length());

    https.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
    int httpCode = https.sendRequest("POST", payload, totalLen);

    if (httpCode > 0) {
      Serial.printf("Telegram sent: %d\n", httpCode);
    } else {
      Serial.printf("Telegram error: %s\n", https.errorToString(httpCode).c_str());
    }

    free(payload);
    https.end();
  }
}

void sendToServer(camera_fb_t* fb) {
  HTTPClient http;
  http.begin(serverUrl);

  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  String contentType = "multipart/form-data; boundary=" + boundary;
  http.addHeader("Content-Type", contentType);

  String bodyStart = "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\n";
  bodyStart += "Content-Type: image/jpeg\r\n\r\n";

  String bodyEnd = "\r\n--" + boundary + "--\r\n";

  int totalLength = bodyStart.length() + fb->len + bodyEnd.length();
  uint8_t *payload = (uint8_t *)malloc(totalLength);
  if (!payload) {
    Serial.println("❌ Not enough heap for payload.");
    return;
  }

  memcpy(payload, bodyStart.c_str(), bodyStart.length());
  memcpy(payload + bodyStart.length(), fb->buf, fb->len);
  memcpy(payload + bodyStart.length() + fb->len, bodyEnd.c_str(), bodyEnd.length());

  int httpResponseCode = http.sendRequest("POST", payload, totalLength);
  if (httpResponseCode > 0) {
    Serial.printf("📤 Sent to server. HTTP code: %d\n", httpResponseCode);
  } else {
    Serial.printf("❌ Server upload failed. Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  free(payload);
  http.end();
}

void handleJPGStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      continue;
    }

    server.sendContent("--frame\r\n");
    server.sendContent("Content-Type: image/jpeg\r\n\r\n");
    client.write(fb->buf, fb->len);
    server.sendContent("\r\n");
    esp_camera_fb_return(fb);
    delay(100);
  }
}

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Unknown time";
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");

  configTime(19800, 0, "pool.ntp.org"); // IST (GMT+5:30)

  startCamera();

  server.on("/", HTTP_GET, handleJPGStream);
  server.begin();
  Serial.println("Web stream available at http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();

  if (digitalRead(PIR_PIN) == HIGH) {
    Serial.println("📡 Motion detected!");
    delay(500); // debounce
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    String timestamp = getTimeString();
    sendToTelegram(fb, timestamp);
    sendToServer(fb);

    esp_camera_fb_return(fb);
    delay(10000); // wait before detecting motion again
  }
}
