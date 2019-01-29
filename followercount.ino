#include "secrets.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>



HTTPClient http;
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
  Serial.println(streamer_id);
}

void loop() {


  delay(30000);

  // https://api.twitch.tv/helix/users?login=<username>

  //"/users/follows?to_id="

}

int getStreamerId(String username) {
  http.begin("https://api.twitch.tv/helix/users?login=sophiedeziel", twitch_thumbprint);

  http.addHeader("Client-ID", twitch_client);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  int id = root["data"][0]["id"];
  return id;
}
