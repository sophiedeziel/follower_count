#include "secrets.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <TM1637Display.h>

#define CLK_PIN D2
#define DIO_PIN D3

TM1637Display display_follows(CLK_PIN, DIO_PIN);

const String twitch_name = "sophiedeziel";
const String twitch_thumbprint = "0819d6edfd12c734212d56383e1428e2cefd61ea";

int streamer_id;

void setup() {
  Serial.begin(115200);
  Serial.println("");

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
  streamer_id = getStreamerId(twitch_name);
  Serial.print("ID: ");
  Serial.println(streamer_id);

  display_follows.setBrightness(0x0a);
}

void loop() {
  if (streamer_id > 0) {
    int count = getStreamerFollows();
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
  }

  delay(5000);
}

int getStreamerId(String username) {
  HTTPClient http;
  http.begin("https://api.twitch.tv/helix/users?login=sophiedeziel", twitch_thumbprint);

  http.addHeader("Client-ID", twitch_client);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();

  DynamicJsonDocument jsonBuffer(1024);
  deserializeJson(jsonBuffer, payload);
  JsonObject root = jsonBuffer.as<JsonObject>();

  int id = root["data"][0]["id"];
  return id;
}

int getStreamerFollows() {
  HTTPClient http;
  http.begin("https://api.twitch.tv/helix/users/follows?to_id=" + String(streamer_id), twitch_thumbprint);

  http.addHeader("Client-ID", twitch_client);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();

  DynamicJsonDocument jsonBuffer(1024);
  deserializeJson(jsonBuffer, payload);
  JsonObject root = jsonBuffer.as<JsonObject>();

  int count = root["total"];
  return count;
}

void animateNumber(int count, int zeroes) {
  if (zeroes == 0) {
    display_follows.showNumberDec(count);
  } else {
    for (int i = 0; i < 6; i++) {
      uint8_t circle[6] = { SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F };
      uint8_t number_frame[4] = {};

      int temp_count = count;
      for (int j = 3; j >= 0; j--) {
        number_frame[j] = display_follows.encodeDigit(temp_count % 10);

        temp_count = temp_count / 10;
      }

      for (int j = 0; j < zeroes; j++) {
        number_frame[3 - j] = circle[i];
      }

      display_follows.setSegments(number_frame);
      delay(150);
    }
    display_follows.showNumberDec(count);
  }
}
