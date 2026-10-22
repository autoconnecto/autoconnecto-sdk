// Claim-code handoff. The QR and this sketch hold the code, not the device token.
// Not board-verified. Poll until a person claims the code, then store the token.
//
// POST http://<host>/api/v1/device-claims/handoff
// Body: {"code":"acc_..."}
// 200: {"deviceId":"...","deviceToken":"..."}  — use the token for MQTT after this.
// 401 before claim: the code has not been claimed yet. Wait and try again.
// 401 after the token was returned once: the code is dead.

#include <WiFi.h>
#include <HTTPClient.h>

const char *WIFI_SSID = "your-ssid";
const char *WIFI_PASSWORD = "your-password";
const char *API_BASE = "https://api.autoconnecto.in";
const char *CLAIM_CODE = "acc_replace_me";

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void loop() {
  HTTPClient http;
  http.begin(String(API_BASE) + "/api/v1/device-claims/handoff");
  http.addHeader("Content-Type", "application/json");
  int status = http.POST(String("{\"code\":\"") + CLAIM_CODE + "\"}");
  Serial.printf("handoff %d %s\n", status, http.getString().c_str());
  http.end();
  if (status == 200) {
    // Store deviceToken and stop. Do not print the token on a label.
    while (true) delay(1000);
  }
  delay(15000);
}
