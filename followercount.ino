#include "secrets.h"

#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;
#include <ArduinoJson.h>

#include <TM1637Display.h>

#include "cacert.h"
extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

#define CLK_PIN D2
#define DIO_PIN D3

#define TWITCH
//#define YOUTUBE

TM1637Display display_follows(CLK_PIN, DIO_PIN);

#if defined TWITCH
const String twitch_name = "sophiedeziel";
int streamer_id;
#endif

#if defined YOUTUBE
const String youtube_id = "UCrdnnnPaAyvQziO0mtz89uw";
#endif

axTLS::WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("Retrying connection…");
    delay(5);
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#if defined TWITCH
  streamer_id = getStreamerId(twitch_name);

  Serial.print("ID: ");
  Serial.println(streamer_id);
#endif

  display_follows.setBrightness(0x0a);

  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  // Load root certificate in DER format into WiFiClientSecure object
  bool res = client.setCACert_P(caCert, caCertLen);
  if (!res) {
    Serial.println("Failed to load root CA certificate!");
    while (true) {
      yield();
    }
  }
  if (!client.connect("api.twitch.tv", 443)) {
    Serial.println("connection failed");
  }
}

void loop() {

#if defined TWITCH
  int count = getStreamerFollows();
#endif
#if defined YOUTUBE
  int count = getYoutuberFollows();
#endif
  Serial.println(count);
  int zeroes = 0;
  if (count % 10 == 0) {
    zeroes++;
  }
  if (count % 100 == 0) {
    zeroes++;
  }
  if (count % 1000 == 0) {
    zeroes++;
  }
  animateNumber(count, zeroes);

  delay(5000);
}

#ifdef TWITCH
int getStreamerId(String username) {

  HTTPClient http;

  http.begin(client, "https://api.twitch.tv/helix/users?login=" + twitch_name);

  http.addHeader("Client-ID", twitch_client);
  int httpCode = http.GET();
  Serial.print("HTTP code: ");
  Serial.println(httpCode);
  String payload = http.getString();
  http.end();

  DynamicJsonDocument jsonBuffer(1024);
  deserializeJson(jsonBuffer, payload);
  JsonObject root = jsonBuffer.as<JsonObject>();

  int id = root["data"][0]["id"];
  return id;
}
#endif

#ifdef TWITCH
int getStreamerFollows() {
  int count = 0;
  if (streamer_id > 0) {
    HTTPClient http;
    http.begin(client, "https://api.twitch.tv/helix/users/follows?to_id=" + String(streamer_id));

    http.addHeader("Client-ID", twitch_client);
    int httpCode = http.GET();
    Serial.print("HTTP code: ");
    Serial.println(httpCode);
    String payload = http.getString();
    Serial.println(payload);
    http.end();

    DynamicJsonDocument jsonBuffer(1024);
    deserializeJson(jsonBuffer, payload);
    JsonObject root = jsonBuffer.as<JsonObject>();

    count = root["total"];
  }
  return count;
}
#endif

#ifdef YOUTUBE
int getYoutuberFollows() {
  int count = 0;

  String youtube_url = "https://www.googleapis.com/youtube/v3/channels?part=statistics&id=" + youtube_id + "&key=" + String(youtube_api_key);

  HTTPClient http;
  http.begin(client, youtube_url);

  int httpCode = http.GET();
  Serial.print("HTTP code: ");
  Serial.println(httpCode);
  String payload = http.getString();
  Serial.println(payload);
  http.end();

  DynamicJsonDocument jsonBuffer(1024);
  deserializeJson(jsonBuffer, payload);
  JsonObject root = jsonBuffer.as<JsonObject>();

  count = root["items"][0]["statistics"]["subscriberCount"];

  return count;
}
#endif

void animateNumber(int count, int zeroes) {
  display_follows.showNumberDec(count);
  if (zeroes > 1) {
    for (int i = 0; i < 13; i++) {
      uint8_t circle[6] = { SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F };
      uint8_t number_frame[2] = {};

      for (int j = 0; j < 4; j++) {
        number_frame[j] = circle[i % 6];
      }
      display_follows.setSegments(number_frame, 4, 4 - zeroes);
      delay(60);
    }
    display_follows.showNumberDec(count);
  }
}

unsigned GetNumberOfDigits (unsigned i)
{
    return i > 0 ? (int) log10 ((double) i) + 1 : 1;
}
